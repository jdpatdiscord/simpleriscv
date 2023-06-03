#include "riscv_vm.hpp"
#include "common.hpp"
#include "bitmask_utility.hpp"

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

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	RISCVContainer runner(rv32_bin, sizeof(rv32_bin));
	runner.Run();
	return 0;
}