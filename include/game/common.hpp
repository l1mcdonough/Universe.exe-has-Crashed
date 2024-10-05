#include <iostream>
#include <array>
#include <map>
#include "raylib.h"
#include <rlgl.h>
#include <raymath.h>
#define RLIGHTS_IMPLEMENTATION
#include <external/rlights.h>
#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_WEB
#define GLSL_VERSION            100
#endif
