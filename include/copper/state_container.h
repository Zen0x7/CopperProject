#pragma once

#include <memory>
#include <mutex>

#include "state.h"

namespace copper {
  class state_container {
    static std::shared_ptr<state> instance;
    static std::mutex mutex_;

  public:
    state_container() = delete;
    ~state_container() = delete;

    static std::shared_ptr<state> get_instance();
  };
}  // namespace copper