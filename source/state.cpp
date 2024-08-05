#include <copper/logger.h>
#include <copper/state.h>
#include <copper/websocket_session.h>

#include <boost/uuid/uuid.hpp>

using namespace copper;

state::state() : id_(boost::uuids::random_generator()()) {}

void state::join(websocket_session* session) {
  std::lock_guard lock(mutex_);
  sessions_.insert(session);
}

void state::leave(websocket_session* session) {
  std::lock_guard lock(mutex_);
  sessions_.erase(session);
}
void state::send(const boost::uuids::uuid id, std::string message) {
  logger::on_broadcast(id, message);

  auto const ss = boost::make_shared<std::string const>(std::move(message));

  std::vector<boost::weak_ptr<websocket_session>> v;
  {
    std::lock_guard lock(mutex_);
    v.reserve(sessions_.size());
    for (auto p : sessions_) v.emplace_back(p->weak_from_this());
  }

  for (auto const& wp : v)
    if (auto sp = wp.lock()) sp->send(ss);
}
