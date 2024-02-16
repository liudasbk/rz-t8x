#include <stdio.h>
#include <string.h>
#include <rz_lib.h>
#include <rz_asm.h>

// addressing modes
enum {
	T8X_IMM,   // Immediate
	T8X_DIR,   // Direct
	T8X_EXT,   // Extended
	T8X_IND,
	T8X_INH,
	T8X_REL,
	T8X_BIT
};

// 0x0a size 3->2
static const ut8 t8x_inst_size[] = {
 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 2, 1, 2, 2, 1, 2, 1, 2, 2, 2, 2, 1, 1, 1, 1,
 2, 2, 2, 3, 2, 3, 2, 3, 2, 2, 2, 2, 1, 1, 1, 1,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 2, 2, 2, 1, 2, 1, 2, 1, 1, 1, 2, 2, 1, 1, 1, 1,
 2, 2, 2, 1, 2, 2, 2, 2, 1, 3, 2, 2, 1, 1, 1, 1,
 2, 2, 1, 1, 2, 2, 3, 3, 3, 3, 1, 3, 3, 3, 3, 3,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};

typedef struct {
	const char *name;
	int addr_mode;
	ut32 type;
	//args
} t8x_inst;

static const t8x_inst
inst[] = {
	{ "nop",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NOP },  // 0x00
	{ "jsr $0x%04x",  T8X_EXT, RZ_ANALYSIS_OP_TYPE_CALL },  // 0x01
	{ "xch a, b",     T8X_INH, RZ_ANALYSIS_OP_TYPE_XCHG | RZ_ANALYSIS_OP_TYPE_REG },  // 0x02
	{ "jmp $0x%04x",  T8X_EXT, RZ_ANALYSIS_OP_TYPE_JMP },  // 0x03
	{ "shr d",        T8X_INH, RZ_ANALYSIS_OP_TYPE_SHR | RZ_ANALYSIS_OP_TYPE_REG },  // 0x04
	{ "di",           T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x05
	{ "shl d",        T8X_INH, RZ_ANALYSIS_OP_TYPE_SHL | RZ_ANALYSIS_OP_TYPE_REG },  // 0x06
	{ "ei",           T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x07
	{ "add a, b",     T8X_INH, RZ_ANALYSIS_OP_TYPE_ADD | RZ_ANALYSIS_OP_TYPE_REG },  // 0x08
	{ "sub a, b",     T8X_INH, RZ_ANALYSIS_OP_TYPE_SUB | RZ_ANALYSIS_OP_TYPE_REG },  // 0x09
	{ "st x, $0x%02x", T8X_DIR, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM },  // 0x0a
	{ "cmp a, b",     T8X_INH, RZ_ANALYSIS_OP_TYPE_CMP | RZ_ANALYSIS_OP_TYPE_REG },  // 0x0b
	{ "add x, a",     T8X_INH, RZ_ANALYSIS_OP_TYPE_ADD | RZ_ANALYSIS_OP_TYPE_REG },  // 0x0c
	{ "add y, a",     T8X_INH, RZ_ANALYSIS_OP_TYPE_ADD | RZ_ANALYSIS_OP_TYPE_REG },  // 0x0d
	{ "add x, b",     T8X_INH, RZ_ANALYSIS_OP_TYPE_ADD | RZ_ANALYSIS_OP_TYPE_REG },  // 0x0e
	{ "add y, b",     T8X_INH, RZ_ANALYSIS_OP_TYPE_ADD | RZ_ANALYSIS_OP_TYPE_REG },  // 0x0f
	{ "shr a",        T8X_INH, RZ_ANALYSIS_OP_TYPE_SHR | RZ_ANALYSIS_OP_TYPE_REG },  // 0x10
	{ "shr b",        T8X_INH, RZ_ANALYSIS_OP_TYPE_SHR | RZ_ANALYSIS_OP_TYPE_REG },  // 0x11
	{ "shl a",        T8X_INH, RZ_ANALYSIS_OP_TYPE_SHL | RZ_ANALYSIS_OP_TYPE_REG },  // 0x12
	{ "shl b",        T8X_INH, RZ_ANALYSIS_OP_TYPE_SHL | RZ_ANALYSIS_OP_TYPE_REG },  // 0x13
	{ "rorc a",       T8X_INH, RZ_ANALYSIS_OP_TYPE_ROR },  // 0x14
	{ "rorc b",       T8X_INH, RZ_ANALYSIS_OP_TYPE_ROR },  // 0x15
	{ "rolc a",       T8X_INH, RZ_ANALYSIS_OP_TYPE_ROL },  // 0x16
	{ "rolc b",       T8X_INH, RZ_ANALYSIS_OP_TYPE_ROL },  // 0x17
	{ "shra a",       T8X_INH, RZ_ANALYSIS_OP_TYPE_SHR },  // 0x18
	{ "shra b",       T8X_INH, RZ_ANALYSIS_OP_TYPE_SHR },  // 0x19
	{ "ld a, [y]",    T8X_INH, RZ_ANALYSIS_OP_TYPE_LOAD | RZ_ANALYSIS_OP_TYPE_REG },  // 0x1a
	{ "ld d, [y]",    T8X_INH, RZ_ANALYSIS_OP_TYPE_LOAD | RZ_ANALYSIS_OP_TYPE_REG },  // 0x1b
	{ "inc x",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL},  // 0x1c
	{ "inc y",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x1d
	{ "dec x",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x1e
	{ "dec y",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x1f
	{ "shr x",        T8X_INH, RZ_ANALYSIS_OP_TYPE_SHR | RZ_ANALYSIS_OP_TYPE_REG },  // 0x20
	{ "jsr %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_MJMP }, // 0x21
	{ "shl x",          T8X_INH, RZ_ANALYSIS_OP_TYPE_SHL | RZ_ANALYSIS_OP_TYPE_REG }, // 0x22
	{ "jmp %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_MJMP }, // 0x23
	{ "rorc %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_ROR | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x24
	{ "clrv",           T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x25
	{ "rolc %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_ROL | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x26
	{ "setv",           T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x27
	{ "shra %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_SHR | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x28
	{ "st s, %c+%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x29
	{ "st x, %c+%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x2a
	{ "st y, %c+%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x2b
	{ "mov x, s",       T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV }, // 0x2c
	{ "inc s",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x2d
	{ "mov s, x",       T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV }, // 0x2e
	{ "dec s",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x2f
	{ "shr $0x%02x",    T8X_DIR, RZ_ANALYSIS_OP_TYPE_SHR | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x30
	{ "jsr $0x%02x",    T8X_DIR, RZ_ANALYSIS_OP_TYPE_MJMP }, // 0x31
	{ "shl $0x%02x",    T8X_DIR, RZ_ANALYSIS_OP_TYPE_SHL | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x32
	{ "ld 0x%02x, $0x%02x",          T8X_DIR, RZ_ANALYSIS_OP_TYPE_LOAD | RZ_ANALYSIS_OP_TYPE_MEM },  // 0x33
	{ "rorc $0x%02x",                T8X_DIR, RZ_ANALYSIS_OP_TYPE_ROR | RZ_ANALYSIS_OP_TYPE_MEM },  // 0x34
	{ "tbbs bit%i, $0x%02x, 0x%02x", T8X_BIT, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x35
	{ "rolc $0x%02x",                T8X_DIR, RZ_ANALYSIS_OP_TYPE_ROL | RZ_ANALYSIS_OP_TYPE_MEM },  // 0x36
	{ "tbbc bit%i, $0x%02x, 0x%02x", T8X_BIT, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x37
	{ "shra $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_SHR | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x38 not confirmed
	{ "st s, $0x%02x", T8X_DIR, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x39
	{ "st x, $0x%02x", T8X_DIR, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x3a looks the same as 0x0a
	{ "st y, $0x%02x", T8X_DIR, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x3b
	{ "mov x, d",      T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV }, // 0x3c
	{ "mov y, d",      T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV }, // 0x3d
	{ "mov d, x",      T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV }, // 0x3e
	{ "mov d, y",      T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV }, // 0x3f
	{ "bra $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x40
	{ "brn $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x41
	{ "bgt $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x42
	{ "ble $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x43
	{ "bcc $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x44
	{ "bcs $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x45
	{ "bne $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x46
	{ "beq $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x47
	{ "bvc $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x48
	{ "bvs $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x49
	{ "bpz $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x4a
	{ "bmi $0x%02x",   T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x4b
	{ "bgea $0x%02x",  T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x4c
	{ "blta $0x%02x",  T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x4d
	{ "bgta $0x%02x",  T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x4e
	{ "blea $0x%02x",  T8X_REL, RZ_ANALYSIS_OP_TYPE_CJMP }, // 0x4f
	{ "dec a",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x50
	{ "dec b",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x51
	{ "clr a",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x52
	{ "clr b",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x53
	{ "neg a",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x54
	{ "neg b",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x55
	{ "inc a",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x56
	{ "inc b",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x57
	{ "cmpz a",       T8X_INH, RZ_ANALYSIS_OP_TYPE_CMP },  // 0x58
	{ "cmpz b",       T8X_INH, RZ_ANALYSIS_OP_TYPE_CMP },  // 0x59
	{ "mov b, a",     T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV },  // 0x5a
	{ "mov a, b",     T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV },  // 0x5b
	{ "mov ocr, a",   T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV },  // 0x5c
	{ "mov a, ocr",   T8X_INH, RZ_ANALYSIS_OP_TYPE_MOV },  // 0x5d
	{ "adj a",        T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x5e
	{ "nmi",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL },  // 0x5f
	{ "dec %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x60
	{ "bsr 0x%02x",    T8X_REL, RZ_ANALYSIS_OP_TYPE_JMP }, // 0x61
	{ "clr %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x62
	{ "ret",           T8X_INH, RZ_ANALYSIS_OP_TYPE_RET }, // 0x63
	{ "neg %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x64
	{ "clrc",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x65
	{ "inc %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x66
	{ "setc",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x67
	{ "push d",        T8X_INH, RZ_ANALYSIS_OP_TYPE_RPUSH }, // 0x68
	{ "xch x, y",      T8X_INH, RZ_ANALYSIS_OP_TYPE_XCHG }, // 0x69
	{ "xch a, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_XCHG }, // 0x6a
	{ "xch b, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_XCHG }, // 0x6b
	{ "push a",           T8X_INH, RZ_ANALYSIS_OP_TYPE_RPUSH }, // 0x6c
	{ "push b",           T8X_INH, RZ_ANALYSIS_OP_TYPE_RPUSH }, // 0x6d
	{ "push x",           T8X_INH, RZ_ANALYSIS_OP_TYPE_RPUSH }, // 0x6e
	{ "push y",           T8X_INH, RZ_ANALYSIS_OP_TYPE_RPUSH }, // 0x6f
	{ "dec $0x%02x",      T8X_DIR, RZ_ANALYSIS_OP_TYPE_NULL},  // 0x70
	{ "tbs bit%i, $0x%02x",   T8X_BIT, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x71
	{ "clr $0x%02x",          T8X_DIR, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x72
	{ "reti",                 T8X_INH, RZ_ANALYSIS_OP_TYPE_RET }, // 0x73
	{ "neg $0x%02x",          T8X_DIR, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x74
	{ "clrb bit%i, $0x%02x",  T8X_BIT, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x75
	{ "inc $0x%02x",          T8X_DIR, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x76
	{ "setb bit%i, $0x%02x",  T8X_BIT, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x77
	{ "pull d",               T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x78
	{ "cmp 0x%02x, $0x%02x",  T8X_EXT, RZ_ANALYSIS_OP_TYPE_CMP }, // 0x79
	{ "xch a, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_XCHG }, // 0x7a
	{ "xch b, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_XCHG }, // 0x7b
	{ "pull a",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x7c
	{ "pull b",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x7d
	{ "pull x",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x7e
	{ "pull y",          T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x7f
	{ "addc a, 0x%02x",  T8X_IMM, RZ_ANALYSIS_OP_TYPE_ADD }, // 0x80
	{ "mul a, 0x%02x",   T8X_IMM, RZ_ANALYSIS_OP_TYPE_MUL }, // 0x81
	{ "st a, [y]",       T8X_INH, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_REG }, // 0x82
	{ "wait",            T8X_INH, RZ_ANALYSIS_OP_TYPE_NULL }, // 0x83
	{ "subc a, 0x%02x",  T8X_IMM, RZ_ANALYSIS_OP_TYPE_SUB }, // 0x84
	{ "div d, 0x%02x",   T8X_IMM, RZ_ANALYSIS_OP_TYPE_DIV }, // 0x85
	{ "ld d, 0x%04x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0x86
	{ "add d, 0x%04x",   T8X_IMM, RZ_ANALYSIS_OP_TYPE_ADD }, // 0x87
	{ "sub d, 0x%04x",   T8X_IMM, RZ_ANALYSIS_OP_TYPE_SUB }, // 0x88
	{ "cmp d, 0x%04x",   T8X_IMM, RZ_ANALYSIS_OP_TYPE_CMP }, // 0x89
	{ "st d, [y]",       T8X_INH, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_REG }, // 0x8a
	{ "ld s, 0x%04x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0x8b
	{ "cmp x, 0x%04x",   T8X_IMM, RZ_ANALYSIS_OP_TYPE_CMP }, // 0x8c
	{ "cmp y, 0x%04x",   T8X_IMM, RZ_ANALYSIS_OP_TYPE_CMP }, // 0x8d
	{ "ld x, 0x%04x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0x8e
	{ "ld y, 0x%04x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0x8f
	{ "addc a, $0x%02x", T8X_DIR, RZ_ANALYSIS_OP_TYPE_ADD }, // 0x90
	{ "mul a, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_MUL }, // 0x91
	{ "st a, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x92
	{ "st b, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x93
	{ "subc a, $0x%02x", T8X_DIR, RZ_ANALYSIS_OP_TYPE_SUB }, // 0x94
	{ "div d, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_DIV }, // 0x95
	{ "ld d, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0x96
	{ "add d, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_ADD }, // 0x97
	{ "sub d, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_SUB }, // 0x98
	{ "cmp d, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_CMP }, // 0x99
	{ "st d, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0x9a
	{ "ld s, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0x9b
	{ "cmp x, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_CMP }, // 0x9c
	{ "cmp y, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_CMP }, // 0x9d
	{ "ld x, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0x9e
	{ "ld y, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0x9f
	{ "addc a, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xa0
	{ "mul a, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_MUL }, // 0xa1
	{ "st a, %c+0x%02x",   T8X_IND, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0xa2
	{ "st b, %c+0x%02x",   T8X_IND, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0xa3
	{ "subc a, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xa4
	{ "div d, %c+%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_DIV }, // 0xa5
	{ "ld d, %c+%02x",   T8X_IND, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xa6
	{ "add d, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xa7
	{ "sub d, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xa8
	{ "cmp d, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xa9
	{ "st d, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0xaa
	{ "ld s, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xab
	{ "cmp x, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xac
	{ "cmp y, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xad
	{ "ld x, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xae
	{ "ld y, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xaf
	{ "addc a, $0x%04x",  T8X_EXT, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xb0
	{ "mul a, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_MUL }, // 0xb1
	{ "st a, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0xb2
	{ "st b, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0xb3
	{ "subc a, $0x%04x",  T8X_EXT, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xb4
	{ "div d, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_DIV }, // 0xb5
	{ "ld d, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xb6
	{ "add d, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xb7
	{ "sub d, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xb8
	{ "cmp d, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xb9
	{ "st d, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_STORE | RZ_ANALYSIS_OP_TYPE_MEM }, // 0xba
	{ "ld s, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xbb
	{ "cmp x, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xbc
	{ "cmp y, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xbd
	{ "ld x, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xbe
	{ "ld y, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xbf
	{ "add a, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xc0
	{ "add b, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xc1
	{ "and a, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_AND }, // 0xc2
	{ "and b, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_AND }, // 0xc3
	{ "sub a, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xc4
	{ "sub b, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xc5
	{ "or a, 0x%02x",     T8X_IMM, RZ_ANALYSIS_OP_TYPE_OR }, // 0xc6
	{ "or b, 0x%02x",     T8X_IMM, RZ_ANALYSIS_OP_TYPE_OR }, // 0xc7
	{ "xor a, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_XOR }, // 0xc8
	{ "xor b, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_XOR }, // 0xc9
	{ "ld a, 0x%02x",     T8X_IMM, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xca
	{ "ld b, 0x%02x",     T8X_IMM, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xcb
	{ "cmp a, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xcc
	{ "cmp b, 0x%02x",    T8X_IMM, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xcd
	{ "cmpb a, 0x%02x",   T8X_IMM, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xce
	{ "cmpb b, 0x%02x",   T8X_IMM, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xcf
	{ "add a, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xd0
	{ "add b, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xd1
	{ "and a, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_AND }, // 0xd2
	{ "and b, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_AND }, // 0xd3
	{ "sub a, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xd4
	{ "sub b, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xd5
	{ "or a, $0x%02x",    T8X_DIR, RZ_ANALYSIS_OP_TYPE_OR }, // 0xd6
	{ "or b, $0x%02x",    T8X_DIR, RZ_ANALYSIS_OP_TYPE_OR }, // 0xd7
	{ "xor a, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_XOR }, // 0xd8
	{ "xor b, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_XOR }, // 0xd9
	{ "ld a, $0x%02x",    T8X_DIR, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xda
	{ "ld b, $0x%02x",    T8X_DIR, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xdb
	{ "cmp a, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xdc
	{ "cmp b, $0x%02x",   T8X_DIR, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xdd
	{ "cmpb a, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xde
	{ "cmpb b, $0x%02x",  T8X_DIR, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xdf
	{ "add a, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xe0
	{ "add b, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xe1
	{ "and a, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_AND }, // 0xe2
	{ "and b, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_AND }, // 0xe3
	{ "sub a, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xe4
	{ "sub b, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xe5
	{ "or a, %c+0x%02x",   T8X_IND, RZ_ANALYSIS_OP_TYPE_OR }, // 0xe6
	{ "or b, %c+0x%02x",   T8X_IND, RZ_ANALYSIS_OP_TYPE_OR }, // 0xe7
	{ "xor a, x+0x%02x",   T8X_IND, RZ_ANALYSIS_OP_TYPE_XOR }, // 0xe8
	{ "xor b, x+0x%02x",   T8X_IND, RZ_ANALYSIS_OP_TYPE_XOR }, // 0xe9
	{ "ld a, %c+0x%02x",   T8X_IND, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xea
	{ "ld b, %c+0x%02x",   T8X_IND, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xeb
	{ "cmp a, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xec
	{ "cmp b, %c+0x%02x",  T8X_IND, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xed
	{ "cmpb a, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xee
	{ "cmpb b, %c+0x%02x", T8X_IND, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xef
	{ "add a, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xf0
	{ "add b, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_ADD }, // 0xf1
	{ "and a, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_AND }, // 0xf2
	{ "and b, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_AND }, // 0xf3
	{ "sub a, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xf4
	{ "sub b, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_SUB }, // 0xf5
	{ "or a, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_OR }, // 0xf6
	{ "or b, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_OR }, // 0xf7
	{ "xor a, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_XOR }, // 0xf8
	{ "xor b, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_XOR }, // 0xf9
	{ "ld a, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xfa
	{ "ld b, $0x%04x",    T8X_EXT, RZ_ANALYSIS_OP_TYPE_LOAD }, // 0xfb
	{ "cmp a, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xfc
	{ "cmp b, $0x%04x",   T8X_EXT, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xfd
	{ "cmpb a, $0x%04x",  T8X_EXT, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xfe
	{ "cmpb b, $0x%04x",  T8X_EXT, RZ_ANALYSIS_OP_TYPE_CMP }, // 0xff
};
