#pragma once

#include <fstream>
//#include <filesystem>

#include <string>
#include <vector>

#include <ms/types.hpp>
#include <ms/bytecode.hpp>

#include <ms/systemfunc.hpp>
#include <ms/common.hpp>

#include <ms/runtime.hpp>

MS_BEGIN


/* How it works
* Use keywords and keyletters to determine what we are understanding, then take the input and try to understand it.


*/

//Keys are used to determine what a character/letter or word means.
#pragma region Keys
struct CLexiler;

struct CKeyBase {
	CLexiler* m_Parent = nullptr;
	bool s_GetWholeLine = false;
	bool s_IsDerived = false; //Do we rely on another key? (Get skipped when being used)
	virtual bool Handle(const std::string& p_String) { return false; }
	virtual void Initialize() {}
	virtual CBytecode Execute(const std::string& p_String) {
		return CBytecode();
	}
	virtual bool GetBreakCondition(const char& p_Current) { return false; }
	//Other keys that are derived or rely on this key
	//LangStd::CList<EKeys*> m_KeyDirectors;
};

struct CKeyword : CKeyBase {
	const char* m_Word = nullptr;
	bool Handle(const std::string& p_String) {
		return (p_String.contains(m_Word));
	}
};

struct CKeyLetter : CKeyBase {
	char m_Letter = 0x00;
	bool Handle(const std::string& p_String) {
		return (p_String.contains(m_Letter));
	}
};

#pragma endregion


enum ELexixerMode : u8 {
	LEX_FIND = 0,
	LEX_SUBWORD = 1,
	LEX_SUBLETTER = 2,
};


std::string GetSpacePartOfString(const std::string& p_String) {
	return p_String.substr(0, p_String.find(' '));
};

//Variable definition, from basic types to custom types, like classes or typedefs.
struct CVariableDefinition {
	enum EType : u8 {
		VAR_BASICTYPE = 0, //Standard types
		VAR_CUSTOMTYPE = 1, //User-defined types that use basic types (#define type type_t = int32)
		VAR_STRUCTURE = 2, //(class) and (struct)
	};
	EType m_Type = VAR_BASICTYPE;
	std::string m_Name;
	std::string m_RealDef;
};
//Function definition, these are user defined.
struct CFunctionDefinition {
	const char* m_Name; //BUG - When using std::string, I think the data would get freed, so it would throw an exception when m_Functions would be destroyed.
	EVariableType retValue = VARIABLE_VOID;
	//This also determines the parameter count
	LangStd::CList<EVariableType> parameterTypes;
	//bool isSystemFunction = true;
};

enum ELexilerError : u8 {
	LEXERR_NONE = 0,
	LEXERR_INVALID_SYNTAX,
	LEXERR_UNKNOWN_TYPE,
	LEXERR_PLACEHOLDER1,
	LEXERR_PLACEHOLDER2,

};

class CCompilerError : public std::runtime_error {
public:
	template<typename... Args>
	CCompilerError(std::format_string<Args...> p_Format, Args&&... p_Args) : std::runtime_error(std::format(p_Format, p_Args...)) {}
};


class CLexiler {
private:
	std::ifstream m_File;
	size_t m_Index = 0;
	LangStd::CList<CKeyBase*> m_Keys;

	CBytecode _handleVariable(const std::string& p_Line, EVariableType p_DetectedType) {
		CBytecode ret;
		size_t space = p_Line.find_first_of(' ');
		std::string varType = p_Line.substr(0, space);
		//TODO - Remove this because we already know if it is signed or unsigned before this.
		bool isSigned = true;
		if (varType.contains("signed")) {
			if (varType.contains("unsigned")) {
				isSigned = false;
			}
			size_t i = space + 1;
			for (;i < p_Line.length();i++) {
				if (p_Line.at(i) == ' ') {
					break;
				}
			}
			varType = p_Line.substr(space + 1, i - (space + 1));
			space = i;
		}
		size_t equals = p_Line.find_first_of('=');


		std::string varName;
		std::string varValue;
		if (equals < std::string::npos) { //We are only defining a variable, we are not giving it a value
			varName = p_Line.substr(space + 1, (equals - 1) - (space + 1));
			std::erase(varName, ' ');
			varValue = p_Line.substr(equals + 1);
			std::erase(varValue, ' ');
			Log("{} {} = {};", varType, varName, varValue);
			//throw CCompilerError("Defining variables without a value is not supported yet!");
			ret.instruction = IN_SETSTACK;
			ret.arg1 = 0; //Push to the latest stack
			ret.arg2 = static_cast<u32>(p_DetectedType);
			int value = atoi(varValue.c_str());
			//Do not convert int -> u32, but treat the bytes the same. If unsigned, then we know what to do.
			ret.arg3 = static_cast<u32>(value);
			return ret;
		}
		size_t parenIndex = p_Line.find('(');
		if (parenIndex == std::string::npos) {
			throw CCompilerError("Defining variables without a value is not supported! (yet)");
		}
		varName = p_Line.substr(space + 1, (parenIndex)-(space + 1));
		std::erase(varName, ' ');
		//varValue = p_Line.substr(equals + 1);
		//std::erase(varValue, ' ');

		//Log("Got a function definition!");
		//Log("{} {}();", varType, varName);
		for (CFunctionDefinition& f : m_Functions) {
			if (varName == f.m_Name) {
				throw CCompilerError("Function name ({}) already exists!", varName);
			}
		}
		Runtime::Executable::CFunction* f = new Runtime::Executable::CFunction();
		LangStd::CList<EVariableType> params;

		//Detect if we have no arguments
		if (p_Line.at(parenIndex + 1) == ')') {
			params.push(VARIABLE_VOID); //This is what C and C++ do, right?
		}
		else {
			size_t i = parenIndex + 1;
			size_t lastPart = i;
			while (true) {
				char cur = p_Line.at(i++);
				

				//Calling function in function parameters
				if (cur == '(' || cur == ';') {
					throw std::runtime_error("Got an unexpected character in the parameter list!");
				}
				else if (cur == ')')
					break;
				if (cur == ',') {
					//Includes type and name (eg: int32 p_Param)
					std::string part = p_Line.substr(lastPart, i - lastPart - 1);
					size_t space = part.find(' ');
					if (space == std::string::npos) {
						throw CCompilerError("Got no type for a function definition's parameter!");
					}
					std::string type = p_Line.substr(0, space - 1);
					//TODO - Replace using m_Variables as a base for an actual variable type
					for (size_t i = 0; i < m_Variables.size();i++) {
						if (type == m_Variables.at(i).m_Name) {
							params.push(static_cast<EVariableType>(i));
							break;
						}
					}
					if (i == m_Variables.size()) {
						throw CCompilerError("Got an unknown type for a function's parameter!");
					}
				}

			}
		}
		char* theName = new char[varName.length() + 1] {'\0'};
		std::memcpy(theName, varName.data(), varName.length());
		//I'm gonna nut all over the place if this doesn't work
		CFunctionDefinition defToAdd = { theName, p_DetectedType, params};
		m_Functions.push_back(defToAdd);
		m_Output->funcData.push(Runtime::Executable::CFunction{ static_cast<u8>(params.size()), m_Output->code.size() + 1 });
		m_CurrentFunction = &m_Functions.back();
		ret.instruction = IN_STARTREGION;
		ret.arg1 = Runtime::REGION_FUNCTION;
		if (varName == "main") {
			if (m_Output->entryIndex != 0)
				throw CCompilerError("We already have an entry point! (Are two main functions defined?) (This should already be errored)");
			m_Output->entryIndex = m_Output->code.size() + 1;
		}
		return ret;

	}
public:
	ELexixerMode m_Mode = LEX_FIND;
	CKeyBase* m_CurrentKey = nullptr;
	LangStd::CList<CVariableDefinition> m_Variables;
	//TODO - Replace!
	std::vector<CFunctionDefinition> m_Functions;
	Runtime::CProgram* m_Output = nullptr;
	CFunctionDefinition* m_CurrentFunction = nullptr;

	

	ELexilerError ParseFile(Runtime::CProgram* p_Output) {
		m_Output = p_Output; //Reference it for stuff
		m_File.open("./test.mxx");
		if (!m_File.is_open()) {
			return LEXERR_UNKNOWN_TYPE;
		}
		std::string curLine;

		size_t lastIndex = 0;
		size_t i = 0;
		auto getLineAsSubString = [curLine, lastIndex, i]() -> std::string {
			return curLine.substr(lastIndex, curLine.length() - (i - lastIndex));
		};
		auto getSpacePartOfString = [](const std::string& p_String) -> std::string {
			return p_String.substr(0, p_String.find(' '));
		};
		size_t subStartIndex = 0;
		bool gotWholeLine = false;
		try {
			while (std::getline(m_File, curLine)) {

				Log("Parsing line...");
				if (curLine.empty()) {
					continue;
				}
				size_t lastSpace = 0;
				size_t curLineLength = curLine.length();
				for (i = 0; i < curLineLength;i++) {
					const char& curChar = curLine.at(i);
					if (curChar == '\n' || curChar == '\t') {
						curLine = curLine.substr(i + 1);
						i -= 1;
						curLineLength = curLine.length();
						continue;
					}
					switch (curChar) {
					case ' ': {
						size_t lI = lastSpace;
						for (; lI < curLine.length();lI++) {
							if (curLine.at(lI) == ' ') {
								break;
							}
						}
						std::string keywordStart = curLine.substr(lastSpace, lI - lastSpace);
						lastSpace = lI + 1;


						if (m_Mode == LEX_FIND) {

							bool foundVar = (keywordStart.contains("signed"));
							//TODO - Optimize by merging these two.
							if (!foundVar) {
								//TODO - When custom types are added, this breaks. Make it so that variables are placed there instead!
								for (size_t i = 0; i < m_Variables.size();i++) {
									const CVariableDefinition& d = m_Variables.at(i);
									if (keywordStart == d.m_Name) {
										m_Output->code.push(_handleVariable(curLine, static_cast<EVariableType>(i)));

										foundVar = true;
										break;
									}
								}
							}
							else {
								size_t index = curLine.find(' ', i + 1);
								if (index == std::string::npos) {
									throw CCompilerError("Failed to find variable type from unsigned/signed type");
								}
								std::string varType = curLine.substr(i, index - i);
								for (size_t i = 0; i < m_Variables.size();i++) {
									const CVariableDefinition& d = m_Variables.at(i);
									if (keywordStart == d.m_Name) {
										m_Output->code.push(_handleVariable(curLine, static_cast<EVariableType>(i)));
										foundVar = true;
										break;
									}
								}
							}
							if (foundVar) {
								gotWholeLine = true;
								break;
							}

							u8 keyI = 1;
							for (; keyI < m_Keys.size();keyI++) {
								CKeyBase* b = m_Keys.at(keyI);
								if (b->Handle(keywordStart)) {
									m_CurrentKey = b;
									b->Initialize();
									if (/*m_CurrentKey->m_KeyDirectors.size() == 0 &&*/m_CurrentKey->s_GetWholeLine) {
										CBytecode c = m_CurrentKey->Execute(curLine.substr(0, curLine.length() - 1));
										if (c.instruction > IN_NOP)
											m_Output->code.push(c);
										gotWholeLine = true;
										break;
									}
									subStartIndex = i;
									m_Mode = LEX_SUBWORD;
									break;
								}
							}
							//if (doBreak)
							//	break;
						}
						else if (m_Mode == LEX_SUBWORD) {
							if (m_CurrentKey->GetBreakCondition(curChar)) {
								CBytecode c = m_CurrentKey->Execute(curLine.substr(subStartIndex, i));
								if (c.instruction > IN_NOP)
									m_Output->code.push(c);
								m_CurrentKey = nullptr;

							}
							/*
							for (EKeys& k : m_CurrentKey->m_KeyDirectors) {
								if (m_Keys.at(k)->Handle(keywordStart)) {
									m_CurrentKey = m_Keys.at(k);
									Log("Detected key!");
								}
							}
							*/
						}
					}	break;

					case ';':
						/*
						if (m_Mode == LEX_FIND) {
							std::string linePart = getLineAsSubString();
							std::string varName = linePart.substr(0, linePart.find(' '));
							for (const CVariableDefinition& def : m_Variables) {
								if (def.m_Name.contains(varName)) {
									//Get variable here
								}
							}
							return LEXERR_UNKNOWN_TYPE;
						}
						*/
						break;
					}
					if (gotWholeLine) {
						gotWholeLine = false;
						break;
					}
					std::string curCharAsStr;
					curCharAsStr += curChar;
					for (CKeyBase* b : m_Keys) {
						if (b->Handle(curCharAsStr)) {
							if (b->s_GetWholeLine) {
								CBytecode c = b->Execute(curLine);
								if (c.instruction > IN_NOP)
									m_Output->code.push(c);
								break;
							}
							m_CurrentKey = b;
							break;
						}
					}
				}
			}
		}
		catch (CCompilerError e) {
			std::println("Failed to compile program! (error: {})", e.what());
			return LEXERR_UNKNOWN_TYPE;
		}

		return LEXERR_NONE;
	}

	CLexiler() {
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "int8", "int8" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "int16", "int16" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "int32", "int32" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "int64", "int64" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "float16", "float16" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "float32", "float32" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "float64", "float64" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "bool", "bool" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "char", "char" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "void", "void" });
		m_Variables.push(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "string", "string" });

		struct CKL_StringConstant : CKeyLetter {
			CKL_StringConstant() {
				m_Letter = '\"';
			}

			bool GetBreakCondition(const char& p_Current) override {
				return (p_Current == '\"');
			}

			CBytecode Execute(const std::string& p_String) override {
				//Expects "string"
				std::string pureContext = p_String.substr(1, p_String.length() - 2); //Skip the first quotation mark, and the last one
				CBytecode ret;
				ret.instruction = IN_STARTREGION;
				ret.arg1 = Runtime::REGION_DATA;


				return ret;
			}

		};
		
		struct CKL_Call : CKeyLetter {
			CKL_Call() {
				m_Letter = '(';
				s_GetWholeLine = true;
			}
			/*
			bool GetBreakCondition(const char& p_Current) override {
				return (p_Current == ')');
			}
			*/
			CBytecode Execute(const std::string& p_String) override {
				size_t callIndex = p_String.find_first_of('(');
				std::string funcName = p_String.substr(0, callIndex);
				size_t lastIndex = 0;
				std::string arguments = p_String.substr(callIndex + 1, p_String.find_last_of(')') - (callIndex + 1));
				size_t len = arguments.length();
				struct __StringDef {
					u8 argID = 0;
					u32 stringID = 0;
				};
				LangStd::CList<__StringDef> stringDefs;
				for (size_t i = 0; i < len;i++) {
					if (arguments.at(i) == ' ') {
						arguments.erase(i, 1);
						len = arguments.length();
						i -= i;
					}
					else if (arguments.at(i) == '\"') {
						i++;
						while (arguments.at(i++) != '\"');
					}
				}
				//std::erase(arguments, ' ');
				LangStd::CList<std::string> argumentsVector;
				for (size_t i = 0; i < arguments.length();i++) {
					if (arguments.at(i) == '\"') [[unlikely]] {
						std::string curString;
						Runtime::Executable::CData toAdd;
						//m_Parent->m_Output->data.push
						i++;
						while (true) {
							char cur = arguments.at(i++);
							if (cur == '\"')
								break;
							curString += cur;
						}
						toAdd.contentSize = curString.length() + 1;
						toAdd.content = new u8[toAdd.contentSize]{ 0x00 };
						memcpy(toAdd.content, curString.data(), curString.size());
						m_Parent->m_Output->data.push(toAdd);
						
						stringDefs.push(__StringDef{static_cast<u8>(arguments.size()), static_cast<u32>(m_Parent->m_Output->data.size() - 1) });
					}
					else if (arguments.at(i) == ',' || i == arguments.length() - 1) {
						Log("Found a parameter!");
						argumentsVector.push(arguments.substr(lastIndex, (i + 1) - lastIndex));
						lastIndex = i + 1;
					}
				}
				std::print("{}(", funcName);
				for (std::string& arg : argumentsVector) {
					std::print("{}, ", arg);
				}
				Log(")");
				CBytecode ret;
				LangStd::CList<EVariableType> argTypes;
				
				size_t i = 0;
				for (; i < Runtime::Functional::g_AllSysFunctions.size();i++) {
					Runtime::Functional::CSystemFunction* f = Runtime::Functional::g_AllSysFunctions.at(i);
					const char* name = f->GetName();
					if (funcName == name) {
						ret.instruction = IN_CALLSYS;
						ret.arg1 = i; //Function ID = i
						
						argTypes = f->GetParams();
						break;
					}
				}
				if (i == Runtime::Functional::g_AllSysFunctions.size()) {
					for (i = 0; i < m_Parent->m_Functions.size();i++) {
						CFunctionDefinition& d = m_Parent->m_Functions.at(i);
						if (funcName.compare(d.m_Name)) {
							ret.instruction = IN_CALL;
							ret.arg1 = i;
							argTypes = d.parameterTypes;
							break;
						}
					}
				}



				CBytecode argCode;
				argCode.instruction = IN_SETREG;
				for (i = 0; i < argumentsVector.size();i++) {
					argCode.arg1 = i;
					argCode.arg2 = argTypes.at(i);
					switch (argCode.arg2) {
					case VARIABLE_BOOL:
					case VARIABLE_CHAR:
					case VARIABLE_INT8:
					case VARIABLE_INT16:
					case VARIABLE_INT32:
						argCode.arg3 = atoi(argumentsVector.at(i).c_str());
						break;
					case VARIABLE_INT64:
						argCode.arg3 = atoll(argumentsVector.at(i).c_str());

						break;

					case VARIABLE_FLOAT16:
					case VARIABLE_FLOAT32:
						argCode.arg3 = atof(argumentsVector.at(i).c_str());

						break;

					case VARIABLE_FLOAT64:
						argCode.arg3 = atof(argumentsVector.at(i).c_str());

						break;

					case VARIABLE_STATICREF: {
						
						for (__StringDef& r : stringDefs) {
							if (r.argID == i) {
								argCode.arg3 = r.stringID;
							}
						}
					} break;
					}
					m_Parent->m_Output->code.push(argCode);
				}

				return ret;
			}
		};

		struct CKL_EndFunction : CKeyLetter {
			CKL_EndFunction() {
				m_Letter = '}';
				s_GetWholeLine = true;
			}
			CBytecode Execute(const std::string& p_String) {
				return CBytecode(IN_ENDREGION);
			}
		};

		struct CKW_Module : CKeyword {

		};

		struct CKW_Return : CKeyword {
			CKW_Return() {
				m_Word = "return";
				s_GetWholeLine = true;
			}
			CBytecode Execute(const std::string& p_String) {
				if (!m_Parent->m_CurrentFunction) {
					throw std::runtime_error("Trying to return a value when not in a function!");
				}
				CBytecode ret;
				ret.instruction = IN_SETREG;
				ret.arg1 = Runtime::REG_RETVALUE;
				ret.arg2 = m_Parent->m_CurrentFunction->retValue;
				Runtime::CVariable v;
				v.Initialize(m_Parent->m_CurrentFunction->retValue, p_String);
				bool error = false;
				u32 val = v.GetAsInteger(error);
				if (error) {
					throw std::runtime_error("Failed to get return value as an u32!");
				}
				ret.arg3 = val;
				return ret;
			}
		};

		m_Keys.push(new CKL_StringConstant());
		m_Keys.push(new CKL_Call());
		m_Keys.push(new CKW_Return());
		m_Keys.push(new CKL_EndFunction());


		for (CKeyBase* b : m_Keys) {
			b->m_Parent = this;
		}

	}
};

MS_END