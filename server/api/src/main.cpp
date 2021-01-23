#include "core/http-server.hpp"
#include <uvw.hpp>

int main(int, char**) {
  core::HttpServer server;
  server.listen("127.0.0.1", 5000);
  return uvw::Loop::getDefault()->run();
}
