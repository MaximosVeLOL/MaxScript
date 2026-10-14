#pragma once

#include <ms/types.hpp>
#include <ms/common.hpp>
#include <ms/runtime.hpp>
#include <ms/impl/list.hpp>


MS_BEGIN


namespace Runtime {

	namespace Functional {

		class CSystemFunction {
		public:
			virtual const char* GetName() = 0;
			virtual u8 const* GetParams() = 0;
			virtual void Execute(CRuntime& p_Runtime) = 0;
		};
		//u32 since this can be modified by user, add a Count template initializer if you want to.
		extern MS_API LangStd::CList<CSystemFunction*> g_AllSysFunctions;

		extern MS_API void Initialize();
	}
}

MS_END