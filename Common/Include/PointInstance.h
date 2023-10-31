#pragma once

#include "Instance.h"

enum class PointType {
	Point_None,
	Point_MostTravelled,
	Point_MostDistant,
	Point_End
};

struct PointInstanceSubOutput : public InstanceSubOutput
{
	PointInstanceSubOutput() : InstanceSubOutput()
	{
	}

};

