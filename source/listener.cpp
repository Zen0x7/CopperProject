#include <copper/listener.h>

copper::listener::listener(boost::asio::io_context& io_context,
                           boost::asio::ip::tcp::endpoint endpoint)
    : io_context_(io_context), acceptor_(io_context) {
  boost::beast::error_code error;

  acceptor_.open(endpoint.protocol(), error);

  if (error) {
    failure::make(error, "open");
    return;
  }

  acceptor_.set_option(boost::asio::socket_base::reuse_address(true), error);

  if (error) {
    failure::make(error, "set_option::reuse_address");
    return;
  }

  acceptor_.bind(endpoint, error);

  if (error) {
    failure::make(error, "bind");
    return;
  }

  acceptor_.listen(boost::asio::socket_base::max_listen_connections, error);

  if (error) {
    failure::make(error, "listen");
    return;
  }
}

void copper::listener::on_accept(boost::beast::error_code error,
                                 boost::asio::ip::tcp::socket socket) {
  if (error) return failure::make(error, "accept");

  boost::make_shared<http_session>(std::move(socket))->run();

  acceptor_.async_accept(make_strand(io_context_), boost::beast::bind_front_handler(
                                                       &listener::on_accept, shared_from_this()));
}

void copper::listener::run() {
  acceptor_.async_accept(
      boost::asio::make_strand(io_context_),
      boost::beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}
