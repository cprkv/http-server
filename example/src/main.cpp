#include "http-server/http-server.hpp"
#include "http-server/log.hpp"
#include "http-server/db/sqlite.hpp"
#include "http-server/ulid.hpp"
#include "http-server/db/tarantool/client.hpp"

struct App {
  http::db::Sqlite db{ { .db_name = "gallery.db" } };
} g_app;

//-----------------------------------------------------------------------

struct ExampleHandler : public http::HttpRequestHandler {
  static inline http::HttpMethod method = http::HttpMethod::GET;
  static inline const char*      path   = "/api/example";

  ~ExampleHandler() override {
    http::g_log->debug("~ExampleHandler");
  }

  HandleResult handle() override {
    return g_app.db
        .with_connection(
            [](sqlite::database& db) {
              int result;
              db << "select count(*) from tests where age > ? ;"
                 << 18 >>
                  [&](int count) {
                    result = count;
                  };
              return result;
            })
        .then([](int count) {
          http::HttpResponse response{};
          response.status(http::HttpStatusCode::OK)
              << "tests rows count: " << count << "\r\n"
              << "some ulid: " << http::UlidGenerator::main().generate().str() << "\r\n";
          return response;
        });
  }
};

//-----------------------------------------------------------------------

struct FillHandler : public http::HttpRequestHandler {
  static inline http::HttpMethod method = http::HttpMethod::GET;
  static inline const char*      path   = "/api/fill";

  ~FillHandler() override = default;

  HandleResult handle() override {
    return g_app.db
        .with_connection(
            [](sqlite::database& db) {
              db << "create table if not exists tests ("
                    "   _id integer primary key autoincrement not null,"
                    "   age int,"
                    "   name text,"
                    "   weight real"
                    ");";
              db << "insert into tests (age,name,weight) values (?,?,?);"
                 << 20 << u"bob" << 83.25;
              db << "insert into tests (age,name,weight) values (?,?,?);"
                 << 21 << u"alice" << 56.4;
              db << "insert into tests (age,name,weight) values (?,?,?);"
                 << 22 << u"dungeon master" << 98.0;
              db << "insert into tests (age,name,weight) values (?,?,?);"
                 << 24 << u"stefany" << 44.25;
              return nullptr;
            })
        .then([](nullptr_t) {
          http::HttpResponse response{};
          response.status(http::HttpStatusCode::Created).with_default_status_message();
          return response;
        });
  }
};

//-----------------------------------------------------------------------

struct TestPartsHandler : public http::HttpRequestHandler {
  static inline http::HttpMethod method = http::HttpMethod::POST;
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
    if (request.body->type() != http::HttpBodyType::Json) {
      return false;
    }

    auto body = dynamic_cast<http::HttpBodyJson*>(request.body.get());
    password  = body->object["password"].get<std::string>();
    username  = body->object["username"].get<std::string>();

    return true;
  }

  HandleResult handle() override {
    return cti::async([this] {
      http::HttpResponse response{};
      response.status(http::HttpStatusCode::OK)
          << "hello from parts handler!\r\n"
          << "current part: " << part << ", name: " << name << "\r\n"
          << "password: '" << password << "'\r\n"
          << "username: '" << username << "'\r\n";
      return response;
    });
  }
};

//-----------------------------------------------------------------------

int main(int, char**) {
  http::TcpClient client{ std::make_unique<http::db::tarantool::Client>() };
  client.connect("127.0.0.1", 3301u);

  // http::HttpServer server;
  // server.add_handler<ExampleHandler>();
  // server.add_handler<TestPartsHandler>();
  // server.add_handler<FillHandler>();
  // server.listen("127.0.0.1", 5000);
  return http::run_main_loop();
}
