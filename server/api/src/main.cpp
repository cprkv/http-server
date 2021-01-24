#include "core/http-server.hpp"
#include "core/log.hpp"
#include "core/utils.hpp"

struct ExampleHandler : public core::HttpRequestHandler {
  void handle() override {
    auto [buf, size] = core::cstr_to_writable_data(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charser=utf-8\r\n"
        "Connection: close\r\n"
        "\r\n"
        "example!\r\n"
        "\r\n");
    writer->write(std::move(buf), size);
    writer->done();
    destroy();
  }

  ~ExampleHandler() override {
    core::g_log->debug("~ExampleHandler");
  }
};

int main(int, char**) {
  core::HttpServer server;
  server.get<ExampleHandler>(std::regex{ "/api/example" });
  server.listen("127.0.0.1", 5000);
  return core::run_main_loop();
}
