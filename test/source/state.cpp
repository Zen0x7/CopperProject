#include <copper/state.h>
#include <copper/version.h>
#include <doctest/doctest.h>

#include <string>

TEST_CASE("State") {
  using namespace copper;

  auto state_ = std::make_shared<state>();
}