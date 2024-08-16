#include <copper/state_container.h>

std::shared_ptr<copper::state> copper::state_container::instance{nullptr};
std::mutex copper::state_container::mutex_;

std::shared_ptr<copper::state> copper::state_container::get_instance() {
  std::lock_guard lock(mutex_);

  if (instance == nullptr) {
    auto configuration_ = boost::make_shared<state_configuration>();
    instance = std::make_shared<state>(configuration_);
  }

  return instance;
}
