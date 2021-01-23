#pragma once
#include "tcp-server.hpp"

namespace gallery {
  class HttpServer {
    TcpServer tcp_;

  public:
    HttpServer();

    void listen(const char* addr, int port);
  };
} // namespace gallery
