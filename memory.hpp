#pragma once

#include <cstdint>

namespace nes_emu {

class Memory {
 public:
  std::uint8_t ReadByte(std::uint16_t address);
  void Write(std::uint16_t address, std::uint8_t data);
};

}  // namespace nes_emu
