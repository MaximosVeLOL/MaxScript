#include <ms/runtime.hpp>
#include <functional>
#include <ms/systemfunc.hpp>

u8* MEM_HEAP = nullptr;
u8 MEM_STACK[sizeof(CVariable) * 32]{0};
bool MEM_STACK_FREES[32]{ true }; //True if free
u8 MEM_REGISTERS[(sizeof(CVariable) * 6) + 16 + (/*Safe space*/8)]{0};



#define AS_KILOBYTE(x) ((x) * 1024)
#define AS_MEGABYTE(x) ((x) * (1024 * 1024) )
#define AS_GIGABYTE(X) ((x) * (1024 * 1024 * 1024) )

instruction_t DEF_NOP = [](u32 p1 = 0, u32 p2 = 0, u32 p3 = 0) -> bool {
	Log("NOP executed");
	return true;
};

instruction_t INSTRUCTIONS[IN_AMOUNT] = {
	DEF_NOP,
	DEF_NOP,
	DEF_NOP,
	//Set stack
	[](u32 p_Offset, u32 p_Type, u32 p_Value) -> bool {
		if (p_Offset == 0) {
			for (u8 i = 0; i < 32;i++) {
				if (MEM_STACK_FREES[i]) {
					MEM_STACK_FREES[i] = false;
					p_Offset = (i * sizeof(CVariable));
				}
			}
		}
		CVariable* offset = reinterpret_cast<CVariable*>(MEM_STACK + p_Offset);
		//offset->m_Type = static_cast<EVariableType>(p_Type);
		offset->Initialize(static_cast<EVariableType>(p_Type), &p_Value);
		return true;
	},
	//Set heap
	[](u32 p_Offset, u32 p_Type, u32 p_Value) -> bool {

		CVariable* offset = reinterpret_cast<CVariable*>(MEM_HEAP + p_Offset);
		//offset->m_Type = static_cast<EVariableType>(p_Type);
		offset->Initialize(static_cast<EVariableType>(p_Type), &p_Value);
		return true;
	},
	//Set register
	[](u32 p_Offset, u32 p_Type, u32 p_Value) -> bool {
		if (p_Offset == 0) {

		}
		CVariable* offset = reinterpret_cast<CVariable*>(MEM_HEAP + p_Offset);
		//offset->m_Type = static_cast<EVariableType>(p_Type);
		offset->Initialize(static_cast<EVariableType>(p_Type), &p_Value);
		return true;
	},
	[](u32 p_ID, u32, u32) -> bool {
		GetAllSysFunctions().at(p_ID)->Execute();
		return true;
	}

};



namespace Runtime {

	CProgram* g_Program = nullptr;

	void Prepare() {
		if (MEM_HEAP) {
			delete[] MEM_HEAP;
		}
		MEM_HEAP = new u8[1024]; //1 KB of memory
	}

	void Execute() {

		for (size_t i = g_Program->funcData.at(0).fileOffset; i < g_Program->code.size();i++) {
			CBytecode& c = g_Program->code.at(i);
			c.Execute();
		}
	}
}