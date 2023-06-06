#include "riscv_vm.hpp"

const uint32_t rv32_bin[] = {
	0x00178793,
	0x00178793,
	0x00178793
};
int main(int argc, char* argv[])
{
	RISCVContainer rvtest(rv32_bin, sizeof(rv32_bin));
	rvtest.Execute();
    return rvtest.xregs[15] != 3;
}