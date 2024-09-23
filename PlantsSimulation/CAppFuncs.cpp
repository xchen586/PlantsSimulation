#include "CAppFuncs.h"

#include <cassert>

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

std::string GetSubRegionInfoOutputCSVFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx)
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

bool LoadRegionInfoFromCSV(const string& filePath, RegionInfoMap& regionInfoMap)
{
	std::cout << "Start to LoadRegionInfoFromCSV from : " << filePath << std::endl;
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open the csv file :" << filePath << std::endl;
		return false;
	}
	regionInfoMap.clear();

	std::string header;
	std::getline(file, header);

	std::string line;

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::string cell;

		std::vector<std::string> row;
		while (std::getline(lineStream, cell, ','))
		{
			row.push_back(cell);
		}
		// This checks for a trailing comma with no data after it.
		if (!lineStream && cell.empty())
		{
			// If there was a trailing comma then add an empty element.
			row.push_back("");
		}

		std::shared_ptr<RegionInfo> info = std::make_shared<RegionInfo>();
		string regionIdString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_Col_RegionId)];
		string areaString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_Col_Area)];
		string averageHeightString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_Col_AvgHeight)];
		string minHeightString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_Col_MinHeight)];
		string maxHeightString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_Col_MaxHeight)];
		string nearSeaString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_Col_NearSea)];
		string averageHumidityString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_Col_AvgHumidity)];
		string treeCountString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_Col_TreeCount)];

		info->regionId = static_cast<unsigned int>(std::stoul(regionIdString));
		info->area = static_cast<unsigned int>(std::stoul(areaString));
		info->averageHeight = static_cast<unsigned int>(std::stoul(averageHeightString));
		info->minHeight = static_cast<unsigned int>(std::stoul(minHeightString));
		info->maxHeight = static_cast<unsigned int>(std::stoul(maxHeightString));
		info->nearSea = static_cast<unsigned int>(std::stoul(nearSeaString));
		info->averageHumidity = static_cast<unsigned int>(std::stoul(averageHumidityString));
		info->treeCount = static_cast<unsigned int>(std::stoul(treeCountString));
		info->eId = info->regionId;

		pair<unsigned int, shared_ptr<RegionInfo>> pair(info->regionId, info);
		regionInfoMap.insert(pair);
	}

	file.close();
	std::cout << "End to LoadRegionInfoFromCSV from : " << filePath << std::endl;
	return true;
}

bool SaveSubRegionInfoToCSVFile(const string& filePath, RegionInfoMap& regionInfoMap, std::set<unsigned int> subSet)
{
	std::cout << "Start to SaveSubRegionInfoToCSVFile to : " << filePath << std::endl;

	std::ofstream outputFile(filePath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the sub csv file " << filePath << std::endl;
		return false;
	}

	outputFile << "RegionId,Area,AvgHeight,MinHeight,MaxHeight,NearSea,AvgHumidity,TreeCount,ExtrId" << std::endl;

	for (unsigned int rid : subSet)
	{
		shared_ptr<RegionInfo> info = regionInfoMap[rid];
		assert(info != nullptr);
		if (info != nullptr)
		{
			outputFile
				<< info->regionId << ","
				<< info->area << ","
				<< info->averageHeight << ","
				<< info->minHeight << ","
				<< info->maxHeight << ","
				<< info->nearSea << ","
				<< info->averageHumidity << ","
				<< info->treeCount << ","
				<< info->eId << std::endl;
		}
	}

	outputFile.close();
	std::cout << "End to SaveSubRegionInfoToCSVFile to : " << filePath << std::endl;
	
	return true;
}

void SetupRegionSubOutput(double posX, double posY, double posZ, const CAffineTransform& transform, double cellScale, int32_t lod, std::shared_ptr<RegionStruct> sub)
{
	const auto vfPosition = transform.WC_TO_VF(CAffineTransform::sAffineVector(posX, posY, posZ));

	//cell min
	int cellX = (int)(vfPosition.X / cellScale);
	int cellY = (int)(vfPosition.Y / cellScale);
	int cellZ = (int)(vfPosition.Z / cellScale);

	//cell max
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
	double vfBoundsSizeX = std::abs(vfPoint1X - vfPointX);
	double vfBoundsSizeY = std::abs(vfPoint1Y - vfPointY);
	double vfBoundsSizeZ = std::abs(vfPoint1Z - vfPointZ);

	//vf min
	double vfMinX = min(vfPointX, vfPoint1X);
	double vfMinY = min(vfPointY, vfPoint1Y);
	double vfMinZ = min(vfPointZ, vfPoint1Z);

	//vf max
	double vfMaxX = max(vfPointX, vfPoint1X);
	double vfMaxY = max(vfPointY, vfPoint1Y);
	double vfMaxZ = max(vfPointZ, vfPoint1Z);

	//world point 0
	auto worldPoint0 = transform.VF_TO_WC(CAffineTransform::sAffineVector{ vfMinX, vfMinY, vfMinZ });

	//world point 1
	auto worldPoint1 = transform.VF_TO_WC(CAffineTransform::sAffineVector{ vfMaxX, vfMaxY, vfMaxZ });

	//world min
	double worldMinX = min(worldPoint0.X, worldPoint1.X);
	double worldMinY = min(worldPoint0.Y, worldPoint1.Y);
	double worldMinZ = min(worldPoint0.Z, worldPoint1.Z);

	//world max
	double worldMaxX = max(worldPoint0.X, worldPoint1.X);
	double worldMaxY = max(worldPoint0.Y, worldPoint1.Y);
	double worldMaxZ = max(worldPoint0.Z, worldPoint1.Z);

	//world bounds size
	double worldBoundsSizeX = std::abs(worldMaxX - worldMinX);
	double worldBoundsSizeY = std::abs(worldMaxY - worldMinY);
	double worldBoundsSizeZ = std::abs(worldMaxZ - worldMinZ);

	//world offset
	double worldOffsetX = (posX - worldMinX);
	double worldOffsetY = (posY - worldMinY);
	double worldOffsetZ = (posZ - worldMinZ);

	if ((posX < worldMinX) ||
		(posY < worldMinY) ||
		(posZ < worldMinZ) ||
		(posX > worldMaxX) ||
		(posY > worldMaxY) ||
		(posZ > worldMaxZ))
	{
		std::cout << "pos is not in the cell" << std::endl;
	}

	if (worldOffsetX < 0 ||
		worldOffsetY < 0 ||
		worldOffsetZ < 0)
	{
		std::cout << "offset is not in the cell" << std::endl;
	}

	//cellY = 0; //Because I only 2D cellX and CellZ;

	sub->cellXIdx = cellX;
	sub->cellYIdx = cellY;
	sub->cellZIdx = cellZ;

	sub->xOffsetW = worldOffsetX;
	sub->yOffsetW = worldOffsetY;
	sub->zOffsetW = worldOffsetZ;

	sub->posX = posX;
	sub->posY = posY;
	sub->posZ = posZ;

	sub->vX = vfPosition.X;
	sub->vY = vfPosition.Y;
	sub->vZ = vfPosition.Z;

	sub->cellId = VoxelFarm::packCellId(lod, cellX, cellY, cellZ);
}

bool OutputArrayFileForSubRegionsTest(const string& filePath, const CAffineTransform& transform, VoxelFarm::CellId cellId, std::shared_ptr<RegionSubOutputVector> subVector)
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

void SetupInstanceSubOutput(double posX, double posY, double posZ, const CAffineTransform& transform, double cellScale, int32_t lod, std::shared_ptr<InstanceSubOutput> sub)
{
	const auto vfPosition = transform.WC_TO_VF(CAffineTransform::sAffineVector(posX, posY, posZ));

	//cell min
	int cellX = (int)(vfPosition.X / cellScale);
	int cellY = (int)(vfPosition.Y / cellScale);
	int cellZ = (int)(vfPosition.Z / cellScale);

	//cell max
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
	double vfBoundsSizeX = std::abs(vfPoint1X - vfPointX);
	double vfBoundsSizeY = std::abs(vfPoint1Y - vfPointY);
	double vfBoundsSizeZ = std::abs(vfPoint1Z - vfPointZ);

	//vf min
	double vfMinX = min(vfPointX, vfPoint1X);
	double vfMinY = min(vfPointY, vfPoint1Y);
	double vfMinZ = min(vfPointZ, vfPoint1Z);

	//vf max
	double vfMaxX = max(vfPointX, vfPoint1X);
	double vfMaxY = max(vfPointY, vfPoint1Y);
	double vfMaxZ = max(vfPointZ, vfPoint1Z);

	//world point 0
	auto worldPoint0 = transform.VF_TO_WC(CAffineTransform::sAffineVector{ vfMinX, vfMinY, vfMinZ });

	//world point 1
	auto worldPoint1 = transform.VF_TO_WC(CAffineTransform::sAffineVector{ vfMaxX, vfMaxY, vfMaxZ });

	//world min
	double worldMinX = min(worldPoint0.X, worldPoint1.X);
	double worldMinY = min(worldPoint0.Y, worldPoint1.Y);
	double worldMinZ = min(worldPoint0.Z, worldPoint1.Z);

	//world max
	double worldMaxX = max(worldPoint0.X, worldPoint1.X);
	double worldMaxY = max(worldPoint0.Y, worldPoint1.Y);
	double worldMaxZ = max(worldPoint0.Z, worldPoint1.Z);

	//world bounds size
	double worldBoundsSizeX = std::abs(worldMaxX - worldMinX);
	double worldBoundsSizeY = std::abs(worldMaxY - worldMinY);
	double worldBoundsSizeZ = std::abs(worldMaxZ - worldMinZ);

	//world offset
	double worldOffsetX = (posX - worldMinX);
	double worldOffsetY = (posY - worldMinY);
	double worldOffsetZ = (posZ - worldMinZ);

	if ((posX < worldMinX) ||
		(posY < worldMinY) ||
		(posZ < worldMinZ) ||
		(posX > worldMaxX) ||
		(posY > worldMaxY) ||
		(posZ > worldMaxZ))
	{
		std::cout << "pos is not in the cell" << std::endl;
	}

	if (worldOffsetX < 0 ||
		worldOffsetY < 0 ||
		worldOffsetZ < 0)
	{
		std::cout << "offset is not in the cell" << std::endl;
	}

	sub->cellXIdx = cellX;
	sub->cellYIdx = cellY;
	sub->cellZIdx = cellZ;

	sub->xOffsetW = worldOffsetX;
	sub->yOffsetW = worldOffsetY;
	sub->zOffsetW = worldOffsetZ;

	sub->posX = posX;
	sub->posY = posY;
	sub->posZ = posZ;

	sub->vX = vfPosition.X;
	sub->vY = vfPosition.Y;
	sub->vZ = vfPosition.Z;

	sub->cellId = VoxelFarm::packCellId(lod, cellX, cellY, cellZ);
}

std::string GetKeyStringForInstance(const string& outputDir, int intXIdx, int intZIdx)
{
	const int MAX_PATH = 250;
	char subFileName[MAX_PATH];
	char subFilePath[MAX_PATH];
	memset(subFileName, 0, sizeof(char) * MAX_PATH);
	memset(subFilePath, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(subFileName, MAX_PATH, "instances_%d_%d.csv", intXIdx, intZIdx);
	snprintf(subFilePath, MAX_PATH, "%s/%s", outputDir.c_str(), subFileName);
#else
	sprintf_s(subFileName, MAX_PATH, "instances_%d_%d.csv", intXIdx, intZIdx);
	sprintf_s(subFilePath, MAX_PATH, "%s\\%s", outputDir.c_str(), subFileName);
#endif
	string ret = subFilePath;
	return ret;
}

bool OutputCSVFileForSubInstances(const string& filePath, std::shared_ptr<InstanceSubOutputVector> subVector)
{
	std::cout << "Start to OutputCSVFileForSubInstances to : " << filePath << std::endl;

	std::ofstream outputFile(filePath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the sub csv file " << filePath << std::endl;
		return false;
	}
#if !USE_OUTPUT_INSTANCE_IDSTRING
	outputFile << "X,Y,Z,ScaleX,ScaleY,ScaleZ,RotationX,RotationY,RotaionZ,InstanceType,Variant,Age,XWorld,YWorld,ZWorld" << std::endl;
#else
	outputFile << "X,Y,Z,ScaleX,ScaleY,ScaleZ,RotationX,RotationY,RotaionZ,InstanceType,Variant,Age,XWorld,YWorld,ZWorld, idString" << std::endl;
#endif
	int fullOutputItemCount = 15;

	for (const std::shared_ptr<InstanceSubOutput>& sub : *subVector)
	{
		int outputItemCount = sub->outputItemCount;

		outputFile
#if USE_OFFSET_FOR_INSTANCE_OUTPUT
			<< sub->xOffsetW << ","
			<< sub->yOffsetW << ","
			//<< sub->zOffsetW << ","
			<< sub->posZ << ","
#else
			<< sub->posX << ","
			<< sub->posY << ","
			<< sub->posZ << ","
#endif
			<< sub->scaleX << ","
			<< sub->scaleY << ","
			<< sub->scaleZ << ","
			<< sub->rotationX << ","
			<< sub->rotationY << ","
			<< sub->rotationZ << ","
			<< sub->instanceType << ","
			<< sub->variant << ","
			<< sub->age << ","
			<< sub->posX << ","
			<< sub->posY << ","
#if !USE_OUTPUT_INSTANCE_IDSTRING
			<< sub->posZ << std::endl;
#else
			<< sub->posZ << ","
			<< sub->idString << std::endl;
#endif


		/*if (outputItemCount != fullOutputItemCount)
		{
			outputFile << 1.0 << std::endl;
		}
		else
		{
			std::shared_ptr<TreeInstanceSubOutput> tree = std::dynamic_pointer_cast<TreeInstanceSubOutput>(sub);
			if (tree != nullptr)
			{
				outputFile << tree->age << std::endl;
			}
			else
			{
				std::shared_ptr<PointInstanceSubOutput> point = std::dynamic_pointer_cast<PointInstanceSubOutput>(sub);
				if (point != nullptr)
				{
					outputFile << point->age << std::endl;
				}
				else
				{
					outputFile << 0 << std::endl;
				}
			}

		}*/
	}

	outputFile.close();
	std::cout << "End to OutputCSVFileForSubInstances to : " << filePath << std::endl;
	return true;
}

bool OutputAllInstance(string outputFilePath, const InstanceSubOutputMap& allInstances)
{
	std::ofstream outputFile(outputFilePath);
	/*if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the sub csv file " << outputFilePath << std::endl;
		return false;
	}*/
	std::cout << "Start to OutputAllInstance to : " << outputFilePath << std::endl;

	outputFile << "XWorld,YWorld,ZWorld,ScaleX,ScaleY,ScaleZ,RotationX,RotationY,RotaionZ,InstanceType,Variant,Age,idString" << std::endl;

	for (auto pair : allInstances)
	{
		std::shared_ptr<InstanceSubOutputVector> subVector = pair.second;
		for (const std::shared_ptr<InstanceSubOutput>& sub : *subVector)
		{
			int outputItemCount = sub->outputItemCount;

			outputFile
				<< sub->posX << ","
				<< sub->posY << ","
				<< sub->posZ << ","
				<< sub->scaleX << ","
				<< sub->scaleY << ","
				<< sub->scaleZ << ","
				<< sub->rotationX << ","
				<< sub->rotationY << ","
				<< sub->rotationZ << ","
				<< sub->instanceType << ","
				<< sub->variant << ","
				<< sub->age << ","
				<< sub->idString << std::endl;
		}
	}
	outputFile.close();
	std::cout << "End to OutputAllInstance to : " << outputFilePath << std::endl;

	return true;
}