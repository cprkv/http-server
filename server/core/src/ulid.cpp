#include "core/ulid.hpp"

using namespace core;

//-----------------------------------------------------------------------

UlidGenerator& UlidGenerator::main() {
  static UlidGenerator generator;
  return generator;
}

//-----------------------------------------------------------------------
