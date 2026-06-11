#pragma once
// Refactor (Phase 1.1/1.2): single source of truth for all per-species tuning values.
// Eliminated 16 species-specific DensityMap subclasses and 4 TreeClass subclasses;
// all species are now created via CTreeSpeciesClass(PlantType) using this table.

// Refactor (Phase 2.2): normalized to forward-slash includes; dropped #if __APPLE__ block.
#include "../Common/include/PsMarco.h"
#include "../Common/include/CColonizationTree.h"

// Parameters for one density map type.
struct DensityParams {
    double minval, maxval, ease;
};

// Everything that varies per tree species.
struct SpeciesConfig {
    PlantType    type;
    unsigned int color;
    int          matureAge;
    int          maxAge;
    int          seedRange;
    DensityParams height;
    DensityParams slope;
    DensityParams moisture;
    DensityParams roughness;
};

// Returns the config for the given species. Slope values depend on USE_SCOPE_ANGLE
// (angle-based radians vs raw slope units) — see CSlopeDensityMap.h for context.
inline const SpeciesConfig& GetSpeciesConfig(PlantType type)
{
#if USE_SCOPE_ANGLE
    static const SpeciesConfig kTable[] = {
        // Oak
        { PlantType::TREE_OAK,   0x00FF0000, 35, 350, 300,
          {1.0,   2500.0, 100.0},
          {0.0,   1.0,                       5.0 * (PS_PI / 180.0)},
          {0.10,  0.37,   0.1},
          {0.100, 0.298,  0.1} },
        // Maple
        { PlantType::TREE_MAPLE, 0x0000FF00, 35, 350, 300,
          {1.0,   2500.0, 100.0},
          {0.0,   1.0,                       5.0 * (PS_PI / 180.0)},
          {0.10,  0.37,   0.1},
          {0.100, 0.298,  0.1} },
        // Birch
        { PlantType::TREE_BIRCH, 0x00FFFF00, 30, 150, 500,
          {600.0, 2400.0, 800.0},
          {0.0,   25.0 * (PS_PI / 180.0),    5.0 * (PS_PI / 180.0)},
          {0.15,  0.85,   0.1},
          {0.100, 0.297,  0.1} },
        // Fir
        { PlantType::TREE_FIR,   0x00D2B48C, 40, 300, 500,
          {600.0, 3300.0, 600.0},
          {0.0,   40.0 * (PS_PI / 180.0),   10.0 * (PS_PI / 180.0)},
          {0.30,  0.90,   0.1},
          {0.135, 0.299,  0.1} },
    };
#else
    static const SpeciesConfig kTable[] = {
        // Oak
        { PlantType::TREE_OAK,   0x00FF0000, 35, 350, 300,
          {1.0,   2500.0, 100.0},
          {5.0,   30.0,    5.0},
          {0.10,  0.37,    0.1},
          {0.100, 0.298,   0.1} },
        // Maple
        { PlantType::TREE_MAPLE, 0x0000FF00, 35, 350, 300,
          {1.0,   2500.0, 100.0},
          {5.0,   30.0,    5.0},
          {0.10,  0.37,    0.1},
          {0.100, 0.298,   0.1} },
        // Birch
        { PlantType::TREE_BIRCH, 0x00FFFF00, 30, 150, 500,
          {600.0, 2400.0, 800.0},
          {0.0,   25.0,    5.0},
          {0.15,  0.85,    0.1},
          {0.100, 0.297,   0.1} },
        // Fir
        { PlantType::TREE_FIR,   0x00D2B48C, 40, 300, 500,
          {600.0, 3300.0, 600.0},
          {0.0,   40.0,   10.0},
          {0.30,  0.90,    0.1},
          {0.135, 0.299,   0.1} },
    };
#endif
    for (const auto& cfg : kTable)
        if (cfg.type == type) return cfg;
    return kTable[0];  // fallback: Oak
}
