#pragma once
#include "http-server/pch.hpp"

#define mEnumUnwrap(Name, value) Name = value,

#define mSimpleEnumUnwrap(Name) Name,

#define mEnumToStringCase(Name, ...) \
  case decltype(v)::Name:              \
    return #Name;

#define mDeclareEnum(EnumName, EnumXMacro)                         \
  enum EnumName { EnumXMacro(mEnumUnwrap) };                       \
                                                                   \
  inline static constexpr std::string_view to_string(EnumName v) { \
    switch (v) {                                                   \
      EnumXMacro(mEnumToStringCase);                               \
      default:                                                     \
        return "<unknown>";                                        \
    }                                                              \
  }

#define mDeclareSimpleEnum(EnumName, EnumXMacro)                   \
  enum EnumName { EnumXMacro(mSimpleEnumUnwrap) };                 \
                                                                   \
  inline static constexpr std::string_view to_string(EnumName v) { \
    switch (v) {                                                   \
      EnumXMacro(mEnumToStringCase);                               \
      default:                                                     \
        return "<unknown>";                                        \
    }                                                              \
  }

namespace http {
  void        next_tick(std::function<void()> func);
  int         run_main_loop();
  std::string replace_all(std::string str, std::string_view from, std::string_view to);

  std::function<void(const std::exception_ptr&)> unwrap_exception_ptr(
      std::function<void(const std::exception&)> on_ex);


  // TODO: reduce copypasta
  // TODO: other types
  namespace url {
    template <typename T>
    T unwrap_base(const std::string& str) {
      if constexpr (std::is_same_v<T, int>) {
        if (str.empty() || str.starts_with('0')) {
          throw std::exception();
        }
        return std::stoi(str);
      } else if constexpr (std::is_same_v<T, std::string>) {
        return str;
      }
      throw std::exception();
    }

    template <typename T1>
    bool unwrap(const std::smatch& url_match, T1& t1) {
      if (url_match.size() != 2) {
        return false;
      }

      try {
        t1 = unwrap_base<T1>(url_match[1]);
      } catch (...) {
        return false;
      }

      return true;
    }

    template <typename T1, typename T2>
    bool unwrap(const std::smatch& url_match, T1& t1, T2& t2) {
      if (url_match.size() != 3) {
        return false;
      }

      try {
        t1 = unwrap_base<T1>(url_match[1]);
        t2 = unwrap_base<T2>(url_match[2]);
      } catch (...) {
        return false;
      }

      return true;
    }

    template <typename T1, typename T2, typename T3>
    bool unwrap(const std::smatch& url_match, T1& t1, T2& t2, T3& t3) {
      if (url_match.size() != 4) {
        return false;
      }

      try {
        t1 = unwrap_base<T1>(url_match[1]);
        t2 = unwrap_base<T2>(url_match[2]);
        t3 = unwrap_base<T3>(url_match[3]);
      } catch (...) {
        return false;
      }

      return true;
    }

    template <typename T1, typename T2, typename T3, typename T4>
    bool unwrap(const std::smatch& url_match, T1& t1, T2& t2, T3& t3, T4& t4) {
      if (url_match.size() != 5) {
        return false;
      }

      try {
        t1 = unwrap_base<T1>(url_match[1]);
        t2 = unwrap_base<T2>(url_match[2]);
        t3 = unwrap_base<T3>(url_match[3]);
        t4 = unwrap_base<T4>(url_match[4]);
      } catch (...) {
        return false;
      }

      return true;
    }
  } // namespace url
} // namespace http
