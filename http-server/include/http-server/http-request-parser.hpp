#pragma once
#include "http-server/pch.hpp"
#include "http-server/http-body-parser.hpp"

namespace core {
  //---------------------------------------------------------------

  using HeadersMap = std::unordered_multimap<std::string, std::string>;

  enum class HttpMethod {
    GET,
    POST
  };

  struct HttpRequest {
    std::string               url;
    std::smatch               url_matches;
    HttpMethod                method;
    HeadersMap                headers;
    std::unique_ptr<HttpBody> body{ nullptr };
  };

  //---------------------------------------------------------------

  struct HttpRequestParser {
    bool           done_{ false }; // after done it could be deleted
    HttpRequest    request_{};
    llhttp_t       parser{};
    std::string    last_header_field_{};
    std::string    last_header_value_{};
    HttpBodyParser body_parser_{};

    HttpRequestParser();
    ~HttpRequestParser();

    bool handle(char* data, size_t size);
  };

  //---------------------------------------------------------------
} // namespace core
