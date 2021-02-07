#pragma once
#include "core/tcp-server.hpp"
#include "core/http-request-parser.hpp"
#include "core/http-info.hpp"
#include <regex>
#include <functional>
#include <map>

struct HttpTcpReader;

namespace core {
  //---------------------------------------------------------------
  // response builder
  struct HttpResponse {
  private:
    std::unique_ptr<ITcpWriter>        writer{ nullptr };
    HttpStatusCode                     status_{ HttpStatusCode::InternalServerError };
    std::string                        message_{};
    std::map<std::string, std::string> headers_{};
    struct HttpRequestHandler*         handler_;

    friend class HttpServer;

  public:
    explicit HttpResponse(HttpRequestHandler* handler)
        : handler_{ handler } {}

    HttpResponse& header(std::string key, std::string value);
    HttpResponse& status(HttpStatusCode code);
    HttpResponse& with_default_status_message(); // requires status(...) call before

    void done();
  };

  //---------------------------------------------------------------
  // base class for users
  struct HttpRequestHandler {
    HttpRequest  request{};
    HttpResponse response;

    HttpRequestHandler()
        : response{ this } {}

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

    HttpHandlers          handlers_{};
    RequestHandlerFactory make_not_found_handler_;
    RequestHandlerFactory make_bad_request_handler_;
    TcpServer             tcp_;

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

    template <typename THandler>
    void not_found() {
      make_not_found_handler_ = [] {
        return new THandler();
      };
    }

    template <typename THandler>
    void bad_request() {
      make_bad_request_handler_ = [] {
        return new THandler();
      };
    }

    void listen(const char* addr, int port);

  private:
    friend struct ::HttpTcpReader;

    void _handle_request(HttpRequest request, std::unique_ptr<ITcpWriter> writer);
    void _handle_request_parse_error(std::unique_ptr<ITcpWriter> writer);
  };

  //---------------------------------------------------------------
} // namespace core
