#include <ms/bytecode.hpp>

void CBytecode::Execute() {
	INSTRUCTIONS[instruction](arg1, arg2, arg3);
}