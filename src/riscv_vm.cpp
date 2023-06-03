#include "common.hpp"
#include "bitmask_utility.hpp"

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <bit>
#include <memory>
#include <vector>
#include <ranges>
#include <cstring>

using std::bit_cast;

namespace stdr = std::ranges;

struct RV32I_TypeR
{
	unsigned opcode : 7;
	unsigned rd : 5;
	unsigned funct3 : 3;
	unsigned rs1 : 5;
	unsigned rs2 : 5;
	unsigned funct7 : 7;
};

struct RV32I_TypeI
{
	unsigned opcode : 7;
	unsigned rd : 5;
	unsigned funct3 : 3;
	unsigned rs1 : 5;
	signed imm12 : 12;
};

struct RV32I_TypeS
{
	unsigned opcode : 7;
	unsigned imm5 : 5;
	unsigned funct3 : 3;
	unsigned rs1 : 5;
	unsigned rs2 : 5;
	unsigned imm7 : 7;
};

struct RV32I_TypeB
{
	unsigned opcode : 7;
	unsigned imm1_1 : 1;
	unsigned imm4 : 4;
	unsigned funct3 : 3;
	unsigned rs1 : 5;
	unsigned rs2 : 5;
	unsigned imm6 : 6;
	unsigned imm1_2 : 1;

	signed offset()
	{
		SRISCV_CX_STATIC constexpr uint32_t mask1 = 0b00000000000000000000000010000000; // << 4
		//                       0b00000000000000000000100000000000;
		SRISCV_CX_STATIC constexpr uint32_t mask2 = 0b00000000000000000000111100000000; // >> 7
		//                       0b00000000000000000000000000011110;
		SRISCV_CX_STATIC constexpr uint32_t mask3 = 0b01111110000000000000000000000000; // >> 20
		//                       0b00000000000000000000011111100000;
		SRISCV_CX_STATIC uint32_t mask4 = 0b10000000000000000000000000000000; // >> 19
		//                       0b00000000000000000001000000000000;
		unsigned fin = 0;
		auto value = [this]{return bit_cast<u32>(*this);};
		fin |= (value() & mask1) << 4;
		fin |= (value() & mask2) >> 7;
		fin |= (value() & mask3) >> 20;
		fin |= (signed)(value() & mask4) >> 19;
		return (signed)fin / 4 - 1;
	}
};

struct RV32I_TypeU
{
	static constexpr uint32_t MaskImm = 0b1111111111111111111100000000000;

	unsigned opcode : 7;
	unsigned rd: 5;
	signed imm20 : 20;
};


struct RV32I_TypeJ
{
	unsigned opcode : 7;
	unsigned rd : 5;
	unsigned imm8 : 8;
	unsigned imm1_1 : 1;
	unsigned imm10 : 10;
	unsigned imm1_2 : 1;

	signed offset()
	{
	  	static constexpr uint32_t MaskUpperInteger = 0b00000000000011111111000000000000;
	  	static constexpr uint32_t MaskMiddleBit    = 0b00000000000100000000000000000000;
	  	static constexpr uint32_t MaskLowerInteger = 0b01111111111000000000000000000000;
	  	static constexpr uint32_t MaskSignBit      = 0b10000000000000000000000000000000;
		unsigned off = 0;
		auto value = [this]{return bit_cast<u32>(*this);};
		off |= (value() & MaskUpperInteger) << 11;
		off |= (value() & MaskMiddleBit) << 2;
		off |= (value() & MaskLowerInteger) >> 9;
		off |= (value() & MaskSignBit);
		return (signed)off >> 12;
	};
};

static constexpr size_t instruction_alignment = 4;

struct alignas(instruction_alignment) RISCVInstruction
{
	u32 _v;
	// unsigned family : 2;
	// unsigned opcode : 5;
	// unsigned value : 25;

	constexpr u32 family() const noexcept {
		return extract_bits<0, 1>(_v);
	}

	constexpr u32 opcode() const noexcept {
		return extract_bits<2, 6>(_v);
	}

	constexpr u32 value() const noexcept {
		return extract_bits<7, 31>(_v);
	}

    constexpr operator u32() const noexcept {
        return _v;
    }
};

static_assert(RISCVInstruction{0x00178793}.family() == 0x3);
static_assert(RISCVInstruction{0x00178793}.opcode() == 0x04);

static_assert(sizeof(RV32I_TypeR) == 4, "sizeof(RV32I_TypeR) != 4 (must add up to 32 bits)");
static_assert(sizeof(RV32I_TypeI) == 4, "sizeof(RV32I_TypeI) != 4 (must add up to 32 bits)");
static_assert(sizeof(RV32I_TypeS) == 4, "sizeof(RV32I_TypeS) != 4 (must add up to 32 bits)");
static_assert(sizeof(RV32I_TypeB) == 4, "sizeof(RV32I_TypeB) != 4 (must add up to 32 bits)");
static_assert(sizeof(RV32I_TypeU) == 4, "sizeof(RV32I_TypeU) != 4 (must add up to 32 bits)");
static_assert(sizeof(RV32I_TypeJ) == 4, "sizeof(RV32I_TypeJ) != 4 (must add up to 32 bits)");
static_assert(sizeof(RISCVInstruction) == 4, "sizeof(RV32Instr) != 4 (must add up to 32 bits)");

#define RV64I_UnimplementedExit fputs("RV64I Only: Unimplemented", stderr); fflush(stderr); exit(1)
#define RV32I_UnimplementedExit fputs("Currently Unimplemented / Unreachable", stderr); fflush(stderr); exit(1)
#define RV32I_IllegalExit fputs("Illegal instruction", stderr); fflush(stderr); exit(1)

__attribute((noreturn)) void RVCore_CriticalError(const char* message)
{
	fputs(message, stderr);
	fflush(stderr);
	exit(1);
}

#define DefaultRISCVStackSize 0x20000
#define DefaultRISCVInstructionBlockSize 0x20000

#define RVE_InstructionOOB 0x1

struct RISCVContainer
{
	// x0 -> zero (Hardwired to zero)
	// x1 -> ra (Return address)
	// x2 -> sp (Stack pointer)
	// x3 -> gp (Global pointer)
	// x4 -> tp (Thread pointer)
	// x5 -> t0 (Temporary 1)
	// x6 -> t1 (Temporary 2)
	// x7 -> t2 (Temporary 3)
	// x8 -> s0/fp (Saved register 1/frame pointer)
	// x9 -> s1 (Saved register 2)
	// x10 -> a0 (Function argument 1/Return value)
	// x11 -> a1 (Function argument 2/Return value)
	// x12 -> a2 (Function argument 3)
	// x13 -> a3 (Function argument 4)
	// x14 -> a4 (Function argument 5)
	// x15 -> a5 (Function argument 6)
	// x16 -> a6 (Function argument 7)
	// x17 -> a7 (Function argument 8)
	// x18 -> s2 (Saved register 2) 
	// x19 -> s3 (Saved register 3) 
	// x20 -> s4 (Saved register 4) 
	// x21 -> s5 (Saved register 5) 
	// x22 -> s6 (Saved register 6) 
	// x23 -> s7 (Saved register 7) 
	// x24 -> s8 (Saved register 8) 
	// x25 -> s9 (Saved register 9) 
	// x26 -> s10 (Saved register 10)
	// x27 -> s11 (Saved register 11)
	// x28 -> t3 (Temporary 3)
	// x29 -> t4 (Temporary 4)
	// x30 -> t5 (Temporary 5)
	// x31 -> t6 (Temporary 6)
	uint32_t xregs[32] = {};

	static constexpr u32 stack_region_size = DefaultRISCVStackSize;

	class InstructionBlock
	{
		std::unique_ptr<RISCVInstruction[]> m_data;
		size_t m_size;

		constexpr InstructionBlock(size_t size)
		  : m_data{new(std::nothrow) RISCVInstruction[size]}
		  , m_size{size}
		{
			if (!m_data)
				RVCore_CriticalError("Failed to allocate instruction block");
			memset(m_data.get(), '\000', size);
		}
	public:
		// accepts any range, even ones with non contiguous memory (a linked list for example (dont do that though))
		template<stdr::range R>
		constexpr InstructionBlock(R&& instructions)
		  : InstructionBlock{stdr::size(instructions)}
		{
			stdr::copy(instructions, m_data.get());
		}

		// uses memcpy when the passed in data is contiguous,
		// (the copy algorithm should also do this, but this is the classic way)
		template<stdr::contiguous_range R>
		constexpr InstructionBlock(R&& instructions)
		  : InstructionBlock{stdr::size(instructions)}
		{
			std::memcpy(m_data.get(), stdr::data(instructions), stdr::size(instructions) * sizeof(stdr::range_value_t<R>));
		}

		constexpr RISCVInstruction const* data() const noexcept {
			return m_data.get();
		}
		constexpr size_t size() const noexcept {
			return m_size;
		}
	};

	InstructionBlock instruction_block;
	// uint8_t* stack_region;
	RISCVInstruction const* pc;

	bool AddressWithinBounds(const void* address)
	{
		return address >= instruction_block.data() &&
			address < instruction_block.data() + instruction_block.size();
	}

	RISCVContainer() = delete;
	RISCVContainer(const uint32_t* instructions, size_t array_size)
	  : instruction_block(std::views::counted(instructions, array_size / 4))
	//   , stack_region{nullptr}
	  , pc{instruction_block.data()} {}

	template<stdr::range R>
	RISCVContainer(R&& instruction_range)
	  : instruction_block(std::forward<R>(instruction_range))
	//   , stack_region{nullptr}
	  , pc{instruction_block.data()} {}

	int PerformCycle()
	{
		if (!AddressWithinBounds(pc))
			return RVE_InstructionOOB;
		xregs[0] = 0;
		RISCVInstruction i = {*pc++};

		auto as_u = [](u32 v){return bit_cast<RV32I_TypeU>(v);};
        auto as_s = [](u32 v){return bit_cast<RV32I_TypeS>(v);};
		auto as_i = [](u32 v){return bit_cast<RV32I_TypeI>(v);};
		auto as_r = [](u32 v){return bit_cast<RV32I_TypeR>(v);};
		auto as_b = [](u32 v){return bit_cast<RV32I_TypeB>(v);};
		auto as_j = [](u32 v){return bit_cast<RV32I_TypeJ>(v);};

		if (i.family() == 0x3 && i.opcode() == 0x00)
		{
			if (as_i(i).funct3 == 0) 
				;

		}
		if (i.family() == 0x3 && i.opcode() == 0x04)
		{
			if (as_i(i).rd == 0)
				return 0;
			if (as_i(i).funct3 == 0) // addi (add signed immediate)
				xregs[as_i(i).rd] = xregs[as_i(i).rs1] + as_i(i).imm12;
			else if (as_i(i).funct3 == 1 && as_s(i).imm7 == 0) // slli
				xregs[as_r(i).rd] = xregs[as_r(i).rs1] << as_r(i).rs2;
			else if (as_i(i).funct3 == 2) // slti
				xregs[as_i(i).rd] = (signed)xregs[as_i(i).rs1] < (signed)as_i(i).imm12;
			else if (as_i(i).funct3 == 3) // sltiu
				xregs[as_i(i).rd] = (unsigned)xregs[as_i(i).rs1] < (unsigned)as_i(i).imm12;
			else if (as_i(i).funct3 == 4) // xori
				xregs[as_i(i).rd] = xregs[as_i(i).rs1] ^ as_i(i).imm12;
			else if (as_i(i).funct3 == 5 && as_s(i).imm7 == 0) // srli
				xregs[as_r(i).rd] = (unsigned)xregs[as_r(i).rs1] >> as_r(i).rs2;
			else if (as_i(i).funct3 == 5 && as_s(i).imm7 == 16) // srai
				xregs[as_r(i).rd] = (signed)xregs[as_r(i).rs1] >> as_r(i).rs2;
			else if (as_i(i).funct3 == 6) // ori
				xregs[as_i(i).rd] = xregs[as_i(i).rs1] | (unsigned)as_i(i).imm12;
			else if (as_i(i).funct3 == 7) // andi
				xregs[as_i(i).rd] = xregs[as_i(i).rs1] & (unsigned)as_i(i).imm12;
			else
				{ RV32I_UnimplementedExit; }
			return 0;
			// illegal, exhausted funct3
		}
		if (i.family() == 0x3 && i.opcode() == 0x05) // auipc
		{
			xregs[as_u(i).rd] = (uint32_t)(pc - instruction_block.data()) + (u32{i} & as_u(i).MaskImm);
			return 0;
		}
		if (i.family() == 0x3 && i.opcode() == 0x0C)
		{
			if (as_i(i).rd == 0)
				return 0;
			if (as_s(i).funct3 == 0 && as_s(i).imm7 == 0) // add
				xregs[as_r(i).rd] = xregs[as_s(i).rs1] + xregs[as_s(i).rs2];
			else if (as_s(i).funct3 == 0 && as_s(i).imm7 == 32) // sub
				xregs[as_r(i).rd] = xregs[as_s(i).rs1] - xregs[as_s(i).rs2];
			else if (as_s(i).funct3 == 1 && as_s(i).imm7 == 0) // sll (shift left logical)
				xregs[as_r(i).rd] = xregs[as_s(i).rs1] << xregs[as_s(i).rs2];
			else if (as_s(i).funct3 == 2 && as_s(i).imm7 == 0) // slt
				xregs[as_r(i).rd] = (signed)xregs[as_s(i).rs1] < (signed)xregs[as_s(i).rs2];
			else if (as_s(i).funct3 == 3 && as_s(i).imm7 == 0) // sltu
				xregs[as_r(i).rd] = (unsigned)xregs[as_s(i).rs1] < (unsigned)xregs[as_s(i).rs2];
			else if (as_s(i).funct3 == 4 && as_s(i).imm7 == 0) // xor
				xregs[as_r(i).rd] = (unsigned)xregs[as_s(i).rs1] ^ (unsigned)xregs[as_s(i).rs2];
			else if (as_s(i).funct3 == 5 && as_s(i).imm7 == 0) // srl (shift right logical)
				xregs[as_r(i).rd] = (unsigned)xregs[as_s(i).rs1] >> (xregs[as_s(i).rs2] & 0b11111);
			else if (as_s(i).funct3 == 5 && as_s(i).imm7 == 32) // sra (shift right arithmetic)
				xregs[as_r(i).rd] = (signed)xregs[as_s(i).rs1] >> (xregs[as_s(i).rs2] & 0b11111);
			else if (as_s(i).funct3 == 6 && as_s(i).imm7 == 0) // or
				xregs[as_r(i).rd] = xregs[as_s(i).rs1] | xregs[as_s(i).rs2];
			else if (as_s(i).funct3 == 7 && as_s(i).imm7 == 0) // and
				xregs[as_r(i).rd] = xregs[as_s(i).rs1] & xregs[as_s(i).rs2];
			else
				{ RV32I_UnimplementedExit; }
			return 0;
			// illegal, exhausted funct3
		}
		if (i.family() == 0x3 && i.opcode() == 0x0D) // lui
		{
			xregs[as_u(i).rd] = u32{i} & as_u(i).MaskImm;
			return 0;
		}
		if (i.family() == 0x3 && i.opcode() == 0x1B) // jal
		{
			printf("%s %i\t; pc is now %i\n", "jal", as_j(i).offset(), instruction_block.data() - pc);
			pc += as_j(i).offset();
			return 0;
		}
		if (i.family() == 0x3 && i.opcode() == 0x18)
		{
			printf("branch x%i, x%i, %i\n", as_b(i).rs1, as_b(i).rs2, as_b(i).offset());
			if (as_b(i).funct3 == 2 || as_b(i).funct3 == 3) [[unlikely]]
				{ RV32I_UnimplementedExit; }

			if ((as_b(i).funct3 == 0 && xregs[as_b(i).rs1] == xregs[as_b(i).rs2])					||	// beq
				(as_b(i).funct3 == 1 && xregs[as_b(i).rs1] == xregs[as_b(i).rs2])					||	// bne
				(as_b(i).funct3 == 4 && (signed)xregs[as_b(i).rs1] <= (signed)xregs[as_b(i).rs2])	||	// blt
				(as_b(i).funct3 == 5 && (signed)xregs[as_b(i).rs1] >= (signed)xregs[as_b(i).rs2])	||	// bge
				(as_b(i).funct3 == 6 && xregs[as_b(i).rs1] <= xregs[as_b(i).rs2])					||	// bltu
				(as_b(i).funct3 == 7 && xregs[as_b(i).rs1] >= xregs[as_b(i).rs2])					)	// bgeu
				pc += as_b(i).offset();

			return 0;
		}


		if (i.family() == 0x3 && i.opcode() == 0x19)
		{
			printf("jalr x%i x%i %i", as_i(i).rd, as_i(i).rs1, as_i(i).imm12);

		}
		printf("@%016llX Unknown instr: f=%i, op=%i, imm7=%i\n", pc, i.family(), i.opcode(), as_s(i).imm7);
		return 0;
	}

	void Run()
	{
		int cont = 1;
		while(cont)
		{
			int result = PerformCycle();
			if (result != 0)
			{
				printf("Exit: %i\n", result);
				cont = 0;
			}
		}
	}
};

const uint32_t rv32_bin[] = {
	// 0x00050e63,	//		beq     a0,zero,.L1			// void square(int num) {
	// 0x00100793,	//		li      a5,1				//     if (num)
	// 0x40f02223,	//		sw      a5,1028(zero)		//     {
	// 0x00200793,	//		li      a5,2				//         *(int*)1028=1;
	// 0x40f02423,	//		sw      a5,1032(zero)		//         *(int*)1032=2;
	// 0x00300793,	//		li      a5,3				//         *(int*)1036=3;
	// 0x40f02623,	//		sw      a5,1036(zero)		//     }
	// 			//	.L1:							//     return;
	// 0x00008067	//		ret							// }

	0x00050a63,	//		beq		a0,zero,.L1	
	0x00000793,	//		li		a5,0			// void square(int num) {
	0x00f7a023,	//	.L3:sw		a5,0(a5)		//     for (unsigned i = 0; i < num; ++i)
	0x00178793,	//		addi	a5,a5,1			//         *(int*)i = i;
	0xfea79ce3,	//		bne		a5,a0,.L3		//     return;
	0x00008067,	//	.L1:ret						// }
};

int main(int argc, char* argv[])
{
	RISCVContainer runner(rv32_bin, sizeof(rv32_bin));
	runner.Run();
	return 0;
}