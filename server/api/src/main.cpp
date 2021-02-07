#include "core/http-server.hpp"
#include "core/log.hpp"
#include "core/utils.hpp"

struct ExampleHandler : public core::HttpRequestHandler {
  static inline core::HttpMethod method = core::HttpMethod::GET;
  static inline const char*      path   = "^/api/example$";

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

struct TestPartsHandler : public core::HttpRequestHandler {
  static inline core::HttpMethod method = core::HttpMethod::GET;
  static inline const char*      path   = "^/api/part/(\\d+)/([^\\/]+)$";

  int         part{ 0 };
  std::string name{};

  ~TestPartsHandler() override = default;

  bool preprocess() override {
    return core::url::unwrap(request.url_matches, part, name);
  }

  void handle() override {
    std::stringstream ss;
    ss << "hello from parts handler!\r\n"
       << "current part: " << part << ", name: " << name << "\r\n";
    response
        .status(core::HttpStatusCode::OK)
        .with_message(ss.str())
        .done();
  }
};


int main(int, char**) {
  core::HttpServer server;
  server.add_handler<ExampleHandler>();
  server.add_handler<TestPartsHandler>();
  server.listen("127.0.0.1", 5000);
  return core::run_main_loop();
}
