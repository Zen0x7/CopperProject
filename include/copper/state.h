#pragma once

#include <boost/smart_ptr.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <mutex>
#include <unordered_set>

namespace copper {
  class websocket_session;

  struct state_config {
    std::string service_host_;
    unsigned short service_port_;
    int service_threads_;
    std::string redis_host_;
    unsigned short redis_port_;
  };

  /**
   * @brief Program shared state
   */
  class state : public boost::enable_shared_from_this<state> {
    state_config config_;
    boost::uuids::uuid id_;
    std::mutex mutex_;
    std::unordered_set<websocket_session*> sessions_;

  public:
    /**
     * @brief Creates a new instance
     */
    state();

    /**
     * @brief Retrieves the instance identifier
     */
    boost::uuids::uuid get_id() const { return id_; }

    /**
     * @brief Adds the fresh session into the session list
     * @param session WebSocket session
     */
    void join(websocket_session* session);

    /**
     * @brief Removes the session from the session list
     * @param session WebSocket session
     */
    void leave(websocket_session* session);

    /**
     * @brief Broadcast a session message
     * @param id WebSocket identifier
     * @param data Payload
     */
    void broadcast(boost::uuids::uuid id, std::string data);

    /**
     * @brief Set state configuration
     * @param config State configuration
     */
    void set_config(const state_config& config) { this->config_ = config; }
  };
}  // namespace copper