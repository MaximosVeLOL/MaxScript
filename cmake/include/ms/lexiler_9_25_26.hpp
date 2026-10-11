#pragma once

#include <string>
#include <thread>
#include <filesystem>
#include <fstream>
#include <vector>

//Debugging
#include <print>

#include <functional>

#include <ms/types.hpp>


namespace fs = std::filesystem;

#define DO_BASIC_STRING_LOOP(p_String) for(std::size_t i = 0; i < p_String.length();i++)
#define STRING_REMOVE(p_String) p_String.erase(std::remove_if(p_String.begin(), p_String.end(), isspace), p_String.end())

#include <ms/bytecode.hpp>

//bool gTaskFunctionExists = false;
//std::function<bool()> gTaskFunction;
bool g_Parser_InCode = false;



std::vector<CBytecode> g_OutputFile;

enum EVariableType : u8 {
	VARIABLE_INT8 = 0,
	VARIABLE_INT16,
	VARIABLE_INT32,
	VARIABLE_INT64,
	VARIABLE_BOOL,
	VARIABLE_FLOAT16,
	VARIABLE_FLOAT32,
	VARIABLE_STRING,
	VARIABLE_VOID,
	VARIABLE_CHAR,
	VARIABLE_COUNT
};
const char* EVariableType_AsString[VARIABLE_COUNT] = {
	"int8",
	"int16",
	"int32",
	"int64",
	"bool",
	"float16",
	"float32",
	"string",
	"void",
	"char",
};

template<typename... Args>
void Log(const std::format_string<Args...> p_Format, Args&&... p_Args) {
	std::println(p_Format, p_Args...);
}


//namespace k {
	struct IKeywordTemplate {
		const char* name = nullptr;
		bool waitForWholeLine = false;
		bool hasMultipleNames = false;
		virtual const char** GetNames(u8* p_OutCount) {
			return nullptr;
		}

		virtual CBytecode OnSelect(const std::string& p_SubString) {
			return CBytecode(IN_NOP);
		}
	};

	struct CKeywordVariable : IKeywordTemplate {
		CKeywordVariable() {
			name = nullptr;
			waitForWholeLine = true;
			hasMultipleNames = true;
		}
		const char** GetNames(u8* p_OutCount) {
			*p_OutCount = 8;
			const char** names = new const char* [] {
				"const",
				"signed",
				"unsigned",
				"int",
				"float",
				"bool",
				"string",
				"void",
				//"()",
				};
			return names;
		}
		CBytecode OnSelect(const std::string& p_SubString) override {
			bool isUnsigned = false;
			EVariableType type = VARIABLE_INT8;
			size_t equalsIndex = p_SubString.find('=');
			std::string beforeEquals = (equalsIndex != std::string::npos ? p_SubString.substr(0, equalsIndex) : p_SubString);
			//STRING_REMOVE(beforeEquals);
			size_t typeIndex = 0;

			if (beforeEquals.contains("unsigned ")) {
				isUnsigned = true;
				typeIndex += 9;
			}
			else if (beforeEquals.contains("signed ")) {
				isUnsigned = false;
				typeIndex += 7;
			}
			for (u8 i = 0; i < VARIABLE_COUNT;i++) {
				if (!p_SubString.contains(EVariableType_AsString[i]))
					continue;
				typeIndex += p_SubString.find_first_of(EVariableType_AsString[i]) + strlen(EVariableType_AsString[i]); //Include the space

				type = static_cast<EVariableType>(i);
				break;
			}
			std::string variableName = beforeEquals.substr(typeIndex);
			//We are callable!
			bool isCallable = variableName.contains("()");
			if (isCallable) {
				variableName = variableName.substr(0, variableName.find("()") + 2);
				g_Parser_InCode = true;
			}
			u8 bitFlags = static_cast<u8>((isUnsigned ? EInstruction_SETVAR_part::F_UNSIGNED : 0 << 0) | (isCallable ? EInstruction_SETVAR_part::F_CALLABLE : 0 << 1));

			CBytecode b;
			b.m_Instruction = IN_SETVAR;
			b.m_Bitflags = bitFlags;
			b.m_SubType = (u16)type;


			if (equalsIndex != std::string::npos) {
				std::string variableValue = p_SubString.substr(equalsIndex + 1);
				Log("{} {} = {}", EVariableType_AsString[type], variableName, variableValue);

				return b;
				//return CBytecode{ IN_SETVAR, static_cast<u8>((isUnsigned ? EInstruction_SETVAR_part::F_UNSIGNED : 0) | EInstruction_SETVAR_part::F_CALLABLE), (u16)type};
			}
			Log("{} {};", EVariableType_AsString[type], variableName);
			return b;
		}
	};

	struct CKeywordModule : IKeywordTemplate {
		CKeywordModule() {
			name = "module";
			waitForWholeLine = true;
		}

		CBytecode OnSelect(const std::string& p_SubString) override {
			//The entire thing
			CBytecode ret(IN_MODULE);
			size_t lastIndex = 0;
			DO_BASIC_STRING_LOOP(p_SubString) {
				if (p_SubString.at(i) == ' ') {
					std::string sub = p_SubString.substr(lastIndex, (lastIndex - i));
					lastIndex = i;
					if (sub.contains("module")) {
						continue;
					}
					if (sub.contains("import")) {
						std::string moduleName = p_SubString.substr(sub.find_last_of(' ') + 1);
						moduleName = moduleName.substr(0, moduleName.length() - 1);
						Log("Importing module {}", moduleName);
						ret.m_SubType = EInstruction_MODULE_part::T_IMPORT;
					}
					else if (sub.contains("define")) {
						std::string moduleName = p_SubString.substr(sub.find_last_of(' ') + 1);
						moduleName = moduleName.substr(0, moduleName.length() - 1);
						Log("Defining module {}", moduleName);
						ret.m_SubType = EInstruction_MODULE_part::T_DEFINE;
					}
				}
			}
		}
	};

	struct CKeywordComment : IKeywordTemplate {
		CKeywordComment() {
			name = "//";
			waitForWholeLine = true;
		}
		CBytecode OnSelect(const std::string& p_SubString) {
			Log("Got a comment ({})", p_SubString);
			return CBytecode(IN_NOP);
		}
	};

	struct CKeywordCall : IKeywordTemplate {
		CKeywordCall() {
			name = "(";
			waitForWholeLine = true;
		}
		CBytecode OnSelect(const std::string& p_SubString) {
			Log("Got a call function! {}", p_SubString);
		}
	};

	struct CKeywordCodeEnd : IKeywordTemplate {
		CKeywordCodeEnd() {
			name = "}";
			waitForWholeLine = false;
		}
		CBytecode OnSelect(const std::string& p_SubString) {
			g_Parser_InCode = false;
			return CBytecode(IN_NOP);
		}
	};

	enum EKeyword : u8 {
		KEYWORD_VARIABLE = 0,
		KEYWORD_MODULE,
		KEYWORD_COMMENT,
		KEYWORD_COUNT
	};

	IKeywordTemplate** ALL_KEYWORDS = new IKeywordTemplate*[KEYWORD_COUNT]{
		new CKeywordVariable(),
		new CKeywordModule(),
		new CKeywordComment(),
	};
//}

void ParseFile_HandleLine(const std::string& p_Line) {
	size_t lastIndex = 0;
	IKeywordTemplate* curKeyword = nullptr;
	std::vector<CBytecode> outCode;
	DO_BASIC_STRING_LOOP(p_Line) {
		const char& current = p_Line.at(i);
		/*
		if (gTaskFunctionExists) {
			if (gTaskFunction()) {
				gTaskFunctionExists = false;
			}
			else continue;
		}
		*/

		if (current == ' ') {
			for (u8 keyI = 0; keyI < KEYWORD_COUNT;keyI++) {
				std::string upToSpace = p_Line.substr(lastIndex, (lastIndex - i) - 1);
				if (ALL_KEYWORDS[keyI]->hasMultipleNames) {
					u8 nameCount = 0;
					const char** names = ALL_KEYWORDS[keyI]->GetNames(&nameCount);
					u8 nameI = 0;
					for (;nameI < nameCount;nameI++) {
						const char* curName = names[nameI];
						if (upToSpace.contains(curName) && upToSpace.find_first_of(curName) == 0) {
							curKeyword = ALL_KEYWORDS[keyI];
							if (curKeyword->waitForWholeLine) {
								curKeyword->OnSelect(p_Line);
								return;
							}
							curKeyword->OnSelect(upToSpace);
							break;
						}
					}
					if (nameI == nameCount)
						Log("Placeholder");
					continue;
				}
				if (upToSpace.contains(ALL_KEYWORDS[keyI]->name) && upToSpace.find_first_of(ALL_KEYWORDS[keyI]->name) == 0) {
					curKeyword = ALL_KEYWORDS[keyI];
					if (curKeyword->waitForWholeLine) {
						curKeyword->OnSelect(p_Line);
						return;
					}
					curKeyword->OnSelect(upToSpace);
					break;
				}
			}
		}
	}
	
}


void ParseFile(fs::path& p_Path) {
	std::ifstream file(p_Path);
	if (!file.is_open())
		return;
	while (true) {
		std::string line;
		std::getline(file, line);
		if (file.eof()) {
			break;
		}
		//std::thread t(ParseFile_HandleLine, line.substr(0, line.length() - (line.contains(";") ? 1 : 0) ));
		//t.join();
		ParseFile_HandleLine(line.substr(0, line.length() - (line.contains(";") ? 1 : 0)));
	}
}