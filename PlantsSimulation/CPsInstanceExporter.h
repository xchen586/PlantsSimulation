#pragma once

#if __APPLE__
#include "../Common/include/TreeInstance.h"
//#include "../Common/include/Utils.h"
#else
#include "..\Common\include\TreeInstance.h"
//#include "..\Common\include\Utils.h"
#endif

using namespace std;
class CCellInfo;
struct InputImageMetaInfo;

class CPsInstanceExporter
{
public:
	CPsInstanceExporter()
		: m_pCellTable(nullptr)
		, m_pMetaInfo(nullptr)
		, m_pFullTreeOutputs(nullptr)
		, m_lod(0)
		, m_tiles(0)
		, m_tileIndexX(0)
		, m_tileIndexY(0)
		, m_isLevel1Instances(false)
	{
	}
	~CPsInstanceExporter()
	{
		//DeInitialize(); Because m_outputMap use shared_ptr, so no need to call DeInitialize
	}
	void setCellTable(std::vector<std::vector<CCellInfo*>>* pCellTable) 
	{
		m_pCellTable = pCellTable;
	}
	void setMetaInfo(InputImageMetaInfo* metaInfo)
	{
		m_pMetaInfo = metaInfo;
	}
	void setMostTravelledPointFilePath(const string& filePath)
	{
		m_mostTravelledPointFilePath = filePath;
	}
	void setMostDistantPointFilePath(const string& filePath)
	{
		m_mostDistantPointFilePath = filePath;
	}
	void setCentroidPointFilePath(const string& filePath)
	{
		m_centroidPointFilePath = filePath;
	}
	void setFullTreeOutputs(vector<TreeInstanceFullOutput>* pFullTreeOutputs)
	{
		m_pFullTreeOutputs = pFullTreeOutputs;
	}
	void setLod(int32_t lod)
	{
		m_lod = lod;
	}
	void setTilesInfo(int t, int x, int y)
	{
		m_tiles = t;
		m_tileIndexX = x;
		m_tileIndexY = y;
	}
	void set2dCaveLevel0Nodes(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel0Nodes)
	{
		m_p2dCaveLevel0Nodes = p2dCaveLevel0Nodes;
	}
	void set2dCaveLevel1Nodes(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel1Nodes)
	{
		m_p2dCaveLevel1Nodes = p2dCaveLevel1Nodes;
	}
	void setIsLevel1Instances(bool isLevel1Instances)
	{
		m_isLevel1Instances = isLevel1Instances;
		std::cout << "Set CPsInstanceExporter isLevel1Instances to " << m_isLevel1Instances << std::endl;
	}
	bool loadPointInstanceFromCSV(const string& filePath, const string& outputSubDir, InstanceSubOutputMap& outputMap, CAffineTransform transform, double voxelSize, int32_t lod, InstanceType instanceType);
	bool outputSubfiles(const std::string& outputSubsDir);
	bool OutputAllInstanceGeoChem(string outputFilePath, const InstanceSubOutputMap& allInstances);
	
protected:
	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	InputImageMetaInfo* m_pMetaInfo;
	string m_mostTravelledPointFilePath;
	string m_mostDistantPointFilePath;
	string m_centroidPointFilePath;
	vector<TreeInstanceFullOutput> * m_pFullTreeOutputs;
	InstanceSubOutputMap m_outputMap;
	int32_t m_lod; 
	std::vector<std::pair<std::vector<Point>, int>>* m_p2dCaveLevel0Nodes;
	std::vector<std::pair<std::vector<Point>, int>>* m_p2dCaveLevel1Nodes;
	int m_tiles;
	int m_tileIndexX;
	int m_tileIndexY;
	bool m_isLevel1Instances;
	void DeInitialize();
};

