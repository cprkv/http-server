#pragma once
#include "http-server/pch.hpp"

namespace http {
  template <typename T>
  void pack(msgpack::packer<msgpack::sbuffer>& p, const T& value) {
    if constexpr (std::is_same_v<T, std::nullptr_t>) {
      p.pack_nil();
    } else if constexpr (std::is_same_v<T, int>) {
      p.pack_int(value);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
      p.pack_unsigned_int(value);
    } else if constexpr (std::is_same_v<T, float>) {
      p.pack_float(value);
    } else if constexpr (std::is_same_v<T, double>) {
      p.pack_double(value);
    } else if constexpr (std::is_same_v<T, bool>) {
      if (value) {
        p.pack_true();
      } else {
        p.pack_false();
      }
    } else {
      static_assert(false, "packing this not implemented");
    }
  }

  void pack(msgpack::packer<msgpack::sbuffer>& p, const char* value) {
    auto size = (uint32_t) strlen(value);
    p.pack_str(size);
    p.pack_str_body(value, size);
  }

  template <>
  void pack<std::string>(msgpack::packer<msgpack::sbuffer>& p, const std::string& value) {
    p.pack_str((uint32_t) value.size());
    p.pack_str_body(value.data(), (uint32_t) value.size());
  }

  template <>
  void pack<std::string_view>(msgpack::packer<msgpack::sbuffer>& p, const std::string_view& value) {
    p.pack_str((uint32_t) value.size());
    p.pack_str_body(value.data(), (uint32_t) value.size());
  }

  template <typename T>
  void pack(msgpack::packer<msgpack::sbuffer>& p, const std::vector<T>& values) {
    p.pack_array((uint32_t) values.size());
    for (const auto& value : values) {
      pack(p, value);
    }
  }

  template <typename K, typename V>
  void pack(msgpack::packer<msgpack::sbuffer>& p, const std::map<K, V>& values) {
    p.pack_map((uint32_t) values.size());
    for (const auto& [key, value] : values) {
      pack(p, key);
      pack(p, value);
    }
  }

  template <typename T>
  void pack_variadic(msgpack::packer<msgpack::sbuffer>& p, T&& arg) {
    pack(p, std::forward<T>(arg));
  }

  template <typename T, typename... TArgs>
  void pack_variadic(msgpack::packer<msgpack::sbuffer>& p, T&& arg, TArgs&&... args) {
    pack_variadic(p, std::forward<T>(arg));
    pack_variadic(p, std::forward<TArgs>(args)...);
  }

  template <typename T>
  void pack_variadic2(msgpack::packer<msgpack::sbuffer>& p, T&& arg1, T&& arg2) {
    pack(p, std::forward<T>(arg1));
    pack(p, std::forward<T>(arg2));
  }

  template <typename T, typename... TArgs>
  void pack_variadic2(msgpack::packer<msgpack::sbuffer>& p, T&& arg1, T&& arg2, TArgs&&... args) {
    pack_variadic(p, std::forward<T>(arg1));
    pack_variadic(p, std::forward<T>(arg2));
    pack_variadic(p, std::forward<TArgs>(args)...);
  }

  template <typename... TArgs>
  void pack_variadic_tuple(msgpack::packer<msgpack::sbuffer>& p, TArgs&&... args) {
    p.pack_array((uint32_t) sizeof...(TArgs));
    pack_variadic(p, std::forward<TArgs>(args)...);
  }

  template <typename... TArgs>
  void pack_variadic_map(msgpack::packer<msgpack::sbuffer>& p, TArgs&&... args) {
    p.pack_map(((uint32_t) sizeof...(TArgs)) / 2);
    pack_variadic(p, std::forward<TArgs>(args)...);
  }
} // namespace http
