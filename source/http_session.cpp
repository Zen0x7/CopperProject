#include <copper/failure.h>
#include <copper/http_session.h>
#include <copper/websocket_session.h>

#include <boost/json/object.hpp>
#include <boost/json/serialize.hpp>

boost::beast::string_view mime_type(boost::beast::string_view path) {
  using boost::beast::iequals;
  auto const ext = [&path] {
    auto const pos = path.rfind(".");
    if (pos == boost::beast::string_view::npos) return boost::beast::string_view{};
    return path.substr(pos);
  }();
  if (iequals(ext, ".htm")) return "text/html";
  if (iequals(ext, ".html")) return "text/html";
  if (iequals(ext, ".php")) return "text/html";
  if (iequals(ext, ".css")) return "text/css";
  if (iequals(ext, ".txt")) return "text/plain";
  if (iequals(ext, ".js")) return "application/javascript";
  if (iequals(ext, ".json")) return "application/json";
  if (iequals(ext, ".xml")) return "application/xml";
  if (iequals(ext, ".swf")) return "application/x-shockwave-flash";
  if (iequals(ext, ".flv")) return "video/x-flv";
  if (iequals(ext, ".png")) return "image/png";
  if (iequals(ext, ".jpe")) return "image/jpeg";
  if (iequals(ext, ".jpeg")) return "image/jpeg";
  if (iequals(ext, ".jpg")) return "image/jpeg";
  if (iequals(ext, ".gif")) return "image/gif";
  if (iequals(ext, ".bmp")) return "image/bmp";
  if (iequals(ext, ".ico")) return "image/vnd.microsoft.icon";
  if (iequals(ext, ".tiff")) return "image/tiff";
  if (iequals(ext, ".tif")) return "image/tiff";
  if (iequals(ext, ".svg")) return "image/svg+xml";
  if (iequals(ext, ".svgz")) return "image/svg+xml";
  return "application/text";
}

std::string path_cat(boost::beast::string_view base, boost::beast::string_view path) {
  if (base.empty()) return std::string(path);
  std::string result(base);
#ifdef BOOST_MSVC
  char constexpr path_separator = '\\';
  if (result.back() == path_separator) result.resize(result.size() - 1);
  result.append(path.data(), path.size());
  for (auto& c : result)
    if (c == '/') c = path_separator;
#else
  char constexpr path_separator = '/';
  if (result.back() == path_separator) result.resize(result.size() - 1);
  result.append(path.data(), path.size());
#endif
  return result;
}

template <class Body, class Allocator> boost::beast::http::message_generator handle_request(
    boost::shared_ptr<copper::state> /* state */,
    boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req) {
  auto const not_found = [&req] {
    boost::json::object message = {{"message", "not found"}, {"status", 404}};
    boost::beast::http::response<boost::beast::http::string_body> res{
        boost::beast::http::status::not_found, req.version()};

    res.set(boost::beast::http::field::server, "Copper");
    res.set(boost::beast::http::field::content_type, "application/json");
    res.keep_alive(req.keep_alive());
    res.body() = serialize(message);
    res.prepare_payload();
    return res;
  };

  return not_found();
}

copper::http_session::http_session(boost::asio::ip::tcp::socket&& socket,
                                   boost::shared_ptr<state> const& state)
    : stream_(std::move(socket)), state_(state) {}

void copper::http_session::run() { do_read(); }

void copper::http_session::do_read() {
  parser_.emplace();

  parser_->body_limit(10000);

  stream_.expires_after(std::chrono::seconds(30));

  boost::beast::http::async_read(
      stream_, buffer_, parser_->get(),
      boost::beast::bind_front_handler(&http_session::on_read, shared_from_this()));
}

void copper::http_session::on_read(boost::beast::error_code error, std::size_t) {
  if (error == boost::beast::http::error::end_of_stream) {
    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, error);
    return;
  }

  if (error) return failure::make(error, "read");

  if (boost::beast::websocket::is_upgrade(parser_->get())) {
    boost::make_shared<websocket_session>(stream_.release_socket(), state_)
        ->run(parser_->release());
    return;
  }

  boost::beast::http::message_generator message = handle_request(state_, parser_->release());

  bool keep_alive = message.keep_alive();

  auto self = shared_from_this();

  boost::beast::async_write(stream_, std::move(message),
                            [self, keep_alive](boost::beast::error_code error, std::size_t bytes) {
                              self->on_write(error, bytes, keep_alive);
                            });
}

void copper::http_session::on_write(boost::beast::error_code error, std::size_t, bool closes) {
  if (error) return failure::make(error, "copper::http_sesion::on_write");

  if (!closes) {
    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, error);
    return;
  }

  do_read();
}
