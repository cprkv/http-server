#pragma once
#include "core/tcp-server.hpp"
#include "core/http-request-parser.hpp"
#include <regex>
#include <functional>

namespace core {
  //---------------------------------------------------------------

  enum class HttpStatusCode : int {
    OK                          = 200,
    Created                     = 201,
    Accepted                    = 202,
    NonAuthoritativeInformation = 203,
    NoContent                   = 204,
    ResetContent                = 205,
    PartialContent              = 206,
    MultiStatus                 = 207,
    AlreadyReported             = 208,
    IMUsed                      = 226,

    MultipleChoices   = 300,
    MovedPermanently  = 301,
    Found             = 302,
    SeeOther          = 303,
    NotModified       = 304,
    UseProxy          = 305,
    SwitchProxy       = 306,
    TemporaryRedirect = 307,
    PermanentRedirect = 308,

    BadRequest                      = 400,
    Unauthorized                    = 401,
    PaymentRequired                 = 402,
    Forbidden                       = 403,
    NotFound                        = 404,
    MethodNotAllowed                = 405,
    NotAcceptable                   = 406,
    ProxyAuthenticationRequired     = 407,
    RequestTimeout                  = 408,
    Conflict                        = 409,
    Gone                            = 410,
    LengthRequired                  = 411,
    PreconditionFailed              = 412,
    PayloadTooLarge                 = 413,
    RequestURITooLong               = 414,
    UnsupportedMediaType            = 415,
    RequestedRangeNotSatisfiable    = 416,
    ExpectationFailed               = 417,
    ImATeapot                       = 418,
    MisdirectedRequest              = 421,
    UnprocessableEntity             = 422,
    Locked                          = 423,
    FailedDependency                = 424,
    UpgradeRequired                 = 426,
    PreconditionRequired            = 428,
    TooManyRequests                 = 429,
    RequestHeaderFieldsTooLarge     = 431,
    ConnectionClosedWithoutResponse = 444,
    UnavailableForLegalReasons      = 451,
    ClientClosedRequest             = 499,

    InternalServerError           = 500,
    NotImplemented                = 501,
    BadGateway                    = 502,
    ServiceUnavailable            = 503,
    GatewayTimeout                = 504,
    HTTPVersionNotSupported       = 505,
    VariantAlsoNegotiates         = 506,
    InsufficientStorage           = 507,
    LoopDetected                  = 508,
    NotExtended                   = 510,
    NetworkAuthenticationRequired = 511,
    NetworkConnectTimeoutError    = 599,
  };

  //---------------------------------------------------------------

  // response builder
  struct HttpResponse {
  private:
    std::unique_ptr<ITcpWriter> writer{ nullptr };
    int                         status_{ 500 };
    std::string                 message_{};

  public:
    HttpResponse() = default;

    HttpResponse& status(HttpStatusCode code);
    HttpResponse& with_default_status_message(); // requires status(...) call before

  };

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
