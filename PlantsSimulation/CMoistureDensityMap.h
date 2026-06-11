#pragma once

// Refactor (Phase 2.2): normalized to forward-slash includes; dropped #if __APPLE__ block.
#include "../Common/include/CColonizationTree.h"

class CMoistureDensityMap : public DensityMap
{
public:
	CMoistureDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_Moisture;
		invert = false;
		ease = 0.1;
	}
};

// Refactor (Phase 1.1): species-specific subclasses removed; params now live in
// TreeSpeciesData.h and are applied by CTreeSpeciesClass at construction time.