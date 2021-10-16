#pragma once
#include "http-server/pch.hpp"

using json = nlohmann::json;

namespace core {
  //---------------------------------------------------------------

  enum class HttpBodyType {
    Unknown,
    Json,
  };

  //---------------------------------------------------------------

  struct HttpBody {
    virtual ~HttpBody() = default;
    virtual HttpBodyType type() { return HttpBodyType::Unknown; }
    virtual bool         parse(const std::string& body) { return true; }
  };

  //---------------------------------------------------------------

  struct HttpBodyJson : public HttpBody {
    json object;

    ~HttpBodyJson() override = default;
    HttpBodyType type() override { return HttpBodyType::Json; }
    bool         parse(const std::string& body) override;
  };

  //---------------------------------------------------------------

  struct HttpBodyParser {
  private:
    std::string buffer_{}; // buffer_

  public:
    void                      add_buffer(const char* at, size_t length) { buffer_ += std::string_view{ at, length }; }
    std::unique_ptr<HttpBody> parse(const std::string& content_type);
  };

  //---------------------------------------------------------------
} // namespace core
