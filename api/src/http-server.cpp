#include "http-server.hpp"
#include "log.hpp"

using namespace gallery;

namespace {
  struct HttpTcpReader : public ITcpReader {
    ~HttpTcpReader() override = default;

    bool read(char* data, size_t size) override {
      g_log->debug("http_tcp_read: {} bytes", size);
      return true;
    }
  };

  struct HttpTcpReaderFactory : public ITcpReaderFactory {
    ~HttpTcpReaderFactory() override = default;
    ITcpReader* create() override { return new HttpTcpReader; }
    void        destroy(ITcpReader* client) override { delete client; }
  };
} // namespace

HttpServer::HttpServer()
    : tcp_{ std::make_unique<HttpTcpReaderFactory>() } {}

void HttpServer::listen(const char* addr, int port) {
  g_log->info("http server listening on {}:{}", addr, port);
  tcp_.listen(addr, port);
}
