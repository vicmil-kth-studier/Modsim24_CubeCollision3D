#pragma once
#define USE_DEBUG_TMP USE_DEBUG
#undef USE_DEBUG
#include "L11_emscripten_help.h"
#define USE_DEBUG USE_DEBUG_TMP
#undef USE_DEBUG_TMP