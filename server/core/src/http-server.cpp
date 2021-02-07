#include "core/http-server.hpp"
#include "core/log.hpp"
#include "core/utils.hpp"
#include <llhttp.h>

using namespace core;

//---------------------------------------------------------------

struct DefaultNotFoundHandler : public HttpRequestHandler {
  ~DefaultNotFoundHandler() override = default;

  void handle() override {
    response
        .status(HttpStatusCode::NotFound)
        .with_default_status_message()
        .done();
  }
};

struct DefaultBadRequestHandler : public HttpRequestHandler {
  ~DefaultBadRequestHandler() override = default;

  void handle() override {
    response
        .status(HttpStatusCode::BadRequest)
        .with_default_status_message()
        .done();
  }
};

//---------------------------------------------------------------

HttpResponse& HttpResponse::status(HttpStatusCode code) {
  status_ = code;
  return *this;
}

HttpResponse& HttpResponse::with_default_status_message() {
  message_ = http_status_code_message(status_);
  return *this;
}

HttpResponse& HttpResponse::header(std::string key, std::string value) {
  auto [it, ok] = headers_.try_emplace(std::move(key), std::move(value));
  if (!ok) {
    g_log->error("error adding header '{}': header with that key already present", key);
  }
  return *this;
}

HttpResponse& HttpResponse::with_message(std::string message) {
  message_ = std::move(message);
  return *this;
}

void HttpResponse::done() {
  static const std::string default_content_type{ Mime::combine(Mime::text_html, "charser=utf-8") };

  if (!headers_.contains("Content-Type")) {
    headers_.emplace(HttpResponseHeaderKey::ContentType, default_content_type);
  }
  if (!headers_.contains("Connection")) {
    headers_.emplace(HttpResponseHeaderKey::Connection, "close");
  }

  writer->data << "HTTP/1.1 " << http_status_code_message(status_) << "\r\n";

  for (auto& [key, value] : headers_) {
    writer->data << key << ": " << value << "\r\n";
  }

  writer->data << "\r\n";

  if (!message_.empty()) {
    writer->data << message_ << "\r\n";
  }

  writer->done();

  if (handler_) {
    handler_->destroy();
  }
}

//---------------------------------------------------------------

struct HttpTcpReader : public ITcpReader {
  HttpServer*                 server;
  std::unique_ptr<ITcpWriter> writer;
  HttpRequestParser           parser;
  bool                        parse_error{ false };

  explicit HttpTcpReader(HttpServer* server, std::unique_ptr<ITcpWriter> writer)
      : server{ server }
      , writer{ std::move(writer) } {}

  ~HttpTcpReader() override { g_log->debug("~HttpTcpReader"); }

  void read(char* data, size_t size) override {
    if (!parser.done_) {
      if (!parse_error && !parser.handle(data, size)) {
        parse_error = true;
        server->_handle_request_parse_error(std::move(this->writer));
      } else if (parser.done_) {
        server->_handle_request(std::move(parser.request_), std::move(this->writer));
      }
    }
  }
};

//---------------------------------------------------------------

struct HttpTcpReaderFactory : public ITcpReaderFactory {
  HttpServer* server;

  explicit HttpTcpReaderFactory(HttpServer* server)
      : server{ server } {}
  ~HttpTcpReaderFactory() override = default;

  ITcpReader* create(std::unique_ptr<ITcpWriter> writer) override { return new HttpTcpReader(server, std::move(writer)); }
  void        destroy(ITcpReader* client) override { delete client; }
};

//---------------------------------------------------------------

HttpServer::HttpServer()
    : make_not_found_handler_{ [] {
      return new DefaultNotFoundHandler();
    } }
    , make_bad_request_handler_{ [] {
      return new DefaultBadRequestHandler();
    } }
    , tcp_{ std::make_unique<HttpTcpReaderFactory>(this) } {}

void HttpServer::listen(const char* addr, int port) {
  g_log->info("http server listening on {}:{}", addr, port);
  tcp_.listen(addr, port);
}

void HttpServer::_handle_request(HttpRequest request, std::unique_ptr<ITcpWriter> writer) {
  HttpRequestHandler* request_handler{ nullptr };
  std::smatch         matches;

  for (auto& handler : handlers_) {
    if (handler.method == request.method && std::regex_match(request.url, matches, handler.url_match)) {
      request_handler     = handler.construct();
      request.url_matches = matches;
      break;
    }
  }

  if (!request_handler) {
    g_log->debug("error handling request: no such handler for '{}'", request.url);
    request_handler = make_not_found_handler_();
  }

  request_handler->request = std::move(request);

  if (!request_handler->preprocess()) {
    g_log->debug("error handling request: preprocess error");
    request = std::move(request_handler->request);
    delete request_handler;
    request_handler          = make_bad_request_handler_();
    request_handler->request = std::move(request);
  }

  request_handler->response.writer = std::move(writer);
  request_handler->handle();
}

void HttpServer::_handle_request_parse_error(std::unique_ptr<ITcpWriter> writer) {
  g_log->debug("handling request parse error answer");
  auto request_handler             = make_bad_request_handler_();
  request_handler->response.writer = std::move(writer);
  request_handler->handle();
}

std::regex HttpServer::preprocess_regex(const std::string& str) {
  std::string result{ "^" + str + "$" };
  result = replace_all(result, "{int}", "(\\d+)");
  result = replace_all(result, "{string}", "([^\\/]+)");
  return std::regex(result);
}

//---------------------------------------------------------------

void HttpRequestHandler::destroy() {
  next_tick([ptr = this]() { delete ptr; });
}

HttpRequestHandler::~HttpRequestHandler() {
  g_log->debug("~HttpRequestHandler");
}

//---------------------------------------------------------------
