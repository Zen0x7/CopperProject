#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/smart_ptr.hpp>
#include <iostream>

#include "failure.h"
#include "http_session.h"
#include "state.h"

namespace copper {
  class listener : public boost::enable_shared_from_this<listener> {
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::shared_ptr<state> state_;

    void on_accept(boost::beast::error_code error, boost::asio::ip::tcp::socket socket);

  public:
    listener(boost::asio::io_context& io_context, boost::asio::ip::tcp::endpoint endpoint, boost::shared_ptr<state> const & state);

    void run();
  };
}  // namespace copper