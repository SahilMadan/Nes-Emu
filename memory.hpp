#pragma once

#include <cstdint>

namespace nes_emu {

class Memory {
 public:
  std::uint8_t ReadByte(std::uint16_t address);
};

}  // namespace nes_emu
