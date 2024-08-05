#include <copper/logger.h>
#include <doctest/doctest.h>

#include <boost/uuid/random_generator.hpp>
#include <string>

TEST_CASE("Logger") {
  using namespace copper;
  const auto id = boost::uuids::random_generator()();
  logger::success("msg");
  logger::on_broadcast(id, "msg");
  logger::on_connect(id);
}