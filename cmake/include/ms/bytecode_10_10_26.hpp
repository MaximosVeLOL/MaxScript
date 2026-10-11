#pragma once

#include <ms/types.hpp>


enum EInstruction : u8 {
	IN_NOP = 0, //This is only compile-time, should throw an error in runtime
	IN_MODULE,
	IN_SETVAR,
	IN_PLACEHOLDER,
	IN_MATH,
	IN_CALL,
	IN_COUNT
};

#define INSTRUCTION_ADDPART(insName) namespace EInstruction_##insName##_part {

#define INSTRUCTION_ENDPART }

INSTRUCTION_ADDPART(MODULE)

enum EType : u8 {
	T_IMPORT = 0,
	T_DEFINE = 1,
	T_USEREGION = 2,
};

INSTRUCTION_ENDPART

INSTRUCTION_ADDPART(SETVAR)

enum EFlag : u8 {
	F_UNSIGNED = 1 << 0,
	F_CALLABLE = 1 << 1,
	F_ARRAY = 1 << 2,
};

//Basic types end at 0x07,
//User defined types are based on basic types
enum EType : u8 {
	TYPE_INT8 = 0,
	TYPE_INT16 = 1,
	TYPE_INT32 = 2,
	TYPE_INT64 = 3,
	TYPE_FLOAT16 = 4,
	TYPE_FLOAT32 = 5, //Regular int
	TYPE_FLOAT64 = 6, //doubles
	TYPE_BOOL = 7,
	TYPE_CHAR = 8,
	TYPE_VOID = 9,
	TYPE_STRING = 10,
	TYPE_STRUCT = 11,
	TYPE_LITERALREFERENCE = 12, //Points to constant memory references (const char[] s)
	TYPE_COUNT = 13,
};

INSTRUCTION_ENDPART

INSTRUCTION_ADDPART(MATH)

enum EType : u8 {
	TYPE_ADD = 0,
	TYPE_SUB = 1,
	TYPE_MUL = 2,
	TYPE_DIV = 3,
	TYPE_POW = 4,
	TYPE_SQRT = 5,
};

INSTRUCTION_ENDPART


struct CArguments {
	u32 m_Argument1 = 0;
	u32 m_Argument2 = 0;
	u32 m_Argument3 = 0;
};

struct CBytecode : CArguments {
	EInstruction m_Instruction = IN_NOP;
	u8 m_Bitflags = 0;
	u16 m_SubType = 0;

	CBytecode() {}
	CBytecode(EInstruction p_Instruction) : m_Instruction(p_Instruction) {}
};