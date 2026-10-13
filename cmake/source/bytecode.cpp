#include <ms/bytecode.hpp>
#include <ms/runtime.hpp>

MS_BEGIN

void CBytecode::Execute(Runtime::CRuntime& p_Runtime) {
	try {
		Runtime::INSTRUCTIONS[instruction](p_Runtime, arg1, arg2, arg3);
	}
	catch (Runtime::CRuntimeError e) {
		std::println("A runtime error has occured! (what: {})", e.what());
		exit(1);
	}
}

MS_END