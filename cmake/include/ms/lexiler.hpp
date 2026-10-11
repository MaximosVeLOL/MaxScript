#pragma once

#include <fstream>
#include <filesystem>

#include <string>
#include <vector>

#include <ms/types.hpp>
#include <ms/bytecode.hpp>

#include <ms/systemfunc.hpp>




/* How it works
* Use keywords and keyletters to determine what we are understanding, then take the input and try to understand it.


*/

enum EKeys : u8 {
	KEY_WORD_MODULE = 0,
	KEY_LETTER_LINEEND = 1,
	KEY_COUNT
};

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
	//std::vector<EKeys*> m_KeyDirectors;
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


enum ELexixerMode : u8 {
	LEX_FIND = 0,
	LEX_SUBWORD = 1,
	LEX_SUBLETTER = 2,
};


std::string GetSpacePartOfString(const std::string& p_String) {
	return p_String.substr(0, p_String.find(' '));
};


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
struct CFunctionDefinition {
	std::string m_Name;
	EVariableType retValue = VARIABLE_VOID;
	//This also determines the parameter count
	std::vector<EVariableType> parameterTypes;
	//bool isSystemFunction = true;
};

enum ELexilerError : u8 {
	LEXERR_NONE = 0,
	LEXERR_INVALID_SYNTAX,
	LEXERR_UNKNOWN_TYPE,
	LEXERR_PLACEHOLDER1,
	LEXERR_PLACEHOLDER2,

};

class CCompilerError : std::runtime_error {
public:
	CCompilerError(const char* p_Message) : std::runtime_error(p_Message) {}
};


class CLexiler {
private:
	std::ifstream m_File;
	size_t m_Index = 0;
	std::vector<CKeyBase*> m_Keys;

public:
	ELexixerMode m_Mode = LEX_FIND;
	CKeyBase* m_CurrentKey = nullptr;
	std::vector<CVariableDefinition> m_Variables;
	std::vector<CFunctionDefinition> m_Functions;
	CProgram m_Output;
	CFunctionDefinition* m_CurrentFunction = nullptr;

	ELexilerError ParseFile() {
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
		while (std::getline(m_File, curLine)) {

			Log("Parsing line...");
			if (curLine.empty()) {
				continue;
			}
			size_t lastSpace = 0;
			for (i = 0; i < curLine.length();i++) {
				const char& curChar = curLine.at(i);
				std::string s = "";
				s += curChar;
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

						if (!foundVar) {
							for (CVariableDefinition& d : m_Variables) {
								if (keywordStart == d.m_Name) {
									m_Output.code.push_back(m_Keys.at(0)->Execute(curLine)) ;
									foundVar = true;
									break;
								}
							}
						}
						else {
							m_Output.code.push_back(m_Keys.at(0)->Execute(curLine));
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
										m_Output.code.push_back(c);
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
								m_Output.code.push_back(c);
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

				for (CKeyBase* b : m_Keys) {
					if (b->Handle(s)) {
						if (b->s_GetWholeLine) {
							CBytecode c = b->Execute(curLine);
							if (c.instruction > IN_NOP)
								m_Output.code.push_back(c);
							break;
						}
						m_CurrentKey = b;
						break;
					}
				}
			}
		}
		return LEXERR_NONE;
	}

	CLexiler() {
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "int8", "int8" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "int16", "int16" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "int32", "int32" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "int64", "int64" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "float16", "float16" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "float32", "float32" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "float64", "float64" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "bool", "bool" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "char", "char" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "void", "void" });
		m_Variables.push_back(CVariableDefinition{ CVariableDefinition::VAR_BASICTYPE, "string", "string" });

		struct CKL_VarDefine : CKeyLetter {

			CKL_VarDefine() {
				s_GetWholeLine = true;
				//m_Letter = '=';
			}

			CBytecode Execute(const std::string& p_String) override {
				CBytecode ret;
				size_t space = p_String.find_first_of(' ');
				std::string varType = p_String.substr(0, space);
				bool isSigned = true;
				if (varType.contains("signed")) {
					if (varType.contains("unsigned")) {
						isSigned = false;
					}
					size_t i = space + 1;
					for (;i < p_String.length();i++) {
						if (p_String.at(i) == ' ') {
							break;
						}
					}
					varType = p_String.substr(space + 1, i - (space + 1));
					space = i;
				}
				size_t equals = p_String.find_first_of('=');



				EVariableType detectedType = VARIABLE_VOID;
				for (size_t i = 0; i < m_Parent->m_Variables.size();i++) {
					if (m_Parent->m_Variables.at(i).m_Name == varType) {
						detectedType = static_cast<EVariableType>(i);
						break;
					}
				}


				std::string varName;
				std::string varValue;
				if (equals < std::string::npos) { //We are only defining a variable, we are not giving it a value
					varName = p_String.substr(space + 1, (equals - 1) - (space + 1));
					std::erase(varName, ' ');
					varValue = p_String.substr(equals + 1);
					std::erase(varValue, ' ');
					Log("{} {} = {};", varType, varName, varValue);
					//throw CCompilerError("Defining variables without a value is not supported yet!");
					ret.instruction = IN_SETSTACK;
					ret.arg1 = 0; //Push to the latest stack
					ret.arg2 = static_cast<u32>(detectedType);
					int value = atoi(varValue.c_str());
					//Do not convert int -> u32, but treat the bytes the same. If unsigned, then we know what to do.
					ret.arg3 = static_cast<u32>(value);
					return ret;
				}
				size_t parenIndex = p_String.find('(');
				if (parenIndex == std::string::npos) {
					throw CCompilerError("Defining variables without a value is not supported! (yet)");
				}
				varName = p_String.substr(space + 1, (parenIndex) - (space + 1));
				std::erase(varName, ' ');
				//varValue = p_String.substr(equals + 1);
				//std::erase(varValue, ' ');

				Log("Got a function definition!");
				Log("{} {}();", varType, varName);
				Executable::CFunction* f = new Executable::CFunction();
				std::vector<EVariableType> params;
				//We have no arguments
				if (p_String.at(parenIndex + 1) == ')') {
					params.push_back(VARIABLE_VOID); //This is what C and C++ do, right?
				}
				else {
					size_t i = parenIndex + 1;
					while (true) {
						char cur = p_String.at(i++);
						//Calling function in function parameters
						if (cur == '(') {
							throw std::runtime_error("Calling functions in function parameters is not implemented! (yet)");
						}
						else if (cur == ')')
							break;
						if (cur == ',') {

						}

					}
				}
				m_Parent->m_Functions.push_back(CFunctionDefinition{ varName, detectedType, params });
				m_Parent->m_Output.funcData.push_back(Executable::CFunction{ static_cast<u8>(params.size()), m_Parent->m_Output.code.size() });
				m_Parent->m_CurrentFunction = &m_Parent->m_Functions.back();
				ret.instruction = IN_STARTREGION;
				ret.arg1 = REGION_FUNCTION;
				return ret;

			}
		};
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
				ret.arg1 = REGION_DATA;


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
				if (p_String.contains('{')) {
					return CBytecode(); //return NOP
				}
				size_t callIndex = p_String.find_first_of('(');
				std::string funcName = p_String.substr(0, callIndex);
				size_t lastIndex = 0;
				std::string arguments = p_String.substr(callIndex + 1, p_String.find_last_of(')') - (callIndex + 1));
				size_t len = arguments.length();
				struct stringDef {
					u8 argID = 0;
					u32 stringID = 0;
				};
				std::vector<stringDef> stringDefs;
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
				std::vector<std::string> argumentsVector;
				for (size_t i = 0; i < arguments.length();i++) {
					if (arguments.at(i) == '\"') [[unlikely]] {
						std::string curString;
						Executable::CData toAdd;
						//m_Parent->m_Output.data.push_back
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
						m_Parent->m_Output.data.push_back(toAdd);
						
						stringDefs.push_back(stringDef{static_cast<u8>(arguments.size()), static_cast<u32>(m_Parent->m_Output.data.size() - 1) });
					}
					else if (arguments.at(i) == ',' || i == arguments.length() - 1) {
						CKL_VarDefine var;
						Log("Found a parameter!");
						argumentsVector.push_back(arguments.substr(lastIndex, (i + 1) - lastIndex));
						lastIndex = i + 1;
					}
				}
				Log("{}({});", funcName, argumentsVector);

				CBytecode ret;
				std::vector<EVariableType> argTypes;
				std::vector<SystemFunction*>& allFuncs = GetAllSysFunctions();
				size_t i = 0;
				for (; i < allFuncs.size();i++) {
					SystemFunction* f = allFuncs.at(i);
					const char* name = f->GetName();
					if (funcName == name) {
						ret.instruction = IN_CALLSYS;
						ret.arg1 = i;
						argTypes = f->GetParams();
						break;
					}
				}
				if (i == allFuncs.size()) {
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
						
						for (stringDef& r : stringDefs) {
							if (r.argID == i) {
								argCode.arg3 = r.stringID;
							}
						}
					} break;
					}
					m_Parent->m_Output.code.push_back(argCode);
				}

				return ret;
			}
		};

		struct CKW_EndFunction : CKeyLetter {
			CKW_EndFunction() {
				m_Letter = '}';
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
				ret.arg1 = REG_RETVALUE;
				ret.arg2 = m_Parent->m_CurrentFunction->retValue;
				CVariable v;
				v.Initialize(m_Parent->m_CurrentFunction->retValue, p_String);
				bool error = false;
				u32 val = v.GetAsInteger(error);
				if (error) {
					throw std::runtime_error("Failed to get return value as an u32!");
				}
				ret.arg3 = val;

			}
		};

		m_Keys.push_back(new CKL_VarDefine());
		m_Keys.push_back(new CKL_StringConstant());
		m_Keys.push_back(new CKL_Call());


		for (CKeyBase* b : m_Keys) {
			b->m_Parent = this;
		}


		//m_Functions.push_back(CFunctionDefinition{ VARIABLE_VOID, {VARIABLE_STRING} });

	}
};