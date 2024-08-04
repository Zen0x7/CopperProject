#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>

namespace copper {
  class failure {
  public:
    void static make(boost::beast::error_code error, char const* what) {
      if (error == boost::asio::error::operation_aborted) return;

      std::cerr << what << ": " << error.message() << "\n";
    }
  };
}  // namespace copper