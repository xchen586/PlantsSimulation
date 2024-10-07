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

void CPlantsSimulation::DeInitialize()
{
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
	if (m_topLayerImage) {
		delete m_topLayerImage;
		m_topLayerImage = nullptr;
	}
	if (m_topLayerMeta) {
		delete m_topLayerMeta;
		m_topLayerMeta = nullptr;
	}
	if (m_pForest) {
		delete m_pForest;
		m_pForest = nullptr;
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
		if (m_topLayerImage) {
			delete m_topLayerImage;
			m_topLayerImage = nullptr;
		}
		if (m_pCellTable) {
			delete m_pCellTable;
			m_pCellTable = nullptr;
		}
		return false;
	}
	if (m_topLayerImage && m_topLayerImage->input_image_data)
	{
		delete m_topLayerImage->input_image_data;
		m_topLayerImage->input_image_data = nullptr;
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

	int exportXLow = 300;
	int exportYLow = 300;
	//std::vector<std::vector<byte>> humidityExportLow = ScaleArray(humidity4K, exportXLow, exportYLow);
	std::vector<std::vector<unsigned char>> humidityExportLowInvert = resample2DUCharWithAverage(humidity4K, exportXLow, exportYLow);
	std::vector<std::vector<unsigned char>> humidityExportLow = invert2DArray(humidityExportLowInvert);
	char byte_humidity_map_low_raw[MAX_PATH];
	memset(byte_humidity_map_low_raw, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(byte_humidity_map_low_raw, MAX_PATH, "%s/%d_%d_%d_%d_%d_byte_humidity_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, exportXLow, exportYLow);
#else
	sprintf_s(byte_humidity_map_low_raw, MAX_PATH, "%s\\%d_%d_%d_%d_%d_byte_humidity_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, exportXLow, exportYLow);
#endif
	bool outputHumidityMapLow = Output2DVectorToRawFile(humidityExportLow, byte_humidity_map_low_raw);

	int exportXHigh = 600;
	int exportYHigh = 600;
	//std::vector<std::vector<byte>> humidityExportHigh = ScaleArray(humidity4K, exportXHigh, exportYHigh);
	std::vector<std::vector<unsigned char>> humidityExportHighInvert = resample2DUCharWithAverage(humidity4K, exportXHigh, exportYHigh);
	std::vector<std::vector<unsigned char>> humidityExportHigh = invert2DArray(humidityExportHighInvert);

	char byte_humidity_map_high_raw[MAX_PATH];
	memset(byte_humidity_map_high_raw, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(byte_humidity_map_high_raw, MAX_PATH, "%s/%d_%d_%d_%d_%d_byte_humidity_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, exportXHigh, exportYHigh);
#else
	sprintf_s(byte_humidity_map_high_raw, MAX_PATH, "%s\\%d_%d_%d_%d_%d_byte_humidity_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, exportXHigh, exportYHigh);
#endif
	bool outputHumidityMapHigh = Output2DVectorToRawFile(humidityExportHigh, byte_humidity_map_high_raw);
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
		delete m_topLayerMeta;
		m_topLayerMeta = nullptr;
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
	const int regionsWidth = 300;
	const int regionsHeight = 300;

	const int worldTileWidth = 30000;
	const int worldTileHeight = 30000;

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

	std::vector<std::vector<int>> regionsShort300 = Read2DIntArray(m_regionsRawFile, regionsWidth, regionsHeight);
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
			if (regionsShort300[row][col] != 0)
			{
				int regionId = regionsShort300[row][col];
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
	const int regionsWidth = 300;
	const int regionsHeight = 300;
	const int worldTileWidth = 30000;
	const int worldTileHeight = 30000;

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
	
	std::vector<std::vector<int>> regionsInt300 = Read2DIntArray(m_regionsRawFile, regionsWidth, regionsHeight);
	//std::vector<std::vector<int>> regionsInt300_Raw = Read2DIntArray(m_regionsRawFile, regionsWidth, regionsHeight);
	//std::vector<std::vector<int>> regionsInt300 = invert2DArray(regionsInt300_Raw);
	bool loadAllRegionInfo = LoadRegionInfoFromCSV(m_regionsInfoFile, m_regionInfoMap);
	
	std::cout << "Total region info count is " << m_regionInfoMap.size() << std::endl;
	
	int regionPlaceCount = 0;
	int maxRegionId = 0;
	for (int x = 0; x < regionsWidth; x++)
	{
		for (int y = 0; y < regionsHeight; y++)
		{
			int regionValue = regionsInt300[x][y];
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
		double posZ = 0;
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

					int regionValue = regionsInt300[iIndexX][iIndexY];
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

		string subRegionInfoPath = GetSubRegionInfoOutputCSVFilePathForRegion(subRegionOutput_Dir, lod, cellX, cellY, cellZ);
		bool saveSubInfo = SaveSubRegionInfoToCSVFile(subRegionInfoPath, m_regionInfoMap, subRegionSet);
	}
	
	std::cout << "Total regions count is " << totalRegionsCount << std::endl;
	return ret;
}

bool CPlantsSimulation::LoadInputHeightMap()
{
	bool needMaskPositive = true;
	if (!m_topLayerImage || !m_pCellTable)
	{
		return false;
	}
	const int width = m_topLayerImage->input_image_width;
	const int height = m_topLayerImage->input_image_height;

	std::vector<std::vector<short>> meshHeightMasksShort4096 = Read2DShortArray(m_meshHeightMasksFile, width, height);
	std::vector<std::vector<short>> mesh2HeightMasksShort4096 = Read2DShortArray(m_mesh2HeightMasksFile, width, height);
	std::vector<std::vector<short>> pcHeightMasksShort4096 = Read2DShortArray(m_pcHeightMasksFile, width, height);
	std::vector<std::vector<short>> l1HeightMasksShort4096 = Read2DShortArray(m_l1HeightMasksFile, width, height);

	std::vector<std::vector<short>> heightMasksShort4096(width, std::vector<short>(height));
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			short meshValue = meshHeightMasksShort4096[x][y];
			short mesh2Value = mesh2HeightMasksShort4096[x][y];
			short pcValue = pcHeightMasksShort4096[x][y];
			short l1Value = l1HeightMasksShort4096[x][y];
			if (needMaskPositive)
			{
				
			}
			//short value = std::max(meshValue, pcValue);
			//short value = FindMaxIn3(meshValue, mesh2Value, pcValue);
#if USE_WITH_BASEMESHES_LEVEL1
			short value = FindMaxIn4(meshValue, mesh2Value, pcValue, l1Value);
#else
			short value = FindMaxIn3(meshValue, pcValue, l1Value);
#endif // USE_WITH_BASEMESHES_LEVEL1
			heightMasksShort4096[x][y] = value;
			if (needMaskPositive)
			{
				if (value < 0)
				{
					heightMasksShort4096[x][y] = 0;
				}
			}
		}
	}

#if USE_DISPLAY_HEIGHTMAP_MASK_RESULT
	unsigned int noHeightCountMesh0 = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (meshHeightMasksShort4096[x][y] == 0)
			{
				noHeightCountMesh0++;
			}
		}
	}
	std::cout << "Mesh level 0 height map mask no data count is : " << noHeightCountMesh0 << std::endl;

	unsigned int noHeightCountMesh1 = 0;
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			if (mesh2HeightMasksShort4096[x][y] == 0)
			{
				noHeightCountMesh1++;
			}
		}
	}
	std::cout << "Mesh level 1 height map mask no data count is : " << noHeightCountMesh1 << std::endl;

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
			if (l1HeightMasksShort4096[x][y] == 0)
			{
				noHeightCountLevel1++;
			}
		}
	}
	std::cout << "Point cloud level 1 height map mask no data count is : " << noHeightCountLevel1 << std::endl;

	unsigned int noHeightCountFinal = 0;
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
	std::cout << "Final height map mask no data count is : " << noHeightCountFinal << std::endl;
#endif
	
	std::vector<std::vector<short>> meshHeightMapShort4096 = Read2DShortArray(m_meshHeightMapFile, width, height);
	std::vector<std::vector<short>> mesh2HeightMapShort4096 = Read2DShortArray(m_mesh2HeightMapFile, width, height);
	std::vector<std::vector<short>> pcHeightMapShort4096 = Read2DShortArray(m_pcHeightMapFile, width, height);
	std::vector<std::vector<short>> l1HeightMapShort4096 = Read2DShortArray(m_l1HeightMapFile, width, height);

	std::vector<std::vector<short>> heightMapShort4096(width, std::vector<short>(height));
	std::vector<std::vector<short>> heightMapAdjust1Short4096(width, std::vector<short>(height)); // meshValue, mesh2Value, pcValue
	std::vector<std::vector<short>> heightMapAdjust2Short4096(width, std::vector<short>(height)); // meshValue, pcValue
	std::vector<std::vector<short>> heightMapAdjust3Short4096(width, std::vector<short>(height)); // pcValue
	std::vector<std::vector<short>> heightMapAdjust4Short4096(width, std::vector<short>(height)); // pcValue, l1Value

	std::vector<std::vector<short>> heightMapRoadDataShort4096(width, std::vector<short>(height)); 
	bool needHeightPositive = false;

	short minHeight = std::numeric_limits<short>::max();
	short maxHeight = std::numeric_limits<short>::min();
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			short meshValue = meshHeightMapShort4096[x][y];
			short mesh2Value = mesh2HeightMapShort4096[x][y];
			short pcValue = pcHeightMapShort4096[x][y];
			short l1Value = l1HeightMapShort4096[x][y];
#if USE_WITH_BASEMESHES_LEVEL1
			short value = FindMaxIn4(meshValue, mesh2Value, pcValue, l1Value);
#else
			short value = FindMaxIn3(meshValue, pcValue, l1Value);
#endif
			if (needHeightPositive && (value < 0))
			{
				value = 0;
			}
			heightMapShort4096[x][y] = value;
			short valueAdjust1 = FindMaxIn3(meshValue, mesh2Value, pcValue);
			if (needHeightPositive && (valueAdjust1 < 0))
			{
				valueAdjust1 = 0;
			}
			heightMapAdjust1Short4096[x][y] = valueAdjust1;
			short valueAdjust2 = std::max(meshValue, pcValue);
			if (needHeightPositive && (valueAdjust2 < 0))
			{
				valueAdjust2 = 0;
			}
			heightMapAdjust2Short4096[x][y] = valueAdjust2;
			short valueAdjust3 = pcValue;
			if (needHeightPositive && (valueAdjust3 < 0))
			{
				valueAdjust3 = 0;
			}
			heightMapAdjust3Short4096[x][y] = valueAdjust3;
			short valueAdjust4 = std::max(pcValue, l1Value);
			if (needHeightPositive && (valueAdjust3 < 0))
			{
				valueAdjust4 = 0;
			}

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

	int exportHeightMapLowRawX = 300;
	int exportHeightMapLowRawY = 300;
	//std::vector<std::vector<short>> heightMapExportLowRawShort = ScaleArray(heightMapAdjust3Short4096, exportHeightMapLowRawX, exportHeightMapLowRawY);
	//std::vector<std::vector<short>> heightMapExportLowRawShort = resample2DArrayByFunc(heightMapRoadDataShort4096, exportHeightMapLowRawX, exportHeightMapLowRawY, findAverageInBlock<short>);
	std::vector<std::vector<short>> heightMapExportLowRawShort = resample2DShortWithAverage(heightMapRoadDataShort4096, exportHeightMapLowRawX, exportHeightMapLowRawY);
	std::vector<std::vector<unsigned short>> heightMapExportLowRawUShortInvert = ConvertShortMatrixToUShort(heightMapExportLowRawShort);
	std::vector<std::vector<unsigned short>> heightMapExportLowRawUShort = invert2DArray(heightMapExportLowRawUShortInvert);

	int exportHeightMapHighRawX = 600;
	int exportHeightMapHighRawY = 600;
	//std::vector<std::vector<short>> heightMapExportHighRawShort = ScaleArray(heightMapAdjust3Short4096, exportHeightMapHighRawX, exportHeightMapHighRawY);
	std::vector<std::vector<short>> heightMapExportHighRawShort = resample2DArrayByFunc(heightMapRoadDataShort4096, exportHeightMapHighRawX, exportHeightMapHighRawY, findAverageInBlock<short>);
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
				cell->SetHasHeightValue(heightMasksShort4096[i][j]);
			}
		}
	}

	const int MAX_PATH = 250;
	
	char mesh_heightmap_raw_export[MAX_PATH];
	char mesh2_heightmap_raw_export[MAX_PATH];
	char pc_heightmap_raw_export[MAX_PATH];
	char l1_heightmap_raw_export[MAX_PATH];
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
	snprintf(short_height_map_export, MAX_PATH, "%s/%d_%d_%d_short_height_map_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#else
	snprintf(mesh_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_mesh_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(mesh2_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_mesh2_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(pc_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_pc_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	snprintf(l1_heightmap_raw_export, MAX_PATH, "%s/%d_%d_%d_11_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
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
	sprintf_s(short_height_map_export, MAX_PATH, "%s\\%d_%d_%d_short_height_map_export.csv", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#else
	sprintf_s(mesh_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_mesh_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(mesh2_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_mesh2_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(pc_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_pc_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(l1_heightmap_raw_export, MAX_PATH, "%s\\%d_%d_%d_l1_heightmap_raw_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(short_height_map_export, MAX_PATH, "%s\\%d_%d_%d_short_height_map_export.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#endif
	sprintf_s(double_height_map_exportout, MAX_PATH, "%s\\%d_%d_%d_double_height_map_exportout.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(height_slope_map_exportout, MAX_PATH, "%s\\%d_%d_%d_height_slope_map_exportout.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
	sprintf_s(angle_slope_map_exportout, MAX_PATH, "%s\\%d_%d_%d_angle_slope_map_exportout.xyz", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY);
#endif

#if __APPLE__
	snprintf(ushort_height_map_low_raw, MAX_PATH, "%s/%d_%d_%d_%d_%d_ushort_height_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, exportHeightMapLowRawX, exportHeightMapLowRawY);
	snprintf(ushort_height_map_high_raw, MAX_PATH, "%s/%d_%d_%d_%d_%d_ushort_height_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, exportHeightMapHighRawX, exportHeightMapHighRawY);
#else
	sprintf_s(ushort_height_map_low_raw, MAX_PATH, "%s\\%d_%d_%d_%d_%d_ushort_height_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, exportHeightMapLowRawX, exportHeightMapLowRawY);
	sprintf_s(ushort_height_map_high_raw, MAX_PATH, "%s\\%d_%d_%d_%d_%d_ushort_height_map_raw.raw", m_outputDir.c_str(), m_tiles, m_tileX, m_tileY, exportHeightMapHighRawX, exportHeightMapHighRawY);
#endif

#if USE_EXPORT_HEIGHT_MAP
	string title = "EXPORT HEIGHT MAP INFO";
	CTimeCounter timeCounter(title);
#if USE_OUTPUT_HEIGHT_MAP_CSV
	ExportShortHeightMap(meshHeightMapShort4096, mesh_heightmap_raw_export, 0x00FF0000, true);
	ExportShortHeightMap(mesh2HeightMapShort4096, mesh2_heightmap_raw_export, 0x00FF0000, true);
	ExportShortHeightMap(pcHeightMapShort4096, pc_heightmap_raw_export, 0x0000FF00, true);
	ExportShortHeightMap(l1HeightMapShort4096, l1_heightmap_raw_export, 0x0000FF00, true);
	ExportShortHeightMap(heightMapShort4096, short_height_map_export, 0x000000FF, true);
#else
	ExportShortHeightMapWithMask(meshHeightMapShort4096, meshHeightMasksShort4096, mesh_heightmap_raw_export, 0x00FF0000, false, true);
	ExportShortHeightMapWithMask(mesh2HeightMapShort4096, mesh2HeightMasksShort4096, mesh2_heightmap_raw_export,  0x00FF0000, false, true);
	ExportShortHeightMapWithMask(pcHeightMapShort4096, pcHeightMasksShort4096, pc_heightmap_raw_export, 0x0000FF00, false, true);
	ExportShortHeightMapWithMask(l1HeightMapShort4096, l1HeightMasksShort4096, l1_heightmap_raw_export, 0x0000FF00, false, true);
	ExportShortHeightMapWithMask(heightMapShort4096, heightMasksShort4096, short_height_map_export, 0x000000FF, false, true);
#endif
	ExportDoubleHeightMap(heightMapDouble4096, double_height_map_exportout, 0x00FFFF00, false);
	ExportShortHeightSlopeMap(slopeShort4096, height_slope_map_exportout, 0x0000FF00, false);
	ExportAngleSlopeMap(slopeDouble4096, angle_slope_map_exportout, 0x00FF0000, false);
#endif
	bool outputHeightMapLow = Output2DVectorToRawFile(heightMapExportLowRawUShort, ushort_height_map_low_raw);
	bool outputHeightMapHigh = Output2DVectorToRawFile(heightMapExportHighRawUShort, ushort_height_map_high_raw);
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
				if (value != 0)
				{
					//std::cout << "height map value is not 0 :  value : " << value << ", i = " << i << " , j = " << j << std::endl;
				}
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
			else
			{
				short value = static_cast<short>(heightMap[i][j] / xRatio);
				if (value != 0)
				{
					//std::cout << "height map value is not 0 :  value : " << value << ", i = " << i << " , j = " << j << std::endl;
				}
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
					if (value != 0)
					{
						//std::cout << "height map value is not 0 :  value : " << value << ", i = " << i << " , j = " << j << std::endl;
					}
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
				else
				{

					short value = static_cast<short>(heightMap[i][j] / xRatio);
					if (value != 0)
					{
						//std::cout << "height map value is not 0 :  value : " << value << ", i = " << i << " , j = " << j << std::endl;
					}
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
bool CPlantsSimulation::LoadInputData()
{
	bool ret = false;
	DeInitialize();
	ret = LoadInputImage();
	if (!ret) {
		DeInitialize();
		return ret;
	}
	ret = LoadImageMetaFile();
	if (!ret)
	{
		DeInitialize();
		return ret;
	}

	ret = LoadAndOutputRegions();

	ret = LoadInputHeightMap();
	if (!ret) {
		DeInitialize();
		return ret;
	}

	//ret = LoadRegions();

	return ret;
}

bool CPlantsSimulation::LoadForest()
{
	m_pForest = new CForest();
	if (!m_pForest) {
		return false;
	}

	m_pForest->setCellTable(m_pCellTable);
	m_pForest->setMetaInfo(m_topLayerMeta);

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
	cout << "Forest xSize is : " << m_pForest->xSize << endl;
	cout << "Forest zSize is : " << m_pForest->zSize << endl;

	bool loadTreeList = m_pForest->parseTreeListCsv(m_inputTreeListCsv);
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
	m_pInstanceExporter = new CPsInstanceExporter();
	if (!m_pInstanceExporter)
	{
		return false;
	}

	m_pInstanceExporter->setCellTable(m_pCellTable);
	m_pInstanceExporter->setMetaInfo(m_topLayerMeta);
	m_pInstanceExporter->setMostTravelledPointFilePath(m_mostTravelledPointFile);
	m_pInstanceExporter->setMostDistantPointFilePath(m_mostDistantPointFile);

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

	cout << "Forest Age is : " << m_forestAge << endl;
	cout << "Toatal iteration count is : " << m_iteration << endl;
	m_pForest->generate(m_forestAge, m_iteration);

	return true;
}

bool CPlantsSimulation::OutputResults()
{
	bool output = m_pForest->outputCSVTreeInstanceResults(m_outputFile);
	if (!output)
	{
		std::cout << "Fail to m_pForest->outputCSVTreeInstanceResults for file : " << m_outputFile << std::endl;
		return false;
	}
	output = m_pForest->outputCSVFullTreeInstanceResults(m_fullOutputFile);
	if (!output)
	{
		std::cout << "Fail to m_pForest->outputCSVFullTreeInstanceResults for file : " << m_fullOutputFile << std::endl;
		return false;
	}
	output = m_pForest->outputPointsCloudFullTreeInstanceResults(m_pcFullOutputFile);
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
		snprintf(subFullOutput_Dir, MAX_PATH, "%s/instanceoutput", m_outputDir.c_str());
	#else
		sprintf_s(subFullOutput_Dir, MAX_PATH, "%s\\instanceoutput", m_outputDir.c_str());
	#endif

	if (output)
	{
		output = m_pInstanceExporter->outputSubfiles(subFullOutput_Dir);
	}
	return output;
}
