#include <copper/state.h>
#include <copper/version.h>
#include <doctest/doctest.h>

#include <string>

TEST_CASE("State") {
  using namespace copper;

  auto io_context_ = boost::make_shared<boost::asio::io_context>();
  auto configuration_ = boost::make_shared<state_configuration>();
  auto first_state_ = std::make_shared<state>(configuration_);
  auto second_state_ = std::make_shared<state>(configuration_);

  CHECK_NE(first_state_->get_id(), second_state_->get_id());
}