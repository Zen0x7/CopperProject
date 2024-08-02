#include <copper/websocket_session.h>

#include "copper/failure.h"

copper::websocket_session::websocket_session(boost::asio::ip::tcp::socket&& socket)
    : websocket_stream_(std::move(socket)) {}

copper::websocket_session::~websocket_session() {
  // should leave
}

void copper::websocket_session::on_accept(boost::beast::error_code error) {
  if (error) return failure::make(error, "copper::websocket_session::on_accept");

  websocket_stream_.async_read(
      buffer_, boost::beast::bind_front_handler(&websocket_session::on_read, shared_from_this()));
}

void copper::websocket_session::on_read(boost::beast::error_code error, std::size_t) {
  if (error) return failure::make(error, "copper::websocket_sesion::on_read");

  buffer_.consume(buffer_.size());

  websocket_stream_.async_read(
      buffer_, boost::beast::bind_front_handler(&websocket_session::on_read, shared_from_this()));
}

void copper::websocket_session::send(boost::shared_ptr<std::string const> const& data) {
  post(websocket_stream_.get_executor(),
       boost::beast::bind_front_handler(&websocket_session::on_send, shared_from_this(), data));
}

void copper::websocket_session::on_send(boost::shared_ptr<std::string const> const& data) {
  queue_.push_back(data);

  if (queue_.size() > 1) return;

  websocket_stream_.async_write(
      boost::asio::buffer(*queue_.front()),
      boost::beast::bind_front_handler(&websocket_session::on_write, shared_from_this()));
}

void copper::websocket_session::on_write(boost::beast::error_code error, std::size_t) {
  if (error) return failure::make(error, "copper::websocket_session::on_write");

  queue_.erase(queue_.begin());

  if (!queue_.empty()) {
    websocket_stream_.async_write(
        boost::asio::buffer(*queue_.front()),
        boost::beast::bind_front_handler(&websocket_session::on_write, shared_from_this()));
  }
}
