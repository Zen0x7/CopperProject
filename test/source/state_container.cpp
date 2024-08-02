#include <copper/state_container.h>
#include <doctest/doctest.h>

TEST_CASE("State container") {
  using namespace copper;
  auto const first_instance = state_container::get_instance();
  auto const second_instance = state_container::get_instance();

  CHECK_EQ(first_instance.get(), second_instance.get());

  CHECK_NE(first_instance.get(), nullptr);
  CHECK_NE(second_instance.get(), nullptr);

  assert(first_instance->get_id() == second_instance->get_id());
}