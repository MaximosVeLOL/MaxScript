#include <ms/systemfunc.hpp>
#include <ms/bytecode.hpp>
#include <print>

MS_BEGIN

namespace Runtime {

	namespace Functional {

#define CREATE(nameString, name, params, code) \
class CSYSFUNCTION_##name##_DEFINITION : public CSystemFunction { \
public: \
	const char* GetName() override {return nameString;} \
	constexpr std::initializer_list<EVariableType> GetParams() override {return params;} \
	void Execute(CRuntime& p_Runtime) override code \
};

		CREATE("print", print, {VARIABLE_STATICREF}, {
			std::println("{}", p_Runtime.GetStringFromRegister(REG_R0));

			});

		LangStd::CList<CSystemFunction*> g_AllSysFunctions;

		void Initialize() {
			g_AllSysFunctions.push(new CSYSFUNCTION_print_DEFINITION());
		}


	}

}

MS_END