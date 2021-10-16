#include "http-server/error.hpp"

using namespace http;

std::string_view http::to_string(ErrorCode code) {
  switch (code) {
#define EXPAND_X_ERROR_CODE_ENUM(val) \
  case ErrorCode::val: return #val;
    X_ERROR_CODE_ENUM(EXPAND_X_ERROR_CODE_ENUM)
#undef EXPAND_X_ERROR_CODE_ENUM
  }
  return "Unknown";
}
