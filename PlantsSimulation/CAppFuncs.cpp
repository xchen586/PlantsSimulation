#include "CAppFuncs.h"

std::string Get2DArrayFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx)
{
	const int MAX_PATH = 250;
	char subFileName[MAX_PATH];
	char subFilePath[MAX_PATH];
	memset(subFileName, 0, sizeof(char) * MAX_PATH);
	memset(subFilePath, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(subFileName, MAX_PATH, "regions_%d_%d.raw", intXIdx, intZIdx);
	snprintf(subFilePath, MAX_PATH, "%s/%s", outputDir.c_str(), subFileName);
#else
	sprintf_s(subFileName, MAX_PATH, "regions_%d_%d.raw", intXIdx, intZIdx);
	sprintf_s(subFilePath, MAX_PATH, "%s\\%s", outputDir.c_str(), subFileName);
#endif

	string ret = subFilePath;
	return ret;
}

std::string GetRawInfoOutputCSVFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx)
{
	const int MAX_PATH = 250;
	char subFileName[MAX_PATH];
	char subFilePath[MAX_PATH];
	memset(subFileName, 0, sizeof(char) * MAX_PATH);
	memset(subFilePath, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(subFileName, MAX_PATH, "regions_%d_%d.csv", intXIdx, intZIdx);
	snprintf(subFilePath, MAX_PATH, "%s/%s", outputDir.c_str(), subFileName);
#else
	sprintf_s(subFileName, MAX_PATH, "regions_%d_%d.csv", intXIdx, intZIdx);
	sprintf_s(subFilePath, MAX_PATH, "%s\\%s", outputDir.c_str(), subFileName);
#endif

	string ret = subFilePath;
	return ret;
}

bool OutputArrayFileForSubRegions(const string& filePath, const CAffineTransform& transform, VoxelFarm::CellId cellId, std::shared_ptr<RegionSubOutputVector> subVector)
{
	std::cout << "Start to OutputArrayFileForSubRegions to : " << filePath << std::endl;

	std::ofstream outputFile(filePath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the sub csv file " << filePath << std::endl;
		return false;
	}

	int cellX = 0;
	int cellY = 0;
	int cellZ = 0;
	int lod = 0;

	VoxelFarm::unpackCellId(cellId, lod, cellX, cellY, cellZ);
	const double cellScale = (1 << lod) * VoxelFarm::CELL_SIZE;

	int cellX1 = cellX + 1;
	int cellY1 = cellY + 1;
	int cellZ1 = cellZ + 1;

	//vf point 0
	double vfPointX = (cellX * cellScale);
	double vfPointY = (cellY * cellScale);
	double vfPointZ = (cellZ * cellScale);

	//vf point 1
	double vfPoint1X = (cellX1 * cellScale);
	double vfPoint1Y = (cellY1 * cellScale);
	double vfPoint1Z = (cellZ1 * cellScale);

	//vf bounds size
	double vfBoundsSizeX = (vfPoint1X - vfPointX);
	double vfBoundsSizeY = (vfPoint1Y - vfPointY);
	double vfBoundsSizeZ = (vfPoint1Z - vfPointZ);

	//vf min
	double vfMinX = min(vfPointX, vfPoint1X);
	double vfMinY = min(vfPointY, vfPoint1Y);
	double vfMinZ = min(vfPointZ, vfPoint1Z);

	//vf max
	double vfMaxX = max(vfPointX, vfPoint1X);
	double vfMaxY = max(vfPointY, vfPoint1Y);
	double vfMaxZ = max(vfPointZ, vfPoint1Z);

	double scaleWidthRate = 100;
	double scaleHeightRate = 100;

	int arrayWidth = static_cast<int>(cellScale / scaleWidthRate);
	int arrayHeight = static_cast<int>(cellScale / scaleHeightRate);

	std::vector<std::vector<uint32_t>> region2D(arrayWidth, std::vector<uint32_t>(arrayHeight));
	for (int x = 0; x < arrayWidth; x++)
	{
		for (int y = 0; y < arrayHeight; y++)
		{
			region2D[x][y] = 0;
		}
	}

	int regionCount = 0;;

	for (const std::shared_ptr<RegionStruct>& sub : *subVector)
	{
		double dIndexX = (sub->vX - vfMinX) / scaleWidthRate;
		double dIndexZ = (sub->vZ - vfMinZ) / scaleHeightRate;
		int iIndexX = static_cast<int>(dIndexX);
		int iIndexZ = static_cast<int>(dIndexZ);

		if ((iIndexX < 0) || (iIndexX > (arrayWidth - 1)))
		{
			//std::cout << "Region Struct iIndexX " << iIndexX << " is out of cell bound X " << "Cell " << cellX << " " << cellZ << std::endl;
			if (iIndexX < 0)
			{
				iIndexX = 0;
			}
			if (iIndexX > (arrayWidth - 1))
			{
				iIndexX = arrayWidth - 1;
			}
		}
		if ((iIndexZ < 0) || (iIndexZ > (arrayHeight - 1)))
		{
			//std::cout << "Region Struct iIndexZ " << iIndexZ << " is out of cell bound X " << "Cell " << cellX << " " << cellZ << std::endl;
			if (iIndexZ < 0)
			{
				iIndexZ = 0;
			}
			if (iIndexZ > (arrayHeight - 1))
			{
				iIndexZ = arrayHeight - 1;
			}
		}
		region2D[iIndexX][iIndexZ] = sub->regionsId;
		regionCount++;
	}
	std::cout << "It has region count is " << regionCount << endl;

	bool saved = Write2DArrayAsRaw(filePath, region2D);

	return saved;
}