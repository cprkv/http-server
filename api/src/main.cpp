#include <uvw.hpp>

int main(int, char**) {
  auto loop = uvw::Loop::getDefault();
  return loop->run();
}
