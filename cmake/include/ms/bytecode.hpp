#pragma once

#include <ms/types.hpp>
#include <functional>
#include <ms/common.hpp>

MS_BEGIN


enum EInstruction : u8 {
	IN_NOP = (u8)0,
	IN_STARTREGION, //id, nop, nop
	IN_ENDREGION, //nop, nop, nop
	//Variables
	IN_SETSTACK, //offset (0 = latest), var type, var value
	IN_SETHEAP, //offset (0 = latest), var type, var value
	IN_SETREG, //index (0 = latest), var type, var value
	//Calls a system function that is defined interpreter level
	IN_CALLSYS, //id (determined at lexile level), nop, nop (all arguments are put in the register beforehand)
	//Calls a user defined function
	IN_CALL, //id (determined at lexile level), nop, nop (all arguments are put in the register beforehand)

	IN_ADD, //variable, amount, output
	IN_SUB, //variable, amount, output
	IN_MUL, //variable, amount, output
	IN_DIV, //variable, amount, output
	IN_POW, //variable, amount, output
	IN_SQRT, //variable, amount, output



	IN_AMOUNT,
};
inline const char* EInstruction_TOSTRING[IN_AMOUNT] = {
	"NOP",
	"STARTREGION",
	"ENDREGION",
	"SETSTACK",
	"SETHEAP",
	"SETREG",
	"CALLSYS",
	"CALL",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"POW",
	"SQRT",
};

namespace Runtime {
	class CRuntime;
}

struct CBytecode {

	EInstruction instruction = IN_NOP;
	u32 arg1 = 0;
	u32 arg2 = 0;
	u32 arg3 = 0;


	void Execute(Runtime::CRuntime& p_Runtime);
	CBytecode() {}
	CBytecode(EInstruction p_In, u32 p_a1 = 0, u32 p_a2 = 0, u32 p_a3 = 0) : instruction(p_In), arg1(p_a1), arg2(p_a2), arg3(p_a3) {}
};


MS_END