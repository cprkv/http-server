#pragma once
#include <exception>
#include <string>
#include <utility>

namespace core {
  //---------------------------------------------------------------

  enum class ErrorCode {
    None,
    Exception,
    NoConnectionsInPool,
  };

  std::string to_string(ErrorCode code);

  //---------------------------------------------------------------

  class Error : std::exception {
    std::string what_{};
    bool        is_error_{ false };
    ErrorCode   code_{ ErrorCode::None };

  public:
    Error() = default;

    explicit Error(const std::exception& ex)
        : what_{ ex.what() }
        , is_error_{ true }
        , code_{ ErrorCode::Exception } {}

    explicit Error(ErrorCode code)
        : what_{ to_string(code) }
        , is_error_{ code != ErrorCode::None }
        , code_{ code } {}

    ~Error() override = default;

    [[nodiscard]] const char* what() const noexcept override { return what_.c_str(); }
    [[nodiscard]] ErrorCode   code() const noexcept { return code_; }

    operator bool() const { return is_error_; }

    //---------------------------------------------------------------
  };
} // namespace core