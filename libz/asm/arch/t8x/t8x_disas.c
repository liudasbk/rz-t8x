#include <stdio.h>
#include <string.h>
#include <rz_lib.h>
#include <rz_asm.h>
#include "t8x.h"

static int disassemble(RzAsm *a, RzAsmOp *op, const ut8 *buf, int len) {

	const char *buf_asm = "invalid";

	if (len < 1) return -1;	

	int size = t8x_inst_size[buf[0]];

	// return if buffer is smaller than actual instruction size
	if (len < size) return -1;

	t8x_inst in = inst[buf[0]];

	if (in.addr_mode == T8X_INH) {
		// instruction without arguments
		buf_asm = sdb_fmt("%s", inst[buf[0]].name);
	}
	else {
		int arg;

		if (size == 2)
			arg = (ut16) buf[1];
		else 
			arg = (ut16) (buf[1] << 8 | buf[2]);

		switch (in.addr_mode) {
			case T8X_DIR:
			case T8X_IMM:
			case T8X_EXT:
				// 0x79 and 0x33 combines both IMM and DIR
				if (buf[0] == 0x79 || buf[0] == 0x33)
					buf_asm = sdb_fmt(in.name, buf[1], buf[2]);
				else
					buf_asm = sdb_fmt(in.name, arg);

				break;
			case T8X_REL:
				// TODO: argument is relative
				buf_asm = sdb_fmt(in.name, arg);
				
				break;
			case T8X_IND:
				if ((arg & 0x80) != 0x80)
					buf_asm = sdb_fmt(in.name, 'x', arg);
				else
					buf_asm = sdb_fmt(in.name, 'y', arg);
				
				break;
			case T8X_BIT:
				// 2 byte instructions:
				// opcode | bit, reg
				//
				// 3 byte instructions:
				// opcode | bit, reg | imm
				if (size == 2)
					buf_asm = sdb_fmt(in.name, arg >> 5, arg & 0x1f);
				else 
					// TODO: second arg is relative
					buf_asm = sdb_fmt(in.name, buf[1] >> 5, buf[1] & 0x14, buf[2]);

				break;
		}
	}

	rz_asm_op_set_asm (op, buf_asm);

	return op->size = size;
}

RzAsmPlugin rz_asm_plugin_mycpu = {
    .name = "t8x",
    .license = "MIT",
    .desc = "Toshiba 8X disassembly plugin",
    .arch = "t8x",
    .bits = 8,
    .endian = RZ_SYS_ENDIAN_BIG,
    .disassemble = &disassemble
};

#ifndef RZ_PLUGIN_INCORE
RZ_API RzLibStruct rizin_plugin = {
    .type = RZ_LIB_TYPE_ASM,
    .data = &rz_asm_plugin_mycpu,
    .version = RZ_VERSION
};
#endif
