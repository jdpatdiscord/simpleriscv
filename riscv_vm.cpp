#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

union RV32I_TypeR
{
	//uint32_t value;
	struct
	{
		unsigned opcode : 7;
		unsigned rd : 5;
		unsigned funct3 : 3;
		unsigned rs1 : 5;
		unsigned rs2 : 5;
		unsigned funct7 : 7;
	};
};

union RV32I_TypeI
{
	//uint32_t value;
	struct
	{
		unsigned opcode : 7;
		unsigned rd : 5;
		unsigned funct3 : 3;
		unsigned rs1 : 5;
		signed imm12 : 12;
	};
};

union RV32I_TypeS
{
	//uint32_t value;
	struct
	{
		unsigned opcode : 7;
		unsigned imm5 : 5;
		unsigned funct3 : 3;
		unsigned rs1 : 5;
		unsigned rs2 : 5;
		unsigned imm7 : 7;
	};
};

union RV32I_TypeB
{
	uint32_t value;
	struct
	{
		unsigned opcode : 7;
		unsigned imm1_1 : 1;
		unsigned imm4 : 4;
		unsigned funct3 : 3;
		unsigned rs1 : 5;
		unsigned rs2 : 5;
		unsigned imm6 : 6;
		unsigned imm1_2 : 1;
	};

	signed offset()
	{
		static constexpr uint32_t mask1 = 0b00000000000000000000000010000000; // << 4
		//                       0b00000000000000000000100000000000;
		static constexpr uint32_t mask2 = 0b00000000000000000000111100000000; // >> 7
		//                       0b00000000000000000000000000011110;
		static constexpr uint32_t mask3 = 0b01111110000000000000000000000000; // >> 20
		//                       0b00000000000000000000011111100000;
		static constexpr uint32_t mask4 = 0b10000000000000000000000000000000; // >> 19
		//                       0b00000000000000000001000000000000;
		unsigned fin = 0;
		fin |= (value & mask1) << 4;
		fin |= (value & mask2) >> 7;
		fin |= (value & mask3) >> 20;
		fin |= (signed)(value & mask4) >> 19;
		return (signed)fin / 4 - 1;
	}
};

union RV32I_TypeU
{
	//uint32_t value;
	static constexpr uint32_t MaskImm = 0b1111111111111111111100000000000;
	struct
	{
		unsigned opcode : 7;
		unsigned rd: 5;
		signed imm20 : 20;
	};
};


union RV32I_TypeJ
{
	uint32_t value;
	struct
	{
		unsigned opcode : 7;
		unsigned rd : 5;
		unsigned imm8 : 8;
		unsigned imm1_1 : 1;
		unsigned imm10 : 10;
		unsigned imm1_2 : 1;
	};

	signed offset()
	{
	  	static constexpr uint32_t MaskUpperInteger = 0b00000000000011111111000000000000;
	  	static constexpr uint32_t MaskMiddleBit    = 0b00000000000100000000000000000000;
	  	static constexpr uint32_t MaskLowerInteger = 0b01111111111000000000000000000000;
	  	static constexpr uint32_t MaskSignBit      = 0b10000000000000000000000000000000;
		unsigned off = 0;
		off |= (value & MaskUpperInteger) << 11;
		off |= (value & MaskMiddleBit) << 2;
		off |= (value & MaskLowerInteger) >> 9;
		off |= (value & MaskSignBit);
		return (signed)off >> 12;
	};
};

union RISCVInstruction
{
	struct {
		unsigned family : 2;
		unsigned opcode : 5;
	};
	unsigned value;
	RV32I_TypeR r;
	RV32I_TypeS s;
	RV32I_TypeI i;
	RV32I_TypeU u;
	RV32I_TypeB b;
	RV32I_TypeJ j;

	RISCVInstruction() : value(0) { }
	RISCVInstruction(uint32_t v) : value(v) { }
	RISCVInstruction(const RISCVInstruction& o) : value(o.value) { }
};

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
	uint32_t xregs[32];

	uint32_t* pc;
	uint32_t stack_region_size;
	uint8_t* stack_region;
	uint32_t instruction_block_size;
	uint32_t* instruction_block;

	const bool AddressWithinBounds(const void* address)
	{
		return address >= instruction_block && address < instruction_block + instruction_block_size / sizeof(*instruction_block);
	}

	RISCVContainer() = delete;
	RISCVContainer(const uint32_t* instructions, size_t array_size) 
	{
		memset(xregs, 0, sizeof(xregs));
		stack_region_size = DefaultRISCVStackSize;
		stack_region = (uint8_t*)_aligned_malloc(stack_region_size, 16);
		if (!stack_region)
			RVCore_CriticalError("Failed to allocate stack region");
		memset(stack_region, 0, stack_region_size);
		instruction_block_size = array_size;
		instruction_block = (uint32_t*)_aligned_malloc(array_size, 4);
		if (!instruction_block)
			RVCore_CriticalError("Failed to allocate instruction block");
		memcpy(instruction_block, instructions, array_size);
		pc = instruction_block;
	}

	~RISCVContainer()
	{
		if (stack_region)
			_aligned_free(stack_region/*, 16, stack_region_size*/);
		if (instruction_block)
			_aligned_free(instruction_block/*, 4, instruction_block_size*/);
		stack_region = NULL;
		instruction_block = NULL;
		stack_region_size = 0;
		instruction_block_size = 0;
	}

	int PerformCycle()
	{
		if (!AddressWithinBounds(pc))
			return RVE_InstructionOOB;
		xregs[0] = 0;
		RISCVInstruction i = *pc++;

		if (i.family == 0x3 && i.opcode == 0x00)
		{
			if (i.i.funct3 == 0) 
				;

		}
		if (i.family == 0x3 && i.opcode == 0x04)
		{
			if (i.i.rd == 0)
				return 0;
			if (i.i.funct3 == 0) // addi (add signed immediate)
				xregs[i.i.rd] = xregs[i.i.rs1] + i.i.imm12;
			else if (i.i.funct3 == 1 && i.s.imm7 == 0) // slli
				xregs[i.r.rd] = xregs[i.r.rs1] << i.r.rs2;
			else if (i.i.funct3 == 2) // slti
				xregs[i.i.rd] = (signed)xregs[i.i.rs1] < (signed)i.i.imm12;
			else if (i.i.funct3 == 3) // sltiu
				xregs[i.i.rd] = (unsigned)xregs[i.i.rs1] < (unsigned)i.i.imm12;
			else if (i.i.funct3 == 4) // xori
				xregs[i.i.rd] = xregs[i.i.rs1] ^ i.i.imm12;
			else if (i.i.funct3 == 5 && i.s.imm7 == 0) // srli
				xregs[i.r.rd] = (unsigned)xregs[i.r.rs1] >> i.r.rs2;
			else if (i.i.funct3 == 5 && i.s.imm7 == 16) // srai
				xregs[i.r.rd] = (signed)xregs[i.r.rs1] >> i.r.rs2;
			else if (i.i.funct3 == 6) // ori
				xregs[i.i.rd] = xregs[i.i.rs1] | (unsigned)i.i.imm12;
			else if (i.i.funct3 == 7) // andi
				xregs[i.i.rd] = xregs[i.i.rs1] & (unsigned)i.i.imm12;
			else
				{ RV32I_UnimplementedExit; }
			return 0;
			// illegal, exhausted funct3
		}
		if (i.family == 0x3 && i.opcode == 0x05) // auipc
		{
			xregs[i.u.rd] = (uint32_t)(pc - instruction_block) + (i.value & i.u.MaskImm);
			return 0;
		}
		if (i.family == 0x3 && i.opcode == 0x0C)
		{
			if (i.i.rd == 0)
				return 0;
			if (i.s.funct3 == 0 && i.s.imm7 == 0) // add
				xregs[i.r.rd] = xregs[i.s.rs1] + xregs[i.s.rs2];
			else if (i.s.funct3 == 0 && i.s.imm7 == 32) // sub
				xregs[i.r.rd] = xregs[i.s.rs1] - xregs[i.s.rs2];
			else if (i.s.funct3 == 1 && i.s.imm7 == 0) // sll (shift left logical)
				xregs[i.r.rd] = xregs[i.s.rs1] << xregs[i.s.rs2];
			else if (i.s.funct3 == 2 && i.s.imm7 == 0) // slt
				xregs[i.r.rd] = (signed)xregs[i.s.rs1] < (signed)xregs[i.s.rs2];
			else if (i.s.funct3 == 3 && i.s.imm7 == 0) // sltu
				xregs[i.r.rd] = (unsigned)xregs[i.s.rs1] < (unsigned)xregs[i.s.rs2];
			else if (i.s.funct3 == 4 && i.s.imm7 == 0) // xor
				xregs[i.r.rd] = (unsigned)xregs[i.s.rs1] ^ (unsigned)xregs[i.s.rs2];
			else if (i.s.funct3 == 5 && i.s.imm7 == 0) // srl (shift right logical)
				xregs[i.r.rd] = (unsigned)xregs[i.s.rs1] >> (xregs[i.s.rs2] & 0b11111);
			else if (i.s.funct3 == 5 && i.s.imm7 == 32) // sra (shift right arithmetic)
				xregs[i.r.rd] = (signed)xregs[i.s.rs1] >> (xregs[i.s.rs2] & 0b11111);
			else if (i.s.funct3 == 6 && i.s.imm7 == 0) // or
				xregs[i.r.rd] = xregs[i.s.rs1] | xregs[i.s.rs2];
			else if (i.s.funct3 == 7 && i.s.imm7 == 0) // and
				xregs[i.r.rd] = xregs[i.s.rs1] & xregs[i.s.rs2];
			else
				{ RV32I_UnimplementedExit; }
			return 0;
			// illegal, exhausted funct3
		}
		if (i.family == 0x3 && i.opcode == 0x0D) // lui
		{
			xregs[i.u.rd] = i.value & i.u.MaskImm;
			return 0;
		}
		if (i.family == 0x3 && i.opcode == 0x1B) // jal
		{
			printf("%s %i\t; pc is now %i\n", "jal", i.j.offset(), instruction_block - pc);
			pc += i.j.offset();
			return 0;
		}
		if (i.family == 0x3 && i.opcode == 0x18)
		{
			printf("branch x%i, x%i, %i\n", i.b.rs1, i.b.rs2, i.b.offset());
			if (i.b.funct3 == 2 || i.b.funct3 == 3) [[unlikely]]
				{ RV32I_UnimplementedExit; }

			if ((i.b.funct3 == 0 && xregs[i.b.rs1] == xregs[i.b.rs2])					||	// beq
				(i.b.funct3 == 1 && xregs[i.b.rs1] == xregs[i.b.rs2])					||	// bne
				(i.b.funct3 == 4 && (signed)xregs[i.b.rs1] <= (signed)xregs[i.b.rs2])	||	// blt
				(i.b.funct3 == 5 && (signed)xregs[i.b.rs1] >= (signed)xregs[i.b.rs2])	||	// bge
				(i.b.funct3 == 6 && xregs[i.b.rs1] <= xregs[i.b.rs2])					||	// bltu
				(i.b.funct3 == 7 && xregs[i.b.rs1] >= xregs[i.b.rs2])					)	// bgeu
				pc += i.b.offset();

			return 0;
		}


		if (i.family == 0x3 && i.opcode == 0x19)
		{
			printf("jalr x%i x%i %i", i.i.rd, i.i.rs1, i.i.imm12);

		}
		printf("@%016llX Unknown instr: f=%i, op=%i, imm7=%i\n", pc, i.family, i.opcode, i.s.imm7);
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