#pragma once
#include "tcp-server.hpp"
#include <string>
#include <regex>
#include <unordered_map>
#include <functional>

namespace gallery {
  //---------------------------------------------------------------

  using HeadersMap = std::unordered_multimap<std::string, std::string>;

  enum class HttpMethod {
    GET,
    POST
  };

  struct HttpRequest {
    bool        done{ false };
    std::string url;
    HttpMethod  method;
    HeadersMap  headers;
    std::string body; // is it really should be string? may be some linked buffer?
  };

  //---------------------------------------------------------------

  // base class for users
  struct HttpRequestHandler {
    virtual void handle(HttpRequest request) = 0;
  };

  //---------------------------------------------------------------

  class HttpServer {
    using RequestHandlerFactory = std::function<std::unique_ptr<HttpRequestHandler>()>;

    struct Handler {
      HttpMethod            method;
      std::regex            url_match;
      RequestHandlerFactory construct;
    };

    using HttpHandlers = std::vector<Handler>;

    HttpHandlers handlers_{};
    TcpServer    tcp_;

  public:
    HttpServer();

    template <typename THandler>
    void get(const std::regex& url_match) {
      handlers_.emplace_back(Handler{
          .url_match = url_match,
          .method    = HttpMethod::GET,
          .construct = [] { return std::make_unique<THandler>(); },
      });
    }

    template <typename THandler>
    void post(const std::regex& url_match) {
      handlers_.emplace_back(Handler{
          .url_match = url_match,
          .method    = HttpMethod::POST,
          .construct = [] { return std::make_unique<THandler>(); },
      });
    }

    void listen(const char* addr, int port);

    // internal usage only
    void handle_request(HttpRequest request, std::unique_ptr<ITcpWriter> writer);
  };

  //---------------------------------------------------------------
} // namespace gallery
