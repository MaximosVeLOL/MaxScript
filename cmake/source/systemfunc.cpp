#include <ms/systemfunc.hpp>
#include <ms/bytecode.hpp>

#include <print>

#define CREATE(nameString, name, params, code) \
class SYSFUNCTION_##name##_DEFINITION : public SystemFunction { \
public: \
	const char* GetName() override {return nameString;} \
	std::vector<EVariableType> GetParams() override {return params;} \
	void Execute() override code \
};

CREATE("print", print, {VARIABLE_STATICREF}, {
	std::println("{}", Runtime::GetStringFromRegister(REG_R0));

	});

std::vector<SystemFunction*> g_AllSysFunctions;

std::vector<SystemFunction*>& GetAllSysFunctions() {
	return g_AllSysFunctions;
}

void Prepare() {
	g_AllSysFunctions.push_back(new SYSFUNCTION_print_DEFINITION());
}