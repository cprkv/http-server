#include "core/http-server.hpp"
#include "core/log.hpp"
#include "utils.hpp"
#include <llhttp.h>

using namespace core;

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
        server->handle_request_parse_error(std::move(this->writer));
      } else if (parser.done_) {
        server->handle_request(std::move(parser.request_), std::move(this->writer));
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

void HttpServer::handle_request(HttpRequest request, std::unique_ptr<ITcpWriter> writer) {
  // TODO
  g_log->info("handling request");
  auto [buf, size] = cstr_to_writable_data("HTTP/1.1 200 OK\r\n"
                                           "Content-Type: text/html; charser=utf-8\r\n"
                                           "Connection: close\r\n"
                                           "\r\n"
                                           "hello, world!\r\n"
                                           "\r\n");
  writer->write(std::move(buf), size);
  writer->done();
}

void HttpServer::handle_request_parse_error(std::unique_ptr<ITcpWriter> writer) {
  // TODO
  g_log->info("handling request parse error answer");
  auto [buf, size] = cstr_to_writable_data("HTTP/1.1 400 Bad Request\r\n"
                                           "Content-Type: text/html; charser=utf-8\r\n"
                                           "Connection: close\r\n"
                                           "\r\n"
                                           "bad request\r\n"
                                           "\r\n");
  writer->write(std::move(buf), size);
  writer->done();
}

//---------------------------------------------------------------