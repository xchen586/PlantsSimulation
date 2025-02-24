#pragma once

#include "CForest.h"
#include "CPsInstanceExporter.h"

#if __APPLE__
    #include "../Common/include/PsHelper.h"
    #include "../Common/include/CCellInfo.h"
	#include "../Common/include/RegionStruct.h"
#else
    #include "..\Common\include\PsHelper.h"
    #include "..\Common\include\CCellInfo.h"
	#include "..\Common\include\RegionStruct.h"
#endif

using namespace std;

class CPlantsSimulation
{
public:
	CPlantsSimulation(const string& outputDir, const string& inputTreeList, const string& inputImageFile, const string& inputImageMataFile, const string& mesh_HeightMapFile, const string& mesh2_HeightMapFile, const string& pc_HeightMapFile, const string& l1_HeightMapFile, const string& bedrock_HeightMapFile,
		const string& mesh_HeightMasksFile, const string& mesh2_HeightMasksFile, const string& pc_HeightMasksFile, const string& l1_HeightMasksFile, const string& bedrock_HeightMaskFile, const string& mostTravelledPointFile, const string& mostDistantPointFile, const string& centroidPointFile, const string& cavesPointCloudLevel0File, const string& cavesPointCloudLevel1File,
		const string& regionsRawFile, const string& regionsInfoFile, const string& outputFile, const string& fullOutputFile, const string& pcFullOutputFile, int32_t lod, float forestAge, int iteration, int tiles, int tileX, int tileY, bool useBaseMeshesLevel1)
		: m_outputDir(outputDir)
		, m_inputTreeListCsv(inputTreeList)
		, m_inputImageFile(inputImageFile)
		, m_inputImageMetaFile(inputImageMataFile)
		, m_meshHeightMapFile(mesh_HeightMapFile)
		, m_mesh2HeightMapFile(mesh2_HeightMapFile)
		, m_pcHeightMapFile(pc_HeightMapFile)
		, m_l1HeightMapFile(l1_HeightMapFile)
		, m_bedrockHeightMapFile(bedrock_HeightMapFile)
		, m_meshHeightMasksFile(mesh_HeightMasksFile)
		, m_mesh2HeightMasksFile(mesh2_HeightMasksFile)
		, m_pcHeightMasksFile(pc_HeightMasksFile)
		, m_l1HeightMasksFile(l1_HeightMasksFile)
		, m_bedrockHeightMasksFile(bedrock_HeightMaskFile)
		, m_mostTravelledPointFile(mostTravelledPointFile)
		, m_mostDistantPointFile(mostDistantPointFile)
		, m_centroidPointFile(centroidPointFile)
		, m_cavesPointCloudLevel0File(cavesPointCloudLevel0File)
		, m_cavesPointCloudLevel1File(cavesPointCloudLevel1File)
		, m_regionsRawFile(regionsRawFile)
		, m_regionsInfoFile(regionsInfoFile)
		, m_outputFile(outputFile)
		, m_fullOutputFile(fullOutputFile)
		, m_pcFullOutputFile(pcFullOutputFile)
		, m_topLayerImage(nullptr)
		, m_topLayerMeta(nullptr)
		, m_pCellTable(nullptr)
		, m_pForest(nullptr)
		, m_currentLod(lod)
		, m_forestAge(forestAge)
		, m_iteration(iteration)
		, m_tiles(tiles)
		, m_tileX(tileX)
		, m_tileY(tileY)
		, m_hasTreeListCsv(false)
		, m_maxHeight(10000)
		, m_useBaseMeshesLevel1(useBaseMeshesLevel1)
		, m_p2dCaveLevel0Nodes(nullptr)
		, m_p2dCaveLevel1Nodes(nullptr)
	{
		
	}

private:
	string m_outputDir;
	string m_inputTreeListCsv;
	string m_inputImageFile;
	string m_inputImageMetaFile;
	string m_meshHeightMapFile;
	string m_mesh2HeightMapFile;
	string m_pcHeightMapFile;
	string m_l1HeightMapFile;
	string m_bedrockHeightMapFile;
	string m_meshHeightMasksFile;
	string m_mesh2HeightMasksFile;
	string m_pcHeightMasksFile;
	string m_l1HeightMasksFile;
	string m_bedrockHeightMasksFile;
	string m_mostTravelledPointFile;
	string m_mostDistantPointFile;
	string m_centroidPointFile;
	string m_cavesPointCloudLevel0File;
	string m_cavesPointCloudLevel1File;
	string m_regionsRawFile;
	string m_regionsInfoFile;
	string m_outputFile;
	string m_fullOutputFile;
	string m_pcFullOutputFile;

	int32_t m_currentLod;

	int m_tiles;
	int m_tileX;
	int m_tileY;

	float m_forestAge = 300;
	int m_iteration = 100;

	bool m_useBaseMeshesLevel1 = true;

	InputImageDataInfo* m_topLayerImage;
	InputImageMetaInfo* m_topLayerMeta;

	RegionSubOutputVector m_regionsVector;
	RegionSubOutputMap m_regionMap;
	RegionInfoMap m_regionInfoMap;

	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	CForest* m_pForest;
	CPsInstanceExporter* m_pInstanceExporter;
	bool m_hasTreeListCsv;

	std::vector<std::pair<std::vector<Point>, int>>* m_p2dCaveLevel0Nodes;
	std::vector<std::pair<std::vector<Point>, int>>* m_p2dCaveLevel1Nodes;
	std::vector<Point> m_PoisLocations;
	int m_maxHeight;
public:
	

private:
	void DeInitialize();
	bool LoadInputImage();
	bool LoadInputHeightMap();
	bool LoadImageMetaFile();
	
	bool ExportDoubleHeightMap(std::vector<std::vector<double>>& heightMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportShortHeightMap(std::vector<std::vector<short>>& heightMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportShortHeightMapWithMask(std::vector<std::vector<short>>& heightMap, std::vector<std::vector<short>>& masks, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio);
	bool ExportShortHeightSlopeMap(std::vector<std::vector<short>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportAngleSlopeMap(std::vector<std::vector<double>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);

	bool LoadRegionsTest();
	bool LoadAndOutputRegions();

	std::vector<std::pair<std::vector<Point>, int>>* LoadCaveNodesFromPointCloud(const std::string& filePath);
	bool loadPoisLocationsFromCSV(const string& filePath, std::vector<Point>& poisLocations);
	bool loadAllPoisLocationsFromCSV();
	
public:
	bool LoadInputData();
	bool LoadForest();
	bool LoadInstanceExporter();
	bool BuildForest();
	bool OutputResults();
};


