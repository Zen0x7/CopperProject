#include <copper/listener.h>
#include <copper/state.h>
#include <copper/version.h>
#include <doctest/doctest.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/thread.hpp>

TEST_CASE("Serve") {
  using namespace copper;

  auto first_state_ = std::make_shared<state>();
  auto second_state_ = std::make_shared<state>();

  auto const host_ = "0.0.0.0";
  auto address_ = boost::asio::ip::make_address(host_);
  unsigned short port_ = 7500;

  boost::asio::io_context server_io_context_;

  auto state_ = boost::make_shared<state>();

  boost::make_shared<listener>(server_io_context_, boost::asio::ip::tcp::endpoint{address_, port_},
                               state_)
      ->run();

  boost::thread runner([&server_io_context_] { server_io_context_.run(); });

  runner.detach();

  boost::asio::io_context client_io_context_;

  boost::asio::ip::tcp::resolver resolver(client_io_context_);
  boost::beast::tcp_stream stream(client_io_context_);

  auto const results = resolver.resolve(host_, "7500");

  stream.connect(results);

  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get,
                                                                   "/", 11};
  req.set(boost::beast::http::field::host, host_);
  req.set(boost::beast::http::field::user_agent, "Copper Client");

  boost::beast::http::write(stream, req);

  boost::beast::flat_buffer buffer;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  boost::beast::http::read(stream, buffer, res);
  boost::beast::error_code ec;
  stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

  if (ec && ec != boost::beast::errc::not_connected) throw boost::beast::system_error{ec};

  server_io_context_.stop();

  while (!server_io_context_.stopped()) {
    continue;
  }

  runner.join();
}