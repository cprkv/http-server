#include "core/http-server.hpp"
#include "core/log.hpp"
#include "core/utils.hpp"
#include <llhttp.h>

using namespace core;

//---------------------------------------------------------------

struct DefaultNotFoundHandler : public HttpRequestHandler {
  ~DefaultNotFoundHandler() override = default;

  HandleResult handle() override {
    return cti::async([] {
      HttpResponse response{};
      response.status(HttpStatusCode::NotFound).with_default_status_message();
      return response;
    });
  }
};

struct DefaultBadRequestHandler : public HttpRequestHandler {
  ~DefaultBadRequestHandler() override = default;

  HandleResult handle() override {
    return cti::async([] {
      HttpResponse response{};
      response.status(HttpStatusCode::BadRequest).with_default_status_message();
      return response;
    });
  }
};

//---------------------------------------------------------------

HttpResponse& HttpResponse::status(HttpStatusCode code) {
  status_ = code;
  return *this;
}

HttpResponse& HttpResponse::with_default_status_message() {
  return *this << http_status_code_message(status_);
}

HttpResponse& HttpResponse::header(std::string key, std::string value) {
  auto [it, ok] = headers_.try_emplace(std::move(key), std::move(value));
  if (!ok) {
    g_log->error("error adding header '{}': header with that key already present", key);
  }
  return *this;
}

void HttpResponse::write(ITcpWriter* writer) {
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

  if (has_message_) {
    writer->data << message_.str() << "\r\n";
  }

  writer->done();
  delete writer;
}

//---------------------------------------------------------------

struct HttpTcpReader : public ITcpReader {
  HttpServer*       server;
  ITcpWriter*       writer;
  HttpRequestParser parser;
  bool              parse_error{ false };

  explicit HttpTcpReader(HttpServer* server, ITcpWriter* writer)
      : server{ server }
      , writer{ writer } {}

  ~HttpTcpReader() override { g_log->debug("~HttpTcpReader"); }

  void read(char* data, size_t size) override {
    if (!parser.done_) {
      if (!parse_error && !parser.handle(data, size)) {
        parse_error = true;
        server->_handle_request_parse_error(writer);
      } else if (parser.done_) {
        server->_handle_request(std::move(parser.request_), writer);
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

  ITcpReader* create(ITcpWriter* writer) override { return new HttpTcpReader(server, writer); }
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

void HttpServer::_handle_request(HttpRequest request, ITcpWriter* writer) {
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

  bool preprocess_ok = false;
  try {
    preprocess_ok = request_handler->preprocess();
  } catch (std::exception& ex) {
    g_log->debug("exception in preprocess: {}", ex.what());
  }

  if (!preprocess_ok) {
    g_log->debug("error handling request: preprocess error");
    request = std::move(request_handler->request);
    delete request_handler;
    request_handler          = make_bad_request_handler_();
    request_handler->request = std::move(request);
  }

  request_handler->handle()
      .then([writer, request_handler](HttpResponse response) mutable {
        request_handler->destroy();
        response.write(writer);
      })
      .fail(core::unwrap_exception_ptr([writer](const std::exception& ex) {
        core::HttpResponse response{};
        g_log->debug("error while handling request: {}", ex.what());
        response.status(core::HttpStatusCode::InternalServerError).with_default_status_message();
        response.write(writer);
      }));
}

void HttpServer::_handle_request_parse_error(ITcpWriter* writer) {
  g_log->debug("handling request parse error answer");
  auto request_handler = make_bad_request_handler_();
  request_handler->handle()
      .then([writer, request_handler](HttpResponse response) mutable {
        request_handler->destroy();
        response.write(writer);
      });
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
