#include <copper/copper.h>
#include <copper/version.h>
#include <doctest/doctest.h>

#include <string>

TEST_CASE("Copper") {
  using namespace copper;

  Copper copper("Tests");

  CHECK(copper.greet(LanguageCode::EN) == "Hello, Tests!");
  CHECK(copper.greet(LanguageCode::DE) == "Hallo Tests!");
  CHECK(copper.greet(LanguageCode::ES) == "¡Hola Tests!");
  CHECK(copper.greet(LanguageCode::FR) == "Bonjour Tests!");
}

TEST_CASE("Copper version") {
  static_assert(std::string_view(COPPER_VERSION) == std::string_view("1.0"));
  CHECK(std::string(COPPER_VERSION) == std::string("1.0"));
}