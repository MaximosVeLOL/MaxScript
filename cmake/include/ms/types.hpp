#pragma once

using s8 = signed __int8;
using u8 = unsigned __int8;
using s16 = signed __int16;
using u16 = unsigned __int16;
using s32 = signed __int32;
using u32 = unsigned __int32;


#include <print>
#include <format>

template<typename... Args>
void Log(const std::format_string<Args...> p_Format, Args&&... p_Args) {
	std::println(p_Format, p_Args...);
}