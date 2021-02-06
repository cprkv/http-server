#include "core/http-server.hpp"
#include "core/log.hpp"
#include "core/utils.hpp"
#include <llhttp.h>

using namespace core;

static const char* status_code_message(int code) {
  switch (code) {
    case 200: return "200 OK";
    case 201: return "201 Created";
    case 202: return "202 Accepted";
    case 203: return "203 Non-Authoritative Information";
    case 204: return "204 No Content";
    case 205: return "205 Reset Content";
    case 206: return "206 Partial Content";
    case 207: return "207 Multi-Status";
    case 208: return "208 Already Reported";
    case 226: return "226 IM Used";

    case 300: return "300 Multiple Choices";
    case 301: return "301 Moved Permanently";
    case 302: return "302 Found";
    case 303: return "303 See Other";
    case 304: return "304 Not Modified";
    case 305: return "305 Use Proxy";
    case 306: return "306 Switch Proxy";
    case 307: return "307 Temporary Redirect";
    case 308: return "308 Permanent Redirect";

    case 400: return "400 Bad Request";
    case 401: return "401 Unauthorized";
    case 402: return "402 Payment Required";
    case 403: return "403 Forbidden";
    case 404: return "404 Not Found";
    case 405: return "405 Method Not Allowed";
    case 406: return "406 Not Acceptable";
    case 407: return "407 Proxy Authentication Required";
    case 408: return "408 Request Timeout";
    case 409: return "409 Conflict";
    case 410: return "410 Gone";
    case 411: return "411 Length Required";
    case 412: return "412 Precondition Failed";
    case 413: return "413 Payload Too Large";
    case 414: return "414 Request-URI Too Long";
    case 415: return "415 Unsupported Media Type";
    case 416: return "416 Requested Range Not Satisfiable";
    case 417: return "417 Expectation Failed";
    case 418: return "418 I'm a teapot";
    case 421: return "421 Misdirected Request";
    case 422: return "422 Unprocessable Entity";
    case 423: return "423 Locked";
    case 424: return "424 Failed Dependency";
    case 426: return "426 Upgrade Required";
    case 428: return "428 Precondition Required";
    case 429: return "429 Too Many Requests";
    case 431: return "431 Request Header Fields Too Large";
    case 444: return "444 Connection Closed Without Response";
    case 451: return "451 Unavailable For Legal Reasons";
    case 499: return "499 Client Closed Request";

    case 500: return "500 Internal Server Error";
    case 501: return "501 Not Implemented";
    case 502: return "502 Bad Gateway";
    case 503: return "503 Service Unavailable";
    case 504: return "504 Gateway Timeout";
    case 505: return "505 HTTP Version Not Supported";
    case 506: return "506 Variant Also Negotiates";
    case 507: return "507 Insufficient Storage";
    case 508: return "508 Loop Detected";
    case 510: return "510 Not Extended";
    case 511: return "511 Network Authentication Required";
    case 599: return "599 Network Connect Timeout Error";

    default:
      g_log->error("unknown http status code: {}", code);
      return "500 Internal Server Error";
  }
}

//---------------------------------------------------------------

HttpResponse& HttpResponse::status(HttpStatusCode code) {
  status_ = static_cast<int>(code);
  return *this;
}

HttpResponse& HttpResponse::with_default_status_message() {
  message_ = status_code_message(status_);
  return *this;
}

HttpResponse& HttpResponse::header(std::string key, std::string value) {
  auto [it, ok] = headers_.try_emplace(std::move(key), std::move(value));
  if (!ok) {
    g_log->error("error adding header '{}': header with that key already present", key);
  }
  return *this;
}

void HttpResponse::done() {
  if (!headers_.contains("Content-Type")) {
    headers_.emplace("Content-Type", "text/html; charser=utf-8");
  }
  if (!headers_.contains("Connection")) {
    headers_.emplace("Connection", "close");
  }
  writer->data << "HTTP/1.1 " << status_code_message(status_) << "\r\n";
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
        core::HttpServer::_handle_request_parse_error(std::move(this->writer));
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
    : tcp_{ std::make_unique<HttpTcpReaderFactory>(this) } {}

void HttpServer::listen(const char* addr, int port) {
  g_log->info("http server listening on {}:{}", addr, port);
  tcp_.listen(addr, port);
}

void HttpServer::_handle_request(HttpRequest request, std::unique_ptr<ITcpWriter> writer) {
  for (auto& handler : handlers_) {
    if (handler.method == request.method && std::regex_match(request.url, handler.url_match)) {
      auto request_handler             = handler.construct();
      request_handler->request         = std::move(request);
      request_handler->response.writer = std::move(writer);
      request_handler->handle();
      return;
    }
  }

  g_log->info("error handling request: no such handler");
  HttpResponse response{ nullptr };
  response.writer = std::move(writer);
  response.status(HttpStatusCode::NotFound).with_default_status_message().done(); // todo: data destroyed here
}

void HttpServer::_handle_request_parse_error(std::unique_ptr<ITcpWriter> writer) {
  g_log->info("handling request parse error answer");
  HttpResponse response{ nullptr };
  response.writer = std::move(writer);
  response.status(HttpStatusCode::BadRequest).with_default_status_message().done(); // todo: data destroyed here
}

//---------------------------------------------------------------

void HttpRequestHandler::destroy() {
  next_tick([ptr = this]() { delete ptr; });
}

HttpRequestHandler::~HttpRequestHandler() {
  g_log->debug("~HttpRequestHandler");
}

//---------------------------------------------------------------
