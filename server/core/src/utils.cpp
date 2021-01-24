#include "core/utils.hpp"
#include <cstring>
#include <uvw.hpp>

using namespace core;

std::pair<std::unique_ptr<char[]>, size_t> core::cstr_to_writable_data(const char* str) {
  size_t message_size = strlen(str);
  auto   message_buff = std::make_unique<char[]>(message_size);
  memcpy(message_buff.get(), str, message_size);
  return std::make_pair(std::move(message_buff), message_size);
}

void core::next_tick(std::function<void()> func) {
  using namespace std::chrono_literals;
  auto timer = uvw::Loop::getDefault()->resource<uvw::TimerHandle>();
  timer->init();
  timer->on<uvw::TimerEvent>([func = std::move(func)](const uvw::TimerEvent&, uvw::TimerHandle& timer) {
    timer.clear();
    func();
    timer.stop();
  });
  timer->start(0ms, 0ms);
}

int core::run_main_loop() {
  return uvw::Loop::getDefault()->run();
}
