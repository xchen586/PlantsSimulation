#pragma once
#include <memory>

#include "CForest.h"
#include "CPsInstanceExporter.h"
#include "SimulationConfig.h"  // Refactor (Phase 4.1): replaces 46-param constructor

// Refactor (Phase 2.2): normalized to forward-slash includes; dropped #if __APPLE__ blocks.
#include "../Common/include/PsHelper.h"
#include "../Common/include/CCellInfo.h"
#include "../Common/include/RegionStruct.h"

using namespace std;

class CPlantsSimulation
{
public:
	// Refactor (Phase 4.1): single config struct replaces 46 individual parameters.
	// All member variables and internal logic are unchanged.
	CPlantsSimulation(const SimulationConfig& config)
		: m_outputDir(config.input.outputDir)
		, m_inputTreeListCsv(config.input.treeListCsv)
		, m_inputLevel1TreeListCsv(config.input.level1TreeListCsv)
		, m_inputImageFile(config.input.topLayerImage)
		, m_inputImageMetaFile(config.input.topLayerImageMeta)
		, m_meshHeightMapFile(config.input.meshHeightMap)
		, m_mesh2HeightMapFile(config.input.mesh2HeightMap)
		, m_pcHeightMapFile(config.input.pcHeightMap)
		, m_l1HeightMapFile(config.input.l1HeightMap)
		, m_bedrockHeightMapFile(config.input.bedrockHeightMap)
		, m_lakesHeightMasksFile(config.input.lakesHeightMask)
		, m_level1LakesHeightMasksFile(config.input.level1LakesHeightMask)
		, m_oceanHeightMasksFile(config.input.oceanHeightMask)
		, m_meshHeightMasksFile(config.input.meshHeightMask)
		, m_mesh2HeightMasksFile(config.input.mesh2HeightMask)
		, m_pcHeightMasksFile(config.input.pcHeightMask)
		, m_l1HeightMasksFile(config.input.l1HeightMask)
		, m_bedrockHeightMasksFile(config.input.bedrockHeightMask)
		, m_mostTravelledPointFile(config.input.mostTravelledPointFile)
		, m_mostDistantPointFile(config.input.mostDistantPointFile)
		, m_level1PoiPointFile(config.input.level1PoiPointFile)
		, m_centroidPointFile(config.input.centroidPointFile)
		, m_cavesPointCloudLevel0File(config.input.cavesPointCloudLevel0)
		, m_cavesPointCloudLevel1File(config.input.cavesPointCloudLevel1)
		, m_dungeonsPOILevel0File(config.input.dungeonsPOILevel0)
		, m_dungeonsPOILevel1File(config.input.dungeonsPOILevel1)
		, m_regionsRawFile(config.input.regionsRaw)
		, m_regionsInfoFile(config.input.regionsInfo)
		, m_outputFile_level0(config.output.outputFileLevel0)
		, m_fullOutputFile_level0(config.output.fullOutputFileLevel0)
		, m_pcFullOutputFile_level0(config.output.pcFullOutputFileLevel0)
		, m_outputFile_level1(config.output.outputFileLevel1)
		, m_fullOutputFile_level1(config.output.fullOutputFileLevel1)
		, m_pcFullOutputFile_level1(config.output.pcFullOutputFileLevel1)
		, m_pCellTable(nullptr)
		, m_pForest(nullptr)
		, m_currentLod(config.sim.lod)
		, m_forestAge(config.sim.forestAge)
		, m_iteration(config.sim.iteration)
		, m_gridDelta(config.sim.gridDelta)
		, m_initialDensity(config.sim.initialDensity)
		, m_seedDensity(config.sim.seedDensity)
		, m_competitionFactor(config.sim.competitionFactor)
		, m_growthFactor(config.sim.growthFactor)
		, m_thinningThreshold(config.sim.thinningThreshold)
		, m_tiles(config.tile.tiles)
		, m_tileX(config.tile.tileX)
		, m_tileY(config.tile.tileY)
		, m_tileScale(config.tile.tileScale)
		, m_roadInputHeightMapWidth(config.tile.roadHeightMapScaleWidth)
		, m_roadInputHeightMapHeight(config.tile.roadHeightMapScaleHeight)
		, m_maxHeight(10000)
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
	string m_oceanHeightMasksFile;
	string m_meshHeightMasksFile;
	string m_mesh2HeightMasksFile;
	string m_pcHeightMasksFile;
	string m_l1HeightMasksFile;
	string m_bedrockHeightMasksFile;
	string m_mostTravelledPointFile;
	string m_mostDistantPointFile;
	string m_level1PoiPointFile;
	string m_centroidPointFile;
	string m_cavesPointCloudLevel0File;
	string m_cavesPointCloudLevel1File;
	string m_dungeonsPOILevel0File;
	string m_dungeonsPOILevel1File;
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
	int m_exportHighRatio = 2;

	const int m_tilePixelMeterWidth = 30000;
	const int m_tilePixelMeterHeight = 30000;

	float m_forestAge = 300;
	int m_iteration = 100;
	int m_gridDelta = 30;                      // [1] HIGHEST IMPACT - Grid sampling density
	double m_initialDensity = 0.1;             // [2] HIGH IMPACT - Initial tree spawn rate
	double m_seedDensity = 0.001;              // [3] MEDIUM-HIGH IMPACT - Seed generation rate
	double m_competitionFactor = 0.9;          // [4] MEDIUM IMPACT - Tree crown size (competition)
	double m_growthFactor = 0.7;               // [5] MEDIUM IMPACT - Crown size reduction
	double m_thinningThreshold = 1.0;          // [6] LOW-MEDIUM IMPACT - Final filter strength

	bool m_isLevel1Instances = false;
	bool m_onlyPOIs = false;
	bool m_keepOldTreeFiles = false;
	bool m_isEnhanced = false;

	std::unique_ptr<InputImageDataInfo> m_topLayerImage;  // Phase 3.3: RAII ownership
	std::unique_ptr<InputImageMetaInfo> m_topLayerMeta;  // Phase 3.3: RAII ownership

	RegionSubOutputVector m_regionsVector;
	RegionSubOutputMap m_regionMap;
	RegionInfoMap m_regionInfoMap;

	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	std::unique_ptr<CForest> m_pForest;  // Phase 3.3: RAII ownership
	std::unique_ptr<CPsInstanceExporter> m_pInstanceExporter;  // Phase 3.3: RAII ownership

	std::vector<CavesPointInfo> m_cavePointInfoList;
	std::unique_ptr<std::vector<std::pair<std::vector<Point>, int>>> m_p2dCaveLevel0Nodes;  // Phase 3.3
	std::unique_ptr<std::vector<std::pair<std::vector<Point>, int>>> m_p2dCaveLevel1Nodes;  // Phase 3.3
	std::vector<Point> m_PoisLocations;
	int m_maxHeight;
	std::vector<std::vector<short>> m_oceanHeightMask;    // Ocean mask for excluding tree generation on ocean
	std::vector<std::vector<short>> m_exposureHeightMask; // Exposure mask for excluding tree generation on exposed surfaces
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
	bool ExportShortHeightMapWithMask(std::vector<std::vector<short>>& heightMap, std::vector<std::vector<short>>& masks, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio, bool useCenterPoint = true);
	bool ExportShortHeightSlopeMap(std::vector<std::vector<short>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportAngleSlopeMap(std::vector<std::vector<double>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);

	bool LoadRegionsTest();

	// Refactor (Phase 4.2): helpers extracted from LoadInputHeightMap.
	void ComputeExposureData(
		const std::vector<std::vector<short>>& mesh0HM,
		const std::vector<std::vector<short>>& pcHM,
		const std::vector<std::vector<short>>& mesh1HM,
		const std::vector<std::vector<short>>& l1SmoothHM,
		const std::vector<std::vector<short>>& bedrockHM,
		const std::vector<std::vector<short>>& pcMask,
		const std::vector<std::vector<short>>& l1SmoothMask,
		const std::vector<std::vector<short>>& bedrockMask,
		int width, int height,
		std::vector<std::vector<double>>& outExposureInit,
		std::vector<std::vector<bool>>&   outExposureMask,
		std::vector<std::vector<double>>& outExposureMap,
		std::vector<std::vector<byte>>&   outExposureByte,
		std::vector<std::vector<byte>>&   outExposureMaskByte);
	void PopulateCellTable(
		const std::vector<std::vector<double>>& heightMapDouble,
		const std::vector<std::vector<short>>&  slopeMap,
		const std::vector<std::vector<double>>& slopeMapDouble,
		const std::vector<std::vector<double>>& l1SmoothHMDouble,
		const std::vector<std::vector<short>>&  heightMask,
		const std::vector<std::vector<short>>&  l1SmoothMask,
		const std::vector<std::vector<double>>& exposureMap,
		const std::vector<std::vector<bool>>&   exposureMaskMap,
		int width, int height);

	std::vector<std::pair<std::vector<Point>, int>>* LoadCaveNodesFromPointCloud(const std::string& filePath);
	bool loadPoisLocationsFromCSV(const string& filePath, std::vector<Point>& poisLocations);
	bool loadAllSurfacePoisLocationsFromCSV();
	bool SaveCavesAsRoadMap(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel0Nodes, bool invert/* = false*/);
	bool SaveCavesAsObj(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel0Nodes, bool highResolution = false);
	
public:
	void setIsLevel1Instances(bool isLevel1Instances) 
	{ 
		m_isLevel1Instances = isLevel1Instances; 
		std::cout << "Set CPlantsSimulation isLevel1Instances to " << m_isLevel1Instances << std::endl;
	}

	void setIsEnhanced(bool isEnhanced)
	{
		m_isEnhanced = isEnhanced;
		std::cout << "Set CPlantsSimulation isEnhanced to " << m_isEnhanced << std::endl;
	}

	void setOnlyPOIs(bool onlyPOIs)
	{
		m_onlyPOIs = onlyPOIs;
		std::cout << "Set CPlantsSimulation isOnlyPOIs to " << m_onlyPOIs << std::endl;
	}

	void setKeepOldTreeFiles(bool keepOldTreeFiles)
	{
		m_keepOldTreeFiles = keepOldTreeFiles;
		std::cout << "Set CPlantsSimulation keepOldTreeFiles to " << m_keepOldTreeFiles << std::endl;
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


