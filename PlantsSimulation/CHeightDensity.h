#pragma once

// Refactor (Phase 2.2): normalized to forward-slash includes; dropped #if __APPLE__ block.
#include "../Common/include/CColonizationTree.h"

const double WaterLevel = 0;

class CHeightDensityMap : public DensityMap
{
public:
	CHeightDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_Height;
		invert = false;
		blur = 0;
	}
};

// Refactor (Phase 1.1): species-specific subclasses removed; params now live in
// TreeSpeciesData.h and are applied by CTreeSpeciesClass at construction time.