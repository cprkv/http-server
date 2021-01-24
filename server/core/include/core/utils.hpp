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

  inline void next_tick(std::function<void()>&& func) {
    using namespace std::chrono_literals;
    auto timer = uvw::Loop::getDefault()->resource<uvw::TimerHandle>();
    timer->init();
    timer->on<uvw::TimerEvent>([func = std::move(func)](const uvw::TimerEvent&, uvw::TimerHandle& timer) {
      func();
      timer.stop();
    });
    timer->start(0ms, 0ms);
  }
} // namespace core
