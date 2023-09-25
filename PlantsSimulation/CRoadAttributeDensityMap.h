#pragma once
#include "CColonizationTree.h"

class CRoadAttributeDensityMap : public DensityMap
{
public:
	CRoadAttributeDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_RoadAttribute;
		invert = true;
		ease = 0.1;
		minval = 0;
		maxval = 0.3;
	}
};