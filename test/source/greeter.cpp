#include <doctest/doctest.h>
#include <elephant/elephant.h>
#include <elephant/version.h>

#include <string>

TEST_CASE("Elephant") {
  using namespace elephant;

  Elephant elephant("Tests");

  CHECK(elephant.greet(LanguageCode::EN) == "Hello, Tests!");
  CHECK(elephant.greet(LanguageCode::DE) == "Hallo Tests!");
  CHECK(elephant.greet(LanguageCode::ES) == "¡Hola Tests!");
  CHECK(elephant.greet(LanguageCode::FR) == "Bonjour Tests!");
}

TEST_CASE("Elephant version") {
  static_assert(std::string_view(ELEPHANT_VERSION) == std::string_view("1.0"));
  CHECK(std::string(ELEPHANT_VERSION) == std::string("1.0"));
}
