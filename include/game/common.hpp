#include <iostream>
#include <array>
#include <map>
#include <vector>
#include <optional>
#include <bitset>
#include <algorithm>
#include <tuple>
#include <variant>
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
