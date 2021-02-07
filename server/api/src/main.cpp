#include "core/http-server.hpp"
#include "core/log.hpp"
#include "core/utils.hpp"

struct ExampleHandler : public core::HttpRequestHandler {
  ~ExampleHandler() override {
    core::g_log->debug("~ExampleHandler");
  }

  void handle() override {
    response
        .status(core::HttpStatusCode::OK)
        .with_default_status_message()
        .done();
  }
};

int main(int, char**) {
  core::HttpServer server;
  server.get<ExampleHandler>(std::regex{ "/api/example" });
  server.listen("127.0.0.1", 5000);
  return core::run_main_loop();
}
