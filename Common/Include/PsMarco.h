#pragma once

// --- Compile-time feature flags (will move to runtime config in Phase 3.2) ---

#ifndef USE_SCOPE_ANGLE
#define USE_SCOPE_ANGLE 1
#endif

#ifndef USE_RANDOM_SEED
#define USE_RANDOM_SEED 1
#endif

#ifndef USE_EXPORT_EXPOSURE_MAP
#define USE_EXPORT_EXPOSURE_MAP 1
#endif

#ifndef USE_INCLUDE_PCVALUE_FOR_EXPOSURE_MAP
#define USE_INCLUDE_PCVALUE_FOR_EXPOSURE_MAP 1
#endif

#ifndef USE_EXPORT_HEIGHT_MAP
#define USE_EXPORT_HEIGHT_MAP 0
#endif

#ifndef USE_MAX_SLOPE_ANGLE
#define USE_MAX_SLOPE_ANGLE 0
#endif

#ifndef USE_POS_RELATIVE
#define USE_POS_RELATIVE 0
#endif

#ifndef USE_SIMPLE_PC_OUTPUT
#define USE_SIMPLE_PC_OUTPUT 0
#endif

#ifndef USE_OUTPUT_HEIGHT_MAP_CSV
#define USE_OUTPUT_HEIGHT_MAP_CSV 0
#endif

#ifndef USE_OUTPUT_ONLY_POSITIVE_HEIGHT
#define USE_OUTPUT_ONLY_POSITIVE_HEIGHT 0
#endif

#ifndef USE_CELLINFO_HEIGHT_FOR_POINT_INSTANCE
#define USE_CELLINFO_HEIGHT_FOR_POINT_INSTANCE 1
#endif

#ifndef USE_DISPLAY_HEIGHTMAP_MASK_RESULT
#define USE_DISPLAY_HEIGHTMAP_MASK_RESULT 1
#endif

#ifndef USE_OUTPUT_INSTANCE_IDSTRING
#define USE_OUTPUT_INSTANCE_IDSTRING 0
#endif

#ifndef USE_OUTPUT_HIGH_ROAD_DATA
#define USE_OUTPUT_HIGH_ROAD_DATA 0
#endif

#ifndef USE_ONLY_TOPLAYER_FOR_ROAD_DATA
#define USE_ONLY_TOPLAYER_FOR_ROAD_DATA 0
#endif

// --- Numeric constants ---
// Refactor: converted from #define macros to constexpr to get type safety and
// proper scoping. Removed duplicate #pragma once that was erroneously placed
// mid-file. PI renamed PS_PI to avoid colliding with system-header definitions.

constexpr double PS_PI = 3.14159265358979323846;

constexpr double UNAVAILBLE_NEG_HEIGHT    = -20000.0;
constexpr double UNAVAILBLE_POS_HEIGHT    =  20000.0;

constexpr int    HEIGHTMAP_MASK_NO_DATA   = 0;
constexpr int    HEIGHTMAP_MASK_HAS_DATA  = 1000;

constexpr double CAVE_DISTANCE_LIMIT_TREE = 150.0;
constexpr double CAVE_DISTANCE_LIMIT_POI  = 100.0;
constexpr double TREE_FROM_POI_DISTANCE_LIMIT = 20.0;