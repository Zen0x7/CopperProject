#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <iostream>
#include <string>
#include <termcolor/termcolor.hpp>

namespace copper {
  /**
   * @brief Program logger
   */
  class logger {
  public:
    /**
     * @brief Logging for success events
     * @param data Payload
     */
    void static success(const std::string &data) {
      std::cout << termcolor::on_grey << termcolor::bold << termcolor::bright_white << " "
                << std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count()
                << " " << termcolor::reset << termcolor::on_green << termcolor::bold
                << termcolor::grey << " " << data << " " << termcolor::reset << std::endl;
    }

    /**
     * @brief Logging for broadcast events
     * @param id WebSocket identifier
     * @param data Payload
     */
    void static on_broadcast(const boost::uuids::uuid id, const std::string &data) {
      std::cout << termcolor::on_grey << termcolor::bold << termcolor::bright_white << " "
                << std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count()
                << " " << termcolor::reset << termcolor::on_green << termcolor::bold
                << termcolor::grey << " Broadcast " << termcolor::reset << termcolor::on_grey
                << termcolor::bold << termcolor::bright_white << " " << to_string(id) << " "
                << termcolor::reset << termcolor::on_grey << termcolor::bright_white << " " << data
                << " " << termcolor::reset << std::endl;
    }

    /**
     * @brief Logging for connect events
     * @param id WebSocket identifier
     */
    void static on_connect(const boost::uuids::uuid id) {
      std::cout << termcolor::on_grey << termcolor::bold << termcolor::bright_white << " "
                << std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count()
                << " " << termcolor::reset << termcolor::on_green << termcolor::bold
                << termcolor::grey << " Connected " << termcolor::reset << termcolor::on_grey
                << termcolor::bold << termcolor::bright_white << " " << to_string(id) << " "
                << termcolor::reset << std::endl;
    }
  };
}  // namespace copper
