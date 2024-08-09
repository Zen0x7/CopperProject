#include <copper/copper.h>
#include <copper/listener.h>
#include <copper/logger.h>
#include <copper/state.h>
#include <copper/state_container.h>
#include <copper/version.h>

#include <coroutine>


#include <boost/redis/connection.hpp>
#include <boost/redis/logger.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/consign.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/redis/src.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/stacktrace.hpp>
#include <boost/thread.hpp>
#include <cxxopts.hpp>
#include <iostream>
#include <string>
#include <termcolor/termcolor.hpp>

using signal_set = boost::asio::deferred_t::as_default_on_t<boost::asio::signal_set>;

auto deep_receiver(boost::shared_ptr<boost::redis::connection> conn) -> boost::asio::awaitable<void> {
  boost::redis::request req;
  req.push("SUBSCRIBE", "channel");

  boost::redis::generic_response resp;

  conn->set_receive_response(resp);

  while (conn->will_reconnect()) {

    co_await conn->async_exec(req, boost::redis::ignore, boost::asio::deferred);

    for (boost::system::error_code ec;;) {
      conn->receive(ec);

      if (ec == boost::redis::error::sync_receive_push_failed) {
        ec = {};

        co_await conn->async_receive(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
      }

      if (ec)
        break;


      std::cout
         << resp.value().at(1).value
         << " " << resp.value().at(2).value
         << " " << resp.value().at(3).value
         << std::endl;

      boost::redis::consume_one(resp);
    }
  }
}

auto co_receiver(boost::redis::config cfg) -> boost::asio::awaitable<void> {
  auto ex = co_await boost::asio::this_coro::executor;
  auto conn = boost::make_shared<boost::redis::connection>(ex);

  boost::asio::co_spawn(ex, deep_receiver(conn), boost::asio::detached);

  conn->async_run(cfg, {}, boost::asio::consign(boost::asio::detached, conn));

  signal_set sig_set(ex, SIGINT, SIGTERM);
  co_await sig_set.async_wait();
  conn->cancel();
}


auto main(int argc, char** argv) -> int {
  using namespace copper;

  cxxopts::Options options(*argv, "A program to welcome the world!");

  std::string service_host_;
  unsigned short service_port_;
  int service_threads_;

  // clang-format off
  options.add_options()
    ("h,help", "Show help")
    ("v,version", "Print the current version number")
    ("service_address", "Service address", cxxopts::value(service_host_)->default_value("0.0.0.0"))
    ("service_port", "Service port", cxxopts::value(service_port_)->default_value("9000"))
    ("service_threads", "Service threads", cxxopts::value(service_threads_)->default_value("4"))
  ;
  // clang-format on

  auto result = options.parse(argc, argv);

  if (result["help"].as<bool>()) {
    std::cout << options.help() << std::endl;
    return EXIT_SUCCESS;
  }

  if (result["version"].as<bool>()) {
    std::cout << "Copper, version " << COPPER_VERSION << std::endl;
    return EXIT_SUCCESS;
  }

  auto service_address_ = boost::asio::ip::make_address(service_host_);
  auto threads_number_ = std::max(1, service_threads_);

  boost::asio::io_context io_context_;
  boost::asio::io_context redis_io_context_;

  auto state_ = boost::make_shared<state>();

  boost::redis::config cfg;

  cfg.addr.host = "127.0.0.1";
  cfg.addr.port = "6379";

  boost::asio::co_spawn(redis_io_context_, co_receiver(cfg), [] (std::exception_ptr p) {
    if (p)
      std::rethrow_exception(p);
  });

  boost::thread thread([&redis_io_context_]() {
    redis_io_context_.run();
  });

  thread.detach();

  boost::make_shared<listener>(
      io_context_, boost::asio::ip::tcp::endpoint{service_address_, service_port_}, state_)
      ->run();

  boost::asio::signal_set signals(io_context_, SIGINT, SIGTERM);
  signals.async_wait([&io_context_](boost::system::error_code const&, int) { io_context_.stop(); });

  std::vector<std::thread> threads_;
  threads_.reserve(threads_number_ - 1);
  for (auto i = threads_number_ - 1; i > 0; --i)
    threads_.emplace_back([&io_context_] { io_context_.run(); });

  io_context_.run();

  for (auto& thread : threads_) thread.join();

  return EXIT_SUCCESS;
}
