#pragma once



//#include <vector>
#include <stdexcept>
#include <ms/types.hpp>
#include <ms/bytecode.hpp>
#include <ms/common.hpp>

#include <ms/impl/list.hpp>

MS_BEGIN

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

namespace Runtime {

	class CRuntime;

	using instruction_t = std::function<bool(CRuntime&, u32, u32, u32)>;

	extern instruction_t INSTRUCTIONS[];

	//Indexes for LangStd::CList<CBytecode> indexes
	//using byteindex_t = size_t;

	//using bytecodes_t = LangStd::CList<CBytecode>;

	
	class CRuntimeError : public std::runtime_error {
	public:
		template<typename... Args>
		CRuntimeError(std::format_string<Args...> p_Format, Args&&... p_Args) : std::runtime_error(std::format(p_Format, p_Args...)) {

		}
	};
	class CRuntimeError_Unimplemented : public CRuntimeError {
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
		struct CDebugInfo {
			LangStd::CList<const char*> funcNames;
			LangStd::CList<LangStd::CList<const char*>> funcVarNames;

		};
	}



	//UNUSED
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
#pragma region CVariable Variable Types
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
#pragma endregion
	};



	enum ERegister : u8 {
		REG_R0 = 0,
		REG_R1, //24
		REG_R2, //48
		REG_R3, //72
		REG_R4, //96
		REG_R5, //120
		REG_R6 = (sizeof(CVariable) * 6), //144
		//These can be up to 32 bit values
		REG_N1,
		REG_N2,
		REG_N3,
		REG_RETVALUE,
		REG_AMOUNT
	};
	const u8 ERegister_OFFSETS[REG_AMOUNT] = {
		0,
		(sizeof(CVariable) * 1),
		(sizeof(CVariable) * 2),
		(sizeof(CVariable) * 3),
		(sizeof(CVariable) * 4),
		(sizeof(CVariable) * 5),
		(sizeof(CVariable) * 6),
		((sizeof(CVariable) * 6) + 1),
		((sizeof(CVariable) * 6) + 5),
		((sizeof(CVariable) * 6) + 9),
		((sizeof(CVariable) * 6) + 13),

	};

	struct CProgram {
		LangStd::CList<Executable::CData> data;
		LangStd::CList<Executable::CFunction> funcData;
		LangStd::CList<CBytecode> code;
		size_t entryIndex = 0; //This points to where our code begins. (eg. beginning of main() functions)
		void DebugPrint() {
			std::println("CProgram List:");
			std::println("Data ({} entries)", data.length());
			for (Executable::CData& d : data) {
				std::print("(size = {})\t\"", d.contentSize);
				for (u16 i = 0; i < d.contentSize;i++) {
					std::print("{}", static_cast<char>(d.content[i]));
				}
				std::println("\"");
			}
			std::println("Function Data ({} entries)", funcData.length());
			for (Executable::CFunction& f : funcData) {
				std::println("offset: {}, parameter count: {}", f.fileOffset, f.neededStackVariables);
			}
			std::println("Bytecode:");
			for (u32 i = 0; i < code.length();i++) {
				CBytecode& b = code[i];
				std::println("({}) {} {} {} {}", i, EInstruction_TOSTRING[b.instruction], b.arg1, b.arg2, b.arg3);

			}


		}

	};

	class MS_API CRuntime {
	private:

	public:
		struct { //1032 - 56 bytes used
			u8* heap = nullptr;
			u8 stack[sizeof(CVariable) * 32] = { 0 };
			bool stackFrees[32] = { false };
			u8 registers[(sizeof(CVariable) * 6) + 16 + (/*Safe space*/8)] = { 0 };
		} m_Memory;

		size_t m_ProgIndex = 0;
		
		LangStd::CList<size_t> m_CallIndexes; //Contains the code indexes of all IN_CALLS executed, so we can return to them.

		void EndCurrentFunction();

		CProgram m_Program;

		void Load(const char* p_Path);

		void Prepare();

		void ExecuteStep();

		void ExecuteAll();

		Executable::CData* GetDataFromRegister(u32 p_ID);

		char* GetStringFromRegister(u32 p_ID);

		CVariable* GetVarRegister(ERegister p_Reg);

		CRuntime();
		~CRuntime();
	};
}

MS_END