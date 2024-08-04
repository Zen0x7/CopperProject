#include <copper/listener.h>

copper::listener::listener(boost::asio::io_context& io_context,
                           boost::asio::ip::tcp::endpoint endpoint,
                           boost::shared_ptr<state> const& state)
    : io_context_(io_context), acceptor_(io_context), state_(state) {
  boost::beast::error_code error;

  acceptor_.open(endpoint.protocol(), error);

  // clang-format off
  if (error) { failure::make(error, "open"); return; }
  // clang-format on

  acceptor_.set_option(boost::asio::socket_base::reuse_address(true), error);

  // clang-format off
  if (error) { failure::make(error, "set_option::reuse_address"); return; }
  // clang-format on

  acceptor_.bind(endpoint, error);

  // clang-format off
  if (error) { failure::make(error, "bind"); return; }
  // clang-format on

  acceptor_.listen(boost::asio::socket_base::max_listen_connections, error);

  // clang-format off
  if (error) { failure::make(error, "listen"); return; }
  // clang-format on
}

void copper::listener::on_accept(boost::beast::error_code error,
                                 boost::asio::ip::tcp::socket socket) {
  if (error) return failure::make(error, "accept");

  boost::make_shared<http_session>(std::move(socket), state_)->run();

  acceptor_.async_accept(
      boost::asio::make_strand(io_context_),
      boost::beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void copper::listener::run() {
  acceptor_.async_accept(
      boost::asio::make_strand(io_context_),
      boost::beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}
