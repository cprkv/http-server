#pragma once
#include <string_view>
#include <string>

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

  /** @brief returns message in format "{Status Code Number} {Status Code Description}" */
  const char* http_status_code_message(HttpStatusCode code);

  //---------------------------------------------------------------

  struct HttpRequestHeaderKey {
    static constexpr std::string_view Authorization   = "Authorization";
    static constexpr std::string_view Accept          = "Accept";
    static constexpr std::string_view Connection      = "Connection";
    static constexpr std::string_view CacheControl    = "Cache-Control";
    static constexpr std::string_view ContentEncoding = "Content-Encoding";
    static constexpr std::string_view ContentLength   = "Content-Length";
    static constexpr std::string_view ContentType     = "Content-Type";
    static constexpr std::string_view Cookie          = "Cookie";
    static constexpr std::string_view Forwarded       = "Forwarded";
    static constexpr std::string_view Host            = "Host";
    static constexpr std::string_view Origin          = "Origin";
    static constexpr std::string_view Referer         = "Referer";
    static constexpr std::string_view UserAgent       = "User-Agent";
  };

  //---------------------------------------------------------------

  struct HttpResponseHeaderKey {
    static constexpr std::string_view Allow              = "Allow";
    static constexpr std::string_view CacheControl       = "Cache-Control";
    static constexpr std::string_view Connection         = "Connection";
    static constexpr std::string_view ContentDisposition = "Content-Disposition";
    static constexpr std::string_view ContentEncoding    = "Content-Encoding";
    static constexpr std::string_view ContentLength      = "Content-Length";
    static constexpr std::string_view ContentType        = "Content-Type";
    static constexpr std::string_view Expires            = "Expires";
    static constexpr std::string_view Location           = "Location";
    static constexpr std::string_view Server             = "Server";
    static constexpr std::string_view SetCookie          = "Set-Cookie";
    static constexpr std::string_view WWWAuthenticate    = "WWW-Authenticate";
  };

  //---------------------------------------------------------------

  struct Mime {
    static constexpr std::string_view application_javascript            = "application/javascript";
    static constexpr std::string_view application_json                  = "application/json";
    static constexpr std::string_view application_pdf                   = "application/pdf";
    static constexpr std::string_view application_x_www_form_urlencoded = "application/x-www-form-urlencoded";
    static constexpr std::string_view application_xml                   = "application/xml";
    static constexpr std::string_view application_zip                   = "application/zip";
    static constexpr std::string_view audio_mpeg                        = "audio/mpeg";
    static constexpr std::string_view audio_ogg                         = "audio/ogg";
    static constexpr std::string_view image_gif                         = "image/gif";
    static constexpr std::string_view image_apng                        = "image/apng";
    static constexpr std::string_view image_flif                        = "image/flif";
    static constexpr std::string_view image_webp                        = "image/webp";
    static constexpr std::string_view image_jpeg                        = "image/jpeg";
    static constexpr std::string_view image_png                         = "image/png";
    static constexpr std::string_view multipart_form_data               = "multipart/form-data";
    static constexpr std::string_view text_css                          = "text/css";
    static constexpr std::string_view text_csv                          = "text/csv";
    static constexpr std::string_view text_html                         = "text/html";
    static constexpr std::string_view text_php                          = "text/php";
    static constexpr std::string_view text_plain                        = "text/plain";
    static constexpr std::string_view text_xml                          = "text/xml";

    inline static std::string combine(std::string_view type, std::string_view param) {
      std::string result{ type };
      result += "; ";
      result += param;
      return result;
    }
  };

  //---------------------------------------------------------------
} // namespace core
