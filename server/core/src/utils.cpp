#include "core/utils.hpp"
#include "core/log.hpp"
#include <uvw.hpp>

using namespace core;

void core::next_tick(std::function<void()> func) {
  using namespace std::chrono_literals;
  auto timer = uvw::Loop::getDefault()->resource<uvw::TimerHandle>();
  timer->init();
  timer->on<uvw::TimerEvent>([func = std::move(func)](const uvw::TimerEvent&, uvw::TimerHandle& timer) {
    func();
    timer.close();
  });
  timer->start(0ms, 0ms);
}

int core::run_main_loop() {
  return uvw::Loop::getDefault()->run();
}

std::string core::replace_all(std::string str, std::string_view from, std::string_view to) {
  if (from.empty()) {
    return str;
  }
  size_t start_pos{ 0 };
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
  return str;
}
