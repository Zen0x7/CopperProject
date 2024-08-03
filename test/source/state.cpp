#include <copper/state.h>
#include <copper/version.h>
#include <doctest/doctest.h>

#include <string>

TEST_CASE("State") {
  using namespace copper;

  auto first_state_ = std::make_shared<state>();
  auto second_state_ = std::make_shared<state>();

  CHECK_NE(first_state_->get_id(), second_state_->get_id());
}