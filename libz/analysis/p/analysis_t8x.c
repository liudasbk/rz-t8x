#include <string.h>
#include <rz_types.h>
#include <rz_lib.h>
#include <rz_asm.h>
#include <rz_analysis.h>
#include "../../asm/arch/t8x/t8x.h"

static int t8x_anop(RzAnalysis *analysis, RzAnalysisOp *op, ut64 addr, const ut8 *data, int len, RzAnalysisOpMask mask) {

	ut8 op_size = t8x_inst_size[data[0]];
	t8x_inst in = inst[data[0]];

	// TODO: debug info
	if (op_size > len) return -1;

	op->type = in.type;
	op->addr = addr;

	switch (in.addr_mode) {
	
	case T8X_DIR:
		// 0x33 ld imm, dir
		if (data[0] == 0x33) {
			op->ptrsize = 8;
			op->ptr = data[2];
		}
		else {
			op->ptrsize = 8;
			op->ptr = data[1];
		}

		if (in.type == RZ_ANALYSIS_OP_TYPE_MJMP)
		       op->jump = (ut8)data[1];	

		break;

	case T8X_EXT:
		op->ptrsize = 16;
		op->ptr = data[1] << 8 | data[2];

		if (in.type == RZ_ANALYSIS_OP_TYPE_MJMP)
			op->jump = op->ptr;

		break;

	case T8X_REL:
		if (data[1] <= 127)
			op->jump = addr + data[1] + op_size;
		else 
			op->jump = addr - (256 - data[1]) + op_size;

		break;

	case T8X_IMM:
		// opcodes 0x86-0x89 and 0x8b-0x8f have arg size of 16
		if ((data[0] >= 0x86 && data[0] <= 0x89) || (data[0] >= 0x8b && data[0] <= 0x8f))
			op->val = data[1] << 8 | data[2];
		else
			op->val = data[1];
	}

	return op->size = op_size;
}

static char *get_reg_profile(RzAnalysis *analysis) {
	const char *p =
		"=PC	pc\n"
		"=SP	sp\n"
		"=A0	a\n"
		"=A1	b\n"

		"gpr	a	.8	0	0\n"
		"gpr	b	.8	1	0\n"
		"gpr	d	.16	0	0\n"
		"gpr	x	.16	2	0\n"
		"gpr	y	.16	4	0\n"
		"gpr	sp	.16	6	0\n"
		"gpr	pc	.16	8	0\n"
		"gpr	ccr	.8	10	0\n"
		"gpr	c	.1	10.0	0\n"
		"gpr	v	.1	10.1	0\n"
		"gpr	z	.1	10.2	0\n"
		"gpr	n	.1	10.3	0\n"
		"gpr	i	.1	10.4	0\n"
		"gpr	h	.1	10.5	0\n";

	return strdup(p);
}

struct rz_analysis_plugin_t rz_analysis_plugin_t8x = {
    .name = "t8x",
    .desc = "Toshiba 8x analysis plugin",
    .license = "MIT",
    .arch = RZ_SYS_ARCH_NONE,
    .bits = 8,
    .get_reg_profile = &get_reg_profile,
    .init = NULL,
    .fini = NULL,
    .op = &t8x_anop,
};

#ifndef RZ_PLUGIN_INCORE
RZ_API RzLibStruct rizin_plugin = {
    .type = RZ_LIB_TYPE_ANALYSIS,
    .data = &rz_analysis_plugin_t8x,
    .version = RZ_VERSION
};
#endif
