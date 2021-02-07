#include "core/error.hpp"

using namespace core;

std::string core::to_string(ErrorCode code) {
  switch (code) {
    case ErrorCode::None: return "None";
    case ErrorCode::Exception: return "Exception";
    case ErrorCode::NoConnectionsInPool: return "NoConnectionsInPool";
  }
  return "Unknown";
}
