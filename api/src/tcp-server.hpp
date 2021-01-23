#pragma once
#include <uvw.hpp>

namespace gallery {
  class TcpServer {
    std::shared_ptr<uvw::TCPHandle> tcp_handle;

  public:
    explicit TcpServer();
    void listen(const char* addr, int port);
  };
} // namespace gallery
