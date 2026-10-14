#include <ms/systemfunc.hpp>
#include <ms/bytecode.hpp>
#include <print>

MS_BEGIN

namespace Runtime {

	namespace Functional {

#define CREATE(nameString, name, params, code) \
const u8 CSYSFUNCTION_##name##_PARAMSARRAY[] = params; \
class CSYSFUNCTION_##name##_DEFINITION : public CSystemFunction { \
public: \
	const char* GetName() override {return nameString;} \
	u8 const* GetParams() override {return CSYSFUNCTION_##name##_PARAMSARRAY;} \
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