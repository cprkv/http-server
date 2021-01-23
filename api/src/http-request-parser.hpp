#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <utility>
#include <llhttp.h>

namespace gallery {
  //---------------------------------------------------------------

  using HeadersMap = std::unordered_multimap<std::string, std::string>;

  enum class HttpMethod {
    GET,
    POST
  };

  struct HttpRequest {
    std::string url;
    HttpMethod  method;
    HeadersMap  headers;
    std::string body; // is it really should be string? may be some linked buffer?
  };

  //---------------------------------------------------------------

  struct HttpRequestParser {
    bool        done_{ false }; // after done it could be deleted
    HttpRequest request_{};
    llhttp_t    parser{};
    std::string last_header_field_{};
    std::string last_header_value_{};

    HttpRequestParser();
    ~HttpRequestParser();

    bool handle(char* data, size_t size);
  };

  //---------------------------------------------------------------
} // namespace gallery
