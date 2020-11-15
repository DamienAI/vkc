#pragma once

#include <cstdint>

namespace Vk {
  namespace utils {
    inline uint32_t divUp(uint32_t x, uint32_t y) { return (x + y - 1) / y; }
  }
}
