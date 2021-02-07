#include "core/http-body-parser.hpp"

using namespace core;

//---------------------------------------------------------------

bool HttpBodyJson::parse(const std::string& body) {
  try {
    object = json::parse(body);
  } catch (...) {
    return false;
  }
  return true;
}

//---------------------------------------------------------------

std::unique_ptr<HttpBody> HttpBodyParser::parse(const std::string& content_type) {
  if (content_type.starts_with("application/json")) {
    auto body = std::make_unique<HttpBodyJson>();
    if (body->parse(buffer_)) {
      return body;
    }
  }
  return nullptr;
}


//---------------------------------------------------------------
