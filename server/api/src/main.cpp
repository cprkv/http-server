#include "core/http-server.hpp"
#include "core/log.hpp"
#include "core/db/sqlite.hpp"

struct App {
  core::db::SqliteOnContinuable db{ core::db::SqliteSettings{ .db_name = "gallery.db" } };
} g_app;

struct ExampleHandler : public core::HttpRequestHandler {
  static inline core::HttpMethod method = core::HttpMethod::GET;
  static inline const char*      path   = "/api/example";

  ~ExampleHandler() override {
    core::g_log->debug("~ExampleHandler");
  }

  void handle() override {
    //    g_app.db.with_context<int>(
    //        [](sqlite::database& db, int& count) {
    //          db << "select count(*) from tests where age > ? ;"
    //             << 18 >>
    //              [&](int count_) {
    //                count = count_;
    //              };
    //        },
    //        [this](int& count, const auto& error) {
    //          std::stringstream    ss;
    //          core::HttpStatusCode code;
    //
    //          if (error) {
    //            ss << error.what() << "\r\n";
    //            code = core::HttpStatusCode::InternalServerError;
    //          } else {
    //            ss << "tests rows count: " << count << "\r\n";
    //            code = core::HttpStatusCode::OK;
    //          }
    //
    //          response
    //              .status(code)
    //              .with_message(ss.str())
    //              .done();
    //        });

    g_app.db.with_context<int>(
                [](sqlite::database& db) {
                  int result;
                  db << "select count(*) from tests where age > ? ;"
                     << 18 >>
                      [&](int count) {
                        result = count;
                      };
                  return result;
                })
        .then([this](int count) {
          std::stringstream ss;
          ss << "tests rows count: " << count << "\r\n";

          response
              .status(core::HttpStatusCode::OK)
              .with_message(ss.str())
              .done();
        })
        .fail([this](const std::exception_ptr& ptr) {
          if (ptr) {
            response
                .status(core::HttpStatusCode::InternalServerError)
                .with_message("some error...")
                .done();
          }
        });
  }
};

struct TestPartsHandler : public core::HttpRequestHandler {
  static inline core::HttpMethod method = core::HttpMethod::POST;
  static inline const char*      path   = "/api/part/{int}/{string}";

  int         part{ 0 };
  std::string name{};
  std::string password{};
  std::string username{};

  ~TestPartsHandler() override = default;

  bool preprocess() override {
    if (!unwrap_url(part, name)) {
      return false;
    }
    if (!request.body) {
      return false;
    }
    if (request.body->type() != core::HttpBodyType::Json) {
      return false;
    }

    auto body = dynamic_cast<core::HttpBodyJson*>(request.body.get());
    password  = body->object["password"].get<std::string>();
    username  = body->object["username"].get<std::string>();

    return true;
  }

  void handle() override {
    std::stringstream ss;
    ss << "hello from parts handler!\r\n"
       << "current part: " << part << ", name: " << name << "\r\n"
       << "password: '" << password << "'\r\n"
       << "username: '" << username << "'\r\n";

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
