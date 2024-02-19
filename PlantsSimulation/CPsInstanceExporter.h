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
	bool loadPointInstanceFromCSV(const string& filePath, const string& outputSubDir, InstanceSubOutputMap& outputMap, unsigned int variant, CAffineTransform transform, double voxelSize, int32_t lod);
	bool outputSubfiles(const std::string& outputSubsDir);
	bool OutputAllInstanceGeoChem(string outputFilePath, const InstanceSubOutputMap& allInstances);
	
protected:
	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	InputImageMetaInfo* m_pMetaInfo;
	string m_mostTravelledPointFilePath;
	string m_mostDistantPointFilePath;
	vector<TreeInstanceFullOutput> * m_pFullTreeOutputs;
	InstanceSubOutputMap m_outputMap;
	int32_t m_lod; 
	int m_tiles;
	int m_tileIndexX;
	int m_tileIndexY;
};

