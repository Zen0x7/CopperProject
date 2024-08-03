#pragma once

#include <boost/smart_ptr.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace copper {
  class state : public boost::enable_shared_from_this<state> {
    boost::uuids::uuid id_;

  public:
    state();

    boost::uuids::uuid get_id() const { return id_; }
  };
}  // namespace copper