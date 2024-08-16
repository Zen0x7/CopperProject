#include <copper/logger.h>
#include <copper/state.h>
#include <copper/websocket_session.h>

#include <boost/asio/strand.hpp>

using signal_set = boost::asio::deferred_t::as_default_on_t<boost::asio::signal_set>;

using namespace copper;

state::state(const boost::shared_ptr<state_configuration>& configuration)
    : configuration_(configuration),
      id_(boost::uuids::random_generator()()),
      redis_connection_(boost::make_shared<boost::redis::connection>(io_context_)) {}

void state::join(websocket_session* session) {
  std::lock_guard lock(mutex_);
  sessions_.insert(session);
}

void state::leave(websocket_session* session) {
  std::lock_guard lock(mutex_);
  sessions_.erase(session);
}

void state::broadcast(const boost::uuids::uuid id, std::string data) {
  logger::on_broadcast(id, data);

  auto const ss = boost::make_shared<std::string const>(std::move(data));

  std::vector<boost::weak_ptr<websocket_session>> v;
  {
    std::lock_guard lock(mutex_);
    v.reserve(sessions_.size());
    for (auto p : sessions_) v.emplace_back(p->weak_from_this());
  }

  for (auto const& wp : v)
    if (auto sp = wp.lock()) sp->send(ss);
}

auto state::receiver(const boost::shared_ptr<boost::redis::connection> connection)
    -> boost::asio::awaitable<void> {
  boost::redis::request req;
  req.push("SUBSCRIBE", "channel");

  boost::redis::generic_response resp;

  connection->set_receive_response(resp);

  while (connection->will_reconnect()) {
    co_await connection->async_exec(req, boost::redis::ignore, boost::asio::deferred);

    for (boost::system::error_code ec;;) {
      connection->receive(ec);

      if (ec == boost::redis::error::sync_receive_push_failed) {
        ec = {};

        co_await connection->async_receive(redirect_error(boost::asio::use_awaitable, ec));
      }

      if (ec) break;

      if (resp.value().at(1).value != "subscribe") {  // ignore subscribe
        std::cout << resp.value().at(1).value << " " << resp.value().at(2).value << " "
                  << resp.value().at(3).value << std::endl;
      }

      boost::redis::consume_one(resp);
    }
  }
}

auto state::co_receiver(boost::redis::config redis_configuration) -> boost::asio::awaitable<void> {
  auto ex = co_await boost::asio::this_coro::executor;
  auto connection = boost::make_shared<boost::redis::connection>(ex);
  co_spawn(ex, receiver(connection), boost::asio::detached);
  connection->async_run(redis_configuration, {}, consign(boost::asio::detached, connection));
  signal_set sig_set(ex, SIGINT, SIGTERM);
  co_await sig_set.async_wait();
  connection->cancel();
}

void state::detach_receiver() const {
  boost::asio::co_spawn(redis_connection_->get_executor(),
                        co_receiver(configuration_->redis_configuration_),
                        [](const std::exception_ptr& error) {
                          if (error) std::rethrow_exception(error);
                        });

  redis_connection_->async_run(configuration_->redis_configuration_, {}, boost::asio::detached);
}