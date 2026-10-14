#include <ms/lexiler.hpp>
#include <ms/runtime.hpp>
using namespace ms;
int main() {
	Runtime::Functional::Initialize();
	CLexiler l;
	Runtime::CProgram program;
	ELexilerError error = l.ParseFile(&program);
	if (error > LEXERR_NONE) {
		Log("Failed to compile the program! Returning...");
		return error;
	}
	program.DebugPrint();
	Runtime::CRuntime r = Runtime::CRuntime();
	r.Prepare();
	r.m_Program = program;
	r.ExecuteAll();

	return 0;
}