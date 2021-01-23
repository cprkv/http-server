#pragma once
#include "core/tcp-server.hpp"
#include "core/http-request-parser.hpp"
#include <regex>
#include <functional>

namespace core {
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

    using HttpHandlers = std::vector<Handler>; // TODO: regex-tree?

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
    void handle_request_parse_error(std::unique_ptr<ITcpWriter> writer);
  };

  //---------------------------------------------------------------
} // namespace core
