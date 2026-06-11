#pragma once
// Refactor (Phase 1.2): replaced COakTreeClass, CMapleTreeClass, CBirchTreeClass,
// CFirTreeClass with a single CTreeSpeciesClass(PlantType). All per-species values
// (colors, ages, density params) come from the table in TreeSpeciesData.h.

#include "TreeSpeciesData.h"
#include "CHeightDensity.h"
#include "CSlopeDensityMap.h"
#include "CMoistureDensityMap.h"
#include "CRoughnessDensityMap.h"
#include "CRoadAttributeDensityMap.h"
#include "CSunLightAffinityDensityMap.h"

class CTreeSpeciesClass : public TreeClass
{
public:
    CTreeSpeciesClass(PlantType speciesType) : TreeClass()
    {
        const SpeciesConfig& cfg = GetSpeciesConfig(speciesType);
        typeId       = static_cast<unsigned int>(cfg.type);
        treeTypeName = PlantTypeToString(cfg.type);
        color        = cfg.color;
        matureAge    = cfg.matureAge;
        maxAge       = cfg.maxAge;
        seedRange    = cfg.seedRange;

        auto insertMask = [&](DensityMap* dm, const DensityParams& p) {
            dm->minval = p.minval;
            dm->maxval = p.maxval;
            dm->ease   = p.ease;
            masks.insert(GetDensityKeyPairFromPlantTypeWithDensityMapType(cfg.type, dm->type, dm));
        };

        insertMask(new CHeightDensityMap(),    cfg.height);
        insertMask(new CSlopeDensityMap(),     cfg.slope);
        insertMask(new CMoistureDensityMap(),  cfg.moisture);
        insertMask(new CRoughnessDensityMap(), cfg.roughness);

        DensityMap* road = new CRoadAttributeDensityMap();
        masks.insert(GetDensityKeyPairFromPlantTypeWithDensityMapType(cfg.type, road->type, road));
    }
};
