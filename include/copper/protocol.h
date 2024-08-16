#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <string>

namespace copper {
  class protocol_response : public boost::enable_shared_from_this<protocol_response> {
    std::string data_;
  public:
    explicit protocol_response(std::string data) : data_(std::move(data)) {}
  };

  class protocol_request : public boost::enable_shared_from_this<protocol_request> {
    std::string data_;
  public:
    explicit protocol_request(std::string data) : data_(std::move(data)) {}
  };
}
