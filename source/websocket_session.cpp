#include <copper/failure.h>
#include <copper/logger.h>
#include <copper/websocket_session.h>

#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>
#include <boost/uuid/uuid_io.hpp>

copper::websocket_session::websocket_session(boost::asio::ip::tcp::socket&& socket,
                                             boost::shared_ptr<state> const& state)
    : id_(boost::uuids::random_generator()()),
      websocket_stream_(std::move(socket)),
      state_(state) {}

copper::websocket_session::~websocket_session() { state_->leave(this); }

void copper::websocket_session::on_accept(boost::beast::error_code error) {
  if (error) return failure::make(error, "copper::websocket_session::on_accept");

  logger::on_connect(id_);

  state_->join(this);

  boost::json::object connected_event_object
      = {{"event", "connected"}, {"payload", {{"id", to_string(id_)}}}};
  const std::string connected_event_serialized = serialize(connected_event_object);
  state_->send(id_, connected_event_serialized);

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
