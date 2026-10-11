#pragma once

#include <ms/types.hpp>
#include <ms/runtime.hpp>
class SystemFunction;
extern std::vector<SystemFunction*>& GetAllSysFunctions();

class SystemFunction {
public:
	virtual const char* GetName() { return nullptr; };
	virtual std::vector<EVariableType> GetParams() { return { VARIABLE_VOID }; };
	virtual void Execute() {};
};


extern void Prepare();

//extern std::vector<SystemFunction*> g_AllSysFunctions;