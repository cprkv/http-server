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
