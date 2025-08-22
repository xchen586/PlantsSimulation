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

// Optimized maximum value propagation function
std::vector<std::vector<double>> PropagateLightingMax(
	const std::vector<std::vector<double>>& exposure_init_map,
	const std::vector<std::vector<bool>>& exposure_mask_map,
	int max_iterations/* = 1000*/,
	double propagation_factor/* = 0.5*/,
	double min_threshold/* = 0.001*/)
{
	// Input validation
	if (exposure_init_map.empty() || exposure_init_map[0].empty()) {
		return exposure_init_map;
	}

	const int rows = static_cast<int>(exposure_init_map.size());
	const int columns = static_cast<int>(exposure_init_map[0].size());

	// Validate all rows have consistent length
	for (const auto& row : exposure_init_map) {
		if (static_cast<int>(row.size()) != columns) {
			throw std::invalid_argument("All rows must have the same length");
		}
	}

	// Validate mask dimensions match
	if (exposure_mask_map.size() != rows || exposure_mask_map[0].size() != columns) {
		throw std::invalid_argument("Mask dimensions must match exposure map dimensions");
	}

	std::vector<std::vector<double>> exposure_map = exposure_init_map;
	std::vector<std::vector<double>> temp_map(rows, std::vector<double>(columns));

	// Pre-defined 8-directional neighbor offsets (using array for better performance)
	constexpr std::array<std::pair<int, int>, 8> directions = { {
		{-1, -1}, {-1, 0}, {-1, 1},
		{0, -1},           {0, 1},
		{1, -1},  {1, 0},  {1, 1}
	} };

	for (int iteration = 0; iteration < max_iterations; iteration++) {
		bool has_significant_change = false;
		double max_change = 0.0;

		// Copy current state to temporary map
		temp_map = exposure_map;

		// Iterate through each pixel for propagation
		for (int y = 0; y < rows; y++) {
			for (int x = 0; x < columns; x++) {
				const double current_value = exposure_map[y][x];

				// Only propagate from pixels with sufficient values
				if (current_value > min_threshold) {
					const double propagation_amount = current_value * propagation_factor;

					// Propagate to 8 directions
					for (const auto& [dx, dy] : directions) {
						const int nx = x + dx;
						const int ny = y + dy;

						// Boundary check
						if (nx >= 0 && nx < columns && ny >= 0 && ny < rows) {
							// Only propagate to exposed areas
							if (exposure_mask_map[ny][nx]) {
								const double old_value = temp_map[ny][nx];
								const double new_value = std::max(old_value, propagation_amount);
								temp_map[ny][nx] = new_value;

								const double change = std::abs(new_value - old_value);
								max_change = std::max(max_change, change);

								if (change > min_threshold) {
									has_significant_change = true;
								}
							}
						}
					}
				}
			}
		}

		// Update exposure_map
		exposure_map = std::move(temp_map);

		// Check convergence
		if (!has_significant_change || max_change < min_threshold) {
			std::cout << "Max propagation converged after " << iteration + 1 << " iterations" << std::endl;
			break;
		}
	}

	return exposure_map;
}

// Optimized average value propagation function
std::vector<std::vector<double>> PropagateLightingAverage(
	const std::vector<std::vector<double>>& exposure_init_map,
	const std::vector<std::vector<bool>>& exposure_mask_map,
	int max_iterations/* = 1000*/,
	double propagation_factor/* = 0.5*/,
	double min_threshold/* = 0.001*/,
	bool beSimple /* = false*/)
{
	// Input validation
	if (exposure_init_map.empty() || exposure_init_map[0].empty()) {
		return exposure_init_map;
	}

	const int rows = static_cast<int>(exposure_init_map.size());
	const int columns = static_cast<int>(exposure_init_map[0].size());

	// Validate all rows have consistent length
	for (const auto& row : exposure_init_map) {
		if (static_cast<int>(row.size()) != columns) {
			throw std::invalid_argument("All rows must have the same length");
		}
	}

	// Validate mask dimensions match
	if (exposure_mask_map.size() != rows || exposure_mask_map[0].size() != columns) {
		throw std::invalid_argument("Mask dimensions must match exposure map dimensions");
	}

	std::vector<std::vector<double>> exposure_map = exposure_init_map;
	std::vector<std::vector<double>> temp_map(rows, std::vector<double>(columns));

	// Pre-defined 8-directional neighbor offsets
	constexpr std::array<std::pair<int, int>, 8> directions = { {
		{-1, -1}, {-1, 0}, {-1, 1},
		{0, -1},           {0, 1},
		{1, -1},  {1, 0},  {1, 1}
	} };

	bool useCompare = false; // Not Use compared propagation by default

	for (int iteration = 0; iteration < max_iterations; iteration++) {
		bool has_significant_change = false;
		double max_change = 0.0;  // Add maximum change tracking

		// Copy current state
		temp_map = exposure_map;

		// Perform averaging for each exposed area
		for (int y = 0; y < rows; y++) {
			for (int x = 0; x < columns; x++) {
				// Only process exposed areas
				if (exposure_mask_map[y][x]) {
					double sum = exposure_map[y][x];
					int count = 1;

					// Calculate weighted average with neighbors
					for (const auto& [dx, dy] : directions) {
						const int nx = x + dx;
						const int ny = y + dy;

						if (nx >= 0 && nx < columns && ny >= 0 && ny < rows) {
							if (exposure_mask_map[ny][nx]) {
								sum += exposure_map[ny][nx] * propagation_factor;
								count++;
							}
						}
					}

					const double old_value = temp_map[y][x];
					const double average_value = sum / count;
					if (useCompare) {
						// Use maximum value propagation
						if (average_value < old_value)
						{
							temp_map[y][x] = std::min(old_value, average_value);
						}
						else {
							temp_map[y][x] = std::max(old_value, average_value);
						}
						
					}
					else {
						// Use average value propagation
						temp_map[y][x] = average_value;
					}

					const double change = std::abs(temp_map[y][x] - old_value);
					max_change = std::max(max_change, change);

					if (change > min_threshold) {
						has_significant_change = true;
					}
				}
			}
		}

		exposure_map = std::move(temp_map);

		// Improved convergence check
		if (!beSimple && (!has_significant_change || max_change < min_threshold)) {
			std::cout << "Average propagation converged after " << iteration + 1 << " iterations (max change: "
				<< max_change << ")" << std::endl;
			break;
		}
	}

	return exposure_map;
}

// More efficient 4-directional propagation version (if diagonal propagation is not needed)
std::vector<std::vector<double>> PropagateLightingMax4Dir(
	const std::vector<std::vector<double>>& exposure_init_map,
	const std::vector<std::vector<bool>>& exposure_mask_map,
	int max_iterations/* = 1000*/,
	double propagation_factor/* = 0.5*/,
	double min_threshold/* = 0.001*/)
{
	if (exposure_init_map.empty() || exposure_init_map[0].empty()) {
		return exposure_init_map;
	}

	const int rows = static_cast<int>(exposure_init_map.size());
	const int columns = static_cast<int>(exposure_init_map[0].size());

	std::vector<std::vector<double>> exposure_map = exposure_init_map;
	std::vector<std::vector<double>> temp_map(rows, std::vector<double>(columns));

	// Use only 4 directions (up, down, left, right)
	constexpr std::array<std::pair<int, int>, 4> directions = { {
		{-1, 0}, {1, 0}, {0, -1}, {0, 1}
	} };

	for (int iteration = 0; iteration < max_iterations; iteration++) {
		bool has_significant_change = false;
		double max_change = 0.0;

		temp_map = exposure_map;

		for (int y = 0; y < rows; y++) {
			for (int x = 0; x < columns; x++) {
				const double current_value = exposure_map[y][x];

				if (current_value > min_threshold) {
					const double propagation_amount = current_value * propagation_factor;

					for (const auto& [dx, dy] : directions) {
						const int nx = x + dx;
						const int ny = y + dy;

						if (nx >= 0 && nx < columns && ny >= 0 && ny < rows) {
							if (exposure_mask_map[ny][nx]) {
								const double old_value = temp_map[ny][nx];
								const double new_value = std::max(old_value, propagation_amount);
								temp_map[ny][nx] = new_value;

								const double change = std::abs(new_value - old_value);
								max_change = std::max(max_change, change);

								if (change > min_threshold) {
									has_significant_change = true;
								}
							}
						}
					}
				}
			}
		}

		exposure_map = std::move(temp_map);

		if (!has_significant_change || max_change < min_threshold) {
			std::cout << "4-directional propagation converged after " << iteration + 1 << " iterations" << std::endl;
			break;
		}
	}

	return exposure_map;
}

std::string Get2DArrayRawCsvFilePathForRegion(const string& outputDir, int lod, int intXIdx, int intYIdx, int intZIdx)
{
	const int MAX_PATH = 250;
	char subFileName[MAX_PATH];
	char subFilePath[MAX_PATH];
	memset(subFileName, 0, sizeof(char) * MAX_PATH);
	memset(subFilePath, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(subFileName, MAX_PATH, "regions_%d_%d_raw.csv", intXIdx, intZIdx);
	snprintf(subFilePath, MAX_PATH, "%s/%s", outputDir.c_str(), subFileName);
#else
	sprintf_s(subFileName, MAX_PATH, "regions_%d_%d_raw.csv", intXIdx, intZIdx);
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

bool LoadRegionInfoFromCSV(const string& filePath, RegionInfoMap& regionInfoMap, std::vector<std::vector<CCellInfo*>>* pCellTable, InputImageMetaInfo* pMetaInfo)
{
	if (pCellTable == nullptr || pMetaInfo == nullptr) {
		std::cerr << "Error: pCellTable or pMetaInfo is null! " << std::endl;
		std::cerr << "Failed to LoadRegionInfoFromCSV due to data is not ready!!!!!!" << std::endl;
		return false;
	}
	std::cout << "Start to LoadRegionInfoFromCSV from : " << filePath << std::endl;
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open the csv file :" << filePath << std::endl;
		return false;
	}
	char delimiter = ',';
	int columnCount = countColumnsInCSV(filePath, delimiter);
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
		//string treeCountString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_Col_TreeCount)];
		string type1String = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_COL_Type1)];
		string nameString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_COL_Name)];
		string centroidXString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_COL_CentroidX)];
		string centroidYString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_COL_CentroidY)];
		
		string centroidZString = "0";
		if (columnCount > 11)
		{
			centroidZString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_COL_CentroidZ)];
			info->hasZPos = true;
		}
		
		string regionLevelString = "1";
		if (columnCount > 12)
		{
			regionLevelString = row[static_cast<size_t>(Region_Info_CSV_Columns::RI_COL_RegionLevel)];
		}

		info->regionId = static_cast<unsigned int>(std::stoul(regionIdString));
		info->area = static_cast<unsigned int>(std::stoul(areaString));
		info->averageHeight = static_cast<unsigned int>(std::stoul(averageHeightString));
		info->minHeight = static_cast<unsigned int>(std::stoul(minHeightString));
		info->maxHeight = static_cast<unsigned int>(std::stoul(maxHeightString));
		info->nearSea = static_cast<unsigned int>(std::stoul(nearSeaString));
		info->averageHumidity = static_cast<unsigned int>(std::stoul(averageHumidityString));
		//info->treeCount = static_cast<unsigned int>(std::stoul(treeCountString));
		info->type1 = type1String;
		info->name = nameString;
		info->centroidX = static_cast<unsigned int>(std::stoul(centroidXString));
		info->centroidY = static_cast<unsigned int>(std::stoul(centroidYString));
		info->centroidZ = static_cast<int>(std::stoul(centroidZString));
		info->regionLevel = static_cast<unsigned short>(std::stoul(regionLevelString));
		info->eId = info->regionId;
		
		/*double zPos = GetHeightFor2DPointFromCellTable(info->centroidX, info->centroidY, pCellTable, pMetaInfo);
		info->hasZPos = (static_cast<int>(zPos) != UNAVAILBLE_NEG_HEIGHT);
		if (!info->hasZPos)
		{
			std::cout << "Error: Region : " << info->regionId << " does not get the height !!!" << std::endl;
		}
		else
		{
			info->centroidZ = static_cast<int>(zPos);
		}*/

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

	outputFile << "RegionId" << ","
		<< "Area" << "," 
		<< "AvgHeight" << ","
		<< "MinHeight" << ","
		<< "MaxHeight" << ","
		<< "NearSea" << ","
		<< "AvgHumidity" << ","
		//<< "TreeCount" << ","
		<< "type 1" << ","
		<< "Name" << ","
		<< "CentroidX" << ","
		<< "CentroidY" << ","
		<< "CentroidZ" << ","
		<< "ExtrId" << ","
		<< "WorldCentroidX" << ","
		<< "WorldCentroidY" << ","
		<< "WorldCentroidZ" << ","
		<< "RegionLevel" << std::endl;

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
				//<< info->treeCount << ","
				<< info->type1 << ","
				<< info->name << ","
				<< info->centroidCoord.xOffsetW << ","
				<< info->centroidCoord.yOffsetW << ","
				<< info->centroidCoord.zOffsetW << ","
				<< info->eId << ","
				<< info->centroidCoord.posX << ","
				<< info->centroidCoord.posY << ","
				<< info->centroidCoord.posZ << ","
				<< info->regionLevel << std::endl;
		}
	}

	outputFile.close();
	std::cout << "End to SaveSubRegionInfoToCSVFile to : " << filePath << std::endl;
	
	return true;
}

double GetHeightFor2DPointFromCellTable(double xPos, double yPos, std::vector<std::vector<CCellInfo*>>* pCellTable, InputImageMetaInfo* pMetaInfo)
{
	double ret = UNAVAILBLE_NEG_HEIGHT;
	if (!pCellTable || !pMetaInfo)
	{
		return ret;
	}

	double xRatio = pMetaInfo->xRatio;
	double yRatio = pMetaInfo->yRatio;

	int tableRowsCount = (*pCellTable).size();
	int tableColsCount = (*pCellTable)[0].size();

	int rowIdx = static_cast<int>(xPos / xRatio);
	int colIdx = static_cast<int>(yPos / yRatio);

	CCellInfo* pCell = nullptr;
	if (((rowIdx >= 0) && (rowIdx < tableRowsCount))
		&& ((colIdx >= 0) && (colIdx < tableColsCount))) {
		pCell = (*pCellTable)[rowIdx][colIdx];
	}
	if ((pCell != nullptr) && (pCell->GetHasHeight()))
	{
		ret = pCell->GetHeight();
	}

	return ret;
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
		double dIndexX = (sub->coord.vX - vfMinX) / scaleWidthRate;
		double dIndexZ = (sub->coord.vZ - vfMinZ) / scaleHeightRate;
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
	outputFile << "X,Y,Z,ScaleX,ScaleY,ScaleZ,RotationX,RotationY,RotaionZ,InstanceType,Variant,Age,XWorld,YWorld,ZWorld,InstanceIndex,Level" << std::endl;
#else
	outputFile << "X,Y,Z,ScaleX,ScaleY,ScaleZ,RotationX,RotationY,RotaionZ,InstanceType,Variant,Age,XWorld,YWorld,ZWorld,idString,Level" << std::endl;
#endif
	int fullOutputItemCount = 15;

	for (const std::shared_ptr<InstanceSubOutput>& sub : *subVector)
	{
		int outputItemCount = sub->outputItemCount;

		outputFile

			<< sub->xOffsetW << ","
			<< sub->yOffsetW << ","
			<< sub->posZ << "," //<< sub->zOffsetW << ","

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
			<< sub->posZ << ","
			<< sub->index << ","
			<< sub->level << std::endl;
#else
			<< sub->posZ << ","
			<< sub->idString << ","
			<< sub->level <<std::endl;
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

bool OutputAllInstance(string outputFilePath, const InstanceSubOutputMap* pTreeInstances, const InstanceSubOutputMap* pPoiInstances)
{
	std::ofstream outputFile(outputFilePath);
	/*if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the sub csv file " << outputFilePath << std::endl;
		return false;
	}*/
	std::cout << "Start to OutputAllInstance to : " << outputFilePath << std::endl;

	outputFile << "XWorld,YWorld,ZWorld,ScaleX,ScaleY,ScaleZ,RotationX,RotationY,RotaionZ,InstanceType,Variant,Age,idString,Level" << std::endl;

	if (pTreeInstances)
	{
		for (auto pair : *pTreeInstances)
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
					<< sub->idString << ","
					<< sub->level << std::endl;
			}
		}
	}
	
	if (pPoiInstances)
	{
		for (auto pair : *pPoiInstances)
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
					<< sub->idString << "," 
					<< sub->level << std::endl;
			}
		}
	}
	
	outputFile.close();
	std::cout << "End to OutputAllInstance to : " << outputFilePath << std::endl;

	return true;
}

int GetInstancesCountFromInstanceSubOutputMap(const InstanceSubOutputMap& instanceMap)
{
	int count = 0;
	for (const auto& pair : instanceMap) {
		const std::shared_ptr<InstanceSubOutputVector>& subVector = pair.second;
		if (subVector) {
			count += static_cast<int>(subVector->size());
		}
	}
	return count;
}