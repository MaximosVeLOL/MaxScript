#include <ms/lexiler.hpp>
#include <ms/runtime.hpp>
using namespace ms;
int main() {
	Runtime::Functional::Initialize();
	CLexiler l;
	Runtime::CProgram program;
	l.ParseFile(&program);
	Runtime::CRuntime r = Runtime::CRuntime();
	r.Prepare();
	r.m_Program = program;
	r.ExecuteAll();

	return 0;
}