#pragma once

#if __APPLE__
#include "../Common/include/PsHelper.h"
#include "../Common/include/CCellInfo.h"
#include "../Common/include/RegionStruct.h"
#else
#include "..\Common\include\PsHelper.h"
#include "..\Common\include\CCellInfo.h"
#include "..\Common\include\RegionStruct.h"
#endif

std::string Get2DArrayFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx);
std::string GetRawInfoOutputCSVFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx);

bool OutputArrayFileForSubRegions(const string& filePath, const CAffineTransform& transform, VoxelFarm::CellId cellId, std::shared_ptr<RegionSubOutputVector> subVector);