#pragma once
#include "CellCoordStruct.h"

enum struct Region_Info_CSV_Columns {
	RI_Col_RegionId,
	RI_Col_Area,
	RI_Col_AvgHeight,
	RI_Col_MinHeight,
	RI_Col_MaxHeight,
	RI_Col_NearSea,
	RI_Col_AvgHumidity,
	//RI_Col_TreeCount,
	RI_COL_Type1,
	RI_COL_Name,
	RI_COL_CentroidX,
	RI_COL_CentroidY,
	RI_COL_CentroidZ,
	RI_COL_RegionLevel,
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
	unsigned int averageHumidity;
	//unsigned int treeCount;
	string type1;
	string name;
	unsigned int centroidX;
	unsigned int centroidY;
	int centroidZ;
	bool hasZPos;
	unsigned short eId;
	unsigned short regionLevel;
	CellCoordStruct centroidCoord;
	RegionInfo()
		: regionId(0)
		, area(0)
		, averageHeight(0)
		, minHeight(0)
		, maxHeight(0)
		, nearSea(0)
		, averageHumidity(0)
		//, treeCount(0)
		, type1("")
		, name("")
		, eId(0)
		, regionLevel(1)
		, centroidX(0)
		, centroidY(0)
		, centroidZ(0)
		, hasZPos(false)
	{
		
	}
};

struct RegionStruct
{
	int regionsId;
	int rX;
	int rY;

	CellCoordStruct coord;

	RegionStruct(int id, int x, int y)
		: regionsId(id)
		, rX(x)
		, rY(y)
	{

	}
};

typedef std::map<int, std::shared_ptr<RegionInfo>> RegionInfoMap;

typedef std::vector<std::shared_ptr<RegionStruct>> RegionSubOutputVector;
typedef std::map<VoxelFarm::CellId, std::shared_ptr<RegionSubOutputVector>> RegionSubOutputMap;
