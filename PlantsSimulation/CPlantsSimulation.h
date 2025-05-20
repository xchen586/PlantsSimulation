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
	CPlantsSimulation(const string& outputDir, const string& inputTreeList, const string& inputLevel1TreeList, const string& inputImageFile, const string& inputImageMataFile, const string& mesh_HeightMapFile, const string& mesh2_HeightMapFile, const string& pc_HeightMapFile, const string& l1_HeightMapFile, const string& bedrock_HeightMapFile,
		const string& mesh_HeightMasksFile, const string& mesh2_HeightMasksFile, const string& pc_HeightMasksFile, const string& l1_HeightMasksFile, const string& bedrock_HeightMaskFile, const string& lakes_HeightMasksFile, const string& level1Lakes_HeightMasksFile, const string& mostTravelledPointFile, const string& mostDistantPointFile, const string& centroidPointFile, const string& cavesPointCloudLevel0File, const string& cavesPointCloudLevel1File,
		const string& regionsRawFile, const string& regionsInfoFile, const string& outputFile_level0, const string& fullOutputFile_level0, const string& pcFullOutputFile_level0, const string& outputFile_level1, const string& fullOutputFile_level1, const string& pcFullOutputFile_level1, int32_t lod, float forestAge, int iteration, int tiles, int tileX, int tileY, int tileScale, int roadHeightMapScaleWidth, int roadHeightMapScaleHeight)
		: m_outputDir(outputDir)
		, m_inputTreeListCsv(inputTreeList)
		, m_inputLevel1TreeListCsv(inputLevel1TreeList)
		, m_inputImageFile(inputImageFile)
		, m_inputImageMetaFile(inputImageMataFile)
		, m_meshHeightMapFile(mesh_HeightMapFile)
		, m_mesh2HeightMapFile(mesh2_HeightMapFile)
		, m_pcHeightMapFile(pc_HeightMapFile)
		, m_l1HeightMapFile(l1_HeightMapFile)
		, m_bedrockHeightMapFile(bedrock_HeightMapFile)
		, m_lakesHeightMasksFile(lakes_HeightMasksFile)
		, m_level1LakesHeightMasksFile(level1Lakes_HeightMasksFile)
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
		, m_outputFile_level0(outputFile_level0)
		, m_fullOutputFile_level0(fullOutputFile_level0)
		, m_pcFullOutputFile_level0(pcFullOutputFile_level0)
		, m_outputFile_level1(outputFile_level1)
		, m_fullOutputFile_level1(fullOutputFile_level1)
		, m_pcFullOutputFile_level1(pcFullOutputFile_level1)
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
		, m_tileScale(tileScale)
		, m_roadInputHeightMapWidth(roadHeightMapScaleWidth)
		, m_roadInputHeightMapHeight(roadHeightMapScaleHeight)
		, m_maxHeight(10000)
		, m_p2dCaveLevel0Nodes(nullptr)
		, m_p2dCaveLevel1Nodes(nullptr)
		, m_pInstanceExporter(nullptr)
	{
		
	}

private:
	string m_outputDir;
	string m_inputTreeListCsv;
	string m_inputLevel1TreeListCsv;
	string m_inputImageFile;
	string m_inputImageMetaFile;
	string m_meshHeightMapFile;
	string m_mesh2HeightMapFile;
	string m_pcHeightMapFile;
	string m_l1HeightMapFile;
	string m_bedrockHeightMapFile;
	string m_lakesHeightMasksFile;
	string m_level1LakesHeightMasksFile;
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
	string m_outputFile_level0;
	string m_fullOutputFile_level0;
	string m_pcFullOutputFile_level0;
	string m_outputFile_level1;
	string m_fullOutputFile_level1;
	string m_pcFullOutputFile_level1;

	int32_t m_currentLod;

	int m_tiles;
	int m_tileX;
	int m_tileY;

	int m_tileScale;
	int m_roadInputHeightMapWidth;
	int m_roadInputHeightMapHeight;

	const int m_tilePixelMeterWidth = 30000;
	const int m_tilePixelMeterHeight = 30000;

	float m_forestAge = 300;
	int m_iteration = 100;

	bool m_isLevel1Instances = false;
	bool m_onlyPOIs = false;

	InputImageDataInfo* m_topLayerImage;
	InputImageMetaInfo* m_topLayerMeta;

	RegionSubOutputVector m_regionsVector;
	RegionSubOutputMap m_regionMap;
	RegionInfoMap m_regionInfoMap;

	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	CForest* m_pForest;
	CPsInstanceExporter* m_pInstanceExporter;

	std::vector<CavesPointInfo> m_cavePointInfoList;
	std::vector<std::pair<std::vector<Point>, int>>* m_p2dCaveLevel0Nodes;
	std::vector<std::pair<std::vector<Point>, int>>* m_p2dCaveLevel1Nodes;
	std::vector<Point> m_PoisLocations;
	int m_maxHeight;
public:
	~CPlantsSimulation()
	{
		DeInitialize();
	}
	
private:
	void ClearCellTable();
	void ClearImage();
	void ClearImageMeta();

	bool LoadInputImage();
	bool LoadInputHeightMap();
	bool LoadImageMetaFile();
	
	bool ExportDoubleHeightMap(std::vector<std::vector<double>>& heightMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportShortHeightMap(std::vector<std::vector<short>>& heightMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportShortHeightMapWithMask(std::vector<std::vector<short>>& heightMap, std::vector<std::vector<short>>& masks, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio);
	bool ExportShortHeightSlopeMap(std::vector<std::vector<short>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportAngleSlopeMap(std::vector<std::vector<double>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);

	bool LoadRegionsTest();
	

	std::vector<std::pair<std::vector<Point>, int>>* LoadCaveNodesFromPointCloud(const std::string& filePath);
	bool loadPoisLocationsFromCSV(const string& filePath, std::vector<Point>& poisLocations);
	bool loadAllPoisLocationsFromCSV();
	bool SaveCavesAsRoadMap(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel0Nodes, bool invert/* = false*/);
	bool SaveCavesAsObj(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel0Nodes);
	
public:
	void setIsLevel1Instances(bool isLevel1Instances) 
	{ 
		m_isLevel1Instances = isLevel1Instances; 
		std::cout << "Set CPlantsSimulation isLevel1Instances to " << m_isLevel1Instances << std::endl;
	}

	void setOnlyPOIs(bool onlyPOIs)
	{
		m_onlyPOIs = onlyPOIs;
		std::cout << "Set CPlantsSimulation isOnlyPOIs to " << m_onlyPOIs << std::endl;
	}

	void DeInitialize();
	void DeInitializeForMakeInstances();
	
	bool MakeRoadData();
	bool MakeInstance(bool isLevel1Instance);

	bool OutputLakeRawData();

	bool LoadPreImage();
	bool LoadAndOutputRegions();

	bool LoadInputData();
	bool LoadForest();
	bool LoadInstanceExporter();
	bool BuildForest();
	bool OutputResults();
};


