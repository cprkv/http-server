#include "tcp-server.hpp"
#include "log.hpp"
#include <uvw.hpp>

using namespace gallery;

TcpServer::TcpServer()
    : tcp_handle{ uvw::Loop::getDefault()->resource<uvw::TCPHandle>() } {
  tcp_handle->on<uvw::ErrorEvent>([](const uvw::ErrorEvent& err, uvw::TCPHandle&) {
    g_log->error("tcp handle error: {}", err.what()); // TODO: handle it properly (close handle ...)
  });
}

void TcpServer::listen(const char* addr, int port) {
  tcp_handle->bind(addr, port);
  tcp_handle->listen();
}
