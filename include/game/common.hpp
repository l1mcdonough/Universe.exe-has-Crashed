#include <iostream>
#include <array>
#include <map>
#include <vector>
#include <optional>
#include <bitset>
#include <algorithm>
#include <tuple>
#include <variant>
#include <format>
#include <string>
#include <sstream>
#include <filesystem>
#ifdef GRAPHICS_API_OPENGL_33
	#undef GRAPHICS_API_OPENGL_33
#endif
#ifndef GRAPHICS_API_OPENGL_43
	#define GRAPHICS_API_OPENGL_43
#endif
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include <rcamera.h>
//#define RLIGHTS_IMPLEMENTATION
//#include <external/rlights.h>
#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_WEB
#define GLSL_VERSION            100
#endif
#ifndef UNIVERSE_EXE_COMMON_HPP_HEADER_INCLUDE_GUARD 
#define UNIVERSE_EXE_COMMON_HPP_HEADER_INCLUDE_GUARD 
namespace Game
{
	template<typename... Args>
	inline auto dynaformat(const std::format_string<Args...> fmt, Args&&... args) // This function adapted from cppreference
	{
	    return std::vformat(fmt.get(), std::make_format_args(args...));
	}
	inline auto cat_impl(std::stringstream& ss, auto x, auto... xs)
	{
		ss << x;
		if constexpr(sizeof...(xs) > 0)
			return cat_impl(ss, xs...);
		else
			return ss.str();
	}
	inline auto cat(auto... xs) {
		std::stringstream ss;
		return cat_impl(ss, xs...);
	}
	const inline std::filesystem::path make_resource_path() {
		return std::filesystem::path(RESOURCE_DIRECTORY).make_preferred();
	}
	const static thread_local std::filesystem::path resource_path = make_resource_path();
	const inline std::filesystem::path shader_path(uint16_t glsl_version = 330) {
		auto glsl_folder = cat("glsl", glsl_version);
		return Game::resource_path / "shaders" / glsl_folder;
	}
}
#endif // UNIVERSE_EXE_COMMON_HPP_HEADER_INCLUDE_GUARD 

