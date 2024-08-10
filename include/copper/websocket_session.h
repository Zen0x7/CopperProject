#pragma once

#include <copper/state.h>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/smart_ptr.hpp>

namespace copper {
  /**
   * @brief WebSocket session
   */
  class websocket_session : public boost::enable_shared_from_this<websocket_session> {
    boost::uuids::uuid id_;
    boost::beast::flat_buffer buffer_;
    boost::beast::websocket::stream<boost::beast::tcp_stream> websocket_stream_;
    std::vector<boost::shared_ptr<std::string const>> queue_;
    boost::shared_ptr<state> state_;

    /**
     * @brief Callback executed when websocket session has been upgraded
     * @param error Error code
     */
    void on_accept(boost::beast::error_code error);

    /**
     * @brief Reads the event
     * @param error Error code
     * @param bytes Buffer size
     */
    void on_read(boost::beast::error_code error, std::size_t bytes);

    /**
     * @brief Writes the event
     * @param error Error code
     * @param bytes Buffer size
     */
    void on_write(boost::beast::error_code error, std::size_t bytes);

  public:
    /**
     * @brief Creates a new instance
     * @param socket Asio socket
     * @param state Shared state
     */
    websocket_session(boost::asio::ip::tcp::socket&& socket, boost::shared_ptr<state> const& state);

    ~websocket_session();

    /**
     * @brief Run
     * @param request Upgrade request
     */
    template <class Body, class Allocator> void run(
        boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> request);

    /**
     * @brief Schedule a write operation
     * @param data Payload
     */
    void send(boost::shared_ptr<std::string const> const& data);

  private:
    /**
     * @brief Writes an event
     * @param data Payload
     */
    void on_send(boost::shared_ptr<std::string const> const& data);
  };
}  // namespace copper

template <class Body, class Allocator> void copper::websocket_session::run(
    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> request) {
  websocket_stream_.set_option(
      boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

  websocket_stream_.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::response_type& response) {
        response.set(boost::beast::http::field::server, "Copper");
      }));

  websocket_stream_.async_accept(
      request, boost::beast::bind_front_handler(&websocket_session::on_accept, shared_from_this()));
}
