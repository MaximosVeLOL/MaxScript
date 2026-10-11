#pragma once



#include <vector>
#include <stdexcept>
#include <ms/types.hpp>
#include <ms/bytecode.hpp>


extern u8 *MEM_HEAP;
extern u8 MEM_STACK[];
extern bool MEM_STACK_FREES[]; //True if free
extern u8 MEM_REGISTERS[];

using instruction_t = std::function<bool(u32, u32, u32)>;

extern instruction_t INSTRUCTIONS[];

//Indexes for std::vector<CBytecode> indexes
//using byteindex_t = size_t;

//using bytecodes_t = std::vector<CBytecode>;

class CRuntimeError : std::runtime_error {
public:
	CRuntimeError(const char* p_Message) : std::runtime_error(p_Message) {

	}
};
class CRuntimeError_Unimplemented : CRuntimeError {
public:
	CRuntimeError_Unimplemented() : CRuntimeError("Unimplemented!") {}
};

namespace Executable {
	struct CData {
		//u32 id = 0;
		u16 contentSize = 0;
		u8* content = nullptr;
	};
	struct CFunction {
		//u32 id = 0;
		u8 neededStackVariables = 0;
		size_t fileOffset = 0;
	};
}

enum EVariableType : u8 {
	VARIABLE_INT8 = 0,
	VARIABLE_INT16 = 1,
	VARIABLE_INT32 = 2,
	VARIABLE_INT64 = 3,

	VARIABLE_FLOAT16 = 4,
	VARIABLE_FLOAT32 = 5,
	VARIABLE_FLOAT64 = 6,

	VARIABLE_BOOL = 7,
	VARIABLE_CHAR = 8,
	VARIABLE_VOID = 9,
	VARIABLE_STRING = 10,
	VARIABLE_STRUCT = 11,
	VARIABLE_POINTER = 12, //Arrays, strings, ect.
	VARIABLE_STATICREF = 13, //Const CData strings
	VARIABLE_COUNT
};




enum ERegionType : u8 {
	REGION_DATA = 0, //Will always have data of information before real data
	REGION_FUNCTION = 1,
};

struct CVariable {

	const u8 SIZES[VARIABLE_COUNT] = {
		1,
		2,
		4,
		8,

		2,
		4,
		8,

		1,
		1,
		0,

		0,
		0,
	};

	bool m_IsSigned = true;
	EVariableType m_Type = VARIABLE_VOID;
	void* m_Value = nullptr;
	template<typename T>
	T GetValue() {
		switch (m_Type) {
		case VARIABLE_INT8:
		case VARIABLE_BOOL:
		case VARIABLE_CHAR:
			if (sizeof(m_Value) != 1)
				throw CRuntimeError("(CVariable::GetValue) Value is not of requested type!");
			break;
		}
		return *static_cast<T*>(m_Value);
	}
	u32 GetAsInteger(bool& p_Error) {
		if (m_Type > VARIABLE_INT64) {
			p_Error = true;
			return 0;
		}
		p_Error = false;
		return GetValue<u32>();
	}
	u32 GetAsFloat(bool& p_Error) {
		if (m_Type <= VARIABLE_INT64) {
			float value = static_cast<float>(GetValue<u32>());
			p_Error = false;
			return value;
		}
		else if (m_Type) {
		}
	}
	template<typename T>
	T& GetReference() {
		return *static_cast<T*>(m_Value);
	}

	void CreateValue(void* p_Value) {
		switch (m_Type) {
		case VARIABLE_INT8:
		case VARIABLE_BOOL: {
			if (m_IsSigned) {
				s8* v = new s8();
				*v = *static_cast<s8*>(p_Value);
				m_Value = static_cast<void*>(v);
			}
			u8* v = new u8();
			*v = *static_cast<u8*>(p_Value);
			m_Value = static_cast<void*>(v);
		}	break;
		case VARIABLE_CHAR: {
			char* v = new char();
			*v = *static_cast<char*>(p_Value);
			m_Value = static_cast<void*>(v);
		}	break;

		case VARIABLE_INT16: {
			if (m_IsSigned) {
				s16* v = new s16();
				*v = *static_cast<s16*>(p_Value);
				m_Value = static_cast<void*>(v);
			}
			u16* v = new u16();
			*v = *static_cast<u16*>(p_Value);
			m_Value = static_cast<void*>(v);
		}	break;
		case VARIABLE_INT32: {
			if (m_IsSigned) {
				s32* v = new s32();
				*v = *static_cast<s32*>(p_Value);
				m_Value = static_cast<void*>(v);
			}
			u32* v = new u32();
			*v = *static_cast<u32*>(p_Value);
			m_Value = static_cast<void*>(v);
		}	break;
		case VARIABLE_INT64: {
			throw CRuntimeError("int64 is not implemented! (yet)");
		}		break;

		case VARIABLE_FLOAT16: {
			throw CRuntimeError("float16 is not implemented! (yet)");
		}			break;

		case VARIABLE_FLOAT32: {

			float* v = new float();
			*v = *static_cast<float*>(p_Value);
			m_Value = static_cast<void*>(v);
			break;
		}		case VARIABLE_FLOAT64: {

			double* v = new double();
			*v = *static_cast<double*>(p_Value);
			m_Value = static_cast<void*>(v);
			break;
		}		case VARIABLE_VOID:
			throw CRuntimeError("voids are not meant to be used for variables.");
			break;
		case VARIABLE_STRING: {
		case VARIABLE_STRUCT:
			throw CRuntimeError("Strings/structs are not implemented! (yet)");
		}			break;

		default:
			throw CRuntimeError("Unknown variable type!");
			break;
		}
	}

	void Initialize(EVariableType p_Type, void* p_Value, bool p_IsSigned = true) {
		m_Type = p_Type;
		m_IsSigned = p_IsSigned;
		CreateValue(p_Value);
		
	}
	void Initialize(EVariableType p_Type, const std::string& p_Value) {
		m_Type = p_Type;
		size_t minus = p_Value.find('-');
		if (minus < std::string::npos) {
			m_IsSigned = true;
		}
		switch (p_Type) {
		case VARIABLE_INT8:
		case VARIABLE_BOOL: {
			int value = atoi(p_Value.data());

			if (m_IsSigned) {
				s8* v = new s8();
				*v = static_cast<s8>(value);
				m_Value = static_cast<void*>(v);
			}

			u8* v = new u8();
			*v = static_cast<s8>(value);
			m_Value = static_cast<void*>(v);
		}	break;
		case VARIABLE_CHAR: {
			char* v = new char();
			size_t backSlash = p_Value.find('\\');

			if (backSlash != std::string::npos) {
				throw CRuntimeError_Unimplemented();
			}
			else *v = static_cast<char>(p_Value.data()[1]); //Since this is parsed as 'a', we put the middle value, or if it has '\x1'
			m_Value = static_cast<void*>(v);
		}	break;

		case VARIABLE_INT16: {
			int value = atoi(p_Value.data());
			if (m_IsSigned) {
				s16* v = new s16();
				*v = static_cast<s16>(value);
				m_Value = static_cast<void*>(v);
			}
			u16* v = new u16();
			*v = static_cast<s16>(value);
			m_Value = static_cast<void*>(v);
		}	break;
		case VARIABLE_INT32: {
			int value = atoi(p_Value.data());
			if (m_IsSigned) {
				s32* v = new s32();
				*v = static_cast<s32>(value);
				m_Value = static_cast<void*>(v);
			}
			u32* v = new u32();
			*v = static_cast<u32>(value);
			m_Value = static_cast<void*>(v);
		}	break;
		case VARIABLE_INT64: {
			throw CRuntimeError("int64 is not implemented! (yet)");
		}		break;

		case VARIABLE_FLOAT16: {
			throw CRuntimeError("float16 is not implemented! (yet)");
		}			break;

		case VARIABLE_FLOAT32: {
			float value = atof(p_Value.data());
			float* v = new float();
			*v = static_cast<float>(value);
			m_Value = static_cast<void*>(v);
		}	break;
		case VARIABLE_FLOAT64: {
			float value = atof(p_Value.data());
			double* v = new double();
			*v = static_cast<double>(value);
			m_Value = static_cast<void*>(v);
		}	break;
		case VARIABLE_VOID:
			throw CRuntimeError("voids are not meant to be used for variables.");
			break;
		case VARIABLE_STRING: {
		case VARIABLE_STRUCT:
			throw CRuntimeError("Strings/structs are not implemented! (yet)");
		}		break;

		default:
			throw CRuntimeError("Unknown variable type!");
			break;
		}
		
	}
};



enum ERegisterType : u8 {
	REG_R0 = 0,
	REG_R1 = (sizeof(CVariable) * 1),
	REG_R2 = (sizeof(CVariable) * 2),
	REG_R3 = (sizeof(CVariable) * 3),
	REG_R4 = (sizeof(CVariable) * 4),
	REG_R5 = (sizeof(CVariable) * 5),
	REG_R6 = (sizeof(CVariable) * 6),
	//These can be up to 32 bit values
	REG_N1 = (REG_R6 + 1),
	REG_N2 = (REG_N1 + 4),
	REG_N3 = (REG_N2 + 4),
	REG_RETVALUE = (REG_N2 + 4), //This is a CVariable btw

};
struct CBytecode;
struct CProgram {
	std::vector<Executable::CData> data;
	std::vector<Executable::CFunction> funcData;
	std::vector<CBytecode> code;
};

inline CVariable* GetVarRegister(ERegisterType p_Type) {
	if (p_Type > REG_R6) {
		throw std::runtime_error("We are getting a invalid register (not a CVariable register!)");
	}
	return reinterpret_cast<CVariable*>(MEM_REGISTERS[p_Type]);
}



namespace Runtime {

	extern CProgram* g_Program;

	extern void Prepare();

	extern void Execute();

	inline Executable::CData* GetDataFromRegister(u32 p_ID) {
		return &g_Program->data.at(p_ID);
	}
	inline char* GetStringFromRegister(u32 p_ID) {
		return reinterpret_cast<char*>(g_Program->data.at(p_ID).content);
	}
}