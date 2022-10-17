#pragma once

#include <cstdint>

#include "memory.hpp"

namespace nes_emu {

class Cpu {
 private:
  enum class AddressMode {
    // Address of data specified by the two operands supplied, least significant
    // byte first.
    ABSOLUTE,
    // Address of data specified by the two operands supplied, least significant
    // byte first and adds the value of index register X.
    ABSOLUTE_X_INDEXED,
    // Address of data specified by the two operands supplied, least significant
    // byte first and adds the value of index register Y.
    ABSOLUTE_Y_INDEXED,
    // Operate directly on the accumulator register.
    ACCUMULATOR,
    // Operate directly on a constant supplied as an operand.
    IMMEDIATE,
    // Single operand which serves as a pointer to an address in zero page
    // ($0000-$00FF).
    // Single-byte instruction.
    ZERO_PAGE,
    // Single operand and adds the value of index register X to it to give an
    // address in zero page ($0000-$00FF).
    ZERO_PAGE_X_INDEXED,
    // Single operand and adds the value of index register Y to it to give an
    // address in zero page ($0000-$00FF).
    ZERO_PAGE_Y_INDEXED,

    // Two operands form 16-byte address LL. Value at address LL and LL + 1 form
    // 16-byte address HH. Actual data value located at address HH.
    INDIRECT,
    // PRE-INDEXED. Single byte operand forms address 0L. Value at address (0L +
    // X) and (0L + X + 1) form 16-byte address HH. Actual value located at
    // address HH.
    X_INDEXED_INDIRECT,
    // POST-INDEXED. Single byte operand forms address 0L. Value at address 0L
    // and 0L + 1 forms 16-byte address HH. Actual value is located at addrress
    // (HH + Y).
    INDIRECT_INDEXED_Y,
    // Relative addressing -- single byte address contained in data.
    RELATIVE
  };

  enum class ProcessorFlag {
    // Set if the last instruction resulted in an overflow from bit 7 or an
    // underflow from bit 0.
    CARRY,
    // Set if the result of the last instruction as 0.
    ZERO,
    // Set if the result of the last instruction is negative (i.e. bit 7 of the
    // result is 1).
    NEGATIVE,
    // Set if an invalid two's complement result was obtained by the previous
    // instruction:
    // -- A negative result obtained when a positive result was expected; or,
    // -- A positive result obtained when a negative result expected.
    // For example, 64 + 64 = 128 (0x80 or 0b1000'0000) which is instead
    // interpreted as -128.
    OVERFLOW
  };

  void RunSingleIteration();
  std::uint16_t GetMemoryAddress(AddressMode address_mode);
  std::uint8_t GetInstructionData(AddressMode address_mode);

  Memory* memory_;

  // Registers:

  std::uint16_t program_counter_;
  std::uint8_t stack_pointer_;
  std::uint8_t accumulator_;
  std::uint8_t index_x_;
  std::uint8_t index_y_;
  std::uint8_t processor_status_;

  // Instruction Set:

  void Adc(AddressMode address_mode);
  void And(AddressMode address_mode);
  void Asl(AddressMode address_mode);
  void Bcc(AddressMode address_mode);
  void Bcs(AddressMode address_mode);
  void Beq(AddressMode address_mode);
  void Bit(AddressMode address_mode);
  void Bmi(AddressMode address_mode);
  void Bne(AddressMode address_mode);
  void Bpl(AddressMode address_mode);
  void Brk();
  void Bvc(AddressMode address_mode);
  void Bvs(AddressMode address_mode);
  void Clc();
  void Cld();
  void Cli();
  void Clv();
  void Cmp(AddressMode address_mode);
  void Cpx(AddressMode address_mode);
  void Cpy(AddressMode address_mode);
  void Dec(AddressMode address_mode);
  void Dex();
  void Dey();
  void Eor(AddressMode address_mode);
  void Inc(AddressMode address_mode);
  void Inx();
  void Iny();
};

}  // namespace nes_emu
