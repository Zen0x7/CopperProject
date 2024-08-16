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
#include <boost/chrono.hpp>
#include <boost/chrono/chrono.hpp>
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

  void run(char const* host, char const* port, char const* target, int version,
           boost::beast::http::verb verb, bool keep_alived) {
    req_.version(version);
    req_.method(verb);
    req_.keep_alive(keep_alived);
    req_.target(target);
    req_.set(boost::beast::http::field::host, host);
    req_.set(boost::beast::http::field::user_agent, "Copper 1.0");

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

    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

    if (ec && ec != boost::beast::errc::not_connected) return fail(ec, "shutdown");
  }
};

class websocket_client : public boost::enable_shared_from_this<websocket_client> {
  boost::asio::ip::tcp::resolver resolver_;
  boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
  boost::beast::flat_buffer buffer_;
  std::string host_;
  std::string text_;

public:
  explicit websocket_client(boost::asio::io_context& ioc)
      : resolver_(boost::asio::make_strand(ioc)), ws_(boost::asio::make_strand(ioc)) {}

  void run(char const* host, char const* port, char const* text) {
    host_ = host;
    text_ = text;

    resolver_.async_resolve(
        host, port,
        boost::beast::bind_front_handler(&websocket_client::on_resolve, shared_from_this()));
  }

  void on_resolve(boost::beast::error_code ec,
                  boost::asio::ip::tcp::resolver::results_type results) {
    if (ec) return fail(ec, "resolve");

    boost::beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    boost::beast::get_lowest_layer(ws_).async_connect(
        results,
        boost::beast::bind_front_handler(&websocket_client::on_connect, shared_from_this()));
  }

  void on_connect(boost::beast::error_code ec,
                  boost::asio::ip::tcp::resolver::results_type::endpoint_type ep) {
    if (ec) return fail(ec, "connect");

    boost::beast::get_lowest_layer(ws_).expires_never();

    ws_.set_option(
        boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

    ws_.set_option(boost::beast::websocket::stream_base::decorator(
        [](boost::beast::websocket::request_type& req) {
          req.set(boost::beast::http::field::user_agent,
                  std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
        }));

    host_ += ':' + std::to_string(ep.port());

    ws_.async_handshake(
        host_, "/",
        boost::beast::bind_front_handler(&websocket_client::on_handshake, shared_from_this()));
  }

  void on_handshake(boost::beast::error_code ec) {
    if (ec) return fail(ec, "handshake");

    ws_.async_write(
        boost::asio::buffer(text_),
        boost::beast::bind_front_handler(&websocket_client::on_write, shared_from_this()));
  }

  void on_write(boost::beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) return fail(ec, "write");

    ws_.async_read(
        buffer_, boost::beast::bind_front_handler(&websocket_client::on_read, shared_from_this()));
  }

  void on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if (ec) return fail(ec, "read");

    ws_.async_close(
        boost::beast::websocket::close_code::normal,
        boost::beast::bind_front_handler(&websocket_client::on_close, shared_from_this()));
  }

  void on_close(boost::beast::error_code ec) {
    if (ec) return fail(ec, "close");

    std::cout << boost::beast::make_printable(buffer_.data()) << std::endl;
  }
};

TEST_CASE("Serve") {
  using namespace copper;

#ifdef BOOST_MSVC
  auto const host_ = "127.0.0.1";
#else
  auto const host_ = "0.0.0.0";
#endif
  auto address_ = boost::asio::ip::make_address(host_);
  unsigned short port_ = 7500;

  auto configuration_ = boost::make_shared<state_configuration>();
  boost::asio::io_context client_io_context_;

  auto state_ = boost::make_shared<state>(configuration_);

  state_->detach_receiver();

  boost::make_shared<listener>(state_->io_context_, boost::asio::ip::tcp::endpoint{address_, port_},
                               state_)
      ->run();

  boost::make_shared<http_client>(client_io_context_)
      ->run(host_, "7500", "/", 11, boost::beast::http::verb::get, true);

  boost::make_shared<http_client>(client_io_context_)
      ->run(host_, "7500", "/", 11, boost::beast::http::verb::get, false);

  boost::make_shared<websocket_client>(client_io_context_)->run(host_, "7500", "EHLO");

  boost::thread server_runner([&state_] { state_->io_context_.run(); });
  boost::thread client_runner([&client_io_context_] { client_io_context_.run(); });

  state_->redis_connection_->async_run(configuration_->redis_configuration_, {},
                                       boost::asio::detached);

  server_runner.detach();

  std::this_thread::sleep_for(std::chrono::seconds(1));

  client_runner.detach();

  std::this_thread::sleep_for(std::chrono::seconds(1));

  state_->io_context_.stop();
  client_io_context_.stop();

  std::this_thread::sleep_for(std::chrono::seconds(1));

  server_runner.join();
  client_runner.join();
}