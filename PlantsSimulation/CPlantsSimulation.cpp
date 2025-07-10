#include "CPlantsSimulation.h"
#include "CAppFuncs.h"

#if __APPLE__
    #include "../Common/include/CTimeCounter.h"
    #include "../Common/include/PsMarco.h"
#else
    #include "..\Common\include\CTimeCounter.h"
    #include "..\Common\include\PsMarco.h"
#endif

/*bool CPlantsSimulation::loadInputImageFile(const string& inputImageFile)
{
	this->input_image_data = stbi_load(inputImageFile.c_str(), &this->input_image_width, &this->input_image_height, &this->input_image_comp, 1);
	if (!this->input_image_data) {
		return false;
	}
	int width = this->input_image_width;
	int height = this->input_image_height;
	unsigned char* image_data = this->input_image_data;
	for (auto i = 0; i < width; i++) {
		for (auto j = 0; j < height; j++) {
			int idx = (width * j + i) * 3;
			uint8_t redValue = image_data[idx + 0];
			uint8_t greenValue = image_data[idx + 0];
			uint8_t blueValue = image_data[idx + 0];
			CCellData* cell = new CCellData(redValue, greenValue, blueValue);
		}
	}
	return true;
}*/

void CPlantsSimulation::DeInitializeForMakeInstances()
{
	if (m_pInstanceExporter) {
		delete m_pInstanceExporter;
		m_pInstanceExporter = nullptr;
	}

	if (m_pForest) {
		delete m_pForest;
		m_pForest = nullptr;
	}

	if (m_pCellTable) {
		int rows = m_pCellTable->size();
		int cols = (*m_pCellTable)[0].size();
		for (int i = 0; i < rows; ++i) {
			for (int j = 0; j < cols; ++j) {
				CCellInfo* cell = (*m_pCellTable)[i][j];
				if (cell) {
					delete cell;
				}
			}
		}
		m_pCellTable = nullptr;
	}

	m_PoisLocations.clear();

	if (m_p2dCaveLevel0Nodes)
	{
		delete m_p2dCaveLevel0Nodes;
		m_p2dCaveLevel0Nodes = nullptr;
	}
	if (m_p2dCaveLevel1Nodes)
	{
		delete m_p2dCaveLevel1Nodes;
		m_p2dCaveLevel1Nodes = nullptr;
	}
	ClearCellTable();
	ClearImage();
	ClearImageMeta();
}

void CPlantsSimulation::DeInitialize()
{
	DeInitializeForMakeInstances();
}

void CPlantsSimulation::ClearCellTable()
{
	if (m_pCellTable != nullptr)
	{
		if (m_topLayerImage)
		{
			const int newWidth = m_topLayerImage->input_image_width;
			const int newHeight = m_topLayerImage->input_image_height;
			for (auto i = 0; i < newWidth; i++) 
			{
				for (auto j = 0; j < newHeight; j++) 
				{
					CCellInfo* cell = (*m_pCellTable)[i][j];
					if (cell)
					{
						delete cell;
						(*m_pCellTable)[i][j] = nullptr;
					}
				}
			}
		}
		delete m_pCellTable;
		m_pCellTable = nullptr;
	}
}

void CPlantsSimulation::ClearImage()
{
	if (m_topLayerImage) {
		if (m_topLayerImage->input_image_data)
		{
			delete m_topLayerImage->input_image_data;
			m_topLayerImage->input_image_data = nullptr;
		}
		delete m_topLayerImage;
		m_topLayerImage = nullptr;
	}
}

void CPlantsSimulation::ClearImageMeta()
{
	if (m_topLayerMeta) {
		delete m_topLayerMeta;
		m_topLayerMeta = nullptr;
	}
}

bool CPlantsSimulation::LoadInputImage()
{
	try
	{
		m_topLayerImage = LoadInputImageFile(m_inputImageFile);
		if (!m_topLayerImage)
		{
			return false;
		}

		const int newWidth = m_topLayerImage->input_image_width;
		const int newHeight = m_topLayerImage->input_image_height;

		m_pCellTable = new std::vector<std::vector<CCellInfo*>>(newWidth, std::vector<CCellInfo*>(newHeight, nullptr));
		if (!m_pCellTable) {
			return false;
		}

		for (auto i = 0; i < newWidth; i++) {
			for (auto j = 0; j < newHeight; j++) {
				int idx = (newWidth * j + i) * 3;
				uint8_t redValue = m_topLayerImage->input_image_data[idx + 0];
				uint8_t greenValue = m_topLayerImage->input_image_data[idx + 1];
				uint8_t blueValue = m_topLayerImage->input_image_data[idx + 2];
				CCellInfo* cell = new CCellInfo(redValue, greenValue, blueValue);
				if (!cell) {
					std::cout << "Fail to create CCellData at i = " << i << " j = " << j << std::endl;
				}
				else {
					(*m_pCellTable)[i][j] = cell;
				}

			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		ClearCellTable();
		ClearImage();
		return false;
	}

	int rows = m_pCellTable->size();
	int cols = m_pCellTable[0].size();
	std::vector<std::vector<unsigned char>> humidity4K(rows, std::vector<unsigned char>(cols));
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			humidity4K[i][j] = static_cast<unsigned char>(GetValueFromNormalized((*m_pCellTable)[i][j]->GetMoisture(), 0, 255));
		}
	}

	const int MAX_PATH = 250;

	int exportXLow = m_roadInputHeightMapWidth * m_tileScale;
	int exportYLow = m_roadInputHeightMapHeight * m_tileScale;
	//std::vector<std::vector<byte>> humidityExportLow = ScaleArray(humidity4K, exportXLow, exportYLow);
	std::vector<std::vector<unsigned char>> humidityExportLowInvert = resample2DUCharWithAverage(humidity4K, exportXLow, exportYLow);
	std::vector<std::vector<unsigned char>> humidityExportLow = invert2DArray(humidityExportLowInvert);
	char byte_humidity_map_low_raw[MAX_PATH];
	memset(byte_humidity_map_low_raw, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(byte_humidity_map_low_raw, MAX_PATH, "%s/%d_%d_%d_%d_%d_%d_byte_humidity_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#else
	sprintf_s(byte_humidity_map_low_raw, MAX_PATH, "%s\\%d_%d_%d_%d_%d_%d_byte_humidity_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#endif
	bool outputHumidityMapLow = Output2DVectorToRawFile(humidityExportLow, byte_humidity_map_low_raw);

#if USE_OUTPUT_HIGH_ROAD_DATA
	int exportHighRatio = 2;
	int exportXHigh = exportXLow * exportHighRatio;
	int exportYHigh = exportYLow * exportHighRatio;
	//std::vector<std::vector<byte>> humidityExportHigh = ScaleArray(humidity4K, exportXHigh, exportYHigh);
	std::vector<std::vector<unsigned char>> humidityExportHighInvert = resample2DUCharWithAverage(humidity4K, exportXHigh, exportYHigh);
	std::vector<std::vector<unsigned char>> humidityExportHigh = invert2DArray(humidityExportHighInvert);

	char byte_humidity_map_high_raw[MAX_PATH];
	memset(byte_humidity_map_high_raw, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(byte_humidity_map_high_raw, MAX_PATH, "%s/%d_%d_%d_%d_%d_%d_byte_humidity_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth * exportHighRatio, m_roadInputHeightMapHeight * exportHighRatio);
#else
	sprintf_s(byte_humidity_map_high_raw, MAX_PATH, "%s\\%d_%d_%d_%d_%d_%d_byte_humidity_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth * exportHighRatio, m_roadInputHeightMapHeight * exportHighRatio);
#endif
	bool outputHumidityMapHigh = Output2DVectorToRawFile(humidityExportHigh, byte_humidity_map_high_raw);
#endif

	return true;
}

bool CPlantsSimulation::LoadImageMetaFile()
{
	bool ret = false;
	std::ifstream inputFile(m_inputImageMetaFile);

	if (!inputFile.is_open()) {
		std::cerr << "Error opening image meta file!" << std::endl;
		return false;
	}

	ClearImageMeta();
	m_topLayerMeta = new InputImageMetaInfo();
	if (!m_topLayerMeta)
	{
		return false;
	}

	try {
		// Read each line from the file and assign it to the struct members
		//std::getline(inputFile, person.name);
		inputFile >> m_topLayerMeta->xRatio;
		inputFile >> m_topLayerMeta->x0;
		inputFile >> m_topLayerMeta->y0;
		inputFile >> m_topLayerMeta->yRatio;
		inputFile >> m_topLayerMeta->batch_min_x;
		inputFile >> m_topLayerMeta->batch_min_y;
		//inputFile.ignore(); // Ignore the newline character left in the stream
		ret = true;
	}
	catch (const std::exception& e) {
		std::cerr << "Error while reading data from : " << e.what() << std::endl;
		ret = false;
	}

	inputFile.close();
	if (!ret)
	{
		ClearImageMeta();
		return ret;
	}
	std::cout << "Image Meta Info xRatio: " << m_topLayerMeta->xRatio << std::endl;
	std::cout << "Image Meta Info x0 : " << m_topLayerMeta->x0 << std::endl;
	std::cout << "Image Meta Info y0 : " << m_topLayerMeta->y0 << std::endl;
	std::cout << "Image Meta Info yRatio: " << m_topLayerMeta->yRatio << std::endl;
	std::cout << "Image Meta Info batch_min_x: " << m_topLayerMeta->batch_min_x << std::endl;
	std::cout << "Image Meta Info batch_min_y: " << m_topLayerMeta->batch_min_y << std::endl;
	return ret;
}

bool CPlantsSimulation::LoadRegionsTest()
{
	bool ret = true;
	const int regionsWidth = m_roadInputHeightMapWidth * m_tileScale;
	const int regionsHeight = m_roadInputHeightMapHeight * m_tileScale;

	const int worldTileWidth = m_tilePixelMeterWidth * m_tileScale;
	const int worldTileHeight = m_tilePixelMeterHeight * m_tileScale;

	int region_lod = 10;
	const double cellScale = (1 << region_lod) * VoxelFarm::CELL_SIZE;

	const double vfScale = 2.0;
	CAffineTransform transform(
		CAffineTransform::sAffineVector{
			0.0, 0.0, 0.0
		}, vfScale, CAffineTransform::eTransformMode::TM_YZ_ROTATE);
	auto worldOriginVF = transform.WC_TO_VF(CAffineTransform::sAffineVector{ 0.0, 0.0, 0.0 });
	std::cout << "current region lod is : " << region_lod << std::endl;
	std::cout << "current region cellScale is : " << cellScale << std::endl;

	std::vector<std::vector<int>> regionsInput = Read2DIntArray(m_regionsRawFile, regionsWidth, regionsHeight);
	m_regionsVector.clear();
	m_regionMap.clear();

	double xRegionRatio = worldTileWidth / regionsWidth;
	double yRegionRatio = worldTileHeight / regionsHeight;
	double xRatio = m_topLayerMeta->xRatio;
	double yRatio = m_topLayerMeta->yRatio;
	double batch_min_x = m_topLayerMeta->batch_min_x;
	double batch_min_y = m_topLayerMeta->batch_min_y;
	double x0 = m_topLayerMeta->x0;
	double y0 = m_topLayerMeta->y0;
	
	int tableRowsCount = (*m_pCellTable).size();
	int tableColsCount = (*m_pCellTable)[0].size();

	int negativeHeightCount = 0;
	int total_non_zero_regions = 0;
	for (int row = 0; row < regionsWidth; row++)
	{
		for (int col = 0; col < regionsHeight; col++)
		{
			if (regionsInput[row][col] != 0)
			{
				int regionId = regionsInput[row][col];
				std::shared_ptr<RegionStruct> reg = std::make_shared<RegionStruct>(regionId, row, col);

				bool hasHeight = true;
				double xPos = row * xRegionRatio;
				double yPos = col * yRegionRatio;
				double zPos = 0;

				int rowIdx = static_cast<int>(xPos / xRatio);
				int colIdx = static_cast<int>(yPos / yRatio);

				CCellInfo* pCell = nullptr;
				if (((rowIdx >= 0) && (rowIdx < tableRowsCount))
					&& ((colIdx >= 0) && (colIdx < tableColsCount))) {
					pCell = (*m_pCellTable)[rowIdx][colIdx];
				}
				if ((pCell != nullptr) && (pCell->GetHasHeight()))
				{
					zPos = pCell->GetHeight();
				}
				else
				{
					hasHeight = false;
				}
				bool negativeZPos = false;
				if ((zPos < 0) && hasHeight)
				{
					negativeZPos = true;
					negativeHeightCount++;
					zPos = 0;
				}
				double posX = xPos + batch_min_x + x0;
				double posY = yPos + batch_min_y + y0;
				double posZ = zPos;

				bool beSetup = reg->coord.SetupCellCoordinate(posX, posY, posZ, transform, region_lod);
				//string keyString = GetKeyStringForRegion(m_outputDir, reg->cellXIdx, reg->cellZIdx);
				VoxelFarm::CellId keyId = reg->coord.cellId;
				RegionSubOutputMap::iterator iter = m_regionMap.find(keyId);
				if (m_regionMap.end() == iter)
				{
					m_regionMap[keyId] = std::make_shared<RegionSubOutputVector>();
				}

				std::shared_ptr<RegionSubOutputVector> subVector = m_regionMap[keyId];
				subVector->push_back(reg);
				m_regionsVector.push_back(reg);
				total_non_zero_regions++;
			}
		}
	}
	std::cout << "Total non region unit count is " << total_non_zero_regions << std::endl;
	std::cout << "Total region cell count is : " << m_regionMap.size() << std::endl;

	const int MAX_PATH = 250;
	char subRegionOutput_Dir[MAX_PATH];
	memset(subRegionOutput_Dir, 0, sizeof(char) * MAX_PATH);

#if __APPLE__ 
	snprintf(subFullOutput_Dir, MAX_PATH, "%s/regionoutput", m_outputDir.c_str());
#else
	sprintf_s(subRegionOutput_Dir, MAX_PATH, "%s\\regionoutput", m_outputDir.c_str());
#endif

	if (!std::filesystem::exists(subRegionOutput_Dir)) {
		if (!std::filesystem::create_directory(subRegionOutput_Dir)) {
			std::cerr << "Failed to create the directory of subRegionOutput_Dir!" << std::endl;
			return false;
		}
	}
	RemoveAllFilesInFolder(subRegionOutput_Dir);

	for (const auto& pair : m_regionMap)
	{
		int cellX = 0;
		int cellY = 0;
		int cellZ = 0;
		int lod = 0;

		VoxelFarm::unpackCellId(pair.first, lod, cellX, cellY, cellZ);
		int subRegionsCount = (*pair.second).size();
		std::cout << "Cell_" << lod << "_" << cellX << "_" << cellY << "_" << cellZ << " has regions count : " << subRegionsCount << std::endl;
		string arrayFilePath = Get2DArrayFilePathForRegion(subRegionOutput_Dir, lod, cellX, cellY, cellZ);
		
		bool beSaved = OutputArrayFileForSubRegionsTest(arrayFilePath, transform, pair.first, pair.second);
	}
	return ret;
}



bool CPlantsSimulation::LoadAndOutputRegions()
{
	if (!std::filesystem::exists(m_regionsRawFile)) {
		std::cerr << "Regions raw file : " << m_regionsRawFile << " does not exist!" << std::endl;
		return false;
	}
	if (!std::filesystem::exists(m_regionsInfoFile)) {
		std::cerr << "Regions info file : " << m_regionsInfoFile << " does not exist!" << std::endl;
		return false;
	}

	bool ret = true;
	const int regionsWidth = m_roadInputHeightMapWidth * m_tileScale;
	const int regionsHeight = m_roadInputHeightMapWidth * m_tileScale;
	const int worldTileWidth = m_tilePixelMeterWidth * m_tileScale;
	const int worldTileHeight = m_tilePixelMeterHeight * m_tileScale;

	const int MAX_PATH = 250;
	char subRegionOutput_Dir[MAX_PATH];
	memset(subRegionOutput_Dir, 0, sizeof(char) * MAX_PATH);

#if __APPLE__ 
	snprintf(subFullOutput_Dir, MAX_PATH, "%s/regionoutput", m_outputDir.c_str());
#else
	sprintf_s(subRegionOutput_Dir, MAX_PATH, "%s\\regionoutput", m_outputDir.c_str());
#endif

	if (!std::filesystem::exists(subRegionOutput_Dir)) {
		if (!std::filesystem::create_directory(subRegionOutput_Dir)) {
			std::cerr << "Failed to create the directory of subRegionOutput_Dir!" << std::endl;
			return false;
		}
	}
	RemoveAllFilesInFolder(subRegionOutput_Dir);
	
	std::vector<std::vector<int>> regionsIntInput = Read2DIntArray(m_regionsRawFile, regionsWidth, regionsHeight);
	bool loadAllRegionInfo = LoadRegionInfoFromCSV(m_regionsInfoFile, m_regionInfoMap, m_pCellTable, m_topLayerMeta);

	char inputRegionRawCSV[MAX_PATH];
	memset(inputRegionRawCSV, 0, sizeof(char) * MAX_PATH);
#if __APPLE__ 
	snprintf(inputRegionRawCSV, MAX_PATH, "%s/region_raw.csv", m_outputDir.c_str());
#else
	sprintf_s(inputRegionRawCSV, MAX_PATH, "%s\\region_raw.csv", m_outputDir.c_str());
#endif
	bool saveRegionRawCSV = Write2DArrayAsCSV(inputRegionRawCSV, regionsIntInput);
	
	std::cout << "Total region info count is " << m_regionInfoMap.size() << std::endl;
	
	int regionPlaceCount = 0;
	int maxRegionId = 0;
	for (int x = 0; x < regionsWidth; x++)
	{
		for (int y = 0; y < regionsHeight; y++)
		{
			int regionValue = regionsIntInput[x][y];
			if ( regionValue > 0)
			{
				regionPlaceCount++;
				maxRegionId = std::max(maxRegionId, regionValue);
			}
		}
	}
	std::cout << "Region place count is " << regionPlaceCount << std::endl;
	std::cout << "Max Region Id (region type count) is " << maxRegionId << std::endl;
	 
	const double vfScale = 2.0;
	CAffineTransform transform(
		CAffineTransform::sAffineVector{
			0.0, 0.0, 0.0
		}, vfScale, CAffineTransform::eTransformMode::TM_YZ_ROTATE);
	auto worldOriginVF = transform.WC_TO_VF(CAffineTransform::sAffineVector{ 0.0, 0.0, 0.0 });
	int region_lod = 10;
	const double cellScale = (1 << region_lod) * VoxelFarm::CELL_SIZE;

	double xRatio = m_topLayerMeta->xRatio;
	double yRatio = m_topLayerMeta->yRatio;
	double batch_min_x = m_topLayerMeta->batch_min_x;
	double batch_min_y = m_topLayerMeta->batch_min_y;
	double x0 = m_topLayerMeta->x0;
	double y0 = m_topLayerMeta->y0;

	for (auto pair : m_regionInfoMap)
	{
		shared_ptr<RegionInfo> reg = pair.second;
		double posX = batch_min_x + x0 + reg->centroidX;
		double posY = batch_min_y + y0 + reg->centroidY;
		double posZ = reg->centroidZ;
		reg->centroidCoord.SetupCellCoordinate(posX, posY, posZ, transform, region_lod);
	}

	auto tileStartWorld = transform.VF_TO_WC(CAffineTransform::sAffineVector{ static_cast<double>(0), 0.0, static_cast<double>(0)});
	auto tileEndWorld = transform.VF_TO_WC(CAffineTransform::sAffineVector{ static_cast<double>(cellScale), 0.0, static_cast<double>(cellScale) });
	auto worldSizeX = std::abs(tileStartWorld.X - tileEndWorld.X);
	//auto worldSizeY = std::abs(tileStartWorld.Y - tileEndWorld.Y);
	const int regionWidthScale = worldTileWidth / regionsWidth; //One point in the cell region array should has 100 meters.
	const int regionHeightScale = worldTileHeight / regionsHeight;

	const int cellArrayWidth = static_cast<int>(worldSizeX / regionWidthScale);
	const int cellArrayHeight = static_cast<int>(worldSizeX / regionHeightScale);
	
	double xRegionRatio = worldTileWidth / regionsWidth;
	double yRegionRatio = worldTileHeight / regionsHeight;
	const int topX = batch_min_x + x0;
	const int topY = batch_min_y + y0;
	const int bottomX = batch_min_x + x0 + worldTileWidth;
	const int bottomY = batch_min_y + y0 + worldTileHeight;

	auto topVf = transform.WC_TO_VF(CAffineTransform::sAffineVector{ static_cast<double>(topX), static_cast<double>(topY), 0.0 });
	auto bottomVf = transform.WC_TO_VF(CAffineTransform::sAffineVector{ static_cast<double>(bottomX), static_cast<double>(bottomY), 0.0 });
	const int min_vx = static_cast<int>(std::min(topVf.X, bottomVf.X) / cellScale);
	const int min_vy = static_cast<int>(std::min(topVf.Z, bottomVf.Z) / cellScale);
	const int max_vx = static_cast<int>(std::max(topVf.X, bottomVf.X) / cellScale);
	const int max_vy = static_cast<int>(std::max(topVf.Z, bottomVf.Z) / cellScale);

	int stepX = max_vx - min_vx;
	int stepY = max_vy - min_vy;
	std::cout << "Cells XStep max_vx - min_vx = " << stepX << std::endl;
	std::cout << "Cells YStep max_vy - min_vy = " << stepY << std::endl;

	std::set<VoxelFarm::CellId> cellSet;
	for (int x = min_vx; x <= max_vx; x++) // use <= because it need involve all the related cells (increase the cells)
	{
		for (int y = min_vy; y <= max_vy; y++) // use <=  to because it need involve all the related cells (increase the cells)
		{
			VoxelFarm::CellId cell = VoxelFarm::packCellId(region_lod, x, 0, y);
			cellSet.insert(cell);
		}
	}

	std::cout << "current region lod is : " << region_lod << std::endl;
	std::cout << "current region cellScale is : " << cellScale << std::endl;
	std::cout << "Total cells count is " << cellSet.size() << std::endl;

	for (auto cellId : cellSet)
	{
		int subRegionsCount = 0;
		int cellX = 0;
		int cellY = 0;
		int cellZ = 0;
		int lod = 0;

		VoxelFarm::unpackCellId(cellId, lod, cellX, cellY, cellZ);
		std::cout << "Cell_" << lod << "_" << cellX << "_" << cellY << "_" << cellZ << std::endl;
	}

	int totalRegionsCount = 0;
	for (auto cellId : cellSet)
	{
		int subRegionsCount = 0;
		int cellX = 0;
		int cellY = 0;
		int cellZ = 0;
		int lod = 0;

		std::set<unsigned int> subRegionSet;

		VoxelFarm::unpackCellId(cellId, lod, cellX, cellY, cellZ);
		std::cout << "Cell_" << lod << "_" << cellX << "_" << cellY << "_" << cellZ << std::endl;
		std::vector<std::vector<uint16_t>> scaledArray(cellArrayWidth, std::vector<uint16_t>(cellArrayHeight));
		for (int x = 0; x < cellArrayWidth; x++)
		{
			for (int y = 0; y < cellArrayHeight; y++)
			{
				double vfX = cellX * cellScale + x * cellScale / cellArrayWidth;
				double vfZ = cellZ * cellScale + y * cellScale / cellArrayHeight;

				auto worldPos = transform.VF_TO_WC(CAffineTransform::sAffineVector{ vfX, 0.0, vfZ});

				int tilePosX = worldPos.X - batch_min_x - x0;
				int tilePosY = worldPos.Y - batch_min_y - y0;

				if ((tilePosX >=  0) && (tilePosX <= worldTileWidth)
					&& (tilePosY >= 0) && (tilePosY <= worldTileHeight))
				{
					int iIndexX = tilePosX / regionWidthScale;
					int iIndexY = tilePosY / regionHeightScale;

					int regionValue = regionsIntInput[iIndexX][iIndexY];
					scaledArray[x][y] = regionValue;
					if (regionValue != 0)
					{
						subRegionsCount++;
						totalRegionsCount++;
						subRegionSet.insert(regionValue);
					}
					else
					{
						//std::cout << "there is x : " << x << " y : " << y << " region value is zero" << std::endl;
					}
						
				}
				else
				{
					scaledArray[x][y] = 0;
				}
			}
		}

		std::cout << "Cell_" << lod << "_" << cellX << "_" << cellY << "_" << cellZ << " has regions count : " << subRegionsCount << std::endl;
		string arrayFilePath = Get2DArrayFilePathForRegion(subRegionOutput_Dir, lod, cellX, cellY, cellZ);
		bool saveSubRaw = Write2DArrayAsRaw(arrayFilePath, scaledArray);
		
		string arrayCSVFilePath = Get2DArrayRawCsvFilePathForRegion(subRegionOutput_Dir, lod, cellX, cellY, cellZ);
		bool saveSubRawCSV = Write2DArrayAsCSV(arrayCSVFilePath, scaledArray);

		string subRegionInfoPath = GetSubRegionInfoOutputCSVFilePathForRegion(subRegionOutput_Dir, lod, cellX, cellY, cellZ);
		bool saveSubInfo = SaveSubRegionInfoToCSVFile(subRegionInfoPath, m_regionInfoMap, subRegionSet);
	}
	
	std::cout << "Total regions count is " << totalRegionsCount << std::endl;
	return ret;
}

bool CPlantsSimulation::LoadInputHeightMap()
{
	bool needMaskPositive = false;
	if (!m_topLayerImage || !m_pCellTable)
	{
		return false;
	}
	const int width = m_topLayerImage->input_image_width;
	const int height = m_topLayerImage->input_image_height;

	string meshHeightMasksFile = m_isLevel1Instances ? m_mesh2HeightMasksFile : m_meshHeightMasksFile;

	std::vector<std::vector<short>> mesh0HeightMasksShort4096 = Read2DShortArray(m_meshHeightMasksFile, width, height);
	std::vector<std::vector<short>> mesh1HeightMasksShort4096 = Read2DShortArray(m_mesh2HeightMasksFile, width, height);
	std::vector<std::vector<short>> meshHeightMasksShort4096 = Read2DShortArray(meshHeightMasksFile, width, height);
	std::vector<std::vector<short>> pcHeightMasksShort4096 = Read2DShortArray(m_pcHeightMasksFile, width, height);
	std::vector<std::vector<short>> l1SmoothHeightMasksShort4096 = Read2DShortArray(m_l1HeightMasksFile, width, height);
	std::vector<std::vector<short>> bedrockHeightMasksShort4096 = Read2DShortArray(m_bedrockHeightMasksFile, width, height);

	string lakesHeightMasksFile = m_isLevel1Instances ? m_level1LakesHeightMasksFile : m_lakesHeightMasksFile;

	std::vector<std::vector<short>> lakes0HeightMasksShort4096 = Read2DShortArray(m_lakesHeightMasksFile, width, height);
	std::vector<std::vector<short>> lakes1HeightMasksShort4096 = Read2DShortArray(m_level1LakesHeightMasksFile, width, height);
	std::vector<std::vector<short>> lakesHeightMasksShort4096 = Read2DShortArray(lakesHeightMasksFile, width, height);

	std::vector<std::vector<short>> heightMasksShort4096(width, std::vector<short>(height));
	/*for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			short mesh0Value = mesh0HeightMasksShort4096[x][y];
			short mesh1Value = mesh1HeightMasksShort4096[x][y];
			short meshValue = meshHeightMasksShort4096[x][y];
			short pcValue = pcHeightMasksShort4096[x][y];
			short l1SmoothValue = l1SmoothHeightMasksShort4096[x][y];
			short bedrockValue = bedrockHeightMasksShort4096[x][y];
			
			short value = 0;

			if (m_isLevel1Instances)
			{
				value = FindMaxIn3(mesh1Value, l1SmoothValue, bedrockValue);
			}
			else
			{
				value = FindMaxIn5(mesh0Value, mesh1Value,l1SmoothValue, pcValue, bedrockValue);
			}

			heightMasksShort4096[x][y] = value;
			if (needMaskPositive)
			{
				if (value < 0)
				{
					heightMasksShort4096[x][y] = 0;
				}
			}
		}
	}*/

#if USE_DISPLAY_HEIGHTMAP_MASK_RESULT
	unsigned int noHeightCountMesh0 = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (mesh0HeightMasksShort4096[x][y] == 0)
			{
				noHeightCountMesh0++;
			}
		}
	}
	std::cout << "Mesh level 0 height map  mask no data count is : " << noHeightCountMesh0 << std::endl;

	unsigned int noHeightCountMesh1 = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (mesh1HeightMasksShort4096[x][y] == 0)
			{
				noHeightCountMesh1++;
			}
		}
	}
	std::cout << "Mesh level 1 height map  mask no data count is : " << noHeightCountMesh1 << std::endl;

	unsigned int noHeightCountMesh = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (meshHeightMasksShort4096[x][y] == 0)
			{
				noHeightCountMesh++;
			}
		}
	}
	if (m_isLevel1Instances)
	{
		std::cout << "Mesh level 1 height map mask no data count is : " << noHeightCountMesh << std::endl;
	}
	else
	{
		std::cout << "Mesh level 0 height map mask no data count is : " << noHeightCountMesh << std::endl;
	}
	

	unsigned int noHeightCountTopLevel = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (pcHeightMasksShort4096[x][y] == 0)
			{
				noHeightCountTopLevel++;
			}
		}
	}
	std::cout << "Point cloud top level height map mask no data count is : " << noHeightCountTopLevel << std::endl;

	unsigned int noHeightCountLevel1 = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (l1SmoothHeightMasksShort4096[x][y] == 0)
			{
				noHeightCountLevel1++;
			}
		}
	}
	std::cout << "Point cloud level 1 height map mask no data count is : " << noHeightCountLevel1 << std::endl;

	unsigned int noHeightCountBedrock = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (bedrockHeightMasksShort4096[x][y] == 0)
			{
				noHeightCountBedrock++;
			}
		}
	}
	std::cout << "Point cloud bedrock height map mask no data count is : " << noHeightCountBedrock << std::endl;

	/*unsigned int noHeightCountFinal = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (heightMasksShort4096[x][y] == 0)
			{
				noHeightCountFinal++;
			}
		}
	}
	std::cout << "Final height map mask no data count is : " << noHeightCountFinal << std::endl;*/
#endif
	
	const string meshHeightMapFile = m_isLevel1Instances ? m_mesh2HeightMapFile : m_meshHeightMapFile;

	std::vector<std::vector<short>> mesh0HeightMapShort4096 = Read2DShortArray(m_meshHeightMapFile, width, height);
	std::vector<std::vector<short>> mesh1HeightMapShort4096 = Read2DShortArray(m_mesh2HeightMapFile, width, height);
	std::vector<std::vector<short>> meshHeightMapShort4096 = Read2DShortArray(meshHeightMapFile, width, height);
	std::vector<std::vector<short>> pcHeightMapShort4096 = Read2DShortArray(m_pcHeightMapFile, width, height);
	std::vector<std::vector<short>> l1SmoothHeightMapShort4096 = Read2DShortArray(m_l1HeightMapFile, width, height);
	std::vector<std::vector<short>> bedrockHeightMapShort4096 = Read2DShortArray(m_bedrockHeightMapFile, width, height);
	
	std::vector<std::vector<double>> exposure_init_map(width, std::vector<double>(height));
	std::vector<std::vector<bool>> exposure_mask_map(width, std::vector<bool>(height));

	int countSmoothNoBedrock = 0;
	int countNoSmoothBedrock = 0;
	int countNoSmoothNoBedrock = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			short pcValue = pcHeightMapShort4096[x][y];
			short mesh1Value = mesh1HeightMapShort4096[x][y];
			short l1SmoothValue = l1SmoothHeightMapShort4096[x][y];
			short bedrockValue = bedrockHeightMapShort4096[x][y];
			short level1Value = std::max(mesh1Value, l1SmoothValue);

			bool hasPCValue = pcHeightMasksShort4096[x][y] ? true : false;
			bool hasl1SmoothValue = l1SmoothHeightMasksShort4096[x][y] ? true : false;
			bool hasBedRockValue = bedrockHeightMasksShort4096[x][y] ? true : false;
			bool hasMesh1Value = mesh1HeightMapShort4096[x][y] ? true : false;
			bool hasLevel1Value = hasMesh1Value || hasl1SmoothValue;
			bool hasSmoothValue = hasPCValue || hasl1SmoothValue;

			short smoothValue = pcValue;
			if (hasSmoothValue && hasBedRockValue) {
				smoothValue = (bedrockValue > pcValue) ? l1SmoothValue : pcValue;
			}
			
			if (hasSmoothValue && hasBedRockValue) {
				if (smoothValue < bedrockValue) {
					exposure_init_map[x][y] = 1.0f; //expose to the sun
				}
				else {
					exposure_init_map[x][y] = 0.0f; //not expose to the sun
				}
				exposure_mask_map[x][y] = true; //expose to the sun
			}
			else if (hasSmoothValue && !hasBedRockValue) {
				// If there is no bedrock, we assume it is not exposed to the sun
				exposure_init_map[x][y] = 0.0f; //expose to the sun
				exposure_mask_map[x][y] = true; //expose to the sun
				countSmoothNoBedrock++;
				std::cout << "-------- Warning: Smooth value exists but no bedrock value at cell (" << x << ", " << y << ") for exposure map. Assuming exposure to the sun." << std::endl;
			}
			else if (hasBedRockValue && !hasSmoothValue) {
				// If there is bedrock but no level 1, we assume it is not exposed to the sun
				exposure_init_map[x][y] = 1.0f; //not expose to the sun
				exposure_mask_map[x][y] = false; //not expose to the sun
				countNoSmoothBedrock++;
				std::cout << "-------- Warning: Bed rock value exists but no smooth value at cell (" << x << ", " << y << ") for exposure map. Assuming exposure to the sun." << std::endl;
			}
			else {
				exposure_init_map[x][y] = 0.0f;
				exposure_mask_map[x][y] = false; //not expose to the sun
				countNoSmoothNoBedrock++;
			}
		}
	}
	std::cout << "Count of Smooth and No bedrock value : " << countSmoothNoBedrock << std::endl;
	std::cout << "Count of No Smooth and bedrock value : " << countNoSmoothBedrock << std::endl;
	std::cout << "Count of No Smooth and No bedrock value : " << countNoSmoothNoBedrock << std::endl;

#if 0 //First version of exposure map
	int countLevel1AndNoBedrock = 0;
	int countBedrockAndNoLevel1 = 0;
	int countNoLevel1AndNoBedrock = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			short pcValue = pcHeightMapShort4096[x][y];
			short mesh1Value = mesh1HeightMapShort4096[x][y];
			short l1SmoothValue = l1SmoothHeightMapShort4096[x][y];
			short bedrockValue = bedrockHeightMapShort4096[x][y];
			short level1Value = std::max(mesh1Value, l1SmoothValue);

			bool hasPCValue = pcHeightMasksShort4096[x][y] ? true : false;
			bool hasl1SmoothValue = l1SmoothHeightMasksShort4096[x][y] ? true : false;
			bool hasBedRockValue = bedrockHeightMasksShort4096[x][y] ? true : false;
			bool hasMesh1Value = mesh1HeightMapShort4096[x][y] ? true : false;
			bool hasLevel1Value = hasMesh1Value || hasl1SmoothValue;

			if (hasLevel1Value && hasBedRockValue) {
#if USE_INCLUDE_PCVALUE_FOR_EXPOSURE_MAP
				if (hasPCValue)
				{
					if (bedrockValue > pcValue)
					{
						exposure_init_map[x][y] = 1.0f; //expose to the sun
					}
					else
					{
						if (bedrockValue < level1Value) {
							exposure_init_map[x][y] = 1.0f; //expose to the sun
						}
						else {
							exposure_init_map[x][y] = 0.0f; //not expose to the sun
						}
					}
				}
				else
				{
					if (bedrockValue < level1Value) {
						exposure_init_map[x][y] = 1.0f; //expose to the sun
					}
					else {
						exposure_init_map[x][y] = 0.0f; //not expose to the sun
					}
				}
#else
				if (bedrockValue < level1Value) {
					exposure_init_map[x][y] = 1.0f; //expose to the sun
				}
				else {
					exposure_init_map[x][y] = 0.0f; //not expose to the sun
				}
#endif
				exposure_mask_map[x][y] = true; //expose to the sun
			}
			else if (hasLevel1Value && !hasBedRockValue) {
				// If there is no bedrock, we assume it is exposed to the sun
				exposure_init_map[x][y] = 1.0f; //expose to the sun
				exposure_mask_map[x][y] = true; //expose to the sun
				countLevel1AndNoBedrock++;
				std::cout << "-------- Warning: Level 1 value exists but no bedrock at cell (" << x << ", " << y << ") for exposure map. Assuming exposure to the sun." << std::endl;
			}
			else if (hasBedRockValue && !hasLevel1Value) {
				// If there is bedrock but no level 1, we assume it is not exposed to the sun
				exposure_init_map[x][y] = 0.0f; //not expose to the sun
				exposure_mask_map[x][y] = false; //not expose to the sun
				countBedrockAndNoLevel1++;
				std::cout << "-------- Warning: Level 1 value exists but no level 1 at cell (" << x << ", " << y << ") for exposure map. Assuming exposure to the sun." << std::endl;
			}
			else {
				exposure_init_map[x][y] = 0.0f;
				exposure_mask_map[x][y] = false; //not expose to the sun
				countNoLevel1AndNoBedrock++;
			}
		}
	}
	std::cout << "Count of Level 1 and no bedrock: " << countLevel1AndNoBedrock << std::endl;
	std::cout << "Count of Bedrock and no Level 1: " << countBedrockAndNoLevel1 << std::endl;
	std::cout << "Count of No Level 1 and no bedrock: " << countNoLevel1AndNoBedrock << std::endl;
#endif

	const double PROPAGATION_FACTOR = 0.5; // This factor can be adjusted based on the desired propagation effect
	const double MIN_THRESHOLD = 0.001; // Minimum value to continue propagation
	int max_iterations = 5000; // Maximum iterations to prevent infinite loops
	//std::vector<std::vector<double>> exposure_map = PropagateLightingMax(exposure_init_map, exposure_mask_map, max_iterations, PROPAGATION_FACTOR, MIN_THRESHOLD);
	//std::vector<std::vector<double>> exposure_map = PropagateLightingMax4Dir(exposure_init_map, exposure_mask_map, max_iterations, PROPAGATION_FACTOR, MIN_THRESHOLD);
	std::vector<std::vector<double>> exposure_map = PropagateLightingAverage(exposure_init_map, exposure_mask_map, max_iterations, PROPAGATION_FACTOR, MIN_THRESHOLD);
	std::vector<std::vector<byte>> exposure_byte_map(width, std::vector<byte>(height));
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			double exposureValue = exposure_map[x][y];
			if (exposureValue > 1.0) {
				exposureValue = 1.0;
			}
			else if (exposureValue < 0.0) {
				exposureValue = 0.0;
			}
			exposure_byte_map[x][y] = static_cast<byte>(exposureValue * 255);
		}
	}

	std::vector<std::vector<short>> heightMapShort4096(width, std::vector<short>(height));

	std::vector<std::vector<short>> heightMapRoadDataShort4096(width, std::vector<short>(height)); 
	bool needHeightPositive = false;

	short minHeight = std::numeric_limits<short>::max();
	short maxHeight = std::numeric_limits<short>::min();
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			short mesh0Value = mesh0HeightMapShort4096[x][y];
			short mesh1Value = mesh1HeightMapShort4096[x][y];
			short meshValue = meshHeightMapShort4096[x][y];
			short pcValue = pcHeightMapShort4096[x][y];
			short l1SmoothValue = l1SmoothHeightMapShort4096[x][y];
			short bedrockValue = bedrockHeightMapShort4096[x][y];

			short toplayerValue = std::max(mesh0Value, pcValue);
			short level1Value = std::max(mesh1Value, l1SmoothValue);

			bool hasMesh0Value = mesh0HeightMapShort4096[x][y] ? true : false;
			bool hasMesh1Value = mesh1HeightMapShort4096[x][y] ? true : false;
			bool hasMeshValue = meshHeightMasksShort4096[x][y] ? true : false;
			bool hasPcValue = pcHeightMasksShort4096[x][y] ? true : false;
			bool hasl1SmoothValue = l1SmoothHeightMasksShort4096[x][y] ? true : false;
			bool hasBedRockValue = bedrockHeightMasksShort4096[x][y] ? true : false;

			bool hasLakes0Value = lakes0HeightMasksShort4096[x][y] ? true : false;
			bool hasLakes1Value = lakes1HeightMasksShort4096[x][y] ? true : false;
			bool hasLakesValue = lakesHeightMasksShort4096[x][y] ? true : false;

			bool hasLevel1Value = hasMesh1Value || hasl1SmoothValue;
			bool hasToplayerValue = hasMeshValue || hasPcValue;

			bool hasOnlyLevel1Instances = (hasLevel1Value && hasBedRockValue) && (bedrockValue > level1Value);
			bool hasCoverValue = false;
			if (hasToplayerValue && hasLevel1Value) {
				if (toplayerValue > level1Value) {
					hasCoverValue = true;
				}
			}

			bool hasSmoothValue = (hasPcValue || hasl1SmoothValue || hasBedRockValue);
			bool hasBaseMeshValue = hasMeshValue;


			bool hasHeight = false;

			short value = UNAVAILBLE_NEG_HEIGHT;
			short smoothValue = UNAVAILBLE_NEG_HEIGHT;
			short baseMeshValue = UNAVAILBLE_NEG_HEIGHT;

			short heightMaskValue = HEIGHTMAP_MASK_NO_DATA;

			if (m_isLevel1Instances) {
				bool level1Validate = false;
				if (!hasLakesValue &&
					hasCoverValue && 
					hasOnlyLevel1Instances
					) {
					
					if ((bedrockValue - level1Value) > 50) { //also let's not plant any trees if the height gap between level1 ground and bedrock is smaller than 50m
						level1Validate = true;
					}
				}
				if (level1Validate) {
					value = level1Value;
					hasHeight = true;
					heightMaskValue = HEIGHTMAP_MASK_HAS_DATA;
				}
			}
			else {
				if (!hasLakesValue
					//&& !hasOnlyLevel1Instances
					) {

					if (hasPcValue && hasl1SmoothValue && hasBedRockValue) {
						smoothValue = (bedrockValue > pcValue) ? l1SmoothValue : pcValue;
					}
					else if (hasPcValue && hasl1SmoothValue) {
						smoothValue = pcValue;
					}
					else if (hasPcValue && hasBedRockValue) {
						smoothValue = pcValue;
					}
					else if (hasPcValue) {
						smoothValue = pcValue;
					}
					else if (hasl1SmoothValue) {
						smoothValue = l1SmoothValue;
					}
					else if (hasBedRockValue) {
						smoothValue = bedrockValue;
					}


					if (hasBaseMeshValue) {
						baseMeshValue = meshValue;
					}

					if (hasBaseMeshValue && hasSmoothValue) {
						value = std::max(baseMeshValue, smoothValue);
					}
					else if (hasBaseMeshValue) {
						value = baseMeshValue;
					}
					else if (hasSmoothValue) {
						value = smoothValue;
					}
				}

				if (value != UNAVAILBLE_NEG_HEIGHT) {
					hasHeight = true;
					heightMaskValue = HEIGHTMAP_MASK_HAS_DATA;
				}
			}

			/*if (hasSmoothValue) {
				if (hasPcValue && hasl1Value && hasBedRockValue) {
					if (bedrockValue > pcValue){
						smoothValue = l1Value;
					}
					else {
						smoothValue = pcValue;
					}
				}
				else {
					if (hasPcValue && hasl1SValue) {
						smoothValue = pcValue;
					}
					else if (hasPcValue && bedrockValue) {
						smoothValue = pcValue;
					}
					else if (hasBedRockValue && hasl1Value) {
						smoothValue = l1Value;
					}
					else {
						if (hasPcValue) {
							smoothValue = pcValue;
						}
						else if (hasl1Value) {
							smoothValue = l1Value;
						}
						else if (hasBedRockValue) {
							smoothValue = bedrockValue;
						}
					}
				}
			}

			if (hasBaseMeshValue) {
				baseMeshValue = meshValue;
			}
			
			if (hasBaseMeshValue && hasSmoothValue) {
				value = std::max(baseMeshValue, smoothValue);
			}
			else if (hasBaseMeshValue) {
				value = baseMeshValue;
			}
			else if (hasSmoothValue) {
				value = smoothValue;
			}*/
			
			/*if (hasSmoothValue) {
				if (hasPcValue && hasl1SmoothValue && hasBedRockValue) {
					smoothValue = (bedrockValue > pcValue) ? l1SmoothValue : pcValue;
				}
				else if (hasPcValue && hasl1SmoothValue) {
					smoothValue = pcValue;
				}
				else if (hasPcValue && hasBedRockValue) {
					smoothValue = pcValue;
				}
				else if (hasBedRockValue && hasl1SmoothValue) {
					smoothValue = l1SmoothValue;
					if (m_isLevel1Instances) {
						bool level1Validate = false;
						if ((bedrockValue > l1SmoothValue)
							&& ((bedrockValue - l1SmoothValue) > 50)) { //also let's not plant any trees if the height gap between level1 ground and bedrock is smaller than 50m
							level1Validate = true;
						}
						if (level1Validate) {
							smoothValue = l1SmoothValue;
						}
						else {
							smoothValue = UNAVAILBLE_NEG_HEIGHT;
							hasSmoothValue = false;
						}
					}
					
				}
				else if (hasPcValue) {
					smoothValue = pcValue;
				}
				else if (hasl1SmoothValue) {
					smoothValue = l1SmoothValue;
				}
				else if (hasBedRockValue) {
					smoothValue = bedrockValue;
				}
			}

			if (hasBaseMeshValue) {
				baseMeshValue = meshValue;
			}

			if (hasBaseMeshValue && hasSmoothValue) {
				value = std::max(baseMeshValue, smoothValue);
			}
			else if (hasBaseMeshValue) {
				value = baseMeshValue;
			}
			else if (hasSmoothValue) {
				value = smoothValue;
			}*/

			heightMasksShort4096[x][y] = heightMaskValue;
			if (needHeightPositive && (value < 0) && (value != UNAVAILBLE_NEG_HEIGHT))
			{
				value = 0;
			}
			heightMapShort4096[x][y] = value;

			heightMapRoadDataShort4096[x][y] = value > 0 ? value : 0;

			if (heightMasksShort4096[x][y] > 0)
			{
				minHeight = std::min(minHeight, value);
				maxHeight = std::max(maxHeight, value);
			}	
		}
	}
	m_maxHeight = maxHeight;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (heightMapRoadDataShort4096[x][y] < 0)
			{
				heightMapRoadDataShort4096[x][y] = 0;
			}
		}
	}
	std::cout << "Final Short Height Map minHeight = " << minHeight << " , maxHeight = " << maxHeight << std::endl;

	//std::vector<std::vector<short>> slopeShort4096 = ComputeAbsMaxHeightSlopeMap(heightMapShort4096);
	std::vector<std::vector<short>> slopeShort4096 = ComputeSlopeMap(heightMapShort4096);
	std::vector<std::vector<double>> heightMapDouble4096 = ConvertShortMatrixToDouble1(heightMapShort4096);
	std::vector<std::vector<unsigned short>> heightMapUShort4096 = ConvertShortMatrixToUShort(heightMapShort4096);

	std::vector<std::vector<short>> heightMapShort1000 = ScaleArray(heightMapShort4096, 1000, 1000);
	std::vector<std::vector<unsigned short>> heightMapUShort1000 = ConvertShortMatrixToUShort(heightMapShort1000);

	int exportHeightMapLowRawX = m_roadInputHeightMapWidth * m_tileScale;
	int exportHeightMapLowRawY = m_roadInputHeightMapHeight * m_tileScale;
	//std::vector<std::vector<short>> heightMapExportLowRawShort = ScaleArray(heightMapAdjust3Short4096, exportHeightMapLowRawX, exportHeightMapLowRawY);
	//std::vector<std::vector<short>> heightMapExportLowRawShort = resample2DArrayByFunc(heightMapRoadDataShort4096, exportHeightMapLowRawX, exportHeightMapLowRawY, findAverageInBlock<short>);
	std::vector<std::vector<short>> heightMapExportLowRawShort = resample2DShortWithAverage(heightMapRoadDataShort4096, exportHeightMapLowRawX, exportHeightMapLowRawY);
	std::vector<std::vector<unsigned short>> heightMapExportLowRawUShortInvert = ConvertShortMatrixToUShort(heightMapExportLowRawShort);
	std::vector<std::vector<unsigned short>> heightMapExportLowRawUShort = invert2DArray(heightMapExportLowRawUShortInvert);

	int exportHeightMapHighRatio = 2;
	int exportHeightMapHighRawX = exportHeightMapLowRawX * exportHeightMapHighRatio;
	int exportHeightMapHighRawY = exportHeightMapLowRawY * exportHeightMapHighRatio;
	//std::vector<std::vector<short>> heightMapExportHighRawShort = ScaleArray(heightMapAdjust3Short4096, exportHeightMapHighRawX, exportHeightMapHighRawY);
	//std::vector<std::vector<short>> heightMapExportHighRawShort = resample2DArrayByFunc(heightMapRoadDataShort4096, exportHeightMapHighRawX, exportHeightMapHighRawY, findAverageInBlock<short>);
	std::vector<std::vector<short>> heightMapExportHighRawShort = resample2DShortWithAverage(heightMapRoadDataShort4096, exportHeightMapHighRawX, exportHeightMapHighRawY);
	std::vector<std::vector<unsigned short>> heightMapExportHighRawUShortInvert = ConvertShortMatrixToUShort(heightMapExportHighRawShort);
	std::vector<std::vector<unsigned short>> heightMapExportHighRawUShort = invert2DArray(heightMapExportHighRawUShortInvert);

	double ratio = 7.32673;
#if USE_MAX_SLOPE_ANGLE
	std::vector<std::vector<double>> slopeDouble4096 = ComputeAbsMaxSlopeAngle(heightMapDouble4096, ratio);
#else
	std::vector<std::vector<double>> slopeDouble4096 = ComputeAbsAverageNeighborSlopeAngle(heightMapDouble4096, ratio);
#endif

	for (auto i = 0; i < width; i++) {
		for (auto j = 0; j < height; j++) {
			CCellInfo* cell = (*m_pCellTable)[i][j];
			if (!cell)
			{
				std::cout << "Fail to get CCellData at i = " << i << " j = " << j << std::endl;
			}
			else
			{
				cell->SetHeightValue(heightMapDouble4096[i][j]);
				//cell->SetHeightValue(pcHeightMapShort4096[i][j]);

				cell->SetSlopeHeightValue(slopeShort4096[i][j]);
				cell->SetSlopeAngleValue(slopeDouble4096[i][j]);
				short hasHeightValue = heightMasksShort4096[i][j];
				if (m_isLevel1Instances)
				{
					//hasHeightValue = (lakesHeightMasksShort4096[i][j] > 0) ? 0 : heightMasksShort4096[i][j];
				}
				cell->SetHasHeightValue(hasHeightValue);

				cell->SetSunlightAffinity(exposure_map[i][j]);
				bool hasSunlightAffinity = exposure_mask_map[i][j];
				cell->SetHasSunlightAffinity(hasSunlightAffinity);
			}
		}
	}

	const int MAX_PATH = 250;
	
	char mesh_heightmap_raw_export[MAX_PATH];
	char mesh2_heightmap_raw_export[MAX_PATH];
	char pc_heightmap_raw_export[MAX_PATH];
	char l1_heightmap_raw_export[MAX_PATH];
	char bedrock_heightmap_raw_export[MAX_PATH];
	char short_height_map_export[MAX_PATH];
	char double_height_map_exportout[MAX_PATH];
	char height_slope_map_exportout[MAX_PATH];
	char angle_slope_map_exportout[MAX_PATH];
	char ushort_height_map_low_raw[MAX_PATH];
	char ushort_height_map_high_raw[MAX_PATH];

	memset(mesh_heightmap_raw_export, 0, sizeof(char) * MAX_PATH);
	memset(mesh2_heightmap_raw_export, 0, sizeof(char) * MAX_PATH);
	memset(pc_heightmap_raw_export, 0, sizeof(char) * MAX_PATH);
	memset(l1_heightmap_raw_export, 0, sizeof(char) * MAX_PATH);
	memset(bedrock_heightmap_raw_export, 0, sizeof(char)* MAX_PATH);
	memset(short_height_map_export, 0, sizeof(char) * MAX_PATH);
	memset(double_height_map_exportout, 0, sizeof(char) * MAX_PATH);
	memset(height_slope_map_exportout, 0, sizeof(char) * MAX_PATH);
	memset(angle_slope_map_exportout, 0, sizeof(char) * MAX_PATH);
	memset(ushort_height_map_low_raw, 0, sizeof(char) * MAX_PATH);
	memset(ushort_height_map_high_raw, 0, sizeof(char) * MAX_PATH);

#if __APPLE__
#if USE_OUTPUT_HEIGHT_MAP_CSV
	snprintf(mesh_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_mesh_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(mesh2_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_mesh2_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(pc_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_pc_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(l1_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_l1_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(bedrock_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_bedrock_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(short_height_map_export, MAX_PATH, "%s/%d_%d_%d_short_height_map_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#else
	snprintf(mesh_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_mesh_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(mesh2_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_mesh2_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(pc_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_pc_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(l1_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_11_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(bedrock_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_bedrock_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(short_height_map_export, MAX_PATH, "%s/%d_%d_%d_short_height_map_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#endif
	snprintf(double_height_map_exportout, MAX_PATH, "%s/%d_%d_%d_double_height_map_exportout.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(height_slope_map_exportout, MAX_PATH, "%s/%d_%d_%d_height_slope_map_exportout.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(angle_slope_map_exportout, MAX_PATH, "%s/%d_%d_%d_angle_slope_map_exportout.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	
#else
#if USE_OUTPUT_HEIGHT_MAP_CSV
	sprintf_s(mesh_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_mesh_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(mesh2_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_mesh2_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(pc_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_pc_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(l1_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_1l_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(bedrock_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_bedrock_heightmap_raw_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(short_height_map_export, MAX_PATH, "%s\\%d_%d_%d_short_height_map_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#else
	sprintf_s(mesh_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_mesh_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(mesh2_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_mesh2_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(pc_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_pc_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(l1_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_l1_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(bedrock_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_bedrock_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(short_height_map_export, MAX_PATH, "%s\\%d_%d_%d_short_height_map_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#endif
	sprintf_s(double_height_map_exportout, MAX_PATH, "%s\\%d_%d_%d_double_height_map_exportout.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(height_slope_map_exportout, MAX_PATH, "%s\\%d_%d_%d_height_slope_map_exportout.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(angle_slope_map_exportout, MAX_PATH, "%s\\%d_%d_%d_angle_slope_map_exportout.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#endif

#if __APPLE__
	snprintf(ushort_height_map_low_raw, MAX_PATH, "%s/%d_%d_%d_%d_%d_%d_ushort_height_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#if USE_OUTPUT_HIGH_ROAD_DATA
	snprintf(ushort_height_map_high_raw, MAX_PATH, "%s/%d_%d_%d_%d_%d_%d_ushort_height_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth * exportHeightMapHighRatio, m_roadInputHeightMapHeight * exportHeightMapHighRatio);
#endif
#else
	sprintf_s(ushort_height_map_low_raw, MAX_PATH, "%s\\%d_%d_%d_%d_%d_%d_ushort_height_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#if USE_OUTPUT_HIGH_ROAD_DATA
	sprintf_s(ushort_height_map_high_raw, MAX_PATH, "%s\\%d_%d_%d_%d_%d_%d_ushort_height_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth * exportHeightMapHighRatio, m_roadInputHeightMapHeight * exportHeightMapHighRatio);
#endif
#endif

#if USE_EXPORT_HEIGHT_MAP
	string title = "EXPORT HEIGHT MAP INFO";
	CTimeCounter timeCounter(title);
#if USE_OUTPUT_HEIGHT_MAP_CSV
	ExportShortHeightMap(meshHeightMapShort4096, mesh_heightmap_raw_export, 0x00FF0000, true);
	ExportShortHeightMap(mesh2HeightMapShort4096, mesh2_heightmap_raw_export, 0x00FF0000, true);
	ExportShortHeightMap(pcHeightMapShort4096, pc_heightmap_raw_export, 0x0000FF00, true);
	ExportShortHeightMap(l1HeightMapShort4096, l1_heightmap_raw_export, 0x0000FF00, true);
	ExportShortHeightMap(bedrockHeightMapShort4096, bedrock_heightmap_raw_export, 0x0000FF00, true);
	ExportShortHeightMap(heightMapShort4096, short_height_map_export, 0x000000FF, true);
#else
	ExportShortHeightMapWithMask(meshHeightMapShort4096, meshHeightMasksShort4096, mesh_heightmap_raw_export, 0x00FF0000, false, true);
	ExportShortHeightMapWithMask(mesh2HeightMapShort4096, mesh2HeightMasksShort4096, mesh2_heightmap_raw_export,  0x00FF0000, false, true);
	ExportShortHeightMapWithMask(pcHeightMapShort4096, pcHeightMasksShort4096, pc_heightmap_raw_export, 0x0000FF00, false, true);
	ExportShortHeightMapWithMask(l1HeightMapShort4096, l1HeightMasksShort4096, l1_heightmap_raw_export, 0x0000FF00, false, true);
	ExportShortHeightMapWithMask(bedrockHeightMapShort4096, bedrockHeightMasksShort4096, bedrock_heightmap_raw_export, 0x0000FF00, false, true);
	ExportShortHeightMapWithMask(heightMapShort4096, heightMasksShort4096, short_height_map_export, 0x000000FF, false, true);
#endif
	ExportDoubleHeightMap(heightMapDouble4096, double_height_map_exportout, 0x00FFFF00, false);
	ExportShortHeightSlopeMap(slopeShort4096, height_slope_map_exportout, 0x0000FF00, false);
	ExportAngleSlopeMap(slopeDouble4096, angle_slope_map_exportout, 0x00FF0000, false);
#endif
	bool outputHeightMapLow = Output2DVectorToRawFile(heightMapExportLowRawUShort, ushort_height_map_low_raw);

#if USE_OUTPUT_HIGH_ROAD_DATA
	bool outputHeightMapHigh = Output2DVectorToRawFile(heightMapExportHighRawUShort, ushort_height_map_high_raw);
#endif

#if USE_EXPORT_EXPOSURE_MAP
	char exposure_init_map_raw_export[MAX_PATH];
	memset(exposure_init_map_raw_export, 0, sizeof(char)* MAX_PATH);
	char exposure_map_raw_export[MAX_PATH];
	memset(exposure_map_raw_export, 0, sizeof(char)* MAX_PATH);
	char exposure_byte_map_raw_export[MAX_PATH];
	memset(exposure_byte_map_raw_export, 0, sizeof(char)* MAX_PATH);
#if __APPLE__
	snprintf(exposure_init_map_raw_export, MAX_PATH, "%s/%d_%d_%d_exposure_init_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(exposure_map_raw_export, MAX_PATH, "%s/%d_%d_%d_exposure_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(exposure_byte_map_raw_export, MAX_PATH, "%s/%d_%d_%d_exposure_byte_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#else
	sprintf_s(exposure_init_map_raw_export, MAX_PATH, "%s\\%d_%d_%d_exposure_init_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(exposure_map_raw_export, MAX_PATH, "%s\\%d_%d_%d_exposure_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(exposure_byte_map_raw_export, MAX_PATH, "%s\\%d_%d_%d_exposure_byte_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#endif
	bool outputExposureInitMap = Output2DVectorToRawFile(exposure_init_map, exposure_init_map_raw_export);
	bool outputExposureMap = Output2DVectorToRawFile(exposure_map, exposure_map_raw_export);
	bool outputExposureByteMap = Output2DVectorToRawFile(exposure_byte_map, exposure_byte_map_raw_export);
#endif
	return true;
}


bool CPlantsSimulation::ExportShortHeightMap(std::vector<std::vector<short>>& heightMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio)
{
	if (!m_topLayerMeta)
	{
		return false;
	}
	std::ofstream outputFile(outputPath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open export short height map file " << outputPath << std::endl;
		return false;
	}

	// Write header row
	if (hasHeader)
	{
		outputFile << "X,Y,Z,Red,Green,Blue" << std::endl;
	}

	// Write data rows
	int redColor = (rgbColor >> 16) & 0xFF;
	int greenColor = (rgbColor >> 8) & 0xFF;
	int blueColor = rgbColor & 0xFF;
	
	double xRatio = m_topLayerMeta->xRatio;
	double yRatio = m_topLayerMeta->yRatio;
	double batch_min_x = m_topLayerMeta->batch_min_x;
	double batch_min_y = m_topLayerMeta->batch_min_y;
	double x0 = m_topLayerMeta->x0;
	double y0 = m_topLayerMeta->y0;
	int mapWidth = heightMap.size();
	int mapHeight = heightMap[0].size();
	int width = heightMap.size();
	int height = heightMap[0].size();
	if (withRatio) {
		//width = static_cast<int>(mapWidth * xRatio);
		//height = static_cast<int>(mapHeight * yRatio);
	}
	
	std::cout << "ExportShortHeightMap file is " << outputPath << ", width " << width << ", height = " << height << std::endl;
 	
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (withRatio)
			{
				//int raw = clamp(static_cast<int>(width / xRatio), 0, mapWidth - 1);
				//int col = clamp(static_cast<int>(width / xRatio), 0, mapWidth - 1);
				short value = (static_cast<short>(heightMap[i][j]));
				if (value > UNAVAILBLE_NEG_HEIGHT)
				{
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
					if (value >= 0)
					{
#endif
						double posX = static_cast<double>(i * xRatio);
						double posY = static_cast<double>(j * yRatio);
						double fullPosX = batch_min_x + x0 + posX;
						double fullPoxY = batch_min_y + y0 + posY;
						outputFile
							<< fullPosX << ","
							<< fullPoxY << ","
							//<< heightMap[raw][col] << ","
							<< value << ","
							<< redColor << ","
							<< greenColor << ","
							<< blueColor << std::endl;
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
					}
#endif
				}
			}
			else
			{
				short value = static_cast<short>(heightMap[i][j] / xRatio);
				if (value > UNAVAILBLE_NEG_HEIGHT)
				{
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
					if (value >= 0)
					{
#endif
						outputFile
							<< i << ","
							<< j << ","
							<< value << ","
							<< redColor << ","
							<< greenColor << ","
							<< blueColor << std::endl;
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
					}
#endif
				}
			}
			
		}
	}

	outputFile.close();

	return true;
}

bool CPlantsSimulation::ExportShortHeightMapWithMask(std::vector<std::vector<short>>& heightMap, std::vector<std::vector<short>>& masks, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio)
{
	if (!m_topLayerMeta)
	{
		return false;
	}
	std::ofstream outputFile(outputPath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open export short height map file " << outputPath << std::endl;
		return false;
	}

	// Write header row
	if (hasHeader)
	{
		outputFile << "X,Y,Z,Red,Green,Blue" << std::endl;
	}

	// Write data rows
	int redColor = (rgbColor >> 16) & 0xFF;
	int greenColor = (rgbColor >> 8) & 0xFF;
	int blueColor = rgbColor & 0xFF;

	double xRatio = m_topLayerMeta->xRatio;
	double yRatio = m_topLayerMeta->yRatio;
	double batch_min_x = m_topLayerMeta->batch_min_x;
	double batch_min_y = m_topLayerMeta->batch_min_y;
	double x0 = m_topLayerMeta->x0;
	double y0 = m_topLayerMeta->y0;
	int mapWidth = heightMap.size();
	int mapHeight = heightMap[0].size();
	int width = heightMap.size();
	int height = heightMap[0].size();
	if (withRatio) {
		//width = static_cast<int>(mapWidth * xRatio);
		//height = static_cast<int>(mapHeight * yRatio);
	}

	std::cout << "ExportShortHeightMap file is " << outputPath << ", width " << width << ", height = " << height << std::endl;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (masks[i][j] > 0)
			{
				if (withRatio)
				{
					//int raw = clamp(static_cast<int>(width / xRatio), 0, mapWidth - 1);
					//int col = clamp(static_cast<int>(width / xRatio), 0, mapWidth - 1);
					short value = (static_cast<short>(heightMap[i][j]));
					if (value > UNAVAILBLE_NEG_HEIGHT)
					{
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
						if (value >= 0)
						{
#endif
							double posX = static_cast<double>(i * xRatio);
							double posY = static_cast<double>(j * yRatio);
							double fullPosX = batch_min_x + x0 + posX;
							double fullPoxY = batch_min_y + y0 + posY;
							outputFile
								<< fullPosX << ","
								<< fullPoxY << ","
								//<< heightMap[raw][col] << ","
								<< value << ","
								<< redColor << ","
								<< greenColor << ","
								<< blueColor << std::endl;
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
						}
#endif
					}
				}
				else
				{

					short value = static_cast<short>(heightMap[i][j] / xRatio);
					if (value > UNAVAILBLE_NEG_HEIGHT)
					{
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
						if (value >= 0)
						{
#endif
							outputFile
								<< i << ","
								<< j << ","
								<< value << ","
								<< redColor << ","
								<< greenColor << ","
								<< blueColor << std::endl;
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
						}
#endif
					}
				}
			}
		}
	}

	outputFile.close();

	return true;
}

bool CPlantsSimulation::ExportDoubleHeightMap(std::vector<std::vector<double>>& heightMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio)
{
	if (!m_topLayerMeta)
	{
		return false;
	}
	std::ofstream outputFile(outputPath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open export short height map file " << outputPath << std::endl;
		return false;
	}

	// Write header row
	if (hasHeader)
	{
		outputFile << "X,Y,Z,Red,Green,Blue" << std::endl;
	}

	// Write data rows
	int redColor = (rgbColor >> 16) & 0xFF;
	int greenColor = (rgbColor >> 8) & 0xFF;
	int blueColor = rgbColor & 0xFF;
	int width = heightMap.size();
	int height = heightMap[0].size();
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			outputFile 
				//<< static_cast<double>(i * m_topLayerMeta->xRatio) << ","
				//<< static_cast<double>(j * m_topLayerMeta->yRatio) << ","
				//<< heightMap[i][j] << ","
				<< i << ","
				<< j << ","
				<< static_cast<short>(heightMap[i][j] / m_topLayerMeta->xRatio) << ","
				<< redColor << ","
				<< greenColor << ","
				<< blueColor << std::endl;
		}
	}

	outputFile.close();

	return true;
}

bool CPlantsSimulation::ExportShortHeightSlopeMap(std::vector<std::vector<short>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio)
{
	if (!m_topLayerMeta)
	{
		return false;
	}
	std::ofstream outputFile(outputPath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open export short height slope file " << outputPath << std::endl;
		return false;
	}

	// Write header row
	if (hasHeader)
	{
		outputFile << "X,Y,Z,Red,Green,Blue" << std::endl;
	}

	// Write data rows
	int redColor = (rgbColor >> 16) & 0xFF;
	int greenColor = (rgbColor >> 8) & 0xFF;
	int blueColor = rgbColor & 0xFF;
	int width = slopeMap.size();
	int height = slopeMap[0].size();
	
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			short heightValue = static_cast<short>(slopeMap[i][j] / m_topLayerMeta->xRatio);
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
			if (heightValue >= 0)
			{
#endif
				outputFile
					//<< static_cast<double>(i * m_topLayerMeta->xRatio)<< ","
					//<< static_cast<double>(j * m_topLayerMeta->yRatio)<< ","
					//<< static_cast<short>(slopeMap[i][j]) << ","
					<< i << ","
					<< j << ","
					<< heightValue << ","
					<< redColor << ","
					<< greenColor << ","
					<< blueColor << std::endl;
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
			}
#endif
		}
	}

	outputFile.close();

	return true;
}

bool CPlantsSimulation::ExportAngleSlopeMap(std::vector<std::vector<double>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio)
{
	if (!m_topLayerMeta)
	{
		return false;
	}
	std::ofstream outputFile(outputPath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open export double angle slope map file " << outputPath << std::endl;
		return false;
	}

	// Write header row
	if (hasHeader)
	{
		outputFile << "X,Y,Z,Red,Green,Blue" << std::endl;
	}

	// Write data rows
	int redColor = (rgbColor >> 16) & 0xFF;
	int greenColor = (rgbColor >> 8) & 0xFF;
	int blueColor = rgbColor & 0xFF;
	int width = slopeMap.size();
	int height = slopeMap[0].size();
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			outputFile 
				<< static_cast<int>(i) << ","
				<< static_cast<int>(j) << ","
				<< static_cast<double>(180 * slopeMap[i][j] / PI) << ","
				//<< redColor << ","
				<< static_cast<int>( 255 * 2 *slopeMap[i][j] / PI)
				<< greenColor << ","
				<< blueColor << std::endl;
		}
	}

	outputFile.close();

	return true;
}

bool CPlantsSimulation::MakeRoadData()
{
	setIsLevel1Instances(false);
	bool ret = LoadPreImage();
	if (!ret) {
		DeInitialize();
		return ret;
	}

	m_p2dCaveLevel0Nodes = LoadCaveNodesFromPointCloud(m_cavesPointCloudLevel0File);

	bool outputLake = OutputLakeRawData();

	ret = LoadInputHeightMap();
	if (!ret) {
		DeInitialize();
		return ret;
	}
}

bool CPlantsSimulation::OutputLakeRawData()
{
	if (!m_topLayerImage)
	{
		return false;
	}
	const int width = m_topLayerImage->input_image_width;
	const int height = m_topLayerImage->input_image_height;

	const int MAX_PATH = 250;
	char lake_map_raw_path[MAX_PATH];
	memset(lake_map_raw_path, 0, sizeof(char) * MAX_PATH);
	char top_lake_map_raw_path[MAX_PATH];
	memset(top_lake_map_raw_path, 0, sizeof(char) * MAX_PATH);
	char level1_lake_map_raw_path[MAX_PATH];
	memset(level1_lake_map_raw_path, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(lake_map_raw_path, MAX_PATH, "%s/%d_%d_%d_%d_%d_%d_byte_lake_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
	snprintf(top_lake_map_raw_path, MAX_PATH, "%s/%d_%d_%d_%d_%d_%d_byte_top_lake_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
	snprintf(level1_lake_map_raw_path, MAX_PATH, "%s/%d_%d_%d_%d_%d_%d_byte_level1_lake_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#else
	sprintf_s(lake_map_raw_path, MAX_PATH, "%s\\%d_%d_%d_%d_%d_%d_byte_lake_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
	sprintf_s(top_lake_map_raw_path, MAX_PATH, "%s\\%d_%d_%d_%d_%d_%d_byte_top_lake_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
	sprintf_s(level1_lake_map_raw_path, MAX_PATH, "%s\\%d_%d_%d_%d_%d_%d_byte_level1_lake_map.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#endif
	std::cout << "lake map raw file is " << lake_map_raw_path << std::endl;
	std::cout << "top lake map raw file is " << top_lake_map_raw_path << std::endl;
	std::cout << "level1 lake map raw file is " << level1_lake_map_raw_path << std::endl;

	std::vector<std::vector<short>> lakes0HeightMasksShort4096 = Read2DShortArray(m_lakesHeightMasksFile, width, height);
	std::vector<std::vector<short>> lakes1HeightMasksShort4096 = Read2DShortArray(m_level1LakesHeightMasksFile, width, height);
	
	const int outputWidth = m_roadInputHeightMapWidth * m_tileScale;
	const int outputHeight = m_roadInputHeightMapHeight * m_tileScale;
	std::vector<std::vector<unsigned char>> lakes0HeightMapByteInvert = resample2DShortMaskToByte(lakes0HeightMasksShort4096, outputWidth, outputHeight);
	std::vector<std::vector<unsigned char>> lakes1HeightMapByteInvert = resample2DShortMaskToByte(lakes1HeightMasksShort4096, outputWidth, outputHeight);
	std::vector<std::vector<unsigned char>> lakesHeightMapByteInvert(outputWidth, std::vector<unsigned char>(outputHeight, 0));

	for (int i = 0; i < outputHeight; i++)
	{
		for (int j = 0; j < outputWidth; j++)
		{
			lakesHeightMapByteInvert[i][j] = (lakes0HeightMapByteInvert[i][j] != 0) || (lakes1HeightMapByteInvert[i][j] != 0) ? 1 : 0;
		}
	}

	std::vector<std::vector<unsigned char>> lakes0HeightMapByte = invert2DArray(lakes0HeightMapByteInvert);
	std::vector<std::vector<unsigned char>> lakes1HeightMapByte = invert2DArray(lakes1HeightMapByteInvert);
	std::vector<std::vector<unsigned char>> lakesHeightMapByte = invert2DArray(lakesHeightMapByteInvert);

	bool outputLakes = Output2DVectorToRawFile(lakesHeightMapByte, lake_map_raw_path);
	bool outputTopLakes = Output2DVectorToRawFile(lakes0HeightMapByte, top_lake_map_raw_path);
	bool outputLevel1Lakes = Output2DVectorToRawFile(lakes1HeightMapByte, level1_lake_map_raw_path);

	return  outputLakes && outputTopLakes && outputLevel1Lakes;
}

bool CPlantsSimulation::MakeInstance(bool islevel1Instances)
{
	DeInitializeForMakeInstances();
	setIsLevel1Instances(islevel1Instances);
	bool isLoadImage = LoadPreImage();
	if (!isLoadImage)
	{
		std::cout << "Failed to load input image for instance level " << (m_isLevel1Instances ? 1 : 0) << std::endl;
		DeInitialize();
		return false;
	}
	bool isLoad = LoadInputData();
	if (!isLoad)
	{
		std::cout << "Failed to load input data for instance level " << (m_isLevel1Instances ? 1 : 0) << std::endl;
		DeInitialize();
		return false;
	}
	bool isLoadRegions = LoadAndOutputRegions();
	if (!isLoadRegions)
	{
		std::cout << "Failed to load and output regions for instance level " << (m_isLevel1Instances ? 1 : 0) << std::endl;
		DeInitialize();
		return false;
	}
	bool loadForest = LoadForest();
	if (!loadForest)
	{
		std::cout << "Failed to load forest for instance level " << (m_isLevel1Instances ? 1 : 0) << std::endl;
		DeInitialize();
		return false;
	}
	bool buildForest = BuildForest();
	if (!buildForest)
	{
		std::cout << "Failed to build forest for instance level " << (m_isLevel1Instances ? 1 : 0) << std::endl;
		DeInitialize();
		return false;
	}
	bool results = OutputResults();
	if (!results)
	{
		std::cout << "Failed to output results for instance level " << (m_isLevel1Instances ? 1 : 0) << std::endl;
		DeInitialize();
		return false;
	}
	return true;
}

bool CPlantsSimulation::LoadPreImage()
{
	bool ret = false;
	DeInitialize();

	ret = LoadImageMetaFile();
	if (!ret)
	{
		std::cout << "Failed to load image meta file!" << std::endl;
		DeInitialize();
		return ret;
	}

	ret = LoadInputImage();
	if (!ret) {
		std::cout << "Failed to load input image!" << std::endl;
		DeInitialize();
		return ret;
	}
}

bool CPlantsSimulation::LoadInputData()
{
	bool ret = false;

	m_p2dCaveLevel0Nodes = LoadCaveNodesFromPointCloud(m_cavesPointCloudLevel0File);

	bool outputLake = OutputLakeRawData();

	bool loadAllPois = loadAllPoisLocationsFromCSV();
	if (!loadAllPois)
	{
		std::cout << "Failed to load all pois locations!" << std::endl;
	}

	ret = LoadInputHeightMap();
	if (!ret) {
		std::cout << "Failed to load input height map for level : " << (m_isLevel1Instances ? 1 : 0) << std::endl;
		DeInitialize();
		return ret;
	}

	//ret = LoadRegions();

	return ret;
}

std::vector<std::pair<std::vector<Point>, int>>* CPlantsSimulation::LoadCaveNodesFromPointCloud(const std::string& filePath)
{
	char delimiter = ',';
	int columnCount = countColumnsInCSV(filePath, delimiter);
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open the cave point cloud file :" << filePath << std::endl;
		return nullptr;
	}
	std::string line;

	double xRatio = m_topLayerMeta->xRatio;
	double yRatio = m_topLayerMeta->yRatio;
	double batch_min_x = m_topLayerMeta->batch_min_x;
	double batch_min_y = m_topLayerMeta->batch_min_y;
	double x0 = m_topLayerMeta->x0;
	double y0 = m_topLayerMeta->y0;

	m_cavePointInfoList.clear();
	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::string field;
		CavesPointInfo cavePointInfo;
		if (std::getline(lineStream, field, ',')) {
			double xValue = std::stod(field);
			cavePointInfo.x = xValue - batch_min_x - x0;
		}
		if (std::getline(lineStream, field, ',')) {
			double yValue = std::stod(field);
			cavePointInfo.y = yValue - batch_min_y - y0;
		}
		if (std::getline(lineStream, field, ',')) {
			cavePointInfo.z = std::stod(field);
		}
		if (std::getline(lineStream, field, ',')) {
			cavePointInfo.generation = std::stoi(field);
		}
		if (std::getline(lineStream, field, ',')) {
			cavePointInfo.index = std::stoi(field);
		}
		if (columnCount >= 7) //New .xyz file has new column not useful for this case
		{
			if (std::getline(lineStream, field, ',')) {

			}
		}
		if (std::getline(lineStream, field, ',')) {
			cavePointInfo.order = std::stoi(field);
		}
		m_cavePointInfoList.push_back(cavePointInfo);
	}
	std::cout << "CavePointInfoList has " << m_cavePointInfoList.size() << " cave points!" << std::endl;
	std::vector<std::pair<std::vector<Point>, int>>* pRet = ConvertCaveInfoToCaveNodes(m_cavePointInfoList);
	if (pRet)
	{
		std::cout << "Cave nodes lines number is " << pRet->size() << std::endl;
		SaveCavesAsRoadMap(pRet, false);
		SaveCavesAsObj(pRet);
	}
	return pRet;
}

bool CPlantsSimulation::SaveCavesAsObj(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel0Nodes)
{
	if (!p2dCaveLevel0Nodes)
	{
		std::cout << "Cave nodes lines number is not loaded!" << std::endl;
		return false;
	}
	const int MAX_PATH = 250;

	char cave_obj_path[MAX_PATH];
	memset(cave_obj_path, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(cave_obj_path, MAX_PATH, "%s/%d_%d_%d_%d_%d_%d_cave_obj.obj", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#else
	sprintf_s(cave_obj_path, MAX_PATH, "%s\\%d_%d_%d_%d_%d_%d_cave_obj.obj", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#endif
	std::cout << "Cave obj file is " << cave_obj_path << std::endl;

	std::ofstream outFile(cave_obj_path);
	if (!outFile.is_open()) {
		std::cerr << "SaveCavesAsObj Error: Unable to open the file " << cave_obj_path << std::endl;
		return false;
	}

	// Counter for global vertex index (OBJ indices start at 1)
	int vertexIndex = 1;

	// Map to store the starting index of each line
	std::vector<int> lineStartIndices;

	// Write vertices
	for (const auto& line : *p2dCaveLevel0Nodes) {
		// Store the starting index for this line
		lineStartIndices.push_back(vertexIndex);

		// Write all vertices for this line
		for (const auto& point : line.first) {
			double relativeX = point.x;
			double relativeY = point.y;
			//outFile << "v " << relativeX << " " << relativeY << " 0.0" << "\n"; //For Tree program
			outFile << "v " << relativeY << " " << relativeX << " 0.0" << "\n"; //For Road program should invert x and y.
			vertexIndex++;
		}
	}

	int currentLineIndex = 0;
	for (const auto& line : *p2dCaveLevel0Nodes) {
		outFile << "l";
		int startIndex = lineStartIndices[currentLineIndex];
		int pointCount = line.first.size();

		// Add all vertex indices for this line
		for (int i = 0; i < pointCount; i++) {
			outFile << " " << (startIndex + i);
		}
		outFile << "\n";

		currentLineIndex++;
	}

	outFile.close();
	std::cout << "Successfully exported lines to " << cave_obj_path << std::endl;
	return true;
}

bool CPlantsSimulation::SaveCavesAsRoadMap(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel0Nodes, bool invert = false)
{
	if (!p2dCaveLevel0Nodes)
	{
		std::cout << "Cave nodes lines number is not loaded!" << std::endl;
		return false;
	}
	const int MAX_PATH = 250;

	const int exportCols = m_roadInputHeightMapWidth * m_tileScale;
	const int exportRows = m_roadInputHeightMapHeight * m_tileScale;
	
	std::vector<std::vector<unsigned short>> caveRoadMapArrayInvert(exportRows, std::vector<unsigned short>(exportCols));
	// Reset to 0
	for (auto& row : caveRoadMapArrayInvert) {
		std::fill(row.begin(), row.end(), 0);
	}
	int outofBoundsCount = 0;
	const double ratioCols = static_cast<double>(m_tilePixelMeterWidth / m_roadInputHeightMapWidth);
	const double ratioRows = static_cast<double>(m_tilePixelMeterHeight / m_roadInputHeightMapHeight);
	for (const auto& [points, generation] : *p2dCaveLevel0Nodes)
	{
		for (const auto& point : points)
		{
			double relativeX = point.x;
			double relativeY = point.y;
			int x = static_cast<int>(relativeX / ratioCols);
			int y = static_cast<int>(relativeY / ratioRows);
			if (x >= 0 && x < exportCols && y >= 0 && y < exportRows)
			{
				caveRoadMapArrayInvert[y][x]++;
			}
			else
			{
				//std::cout << "Cave point out of bounds: (" << relativeX << ", " << relativeY << ") -> (" << x << ", " << y << ")" << std::endl;
				outofBoundsCount++;
			}
		}
	}
	std::cout << "Cave point out of bounds count: " << outofBoundsCount << std::endl;
	std::vector<std::vector<unsigned short>> caveRoadMapArray = invert2DArray(caveRoadMapArrayInvert);
	char ushort_cave_roadmap_raw[MAX_PATH];
	memset(ushort_cave_roadmap_raw, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(ushort_cave_roadmap_raw, MAX_PATH, "%s/%d_%d_%d_%d_%d_%d_ushort_cave_roadmap_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#else
	sprintf_s(ushort_cave_roadmap_raw, MAX_PATH, "%s\\%d_%d_%d_%d_%d_%d_ushort_cave_roadmap_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, m_tileScale, m_roadInputHeightMapWidth, m_roadInputHeightMapHeight);
#endif
	std::cout << "Cave Road Map file is " << ushort_cave_roadmap_raw << std::endl;
	bool outputCaveRoadMap = Output2DVectorToRawFile(caveRoadMapArray, ushort_cave_roadmap_raw);
	return outputCaveRoadMap;
}

bool CPlantsSimulation::loadAllPoisLocationsFromCSV()
{
	m_PoisLocations.clear();
	bool loadMostDistant = loadPoisLocationsFromCSV(m_mostDistantPointFile, m_PoisLocations);
	bool loadMostTravelled = loadPoisLocationsFromCSV(m_mostTravelledPointFile, m_PoisLocations);
	bool loadCentroid = loadPoisLocationsFromCSV(m_centroidPointFile, m_PoisLocations);
	return loadMostDistant && loadMostTravelled && loadCentroid;
}

bool CPlantsSimulation::loadPoisLocationsFromCSV(const string& filePath, std::vector<Point>& poisLocations)
{
	char delimiter = ',';
	int columnCount = countColumnsInCSV(filePath, delimiter);
	std::cout << "The PointInstance csv file " << filePath << " has " << columnCount << " columns" << std::endl;

	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open the csv file :" << filePath << std::endl;
		return false;
	}

	std::string header;
	std::getline(file, header);

	std::string line;

	int tableRowsCount = (*m_pCellTable).size();
	int tableColsCount = (*m_pCellTable)[0].size();

	int negativeHeightCount = 0;
	int index = 0;
	int originalCount = 0;

	unsigned int variant = 0;

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::string field;

		originalCount++;

		double xPos = 0.0;
		double yPos = 0.0;
		double zPos = 0.0;

		bool hasHeight = true;
		if (std::getline(lineStream, field, ',')) {
			//xPos = std::stod(field);
			yPos = std::stod(field);
		}
		if (std::getline(lineStream, field, ',')) {
			//yPos = std::stod(field);
			xPos = std::stod(field);
		}
		if (std::getline(lineStream, field, ',')) {
			zPos = std::stod(field);
		}
		if (std::getline(lineStream, field, ',')) {

		}
		if (std::getline(lineStream, field, ',')) {

		}
		if (std::getline(lineStream, field, ',')) {

		}
		if (columnCount >= 7) //Has regionId column
		{
			if (std::getline(lineStream, field, ',')) {

			}
		}
		if (columnCount >= 8) //Has resource type (or variant) column
		{
			if (std::getline(lineStream, field, ',')) {
				variant = std::stoi(field);
			}
		}

		Point poi = Point(xPos, yPos);
		poisLocations.push_back(poi);
	}

	file.close();
	return true;
}

bool CPlantsSimulation::LoadForest()
{
	if (m_pForest) {
		delete m_pForest;
		m_pForest = nullptr;
	}
	m_pForest = new CForest();
	if (!m_pForest) {
		return false;
	}

	m_pForest->setIsLevel1Instances(m_isLevel1Instances);
	m_pForest->setCellTable(m_pCellTable);
	m_pForest->setMetaInfo(m_topLayerMeta);
	
	m_pForest->set2dCaveLevel0Nodes(m_p2dCaveLevel0Nodes);
	m_pForest->set2dCaveLevel1Nodes(m_p2dCaveLevel1Nodes);

	m_pForest->setPoisLocations(&m_PoisLocations);

	if (!m_topLayerImage) {
		return false;
	}
	if (!m_topLayerMeta)
	{
		return false;
	}

	const int forestXSize = m_topLayerImage->input_image_width;
	const int forestZSize = m_topLayerImage->input_image_width;
	//const int forestXSize = 4096;
	//const int forestZSize = 4096;

	m_pForest->xSize = static_cast<int>(forestXSize * m_topLayerMeta->xRatio);
	m_pForest->zSize = static_cast<int>(forestZSize * m_topLayerMeta->yRatio);
	m_pForest->xo = 0;
	m_pForest->zo = 0;
	m_pForest->maxHeight = m_maxHeight;
	std::cout << "Forest xSize is : " << m_pForest->xSize << std::endl;
	std::cout << "Forest zSize is : " << m_pForest->zSize << std::endl;

	std::string inputTreeListCsv = m_isLevel1Instances ? m_inputLevel1TreeListCsv : m_inputTreeListCsv;
	bool loadTreeList = m_pForest->parseTreeListCsv(inputTreeListCsv);
	if (!loadTreeList)
	{
		m_pForest->loadDefaultTreeClasses();
		m_pForest->loadDefaultMasks();
		m_pForest->loadDefaultGlobalMasks();
	}

	return true;
}

bool CPlantsSimulation::LoadInstanceExporter()
{
	if (m_pInstanceExporter)
	{
		delete m_pInstanceExporter;
		m_pInstanceExporter = nullptr;
	}
	m_pInstanceExporter = new CPsInstanceExporter();
	if (!m_pInstanceExporter)
	{
		return false;
	}

	m_pInstanceExporter->setIsLevel1Instances(m_isLevel1Instances);
	m_pInstanceExporter->setIsOnlyPoIs(m_onlyPOIs);
	m_pInstanceExporter->setKeepOldTreeFiles(m_keepOldTreeFiles);	
	m_pInstanceExporter->setCellTable(m_pCellTable);
	m_pInstanceExporter->setMetaInfo(m_topLayerMeta);
	m_pInstanceExporter->setMostTravelledPointFilePath(m_mostTravelledPointFile);
	m_pInstanceExporter->setMostDistantPointFilePath(m_mostDistantPointFile);
	m_pInstanceExporter->setCentroidPointFilePath(m_centroidPointFile);

	m_pInstanceExporter->set2dCaveLevel0Nodes(m_p2dCaveLevel0Nodes);
	m_pInstanceExporter->set2dCaveLevel1Nodes(m_p2dCaveLevel1Nodes);

	m_pInstanceExporter->setFullTreeOutputs(m_pForest->getTreeInstanceFullOutput());
	m_pInstanceExporter->setTilesInfo(m_tiles, m_tileX, m_tileY);
	m_pInstanceExporter->setLod(m_currentLod);

	return true;
}

bool CPlantsSimulation::BuildForest()
{
	if (!m_pForest) {
		return false;
	}

	string title = "Build Forest";
	CTimeCounter timeCounter(title);

	//float forestAge = 500;
	//int iteration = 100;

	if (!m_onlyPOIs)
	{
		std::cout << "Forest Age is : " << m_forestAge << std::endl;
		std::cout << "Toatal iteration count is : " << m_iteration << std::endl;
		m_pForest->generate(m_forestAge, m_iteration);
	}
	else
	{
		std::cout << "Only Generate POIs" << std::endl;
	}
	
	return true;
}

bool CPlantsSimulation::OutputResults()
{
	string outputFile = m_isLevel1Instances ? m_outputFile_level1: m_outputFile_level0;
	bool output = m_pForest->outputCSVTreeInstanceResults(outputFile);
	if (!output)
	{
		std::cout << "Fail to m_pForest->outputCSVTreeInstanceResults for file : " << outputFile << std::endl;
		return false;
	}
	string fullOutputFile = m_isLevel1Instances ? m_fullOutputFile_level1 : m_fullOutputFile_level0;
	output = m_pForest->outputCSVFullTreeInstanceResults(fullOutputFile);
	if (!output)
	{
		std::cout << "Fail to m_pForest->outputCSVFullTreeInstanceResults for file : " << fullOutputFile << std::endl;
		return false;
	}
	string pcFullOutputFile = m_isLevel1Instances ? m_pcFullOutputFile_level1 : m_pcFullOutputFile_level0;
	output = m_pForest->outputPointsCloudFullTreeInstanceResults(pcFullOutputFile);
	//std::string pcFullOutputFileWithRatio = m_pcFullOutputFile + ".ratio.xyz";
	//output = m_pForest->outputPointsCloudFullTreeInstanceResultsWithRatio(pcFullOutputFileWithRatio);

	output = LoadInstanceExporter();
	if (!output)
	{
		std::cout << "Fail to m_pForest->LoadInstanceExporter()" << std::endl;
		return false;
	}
	const int MAX_PATH = 250;
	char subFullOutput_Dir[MAX_PATH];
	memset(subFullOutput_Dir, 0, sizeof(char) * MAX_PATH);

#if __APPLE__ 
	if (m_isLevel1Instances)
	{
		snprintf(subFullOutput_Dir, MAX_PATH, "%s/instanceoutput_level1", m_outputDir.c_str());
	}
	else
	{
		snprintf(subFullOutput_Dir, MAX_PATH, "%s/instanceoutput_level0", m_outputDir.c_str());
	}
#else
	if (m_isLevel1Instances)
	{
		sprintf_s(subFullOutput_Dir, MAX_PATH, "%s\\instanceoutput_level1", m_outputDir.c_str());
	}
	else
	{
		sprintf_s(subFullOutput_Dir, MAX_PATH, "%s\\instanceoutput_level0", m_outputDir.c_str());
	}
#endif

	if (output)
	{
		output = m_pInstanceExporter->outputSubfiles(subFullOutput_Dir);
	}
	return output;
}
