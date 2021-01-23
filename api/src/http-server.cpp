#include "http-server.hpp"
#include "log.hpp"
#include <llhttp.h>

using namespace gallery;

//---------------------------------------------------------------

static llhttp_settings_t get_http_parser_settings() noexcept;
static llhttp_settings_t http_parser_settings = get_http_parser_settings();

static constexpr const size_t max_url_size{ 128 };
static constexpr const size_t max_header_field_size{ 128 };
static constexpr const size_t max_header_value_size{ 2048 };

struct HttpTcpReaderFactory;

//---------------------------------------------------------------

struct HttpTcpReader : public ITcpReader {
  HttpServer*                 server;
  std::unique_ptr<ITcpWriter> writer;
  llhttp_t                    parser{};
  std::string                 last_header_field{};
  std::string                 last_header_value{};
  HttpRequest                 request{};

  explicit HttpTcpReader(HttpServer* server, std::unique_ptr<ITcpWriter> writer);
  ~HttpTcpReader() override = default;
  bool read(char* data, size_t size) override;
};

//---------------------------------------------------------------

struct HttpTcpReaderFactory : public ITcpReaderFactory {
  HttpServer* server;

  explicit HttpTcpReaderFactory(HttpServer* server)
      : server{ server } {}
  ~HttpTcpReaderFactory() override = default;
  ITcpReader* create(std::unique_ptr<ITcpWriter> writer) override { return new HttpTcpReader(server, std::move(writer)); }
  void        destroy(ITcpReader* client) override { delete client; }
};

//---------------------------------------------------------------

HttpTcpReader::HttpTcpReader(HttpServer* server, std::unique_ptr<ITcpWriter> writer)
    : server{ server }
    , writer{ std::move(writer) } {
  llhttp_init(&parser, HTTP_REQUEST, &http_parser_settings);
  parser.data = this;
}

bool HttpTcpReader::read(char* data, size_t size) {
  g_log->debug("http_tcp_read: {} bytes", size);

  if (auto err = llhttp_execute(&parser, data, size); err != HPE_OK) {
    g_log->error("http_tcp_read: parse error: %s %s\n", llhttp_errno_name(err), parser.reason);
    return false;
  }

  g_log->debug("http_tcp_read return ok");

  return true;
};

//---------------------------------------------------------------

static llhttp_settings_t get_http_parser_settings() noexcept {
  return {
    .on_message_begin = nullptr,

    .on_url = [](llhttp_t* http, const char* at, size_t length) -> int {
      auto reader = reinterpret_cast<HttpTcpReader*>(http->data);
      if (reader->request.url.length() + length > max_url_size) {
        return -1;
      }
      reader->request.url += std::string_view{ at, length };
      return 0;
    },

    .on_status = nullptr,

    .on_header_field = [](llhttp_t* http, const char* at, size_t length) -> int {
      auto reader = reinterpret_cast<HttpTcpReader*>(http->data);
      if (reader->last_header_field.length() + length > max_header_field_size) {
        return -1;
      }
      reader->last_header_field += std::string_view{ at, length };
      return 0;
    },

    .on_header_value = [](llhttp_t* http, const char* at, size_t length) -> int {
      auto reader = reinterpret_cast<HttpTcpReader*>(http->data);
      if (reader->last_header_value.length() + length > max_header_value_size) {
        return -1;
      }
      reader->last_header_value += std::string_view{ at, length };
      return 0;
    },

    .on_headers_complete = nullptr,

    .on_body = [](llhttp_t* http, const char* at, size_t length) -> int {
      auto reader = reinterpret_cast<HttpTcpReader*>(http->data);
      reader->request.body += std::string_view{ at, length };
      g_log->debug("[llhttp] on body {} bytes", length);
      return 0;
    },

    .on_message_complete = [](llhttp_t* http) -> int {
      auto reader      = reinterpret_cast<HttpTcpReader*>(http->data);
      auto method_name = llhttp_method_name((llhttp_method_t) http->method);
      g_log->debug("[llhttp] method: {}", method_name);

      if (strcmp(method_name, "GET") == 0) {
        reader->request.method = gallery::HttpMethod::GET;
      } else if (strcmp(method_name, "POST") == 0) {
        reader->request.method = gallery::HttpMethod::POST;
      } else {
        g_log->error("[llhttp] unknown method: {}", method_name);
        return -1;
      }

      g_log->debug("[llhttp] message complete, execute handling");

      reader->request.done = true;
      reader->server->handle_request(std::move(reader->request), std::move(reader->writer));
      return 0;
    },

    .on_chunk_header          = nullptr,
    .on_chunk_complete        = nullptr,
    .on_url_complete          = nullptr,
    .on_status_complete       = nullptr,
    .on_header_field_complete = nullptr,

    .on_header_value_complete = [](llhttp_t* http) -> int {
      auto reader = reinterpret_cast<HttpTcpReader*>(http->data);
      reader->request.headers.insert(std::make_pair(reader->last_header_field, reader->last_header_value));
      g_log->debug(R"([llhttp] header "{}": "{}")", reader->last_header_field, reader->last_header_value);
      reader->last_header_field = {};
      reader->last_header_value = {};
      return 0;
    },
  };
}

//---------------------------------------------------------------

HttpServer::HttpServer()
    : tcp_{ std::make_unique<HttpTcpReaderFactory>(this) } {}

void HttpServer::listen(const char* addr, int port) {
  g_log->info("http server listening on {}:{}", addr, port);
  tcp_.listen(addr, port);
}

void HttpServer::handle_request(HttpRequest request, std::unique_ptr<ITcpWriter> writer) {
  // TODO
  g_log->info("handling request");
  const char* message = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html; charser=utf-8\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "hello, world!\r\n"
                        "\r\n";
  size_t message_size = strlen(message);
  auto   message_buff = std::make_unique<char[]>(message_size);
  memcpy(message_buff.get(), message, message_size);
  writer->write(std::move(message_buff), message_size);
  writer->done();
}

//---------------------------------------------------------------