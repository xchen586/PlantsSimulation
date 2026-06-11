#pragma once

// Refactor (Phase 2.2): normalized to forward-slash includes; dropped #if __APPLE__ block.
#include "../Common/include/CColonizationTree.h"

class CSunLightAffinityDensityMap : public DensityMap
{
public:
	CSunLightAffinityDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_SunLightAffinity;
		invert = false;
		ease = 0.1;
		minval = 0.0;
		maxval = 1.0;
	}
};
