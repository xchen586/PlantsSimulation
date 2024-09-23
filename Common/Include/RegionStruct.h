#pragma once
#include "PsHelper.h"

enum struct Region_Info_CSV_Columns {
	RI_Col_RegionId,
	RI_Col_Area,
	RI_Col_AvgHeight,
	RI_Col_MinHeight,
	RI_Col_MaxHeight,
	RI_Col_NearSea,
	RI_Col_AvgHumidity,
	RI_Col_TreeCount,
	RI_Col_EId,
};

struct RegionInfo
{
	unsigned int regionId;
	unsigned int area;
	unsigned int averageHeight;
	unsigned int minHeight;
	unsigned int maxHeight;
	unsigned int nearSea;
	unsigned int avarageHumidity;
	unsigned int treeCount;
	unsigned short eId;
	RegionInfo()
		: regionId(0)
		, area(0)
		, averageHeight(0)
		, minHeight(0)
		, maxHeight(0)
		, nearSea(0)
		, avarageHumidity(0)
		, treeCount(0)
		, eId(0)
	{
		
	}
};

struct RegionStruct
{
	int regionsId;
	int rX;
	int rY;
	int cellXIdx;
	int cellYIdx;
	int cellZIdx;
	double posX;
	double posY;
	double posZ;
	double vX;
	double vY;
	double vZ;
	
	double xOffsetW;
	double yOffsetW;
	double zOffsetW;

	VoxelFarm::CellId cellId;

	RegionStruct(int id, int x, int y)
		: regionsId(id)
		, rX(x)
		, rY(y)
		, cellXIdx(0)
		, cellYIdx(0)
		, cellZIdx(0)
		, posX(0.0)
		, posY(0.0)
		, posZ(0.0)
		, vX(0.0)
		, vY(0.0)
		, vZ(0.0)
		, xOffsetW(0.0)
		, yOffsetW(0.0)
		, zOffsetW(0.0)
		, cellId(0)
	{

	}
};

typedef std::map<int, std::shared_ptr<RegionInfo>> RegionInfoMap;

typedef std::vector<std::shared_ptr<RegionStruct>> RegionSubOutputVector;
typedef std::map<VoxelFarm::CellId, std::shared_ptr<RegionSubOutputVector>> RegionSubOutputMap;
