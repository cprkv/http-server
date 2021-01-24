#pragma once
#include "core/tcp-server.hpp"
#include "core/http-request-parser.hpp"
#include <regex>
#include <functional>

namespace core {
  //---------------------------------------------------------------

  // base class for users
  struct HttpRequestHandler {
    std::unique_ptr<ITcpWriter> writer{ nullptr };
    HttpRequest                 request;

    virtual ~HttpRequestHandler();
    virtual void handle() = 0;
    void         destroy();
  };

  //---------------------------------------------------------------

  class HttpServer {
    using RequestHandlerFactory = std::function<HttpRequestHandler*()>; // TODO should not be unique ptr?

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
          .method    = HttpMethod::GET,
          .url_match = url_match,
          .construct = [] { return new THandler(); },
      });
    }

    template <typename THandler>
    void post(const std::regex& url_match) {
      handlers_.emplace_back(Handler{
          .method    = HttpMethod::POST,
          .url_match = url_match,
          .construct = [] { return new THandler(); },
      });
    }

    void listen(const char* addr, int port);

    // internal usage only
    void        _handle_request(HttpRequest request, std::unique_ptr<ITcpWriter> writer);
    static void _handle_request_parse_error(std::unique_ptr<ITcpWriter> writer);
  };

  //---------------------------------------------------------------
} // namespace core
