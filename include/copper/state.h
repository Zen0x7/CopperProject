#pragma once

#include <boost/smart_ptr.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <unordered_set>

namespace copper {
  class websocket_session;

  class state : public boost::enable_shared_from_this<state> {
    boost::uuids::uuid id_;
    std::mutex mutex_;
    std::unordered_set<websocket_session*> sessions_;

  public:
    state();

    boost::uuids::uuid get_id() const { return id_; }

    void join(websocket_session* session);
    void leave(websocket_session* session);
    void send(std::string message);
  };
}  // namespace copper