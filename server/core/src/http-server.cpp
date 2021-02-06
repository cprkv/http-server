#include "core/http-server.hpp"
#include "core/log.hpp"
#include "core/utils.hpp"
#include <llhttp.h>

using namespace core;

//---------------------------------------------------------------

HttpResponse& HttpResponse::status(HttpStatusCode code) {
  status_ = static_cast<int>(code);
  return *this;
}

HttpResponse& HttpResponse::with_default_status_message() {
  switch (status_) {
    case 200: message_ = "200 OK"; break;
    case 201: message_ = "201 Created"; break;
    case 202: message_ = "202 Accepted"; break;
    case 203: message_ = "203 Non-Authoritative Information"; break;
    case 204: message_ = "204 No Content"; break;
    case 205: message_ = "205 Reset Content"; break;
    case 206: message_ = "206 Partial Content"; break;
    case 207: message_ = "207 Multi-Status"; break;
    case 208: message_ = "208 Already Reported"; break;
    case 226: message_ = "226 IM Used"; break;

    case 300: message_ = "300 Multiple Choices"; break;
    case 301: message_ = "301 Moved Permanently"; break;
    case 302: message_ = "302 Found"; break;
    case 303: message_ = "303 See Other"; break;
    case 304: message_ = "304 Not Modified"; break;
    case 305: message_ = "305 Use Proxy"; break;
    case 306: message_ = "306 Switch Proxy"; break;
    case 307: message_ = "307 Temporary Redirect"; break;
    case 308: message_ = "308 Permanent Redirect"; break;

    case 400: message_ = "400 Bad Request"; break;
    case 401: message_ = "401 Unauthorized"; break;
    case 402: message_ = "402 Payment Required"; break;
    case 403: message_ = "403 Forbidden"; break;
    case 404: message_ = "404 Not Found"; break;
    case 405: message_ = "405 Method Not Allowed"; break;
    case 406: message_ = "406 Not Acceptable"; break;
    case 407: message_ = "407 Proxy Authentication Required"; break;
    case 408: message_ = "408 Request Timeout"; break;
    case 409: message_ = "409 Conflict"; break;
    case 410: message_ = "410 Gone"; break;
    case 411: message_ = "411 Length Required"; break;
    case 412: message_ = "412 Precondition Failed"; break;
    case 413: message_ = "413 Payload Too Large"; break;
    case 414: message_ = "414 Request-URI Too Long"; break;
    case 415: message_ = "415 Unsupported Media Type"; break;
    case 416: message_ = "416 Requested Range Not Satisfiable"; break;
    case 417: message_ = "417 Expectation Failed"; break;
    case 418: message_ = "418 I'm a teapot"; break;
    case 421: message_ = "421 Misdirected Request"; break;
    case 422: message_ = "422 Unprocessable Entity"; break;
    case 423: message_ = "423 Locked"; break;
    case 424: message_ = "424 Failed Dependency"; break;
    case 426: message_ = "426 Upgrade Required"; break;
    case 428: message_ = "428 Precondition Required"; break;
    case 429: message_ = "429 Too Many Requests"; break;
    case 431: message_ = "431 Request Header Fields Too Large"; break;
    case 444: message_ = "444 Connection Closed Without Response"; break;
    case 451: message_ = "451 Unavailable For Legal Reasons"; break;
    case 499: message_ = "499 Client Closed Request"; break;

    case 500: message_ = "500 Internal Server Error"; break;
    case 501: message_ = "501 Not Implemented"; break;
    case 502: message_ = "502 Bad Gateway"; break;
    case 503: message_ = "503 Service Unavailable"; break;
    case 504: message_ = "504 Gateway Timeout"; break;
    case 505: message_ = "505 HTTP Version Not Supported"; break;
    case 506: message_ = "506 Variant Also Negotiates"; break;
    case 507: message_ = "507 Insufficient Storage"; break;
    case 508: message_ = "508 Loop Detected"; break;
    case 510: message_ = "510 Not Extended"; break;
    case 511: message_ = "511 Network Authentication Required"; break;
    case 599: message_ = "599 Network Connect Timeout Error"; break;
  }

  return *this;
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
      auto request_handler     = handler.construct();
      request_handler->writer  = std::move(writer);
      request_handler->request = std::move(request);
      request_handler->handle();
      return;
    }
  }

  g_log->info("error handling request: no such handler");
  _handle_request_parse_error(std::move(writer)); // TODO 404 not found
}

void HttpServer::_handle_request_parse_error(std::unique_ptr<ITcpWriter> writer) {
  // TODO
  g_log->info("handling request parse error answer");
  auto [buf, size] = cstr_to_writable_data(
      "HTTP/1.1 400 Bad Request\r\n"
      "Content-Type: text/html; charser=utf-8\r\n"
      "Connection: close\r\n"
      "\r\n"
      "bad request\r\n"
      "\r\n");
  writer->write(std::move(buf), size);
  writer->done();
}

//---------------------------------------------------------------

void HttpRequestHandler::destroy() {
  next_tick([ptr = this]() { delete ptr; });
}

HttpRequestHandler::~HttpRequestHandler() {
  g_log->debug("~HttpRequestHandler");
}

//---------------------------------------------------------------
