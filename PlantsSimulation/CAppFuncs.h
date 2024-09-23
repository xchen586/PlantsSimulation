#pragma once

#if __APPLE__
#include "../Common/include/PsHelper.h"
#include "../Common/include/CCellInfo.h"
#include "../Common/include/RegionStruct.h"
#include "../Common/include/Instance.h"
#include "../Common/include/TreeInstance.h"
#else
#include "..\Common\include\PsHelper.h"
#include "..\Common\include\CCellInfo.h"
#include "..\Common\include\RegionStruct.h"
#include "..\Common\include\Instance.h"
#include "..\Common\include\TreeInstance.h"
#endif

std::string Get2DArrayFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx);
std::string GetSubRegionInfoOutputCSVFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx);
bool LoadRegionInfoFromCSV(const string& filePath, RegionInfoMap& regionInfoMap);
bool SaveSubRegionInfoToCSVFile(const string& filePath, RegionInfoMap& regionInfoMap, std::set<unsigned int> subSet);
void SetupRegionSubOutput(double posX, double posY, double posZ, const CAffineTransform& transform, double cellSize, int32_t lod, std::shared_ptr<RegionStruct> sub);
bool OutputArrayFileForSubRegionsTest(const string& filePath, const CAffineTransform& transform, VoxelFarm::CellId cellId, std::shared_ptr<RegionSubOutputVector> subVector);

std::string GetKeyStringForInstance(const string& outputDir, int intXIdx, int intZIdx);
void SetupInstanceSubOutput(double posX, double posY, double posZ, const CAffineTransform& transform, double cellScale, int32_t lod, std::shared_ptr<InstanceSubOutput> sub);
bool OutputCSVFileForSubInstances(const string& filePath, std::shared_ptr<InstanceSubOutputVector> subVector);
bool OutputAllInstance(string outputFilePath, const InstanceSubOutputMap& allInstances);

