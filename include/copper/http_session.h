#pragma once

#include <boost/beast.hpp>
#include <boost/smart_ptr.hpp>

namespace copper {
  class http_session : public boost::enable_shared_from_this<http_session> {
    boost::beast::tcp_stream stream_;
    boost::beast::flat_buffer buffer_;

    boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;

    void do_read();
    void on_read(boost::beast::error_code error, std::size_t);
    void on_write(boost::beast::error_code error, std::size_t, bool closes);

  public:
    http_session(boost::asio::ip::tcp::socket&& socket);

    void run();
  };
}  // namespace copper