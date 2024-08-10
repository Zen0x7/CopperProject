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
  /**
   * @brief TCP listener
   */
  class listener : public boost::enable_shared_from_this<listener> {
    boost::asio::io_context& io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    boost::shared_ptr<state> state_;

    /**
     * @brief Callback executed when generic session has been accepted
     * @param error Error code
     * @param socket Socket
     */
    void on_accept(boost::beast::error_code error, boost::asio::ip::tcp::socket socket);

  public:
    /**
     * @brief Creates a new instance
     * @param io_context Asio IO Context
     * @param endpoint Asio endpoint
     * @param state Shared state
     */
    listener(boost::asio::io_context& io_context, boost::asio::ip::tcp::endpoint endpoint,
             boost::shared_ptr<state> const& state);

    /**
     * @brief Run
     */
    void run();
  };
}  // namespace copper