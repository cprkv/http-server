#pragma once
#include "core/tcp-server.hpp"
#include "core/http-request-parser.hpp"
#include "core/http-info.hpp"
#include "core/utils.hpp"
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
    HttpResponse& with_message(std::string message);
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
    virtual bool preprocess() { return true; }
    virtual void handle() = 0;
    void         destroy();

    template <typename... TArgs>
    bool unwrap_url(TArgs&&... args) {
      return url::unwrap(request.url_matches, std::forward<TArgs>(args)...);
    }
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

    static std::regex preprocess_regex(const std::string& str);

  public:
    HttpServer();

    template <typename THandler>
    void add_handler() {
      handlers_.emplace_back(Handler{
          .method    = THandler::method,
          .url_match = preprocess_regex(THandler::path),
          .construct = [] { return new THandler(); },
      });
    }

    template <typename THandler>
    void set_not_found_handler() {
      make_not_found_handler_ = [] {
        return new THandler();
      };
    }

    template <typename THandler>
    void set_bad_request_handler() {
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
