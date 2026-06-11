// Refactor (Phase 4.1): Groups the 46 CPlantsSimulation constructor parameters into 4 focused structs.
// Logic and member variables inside CPlantsSimulation are unchanged.
#pragma once
#include <string>
#include <cstdint>

// All input file paths fed into the simulation.
struct InputPaths {
    std::string outputDir;               // base output directory (also acts as an input context)
    std::string treeListCsv;
    std::string level1TreeListCsv;
    std::string topLayerImage;
    std::string topLayerImageMeta;
    // Heightmaps
    std::string meshHeightMap;           // BaseMeshes Level-0
    std::string mesh2HeightMap;          // BaseMeshes Level-1
    std::string pcHeightMap;             // TopLayer (point cloud)
    std::string l1HeightMap;             // Level-1 layer
    std::string bedrockHeightMap;
    // Heightmap masks
    std::string meshHeightMask;
    std::string mesh2HeightMask;
    std::string pcHeightMask;
    std::string l1HeightMask;
    std::string bedrockHeightMask;
    std::string lakesHeightMask;
    std::string level1LakesHeightMask;
    std::string oceanHeightMask;
    // Point files
    std::string mostTravelledPointFile;
    std::string mostDistantPointFile;
    std::string level1PoiPointFile;
    std::string centroidPointFile;
    // Cave / dungeon data
    std::string cavesPointCloudLevel0;
    std::string cavesPointCloudLevel1;
    std::string dungeonsPOILevel0;
    std::string dungeonsPOILevel1;
    // Region data
    std::string regionsRaw;
    std::string regionsInfo;
};

// Paths for all output files produced by the simulation.
struct OutputPaths {
    std::string outputFileLevel0;
    std::string fullOutputFileLevel0;
    std::string pcFullOutputFileLevel0;
    std::string outputFileLevel1;
    std::string fullOutputFileLevel1;
    std::string pcFullOutputFileLevel1;
};

// Tuneable simulation parameters (forest growth model).
struct SimParams {
    int32_t lod              = 0;
    float   forestAge        = 300.0f;
    int     iteration        = 100;
    int     gridDelta        = 30;       // grid sampling density (highest impact)
    double  initialDensity   = 0.1;      // initial tree spawn rate
    double  seedDensity      = 0.001;    // seed generation rate per mature tree
    double  competitionFactor = 0.9;     // crown overlap competition strength
    double  growthFactor     = 0.7;      // crown radius growth rate
    double  thinningThreshold = 1.0;     // final mask filter cutoff
};

// Tile / spatial layout describing which tile of the world grid to process.
struct TileConfig {
    int tiles                  = 1;
    int tileX                  = 0;
    int tileY                  = 0;
    int tileScale              = 1;
    int roadHeightMapScaleWidth  = 300;
    int roadHeightMapScaleHeight = 300;
};

// Top-level config passed to CPlantsSimulation. Add a new parameter by extending
// the appropriate sub-struct — no constructor signature change required.
struct SimulationConfig {
    InputPaths  input;
    OutputPaths output;
    SimParams   sim;
    TileConfig  tile;
};
