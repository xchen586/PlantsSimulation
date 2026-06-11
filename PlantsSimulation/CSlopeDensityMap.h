#pragma once

// Refactor (Phase 2.2): normalized to forward-slash includes; dropped #if __APPLE__ block.
#include "../Common/include/CColonizationTree.h"
#include "../Common/include/PsMarco.h"

#if USE_SCOPE_ANGLE
class CSlopeDensityMap : public DensityMap
{
public:
	CSlopeDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_Slope;
		invert = false;
	}
};

// Refactor (Phase 1.1): species-specific subclasses removed; params now live in
// TreeSpeciesData.h and are applied by CTreeSpeciesClass at construction time.
#endif