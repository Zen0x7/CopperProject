#pragma once

#include <memory>
#include <mutex>

#include "state.h"

namespace copper {

  /**
   * @brief Program shared state container
   */
  class state_container {
    static std::shared_ptr<state> instance;
    static std::mutex mutex_;

  public:
    state_container() = delete;
    ~state_container() = delete;

    /**
     * @brief Retrieves the state instance
     */
    static std::shared_ptr<state> get_instance();
  };
}  // namespace copper