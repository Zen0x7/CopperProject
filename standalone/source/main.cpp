#include <copper/copper.h>
#include <copper/listener.h>
#include <copper/state.h>
#include <copper/state_container.h>
#include <copper/version.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/stacktrace.hpp>
#include <cxxopts.hpp>
#include <iostream>
#include <string>

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

  auto state_ = boost::make_shared<state>();

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
