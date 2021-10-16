#pragma once
#include "http-server/pch.hpp"

namespace core {
  //-----------------------------------------------------------------------

  class Ulid {
    unsigned char bytes_[16]{ 0 };

    friend class UlidGenerator;

  public:
    Ulid() = default;

    bool decode(const char* text) { return ulid_decode(bytes_, text) == 0; }
    bool decode(const std::string& text) { return decode(text.c_str()); }

    [[nodiscard]] std::string str() const {
      std::string result(26, '\0');
      ulid_encode(result.data(), bytes_);
      return result;
    }
  };

  //-----------------------------------------------------------------------

  class UlidGenerator {
    ulid_generator gen_;

  public:
    UlidGenerator() {
      ulid_generator_init(&gen_, ULID_RELAXED);
    }

    Ulid generate() {
      Ulid ulid;
      ulid_generate(&gen_, ulid.bytes_);
      return ulid;
    }

    static UlidGenerator& main();
  };

  //-----------------------------------------------------------------------
} // namespace core
