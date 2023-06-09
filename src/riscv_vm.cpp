#include "riscv_vm.hpp"

// Currently implemented:
// From I-base:
// addi, slli, slti, sltiu, xori, srli, srai, ori, andi
// auipc
// add, sub, sll, slt, sltu, xor, srl, sra, or, and
// lui
// jal
// beq bne blt bge bltu bgeu

// From Zbb-extension:
// clz, ctz, max, maxu, min, minu, orn

int RISCVContainer::BaseI()
{
    RISCVInstruction insn = *pc++;
    if (insn.family() != 3)
        return ErrorNotHandled;
    if (insn.opcode() == 4)
    {
        if (i.funct3() == 0) // addi (add signed immediate)
            xregs[i.rd()] = xregs[i.rs1()] + (signed)i.imm12();
    }
    return ErrorNotHandled;
};
int RISCVContainer::ExtensionA()
{
    return 0;
};
int RISCVContainer::ExtensionB()
{
    return 0;
};
int RISCVContainer::ExtensionC()
{
    return 0;
};
int RISCVContainer::ExtensionD()
{
    return 0;
};
int RISCVContainer::ExtensionF()
{
    return 0;
};
int RISCVContainer::ExtensionQ()
{
    return 0;
};
int RISCVContainer::ExtensionZbb()
{
    return 0;
};

int RISCVContainer::Execute()
{
    while (1)
    {
        xregs[0] = 0;
        if (!AddressWithinBounds(pc))
            return ErrorOutOfBounds;
        if (BaseI()) continue;
        if (ExtensionC()) continue;
        if (ExtensionB()) continue;
        if (ExtensionF()) continue;
        if (ExtensionD()) continue;
        if (ExtensionA()) continue;
        return ErrorNotHandled;
    }
};

/*
int RISCVContainer::PerformCycle()
{
    if (!AddressWithinBounds(pc))
        return RVE_InstructionOOB;
    xregs[0] = 0;
    RISCVInstruction insn = {*pc++};
    if (insn.family() == 0x3 && insn.opcode() == 0x00)
    {
        if (as_i(insn).funct3() == 0) { // lb
            ;
        }
    }
    if (insn.family() == 0x3 && insn.opcode() == 0x04)
    {
        // RISC-V documentation calls SLLI/SRLI/SRAI a special I-type format, but it matches up with R-type.
        auto i = as_i(insn);
        auto r = as_r(insn);
        if (i.rd() == 0)
            return 0;
        if (i.funct3() == 0) // addi (add signed immediate)
            xregs[i.rd()] = xregs[i.rs1()] + (signed)i.imm12();
        else if (r.funct3() == 1 && r.funct7() == 0) // slli
            xregs[r.rd()] = xregs[r.rs1()] << r.rs2(); // rs2 == shamt
        else if (r.funct3() == 1 && r.funct7() == 48 && r.rs2() == 0) // clz (Zbb-ext)
            xregs[r.rd()] = std::countr_zero(xregs[r.rs1()]);
        else if (r.funct3() == 1 && r.funct7() == 48 && r.rs2() == 1) // ctz (Zbb-ext)
            xregs[r.rd()] = std::countl_zero(xregs[r.rs1()]);
        else if (i.funct3() == 2) // slti
            xregs[i.rd()] = (signed)xregs[i.rs1()] < (signed)i.imm12();
        else if (i.funct3() == 3) // sltiu
            xregs[i.rd()] = (unsigned)xregs[i.rs1()] < i.imm12();
        else if (i.funct3() == 4) // xori
            xregs[i.rd()] = xregs[i.rs1()] ^ i.imm12();
        else if (i.funct3() == 5 && r.funct7() == 0) // srli
            xregs[r.rd()] = (unsigned)xregs[r.rs1()] >> r.rs2(); // rs2 == shamt
        else if (i.funct3() == 5 && r.funct7() == 16) // srai
            xregs[r.rd()] = (signed)xregs[r.rs1()] >> r.rs2(); // rs2 == shamt
        else if (i.funct3() == 6) // ori
            xregs[i.rd()] = xregs[i.rs1()] | i.imm12();
        else if (i.funct3() == 7) // andi
            xregs[i.rd()] = xregs[i.rs1()] & i.imm12();
        else
            { RV32I_UnimplementedExit; }
        return 0;
        // illegal, exhausted funct3
    }
    if (insn.family() == 0x3 && insn.opcode() == 0x05) // auipc
    {
        xregs[as_u(insn).rd()] = (uint32_t)(pc - instruction_block.data()) + (u32{insn} & as_u(insn).MaskImm);
        return 0;
    }
    if (insn.family() == 0x3 && insn.opcode() == 0x0C)
    {
        auto r = as_r(insn);
        if (r.rd() == 0)
            return 0;
        if (r.funct3() == 0 && r.funct7() == 0) // add
            xregs[r.rd()] = xregs[r.rs1()] + xregs[r.rs2()];
        else if (r.funct3() == 0 && r.funct7() == 32) // sub
            xregs[r.rd()] = xregs[r.rs1()] - xregs[r.rs2()];
        else if (r.funct3() == 1 && r.funct7() == 0) // sll (shift left logical)
            xregs[r.rd()] = xregs[r.rs1()] << xregs[r.rs2()];
        else if (r.funct3() == 2 && r.funct7() == 0) // slt
            xregs[r.rd()] = (signed)xregs[r.rs1()] < (signed)xregs[r.rs2()];
        else if (r.funct3() == 3 && r.funct7() == 0) // sltu
            xregs[r.rd()] = (unsigned)xregs[r.rs1()] < (unsigned)xregs[r.rs2()];
        else if (r.funct3() == 4 && r.funct7() == 0) // xor
            xregs[r.rd()] = (unsigned)xregs[r.rs1()] ^ (unsigned)xregs[r.rs2()];
        else if (r.funct3() == 4 && r.funct7() == 5) // min (signed) (Zbb-ext)
            xregs[r.rd()] = std::min((signed)xregs[r.rs1()], (signed)xregs[r.rs2()]);
        else if (r.funct3() == 5 && r.funct7() == 5) // minu (unsigned) (Zbb-ext)
            xregs[r.rd()] = std::min(xregs[r.rs1()], xregs[r.rs2()]);
        else if (r.funct3() == 5 && r.funct7() == 0) // srl (shift right logical)
            xregs[r.rd()] = (unsigned)xregs[r.rs1()] >> (xregs[r.rs2()] & 0b11111);
        else if (r.funct3() == 5 && r.funct7() == 32) // sra (shift right arithmetic)
            xregs[r.rd()] = (signed)xregs[r.rs1()] >> (xregs[r.rs2()] & 0b11111);
        else if (r.funct3() == 6 && r.funct7() == 32) // orn (Zbb-ext, Zbkb-ext)
            xregs[r.rd()] = xregs[r.rs1()] | ~xregs[r.rs2()];
        else if (r.funct3() == 6 && r.funct7() == 0) // or
            xregs[r.rd()] = xregs[r.rs1()] | xregs[r.rs2()];
        else if (r.funct3() == 6 && r.funct7() == 5) // max (signed) (Zbb-ext)
            xregs[r.rd()] = std::max((signed)xregs[r.rs1()], (signed)xregs[r.rs2()]);
        else if (r.funct3() == 7 && r.funct7() == 5) // maxu (unsigned) (Zbb-ext)
            xregs[r.rd()] = std::max(xregs[r.rs1()], xregs[r.rs2()]);
        else if (r.funct3() == 7 && r.funct7() == 0) // and
            xregs[r.rd()] = xregs[r.rs1()] & xregs[r.rs2()];
        else
            { RV32I_UnimplementedExit; }
        return 0;
        // illegal, exhausted funct3
    }
    if (insn.family() == 0x3 && insn.opcode() == 0x0D) // lui
    {
        auto u = as_u(insn);
        xregs[u.rd()] = u32(insn) & u.MaskImm;
        return 0;
    }
    if (insn.family() == 0x3 && insn.opcode() == 0x1B) // jal
    {
        pc += as_j(insn).offset();
        return 0;
    }
    if (insn.family() == 0x3 && insn.opcode() == 0x18)
    {
        auto b = as_b(insn);
        if (b.funct3() == 2 || b.funct3() == 3)
            { RV32I_UnimplementedExit; }
        if ((b.funct3() == 0 && xregs[b.rs1()] == xregs[b.rs2()])                       // beq
            || (b.funct3() == 1 && xregs[b.rs1()] != xregs[b.rs2()])                    // bne
            || (b.funct3() == 4 && (signed)xregs[b.rs1()] < (signed)xregs[b.rs2()])     // blt
            || (b.funct3() == 5 && (signed)xregs[b.rs1()] >= (signed)xregs[b.rs2()])    // bge
            || (b.funct3() == 6 && xregs[b.rs1()] < xregs[b.rs2()])                     // bltu
            || (b.funct3() == 7 && xregs[b.rs1()] >= xregs[b.rs2()]))                   // bgeu
        {
            pc += as_b(insn).offset();
        }
        return 0;
    }
    if (insn.family() == 0x3 && insn.opcode() == 0x19) // jalr
    {
        auto i = as_i(insn);
        xregs[i.rd()] = xregs[i.rs1()] + i.imm12();
        return 0;
    }
    printf("@%p Unknown instr: f=%i, op=%i, imm7=%i\n", (void*)pc, insn.family(), insn.opcode(), as_s(insn).imm7());
    return 0;
}


void RISCVContainer::Run()
{
    int cont = 1;
    while(cont)
    {
        int result = PerformCycle();
        if (result != 0)
        {
            //printf("Exit: %i\n", result);
            cont = 0;
        }
    }
}
*/

/*
const uint32_t rv32_bin[] = {
    // 0x00050e63,    //        beq     a0,zero,.L1            // void square(int num) {
    // 0x00100793,    //        li      a5,1                //     if (num)
    // 0x40f02223,    //        sw      a5,1028(zero)        //     {
    // 0x00200793,    //        li      a5,2                //         *(int*)1028=1;
    // 0x40f02423,    //        sw      a5,1032(zero)        //         *(int*)1032=2;
    // 0x00300793,    //        li      a5,3                //         *(int*)1036=3;
    // 0x40f02623,    //        sw      a5,1036(zero)        //     }
    //             //    .L1:                            //     return;
    // 0x00008067    //        ret                            // }

    0x00050a63,    //        beq        a0,zero,.L1    
    0x00000793,    //        li        a5,0            // void square(int num) {
    0x00f7a023,    //    .L3:sw        a5,0(a5)        //     for (unsigned i = 0; i < num; ++i)
    0x00178793,    //        addi    a5,a5,1            //         *(int*)i = i;
    0xfea79ce3,    //        bne        a5,a0,.L3        //     return;
    0x00008067,    //    .L1:ret                        // }
};


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    RISCVContainer runner(rv32_bin, sizeof(rv32_bin));
    runner.Run();
    return 0;
}
*/