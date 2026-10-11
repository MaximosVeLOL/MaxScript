#include <ms/lexiler.hpp>
#include <ms/runtime.hpp>

int main() {
	Prepare();
	CLexiler l;
	l.ParseFile();
	Runtime::Prepare();
	Runtime::g_Program = &l.m_Output;
	Runtime::Execute();
	return 0;
}