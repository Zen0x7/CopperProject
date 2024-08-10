#pragma once

#include <copper/state.h>

#include <boost/beast.hpp>
#include <boost/smart_ptr.hpp>

namespace copper {
  /**
   * @brief HTTP session
   */
  class http_session : public boost::enable_shared_from_this<http_session> {
    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_;
    boost::shared_ptr<state> state_;

    boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;

    /**
     * @brief Schedule a read operation
     */
    void do_read();

    /**
     * @brief Reads the HTTP Request
     * @param error Error code
     * @param bytes Buffer size
     */
    void on_read(boost::beast::error_code error, std::size_t bytes);

    /**
     * @brief Writes the HTTP Response
     * @param error Error code
     * @param bytes Buffer size
     * @param closes Should be closed?
     */
    void on_write(boost::beast::error_code error, std::size_t bytes, bool closes);

  public:
    /**
     * @brief Creates a new instance
     * @param socket Asio socket
     * @param state Shared state
     */
    http_session(boost::asio::ip::tcp::socket&& socket, boost::shared_ptr<state> const& state);

    /**
     * @brief Run
     */
    void run();
  };
}  // namespace copper