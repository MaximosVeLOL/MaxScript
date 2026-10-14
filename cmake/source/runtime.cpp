#include <ms/runtime.hpp>
#include <ms/systemfunc.hpp>
#include <ms/bytecode.hpp>

MS_BEGIN



namespace Runtime {

#pragma region CRuntime

	CRuntime::CRuntime() {
		Prepare();
	}
	CRuntime::~CRuntime() {
		delete[] m_Memory.heap;
	}

	void CRuntime::Load(const char* p_Path) {

	}

	void CRuntime::Prepare() {
		if (m_Memory.heap) {
			delete[] m_Memory.heap;
		}
		m_Memory.heap = new u8[1024]; //1 KB of memory
	}

	void CRuntime::ExecuteStep() {
		m_Program.code.at(m_ProgIndex++).Execute(*this);
	}

	void CRuntime::ExecuteAll() {
		for (m_ProgIndex = m_Program.entryIndex; m_ProgIndex < m_Program.code.size();m_ProgIndex++) {
			m_Program.code.at(m_ProgIndex++).Execute(*this);
		}
	}
	void CRuntime::EndCurrentFunction() {
		m_ProgIndex = m_CallIndexes.back();
		m_CallIndexes.pop();
	}

	Executable::CData* CRuntime::GetDataFromRegister(u32 p_ID) {
		return &m_Program.data.at(p_ID);
	}

	char* CRuntime::GetStringFromRegister(u32 p_ID) {
		return reinterpret_cast<char*>(m_Program.data.at(p_ID).content);
	}

	CVariable* CRuntime::GetVarRegister(ERegister p_Reg) {
		if (p_Reg > REG_R6) {
			throw std::runtime_error("We are getting a invalid register (not a CVariable register!)");
		}
		return reinterpret_cast<CVariable*>(m_Memory.registers[ERegister_OFFSETS[p_Reg]]);
	}
#pragma endregion

	instruction_t DEF_NOP = [](CRuntime& rt, u32 p1 = 0, u32 p2 = 0, u32 p3 = 0) -> bool {
		Log("NOP executed");
		return true;
		};
	instruction_t INSTRUCTIONS[IN_AMOUNT] = {
		DEF_NOP,
		[](CRuntime& p_Runtime, u32, u32, u32) -> bool {
			p_Runtime.EndCurrentFunction();
			return true;
		},
		DEF_NOP,
		//Set stack
		[](CRuntime& p_Runtime, u32 p_Offset, u32 p_Type, u32 p_Value) -> bool {
			if (p_Offset == 0) {
				for (u8 i = 0; i < 32;i++) {
					if (p_Runtime.m_Memory.stackFrees[i]) {
						p_Runtime.m_Memory.stackFrees[i] = false;
						p_Offset = (i * sizeof(CVariable));
					}
				}
			}
			CVariable* offset = reinterpret_cast<CVariable*>(p_Runtime.m_Memory.stack + p_Offset);
			//offset->m_Type = static_cast<EVariableType>(p_Type);
			offset->Initialize(static_cast<EVariableType>(p_Type), &p_Value);
			return true;
		},
		//Set heap
		[](CRuntime& p_Runtime, u32 p_Offset, u32 p_Type, u32 p_Value) -> bool {

			CVariable* offset = reinterpret_cast<CVariable*>(p_Runtime.m_Memory.heap + p_Offset);
			//offset->m_Type = static_cast<EVariableType>(p_Type);
			offset->Initialize(static_cast<EVariableType>(p_Type), &p_Value);
			return true;
		},
		//Set register
		[](CRuntime& p_Runtime, u32 p_Offset, u32 p_Type, u32 p_Value) -> bool {
			if (p_Offset == 0) {

			}
			CVariable* offset = reinterpret_cast<CVariable*>(p_Runtime.m_Memory.registers + ERegister_OFFSETS[static_cast<ERegister>(p_Offset)]);
			//offset->m_Type = static_cast<EVariableType>(p_Type);
			offset->Initialize(static_cast<EVariableType>(p_Type), &p_Value);
			return true;
		},
		[](CRuntime& p_Runtime, u32 p_ID, u32, u32) -> bool {
			Functional::g_AllSysFunctions.at(p_ID)->Execute(p_Runtime);
			return true;
		},
		[](CRuntime& p_Runtime, u32 p_ID, u32, u32) -> bool {
			//Hopefully this features no stupid errors!
			p_Runtime.m_CallIndexes.push(p_Runtime.m_ProgIndex + 1);
			p_Runtime.m_ProgIndex = p_Runtime.m_Program.funcData.at(p_ID).fileOffset;
			return true;
		},
		DEF_NOP,
		DEF_NOP,
		DEF_NOP,
		DEF_NOP,
		DEF_NOP,
		DEF_NOP,

	};
	//instruction_t INSTRUCTIONS[IN_AMOUNT] = { DEF_NOP };
}

MS_END