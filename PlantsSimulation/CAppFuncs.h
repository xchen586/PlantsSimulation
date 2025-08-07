#pragma once

#if __APPLE__
#include "../Common/include/PsMarco.h"
#include "../Common/include/PsHelper.h"
#include "../Common/include/CCellInfo.h"
#include "../Common/include/RegionStruct.h"
#include "../Common/include/Instance.h"
#include "../Common/include/TreeInstance.h"
#include
#else
#include "..\Common\include\PsMarco.h"
#include "..\Common\include\PsHelper.h"
#include "..\Common\include\CCellInfo.h"
#include "..\Common\include\RegionStruct.h"
#include "..\Common\include\Instance.h"
#include "..\Common\include\TreeInstance.h"
#endif

std::string Get2DArrayFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx);
std::string Get2DArrayRawCsvFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx);
std::string GetSubRegionInfoOutputCSVFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx);
bool LoadRegionInfoFromCSV(const string& filePath, RegionInfoMap& regionInfoMap, std::vector<std::vector<CCellInfo*>>* pCellTable, InputImageMetaInfo* pMetaInfo);
bool SaveSubRegionInfoToCSVFile(const string& filePath, RegionInfoMap& regionInfoMap, std::set<unsigned int> subSet);
bool OutputArrayFileForSubRegionsTest(const string& filePath, const CAffineTransform& transform, VoxelFarm::CellId cellId, std::shared_ptr<RegionSubOutputVector> subVector);

std::string GetKeyStringForInstance(const string& outputDir, int intXIdx, int intZIdx);
void SetupInstanceSubOutput(double posX, double posY, double posZ, const CAffineTransform& transform, double cellScale, int32_t lod, std::shared_ptr<InstanceSubOutput> sub);
bool OutputCSVFileForSubInstances(const string& filePath, std::shared_ptr<InstanceSubOutputVector> subVector);
bool OutputAllInstance(string outputFilePath, const InstanceSubOutputMap* treeInstances, const InstanceSubOutputMap* poiInstances);

int GetInstancesCountFromInstanceSubOutputMap(const InstanceSubOutputMap& instanceMap);

double GetHeightFor2DPointFromCellTable(double xPos, double yPos, std::vector<std::vector<CCellInfo*>>* pCellTable, InputImageMetaInfo* pMetaInfo);

std::vector<std::vector<double>> PropagateLightingMax(const std::vector<std::vector<double>>& exposure_init_map, const std::vector<std::vector<bool>>& exposure_mask_map, int max_iterations = 1000, const double PROPAGATION_FACTOR = 0.5, const double MIN_THRESHOLD = 0.001);
std::vector<std::vector<double>> PropagateLightingAverage(const std::vector<std::vector<double>>& exposure_init_map, const std::vector<std::vector<bool>>& exposure_mask_map, int max_iterations = 1000, const double PROPAGATION_FACTOR = 0.5, const double MIN_THRESHOLD = 0.001, bool beSimple = false);
std::vector<std::vector<double>> PropagateLightingMax4Dir(
	const std::vector<std::vector<double>>& exposure_init_map,
	const std::vector<std::vector<bool>>& exposure_mask_map,
	int max_iterations = 1000,
	double propagation_factor = 0.5,
	double min_threshold = 0.001);