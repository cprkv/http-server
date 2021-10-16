#include "http-server/ulid.hpp"

using namespace http;

//-----------------------------------------------------------------------

UlidGenerator& UlidGenerator::main() {
  static UlidGenerator generator;
  return generator;
}

//-----------------------------------------------------------------------
