#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/consign.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/redis.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <mutex>
#include <unordered_set>

namespace copper {
  class websocket_session;

  class state_configuration {
  public:
    std::string service_host_;
    unsigned short service_port_;
    int service_threads_;
    std::string redis_host_;
    unsigned short redis_port_;
    boost::redis::config redis_configuration_;
  };

  /**
   * @brief Program shared state
   */
  class state : public boost::enable_shared_from_this<state> {
    boost::shared_ptr<boost::asio::io_context> io_context_;
    boost::shared_ptr<state_configuration> configuration_;
    boost::uuids::uuid id_;
    std::mutex mutex_;
    std::unordered_set<websocket_session*> sessions_;

  public:
    /**
     * @brief Creates a new instance
     */
    state(const boost::shared_ptr<boost::asio::io_context>& io_context,
          const boost::shared_ptr<state_configuration>& configuration);

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
     * @brief Detach receiver
     */
    void detach_receiver() const;

    /**
     * @brief Connection callback
     * @param redis_configuration Redis configuration
     */
    auto static co_receiver(boost::redis::config redis_configuration)
        -> boost::asio::awaitable<void>;

    /**
     * @brief Receiver infinite loop
     * @param connection Redis connection
     */
    auto static receiver(boost::shared_ptr<boost::redis::connection> connection)
        -> boost::asio::awaitable<void>;
  };
}  // namespace copper