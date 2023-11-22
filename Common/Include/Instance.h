#pragma once
#include "Utils.h"

enum struct InstanceType
{
	InstanceType_None,
	InstanceType_Tree,
	InstanceType_Point,
	InstanceType_End,
};

struct InstanceSubOutput
{
	int cellXIdx;
	int cellYIdx;
	int cellZIdx;
	double xOffsetW;
	double yOffsetW;
	double zOffsetW;
	double posX;
	double posY;
	double posZ;
	double scaleX;
	double scaleY;
	double scaleZ;
	double rotationX;
	double rotationY;
	double rotationZ;
	unsigned int instanceType;
	unsigned int variant;
	double age;
	int outputItemCount;
	
	VoxelFarm::CellId cellId;
	unsigned int index;
	string idString;
	

	InstanceSubOutput()
		: cellXIdx(0)
		, cellYIdx(0)
		, cellZIdx(0)
		, xOffsetW(0.0)
		, yOffsetW(0.0)
		, zOffsetW(0.0)
		, posX(0.0)
		, posY(0.0)
		, posZ(0.0)
		, scaleX(1.0)
		, scaleY(1.0)
		, scaleZ(1.0)
		, rotationX(0.0)
		, rotationY(0.0)
		, rotationZ(0.0)
		, instanceType(0)
		, variant(0)
		, age(0)
		, cellId(0)
		, index(0)
		, idString("")
		, outputItemCount(15)
	{

	}

	virtual int GetOutputItemCount()
	{
		return outputItemCount;
	}

	void MakeIdString()
	{
		std::stringstream ss;
		ss << cellId << "_" << instanceType << "_" << instanceType << "_" << variant << "_" << index;
		idString = ss.str();
	}
};
