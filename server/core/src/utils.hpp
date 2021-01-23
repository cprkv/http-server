#pragma once
#include <utility>
#include <memory>

namespace core {
  inline std::pair<std::unique_ptr<char[]>, size_t> cstr_to_writable_data(const char* str) {
    size_t message_size = strlen(str);
    auto   message_buff = std::make_unique<char[]>(message_size);
    memcpy(message_buff.get(), str, message_size);
    return std::make_pair(std::move(message_buff), message_size);
  }
} // namespace core
