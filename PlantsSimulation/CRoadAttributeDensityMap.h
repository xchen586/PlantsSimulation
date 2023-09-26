#pragma once
#include "CColonizationTree.h"

class CRoadAttributeDensityMap : public DensityMap
{
public:
	CRoadAttributeDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_RoadAttribute;
		invert = false;
		ease = 0.1;
		minval = 0.0;
		//maxval = 0.78;
		maxval = 0.1;
	}
};