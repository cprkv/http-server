#include "http-server/http-request-parser.hpp"
#include "http-server/log.hpp"
#include "http-server/http-info.hpp"

using namespace core;

static llhttp_settings_t get_http_parser_settings() noexcept;


static constexpr const size_t  max_url_size{ 128 };
static constexpr const size_t  max_header_field_size{ 128 };
static constexpr const size_t  max_header_value_size{ 2048 };
static const llhttp_settings_t g_parser_settings{ get_http_parser_settings() };

static llhttp_settings_t get_http_parser_settings() noexcept {
  return {
    .on_message_begin = nullptr,

    .on_url = [](llhttp_t* http, const char* at, size_t length) -> int {
      auto reader = reinterpret_cast<HttpRequestParser*>(http->data);
      if (reader->request_.url.length() + length > max_url_size) {
        return -1;
      }
      reader->request_.url += std::string_view{ at, length };
      return 0;
    },

    .on_status = nullptr,

    .on_header_field = [](llhttp_t* http, const char* at, size_t length) -> int {
      auto reader = reinterpret_cast<HttpRequestParser*>(http->data);
      if (reader->last_header_field_.length() + length > max_header_field_size) {
        return -1;
      }
      reader->last_header_field_ += std::string_view{ at, length };
      return 0;
    },

    .on_header_value = [](llhttp_t* http, const char* at, size_t length) -> int {
      auto reader = reinterpret_cast<HttpRequestParser*>(http->data);
      if (reader->last_header_value_.length() + length > max_header_value_size) {
        return -1;
      }
      reader->last_header_value_ += std::string_view{ at, length };
      return 0;
    },

    .on_headers_complete = nullptr,

    .on_body = [](llhttp_t* http, const char* at, size_t length) -> int {
      auto reader = reinterpret_cast<HttpRequestParser*>(http->data);
      reader->body_parser_.add_buffer(at, length);
      g_log->debug("[llhttp] on body {} bytes", length);
      return 0;
    },

    .on_message_complete = [](llhttp_t* http) -> int {
      auto reader      = reinterpret_cast<HttpRequestParser*>(http->data);
      auto method_name = llhttp_method_name((llhttp_method_t) http->method);
      g_log->debug("[llhttp] method: {}", method_name);

      if (strcmp(method_name, "GET") == 0) {
        reader->request_.method = core::HttpMethod::GET;
      } else if (strcmp(method_name, "POST") == 0) {
        reader->request_.method = core::HttpMethod::POST;
      } else {
        g_log->error("[llhttp] unknown method: {}", method_name);
        return -1;
      }

      g_log->debug("[llhttp] message complete, execute handling");

      auto content_type_it = reader->request_.headers.find(std::string{ HttpRequestHeaderKey::ContentType });
      if (content_type_it != reader->request_.headers.end()) {
        auto content_type = content_type_it->second;
        auto body         = reader->body_parser_.parse(content_type);
        if (body) {
          reader->request_.body = std::move(body);
        } else {
          g_log->debug("content couldn't be parsed by type {}", content_type);
          return -1;
        }
      }

      reader->done_ = true;
      return 0;
    },

    .on_chunk_header          = nullptr,
    .on_chunk_complete        = nullptr,
    .on_url_complete          = nullptr,
    .on_status_complete       = nullptr,
    .on_header_field_complete = nullptr,

    .on_header_value_complete = [](llhttp_t* http) -> int {
      auto reader = reinterpret_cast<HttpRequestParser*>(http->data);
      reader->request_.headers.insert(std::make_pair(reader->last_header_field_, reader->last_header_value_));
      g_log->debug(R"([llhttp] header "{}": "{}")", reader->last_header_field_, reader->last_header_value_);
      reader->last_header_field_ = {};
      reader->last_header_value_ = {};
      return 0;
    },
  };
}

HttpRequestParser::HttpRequestParser() {
  llhttp_init(&parser, HTTP_REQUEST, &g_parser_settings);
  parser.data = this;
}

bool HttpRequestParser::handle(char* data, size_t size) {
  g_log->debug("http_tcp_read: {} bytes", size);

  if (auto err = llhttp_execute(&parser, data, size); err != HPE_OK) {
    g_log->error("http_tcp_read: parse error: %s %s\n", llhttp_errno_name(err), parser.reason);
    return false;
  }

  if (done_) {
    g_log->debug("http_tcp_read DONE");
  }

  return true;
}

HttpRequestParser::~HttpRequestParser() {
  g_log->debug("~HttpRequestParser");
}
