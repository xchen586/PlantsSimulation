#pragma once

#include "Instance.h"

enum class PointType {
	Point_None,
	Point_Centroid,
	Point_MostTravelled,
	Point_MostDistant,
	Point_End
};

struct PointInstanceSubOutput : public InstanceSubOutput
{
	PointInstanceSubOutput() : InstanceSubOutput()
	{
	}
	unsigned int slopeValue = 0; // Resource type or variant
};

