#pragma once

namespace cdc {

enum {
	SCMD_NOP = 0x0,
	SCMD_POP = 0x1,

	SCMD_PUSH = 0x2,
	SCMD_PUSHI = 0x3,
	SCMD_PUSHI8 = 0x4,
	SCMD_PUSHC = 0x5,

	SCMD_PUSHL8 = 0x6,
	SCMD_PUSHL16 = 0x7,
	SCMD_PUSHL32 = 0x8,
	SCMD_PUSHLA = 0x9,
	SCMD_PUSHLR = 0xA,

	SCMD_PUSHS = 0xB,

	SCMD_PUSHS8 = 0xC,
	SCMD_PUSHS16 = 0xD,
	SCMD_PUSHS32 = 0xE,
	SCMD_PUSHSA = 0xF,
	SCMD_PUSHSR = 0x10,

	SCMD_PUSHM8 = 0x11,
	SCMD_PUSHM16 = 0x12,
	SCMD_PUSHM32 = 0x13,
	SCMD_PUSHMA = 0x14,
	SCMD_PUSHMR = 0x15,

	SCMD_PUSHT8 = 0x16,
	SCMD_PUSHT16 = 0x17,
	SCMD_PUSHT32 = 0x18,
	SCMD_PUSHTA = 0x19,
	SCMD_PUSHTR = 0x1A,

	SCMD_DREF8 = 0x1B,
	SCMD_DREF16 = 0x1C,
	SCMD_DREF32 = 0x1D,
	SCMD_DREFA = 0x1E,
	SCMD_DREFR = 0x1F,

	SCMD_DREFD = 0x20,

	SCMD_BOUNDS = 0x21,
	SCMD_DBOUNDS = 0x22,

	SCMD_MAP8 = 0x23,
	SCMD_MAP16 = 0x24,
	SCMD_MAP32 = 0x25,
	SCMD_MAPA = 0x26,
	SCMD_MAPR = 0x27,

	SCMD_MAPG = 0x28,

	SCMD_IMAP8 = 0x29,
	SCMD_IMAP16 = 0x2A,
	SCMD_IMAP32 = 0x2B,
	SCMD_IMAPA = 0x2C,
	SCMD_IMAPR = 0x2D,

	SCMD_MOVE8 = 0x2E,
	SCMD_MOVE32 = 0x2F,
	SCMD_MOVER = 0x30,
	SCMD_MOVEC = 0x31,
	SCMD_MOVEM = 0x32,
	SCMD_MOVED = 0x33,
	SCMD_MOVEU = 0x34,
	SCMD_MOVET = 0x35,
	SCMD_MOVEN = 0x36,
	SCMD_MOVEAR = 0x37,
	SCMD_MOVEAC = 0x38,
	SCMD_MOVEAM = 0x39,
	SCMD_MOVEAD = 0x3A,
	SCMD_MOVEAU = 0x3B,
	SCMD_MOVEAT = 0x3C,

	SCMD_NEGI = 0x3D,
	SCMD_ADDI = 0x3E,
	SCMD_SUBI = 0x3F,
	SCMD_MULI = 0x40,
	SCMD_DIVI = 0x41,
	SCMD_MODI = 0x42,
	SCMD_LTI = 0x43,
	SCMD_LTEQI = 0x44,
	SCMD_EQI = 0x45,
	SCMD_EQI8 = 0x46,

	SCMD_NEGF = 0x47,
	SCMD_ADDF = 0x48,
	SCMD_SUBF = 0x49,
	SCMD_MULF = 0x4A,
	SCMD_DIVF = 0x4B,
	SCMD_LTF = 0x4C,
	SCMD_LTEQF = 0x4D,
	SCMD_EQF = 0x4E,

	SCMD_EQC = 0x4F,

	SCMD_NOTB = 0x50,
	SCMD_ANDB = 0x51,
	SCMD_ORB = 0x52,
	SCMD_XORB = 0x53,

	SCMD_AND = 0x54,
	SCMD_OR = 0x55,
	SCMD_XOR = 0x56,
	SCMD_SHL = 0x57,
	SCMD_SHR = 0x58,
	SCMD_INV = 0x59,

	SCMD_FTOI = 0x5A,
	SCMD_FTOB = 0x5B,
	SCMD_ITOF = 0x5C,
	SCMD_ITOB = 0x5D,
	SCMD_BTOI = 0x5E,
	SCMD_BTOF = 0x5F,
	SCMD_CTOI = 0x60,
	SCMD_CTOF = 0x61,
	SCMD_CTOB = 0x62,

	SCMD_JMP = 0x63,
	SCMD_JMPF = 0x64,

	SCMD_CALLU = 0x65,
	SCMD_CALL1 = 0x66,
	SCMD_CALL2 = 0x67,
	SCMD_CALL3 = 0x68,
	SCMD_CALL4 = 0x69,
	SCMD_CALL5 = 0x6A,

	SCMD_SSEQU = 0x6B,
	SCMD_SSEQ1 = 0x6C,
	SCMD_SSEQ2 = 0x6D,
	SCMD_SSEQ3 = 0x6E,
	SCMD_SSEQ4 = 0x6F,
	SCMD_SSEQ5 = 0x70,

	SCMD_PSEQU = 0x71,
	SCMD_PSEQ1 = 0x72,
	SCMD_PSEQ2 = 0x73,
	SCMD_PSEQ3 = 0x74,
	SCMD_PSEQ4 = 0x75,
	SCMD_PSEQ5 = 0x76,

	SCMD_START = 0x77,
	SCMD_TSSTART = 0x78,
	SCMD_TPSTART = 0x79,
	SCMD_TEND = 0x7A,

	SCMD_RET = 0x7B,
	SCMD_SLEEP = 0x7C,
	SCMD_STATE = 0x7D,
	SCMD_ISTYPE = 0x7E,
	SCMD_NEW = 0x7F,
	SCMD_WAIT = 0x80,
	SCMD_PUSHD8 = 0x81,
	SCMD_PUSHD16 = 0x82,
	SCMD_PUSHD32 = 0x83,
	SCMD_PUSHDA = 0x84,
	SCMD_MOVEP = 0x85,
	SCMD_NUM_BYTECODE = 0x86
};

}
