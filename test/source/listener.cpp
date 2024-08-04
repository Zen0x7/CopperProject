#include <copper/listener.h>
#include <copper/state.h>
#include <copper/version.h>
#include <doctest/doctest.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/thread.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

void fail(boost::beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

class http_client : public boost::enable_shared_from_this<http_client> {
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::tcp_stream stream_;
  boost::beast::flat_buffer buffer_;  // (Must persist between reads)
  boost::beast::http::request<boost::beast::http::empty_body> req_;
  boost::beast::http::response<boost::beast::http::string_body> res_;

public:
  explicit http_client(boost::asio::io_context& ioc)
      : resolver_(boost::asio::make_strand(ioc)), stream_(boost::asio::make_strand(ioc)) {}

  void run(char const* host, char const* port, char const* target, int version) {
    req_.version(version);
    req_.method(boost::beast::http::verb::get);
    req_.target(target);
    req_.set(boost::beast::http::field::host, host);
    req_.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    resolver_.async_resolve(
        host, port, boost::beast::bind_front_handler(&http_client::on_resolve, shared_from_this()));
  }

  void on_resolve(boost::beast::error_code ec,
                  boost::asio::ip::tcp::resolver::results_type results) {
    if (ec) return fail(ec, "resolve");

    stream_.expires_after(std::chrono::seconds(30));

    stream_.async_connect(
        results, boost::beast::bind_front_handler(&http_client::on_connect, shared_from_this()));
  }

  void on_connect(boost::beast::error_code ec,
                  boost::asio::ip::tcp::resolver::results_type::endpoint_type) {
    if (ec) return fail(ec, "connect");

    stream_.expires_after(std::chrono::seconds(30));

    boost::beast::http::async_write(
        stream_, req_,
        boost::beast::bind_front_handler(&http_client::on_write, shared_from_this()));
  }

  void on_write(boost::beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) return fail(ec, "write");

    boost::beast::http::async_read(
        stream_, buffer_, res_,
        boost::beast::bind_front_handler(&http_client::on_read, shared_from_this()));
  }

  void on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) return fail(ec, "read");

    std::cout << res_ << std::endl;

    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

    if (ec && ec != boost::beast::errc::not_connected) return fail(ec, "shutdown");
  }
};

TEST_CASE("Serve") {
  using namespace copper;

  auto const host_ = "0.0.0.0";
  auto address_ = boost::asio::ip::make_address(host_);
  unsigned short port_ = 7500;

  boost::asio::io_context io_context_;

  auto state_ = boost::make_shared<state>();

  boost::make_shared<listener>(io_context_, boost::asio::ip::tcp::endpoint{address_, port_}, state_)
      ->run();

  boost::make_shared<http_client>(io_context_)->run("0.0.0.0", "7500", "/", 11);

  boost::thread runner([&io_context_] { io_context_.run(); });

  runner.detach();

  io_context_.stop();

  runner.join();
}