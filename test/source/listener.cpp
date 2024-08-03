#include <copper/listener.h>
#include <copper/state.h>
#include <copper/version.h>
#include <doctest/doctest.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <string>

TEST_CASE("Listener") {
  using namespace copper;

  auto first_state_ = std::make_shared<state>();
  auto second_state_ = std::make_shared<state>();

  auto address_ = boost::asio::ip::make_address("0.0.0.0");
  auto port_ = static_cast<unsigned short>(8000);
  // constexpr auto threads_ = 4;

  boost::asio::io_context io_context_;

  auto state_ = boost::make_shared<state>();

  boost::make_shared<listener>(io_context_, boost::asio::ip::tcp::endpoint{address_, port_}, state_)->run();

  // std::vector<std::thread> v;
  // v.reserve(threads_ - 1);
  // for(auto i = threads_ - 1; i > 0; --i)
  //   v.emplace_back(
  //   [&io_context_]
  //   {
  //       io_context_.run();
  //   });

  CHECK_NE(first_state_->get_id(), second_state_->get_id());
}