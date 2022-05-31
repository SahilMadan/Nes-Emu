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
    std::uint8_t processor_status, std::uint8_t value) {
  if (processor_status & 0b0100'0000) {
    return processor_status | 0b0100'0000;
  }
  return processor_status & 0b1011'1111;
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

// `bit` should either be 0 or 1. All other values will corrupt the processor
// status.
constexpr std::uint8_t UpdateProcessorStatusInterruptFlag(
    std::uint8_t processor_status, std::uint8_t bit) {
  const std::uint8_t mask = (bit << 2) & 0b1111'1111;
  return processor_status & mask;
}

}  // namespace

namespace nes_emu {

void Cpu::RunSingleIteration() {}

std::uint8_t Cpu::GetInstructionData(AddressMode address_mode) {
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
    case AddressMode::ACCUMULATOR:
      return accumulator_;
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
    case AddressMode::RELATIVE:
      return memory_->ReadByte(program_counter_ + 1);
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
  processor_status_ =
      UpdateProcessorStatusZeroFlag(processor_status_, accumulator_);
  processor_status_ = UpdateProcessorStatusOverflowFlag(
      processor_status_, accumulator_, previous_accumulator);
  processor_status_ =
      UpdateProcessorStatusZeroFlag(processor_status_, accumulator_);
}

void Cpu::And(AddressMode address_mode) {
  auto previous_accumulator = accumulator_;
  accumulator_ &= GetInstructionData(address_mode);

  processor_status_ =
      UpdateProcessorStatusZeroFlag(processor_status_, accumulator_);
  processor_status_ =
      UpdateProcessorStatusNegativeFlag(processor_status_, accumulator_);
}

void Cpu::Asl(AddressMode address_mode) {
  auto previous_accumulator_ = accumulator_;
  accumulator_ = GetInstructionData(address_mode) << 1;

  processor_status_ = UpdateProcessorStatusCarryFlag(
      processor_status_, accumulator_, previous_accumulator_);
  processor_status_ =
      UpdateProcessorStatusNegativeFlag(processor_status_, accumulator_);
  processor_status_ =
      UpdateProcessorStatusZeroFlag(processor_status_, accumulator_);
}

void Cpu::Bcc(AddressMode address_mode) {
  if (!GetCarryFlag(processor_status_)) {
    program_counter_ += GetInstructionData(address_mode);
  }
}

void Cpu::Bcs(AddressMode address_mode) {
  if (GetCarryFlag(processor_status_)) {
    program_counter_ += GetInstructionData(address_mode);
  }
}

void Cpu::Beq(AddressMode address_mode) {
  if (GetZeroFlag(processor_status_)) {
    program_counter_ += GetInstructionData(address_mode);
  }
}

void Cpu::Bit(AddressMode address_mode) {
  const auto value = GetInstructionData(address_mode);
  processor_status_ =
      UpdateProcessorStatusNegativeFlag(processor_status_, value);
  processor_status_ =
      UpdateProcessorStatusOverflowFlag(processor_status_, value);
  processor_status_ =
      UpdateProcessorStatusZeroFlag(processor_status_, accumulator_ & value);
}

void Cpu::Bmi(AddressMode address_mode) {
  if (GetNegativeFlag(processor_status_)) {
    program_counter_ += GetInstructionData(address_mode);
  }
}

void Cpu::Bne(AddressMode address_mode) {
  if (!GetZeroFlag(processor_status_)) {
    program_counter_ += GetInstructionData(address_mode);
  }
}

void Cpu::Bpl(AddressMode address_mode) {
  if (!GetNegativeFlag(processor_status_)) {
    program_counter_ += GetInstructionData(address_mode);
  }
}

void Cpu::Brk() {
  const std::uint16_t data = program_counter_ + 2;
  memory_->Write(0x0100 + stack_pointer_, static_cast<std::uint8_t>(data));
  memory_->Write(0x0100 + stack_pointer_ + 1,
                 static_cast<std::uint8_t>(data >> 8));
  processor_status_ = UpdateProcessorStatusInterruptFlag(processor_status_, 1);
}

void Cpu::Bvc(AddressMode address_mode) {
  if (!GetOverflowFlag(processor_status_)) {
    program_counter_ += GetInstructionData(address_mode);
  }
}

void Cpu::Bvs(AddressMode address_mode) {
  if (GetOverflowFlag(processor_status_)) {
    program_counter_ += GetInstructionData(address_mode);
  }
}

void Cpu::Clc() { processor_status_ &= 0b1111'1110; }

void Cpu::Cld() { processor_status_ &= 0b1111'0111; }

void Cpu::Cli() { processor_status_ &= 0b1111'1011; }

void Cpu::Clv() { processor_status_ &= 0b1011'1111; }

void Cpu::Cmp(AddressMode address_mode) {
  const auto value = accumulator_ - GetInstructionData(address_mode);
  // TODO: Needs more research on this carry flag value. For instance, see:
  // http://6502.org/tutorials/compare_instructions.html. Here, it states that
  // Carry value is set if (A > M). However, the instruction states that it
  // should be A - M (not M - A); see:
  // https://www.masswerk.at/6502/6502_instruction_set.html#CMP.
  processor_status_ =
      UpdateProcessorStatusCarryFlag(processor_status_, value, accumulator_);
  processor_status_ = UpdateProcessorStatusZeroFlag(processor_status_, value);
  processor_status_ =
      UpdateProcessorStatusNegativeFlag(processor_status_, value);
}

void Cpu::Cmx(AddressMode address_mode) {
  const auto value = index_x_ - GetInstructionData(address_mode);
  // TODO: See comment in CMX. 
  processor_status_ =
      UpdateProcessorStatusCarryFlag(processor_status_, value, accumulator_);
  processor_status_ = UpdateProcessorStatusZeroFlag(processor_status_, value);
  processor_status_ =
      UpdateProcessorStatusNegativeFlag(processor_status_, value);
}

void Cpu::Cmy(AddressMode address_mode) {
  const auto value = index_y_ - GetInstructionData(address_mode);
  // TODO: See comment in CMX. 
  processor_status_ =
      UpdateProcessorStatusCarryFlag(processor_status_, value, accumulator_);
  processor_status_ = UpdateProcessorStatusZeroFlag(processor_status_, value);
  processor_status_ =
      UpdateProcessorStatusNegativeFlag(processor_status_, value);
}

}  // namespace nes_emu
