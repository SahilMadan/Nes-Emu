#include "cpu.hpp"

#include "memory.hpp"

namespace {

// Processor Status Getters:
constexpr std::uint8_t GetCarryFlag(std::uint8_t processor_status) {
  return (processor_status & 0b0000'0001);
}
constexpr std::uint8_t GetZeroFlag(std::uint8_t processor_status) {
  return (processor_status & 0b0000'0010);
}
constexpr std::uint8_t GetOverflowFlag(std::uint8_t processor_status) {
  return (processor_status & 0b0100'0000);
}
constexpr std::uint8_t GetNegativeFlag(std::uint8_t processor_status) {
  return (processor_status & 0b1000'0000);
}

// Processor Status Setters:
constexpr std::uint8_t UpdateProcessorStatusCarryFlag(
    std::uint8_t processor_status, std::uint8_t value,
    std::uint8_t previous_value) {
  if (value < previous_value) {
    return (processor_status | 0b0000'0001);
  }
  return processor_status & 0b1111'1110;
}

constexpr std::uint8_t UpdateProcessorStatusZeroFlag(
  std::uint8_t processor_status, std::uint8_t value) {
  if (value == 0) {
    return processor_status | 0b0000'0010;
  }
  return processor_status & 0b1111'1101;
}

constexpr std::uint8_t UpdateProcessorStatusOverflowFlag(
  std::uint8_t processor_status, std::uint8_t value,
  std::uint8_t previous_value) {
  if ((previous_value & 0b1000'0000) ^ (value & 0b1000'0000)) {
    return processor_status | 0b0100'0000;
  }
  return processor_status & 0b1011'1111;
}

constexpr std::uint8_t UpdateProcessorStatusNegativeFlag(
  std::uint8_t processor_status, std::uint8_t value) {
  if (value & 0b1000'0000) {
    return processor_status | 0b1000'0000;
  }
  return processor_status & 0b0111'1111;
}

}  // namespace

namespace nes_emu {

void Cpu::RunSingleIteration() {}

std::uint16_t Cpu::GetInstructionData(AddressMode address_mode) {
  switch (address_mode) {
    case AddressMode::ABSOLUTE: {
      const std::uint16_t address =
          (static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 2))
           << 8) +
          static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 1));
      return memory_->ReadByte(address);
    }
    case AddressMode::ABSOLUTE_X_INDEXED: {
      const std::uint16_t address =
          (static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 2))
           << 8) +
          static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 1)) +
          static_cast<uint16_t>(index_x_);
      return memory_->ReadByte(address);
    }
    case AddressMode::ABSOLUTE_Y_INDEXED: {
      const std::uint16_t address =
          (static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 2))
           << 8) +
          static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 1)) +
          static_cast<uint16_t>(index_x_);
      return memory_->ReadByte(address);
    }
    case AddressMode::IMMEDIATE:
      return memory_->ReadByte(program_counter_ + 1);
    case AddressMode::ZERO_PAGE:
      return memory_->ReadByte(memory_->ReadByte(program_counter_ + 1));
    case AddressMode::ZERO_PAGE_X_INDEXED:
      return memory_->ReadByte(memory_->ReadByte(program_counter_ + 1) +
                               index_x_);
    case AddressMode::INDIRECT: {
      const std::uint16_t indirect_address =
          (static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 2))
           << 8) +
          static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 1));

      const std::uint16_t address =
          (static_cast<std::uint16_t>(memory_->ReadByte(indirect_address + 1))
           << 8) +
          static_cast<std::uint16_t>(memory_->ReadByte(indirect_address));

      return memory_->ReadByte(address);
    }
    case AddressMode::X_INDEXED_INDIRECT: {
      const std::uint16_t indirect_address =
          (static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 2))
           << 8) +
          static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 1));

      const std::uint16_t address =
          (static_cast<std::uint16_t>(
               memory_->ReadByte(indirect_address + 1 + index_x_))
           << 8) +
          static_cast<std::uint16_t>(
              memory_->ReadByte(indirect_address + index_x_));

      return memory_->ReadByte(address);
    }
    case AddressMode::INDIRECT_INDEXED_Y: {
      const std::uint16_t indirect_address =
          (static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 2))
           << 8) +
          static_cast<std::uint16_t>(memory_->ReadByte(program_counter_ + 1));

      const std::uint16_t address =
          (static_cast<std::uint16_t>(memory_->ReadByte(indirect_address + 1))
           << 8) +
          static_cast<std::uint16_t>(memory_->ReadByte(indirect_address));

      return memory_->ReadByte(address + index_y_);
    }
    default:
      // TODO: Handle this error condition.
  }
}

void Cpu::Adc(AddressMode address_mode) {
  auto previous_accumulator = accumulator_;
  accumulator_ = previous_accumulator + GetInstructionData(address_mode) +
                 GetCarryFlag(processor_status_);

  processor_status_ = UpdateProcessorStatusCarryFlag(
      processor_status_, accumulator_, previous_accumulator);
}

}  // namespace nes_emu
