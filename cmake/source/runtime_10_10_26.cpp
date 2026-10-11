#include <ms/runtime.hpp>

#define DECLARE_INSTRUCTION(name) void INSTRUCTION_##name##_(const CBytecode& b)
#define GET_INSTRUCTION(name) INSTRUCTION_##name##_

CRuntime* g_Runtime = nullptr;



DECLARE_INSTRUCTION(NOP) {
	throw CRuntimeError("There should be an operation!");
}

DECLARE_INSTRUCTION(MODULE) {
	switch (b.m_SubType) {
	case EInstruction_MODULE_part::T_IMPORT:

		break;
	case EInstruction_MODULE_part::T_DEFINE:

		break;
	case EInstruction_MODULE_part::T_USEREGION:

		break;
	}
}
DECLARE_INSTRUCTION(SETVAR) {
	using namespace EInstruction_SETVAR_part;
	EType type = static_cast<EType>(b.m_Argument1);
	constexpr u8 SIZES[TYPE_COUNT] = {
		//Ints
		1,
		2,
		4,
		8,
		//Floats
		2,
		4,
		8,
		//Bool
		1,
		//Char
		1,
		//Void
		0,
		//All dynamic types
		0,
		0,
		0
	};
	const u8 size = SIZES[type];
	const u32& value = b.m_Argument2;

}
DECLARE_INSTRUCTION(PLACEHOLDER) {
	Log("Placeholder instruction reached, ignoring...");
}
DECLARE_INSTRUCTION(MATH) {
	using namespace EInstruction_MATH_part;
	EType t = static_cast<EType>(b.m_Argument1);
	const u32& funcName = b.m_Argument2;

}

void(*ALL_INSTRUCTIONS[])(const CBytecode&) = {
	GET_INSTRUCTION(NOP),
	GET_INSTRUCTION(MODULE),
	GET_INSTRUCTION(SETVAR),
	GET_INSTRUCTION(PLACEHOLDER),
	GET_INSTRUCTION(MATH),
};

CRuntime::CRuntime() {
	g_Runtime = this;
}