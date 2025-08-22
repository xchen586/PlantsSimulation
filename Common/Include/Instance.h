#pragma once
#include "PsHelper.h"

enum struct InstanceType
{
	//InstanceType_None,
	InstanceType_Tree = 0,
	InstanceType_Spawn_Point = 1,
	InstanceType_NPC = 2,
	InstanceType_Resource = 3,
	InstanceType_Tree_Level1 = 4,
	InstanceType_Dungeon_Quest = 5,
	InstanceType_Dungeon_Loot = 6,
	InstanceType_Dungeon_Mob = 7,
	InstanceType_POI_Level1 = 8,
	InstanceType_Count = 9
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
	double vX;
	double vY;
	double vZ;
	double scaleX;
	double scaleY;
	double scaleZ;
	double rotationX;
	double rotationY;
	double rotationZ;
	unsigned int instanceType;
	unsigned int variant;
	unsigned int level;
	double slopeValue;
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
		, vX(0.0)
		, vY(0.0)
		, vZ(0.0)
		, scaleX(1.0)
		, scaleY(1.0)
		, scaleZ(1.0)
		, rotationX(0.0)
		, rotationY(0.0)
		, rotationZ(0.0)
		, instanceType(0)
		, variant(0)
		, level(0)
		, slopeValue(0.0)
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
		ss << cellId << "_" << instanceType << "_" << instanceType << "_" << variant << "_"  << level << "_" << index;
		idString = ss.str();
	}
};
