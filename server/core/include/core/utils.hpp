#pragma once
#include <utility>
#include <memory>
#include <functional>

namespace core {
  using WritableData = std::pair<std::unique_ptr<char[]>, size_t>;

  WritableData cstr_to_writable_data(const char* str);
  void         next_tick(std::function<void()> func);
  int          run_main_loop();
} // namespace core
