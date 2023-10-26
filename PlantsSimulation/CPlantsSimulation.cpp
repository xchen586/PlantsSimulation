#include "CPlantsSimulation.h"
#include "CCellInfo.h"
#include "CTimeCounter.h"
#include "PsMarco.h"
//#include "PsHelper.h"

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

bool CPlantsSimulation::LoadInputHeightMap()
{
	if (!m_topLayerImage || !m_pCellTable)
	{
		return false;
	}
	const int width = m_topLayerImage->input_image_width;
	const int height = m_topLayerImage->input_image_height;
	std::vector<std::vector<short>> meshHeightMapShort4096 = Read2DShortArray(m_meshHeightMapFile, width, height);
	std::vector<std::vector<short>> mesh2HeightMapShort4096 = Read2DShortArray(m_mesh2HeightMapFile, width, height);
	std::vector<std::vector<short>> pcHeightMapShort4096 = Read2DShortArray(m_pcHeightMapFile, width, height);
	std::vector<std::vector<short>> l1HeightMapShort4096 = Read2DShortArray(m_l1HeightMapFile, width, height);

	std::vector<std::vector<short>> heightMapShort4096(width, std::vector<short>(height));
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
			//short value = std::max(meshValue, pcValue);
			//short value = FindMaxIn3(meshValue, mesh2Value, pcValue);
			short value = FindMaxIn4(meshValue, mesh2Value, pcValue, l1Value);
			heightMapShort4096[x][y] = value;
			minHeight = std::min(minHeight, value);
			maxHeight = std::max(maxHeight, value);
		}
	}
	std::cout << "Final Short Height Map minHeight = " << minHeight << " , maxHeight = " << maxHeight << std::endl;

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
			//short value = std::max(meshValue, pcValue);
			//short value = FindMaxIn3(meshValue, mesh2Value, pcValue);
			short value = FindMaxIn4(meshValue, mesh2Value, pcValue, l1Value);
			heightMasksShort4096[x][y] = value;
		}
	}

	//std::vector<std::vector<short>> slopeShort4096 = ComputeAbsMaxHeightSlopeMap(heightMapShort4096);
	std::vector<std::vector<short>> slopeShort4096 = ComputeSlopeMap(heightMapShort4096);
	std::vector<std::vector<double>> heightMapDouble4096 = ConvertShortMatrixToDouble1(heightMapShort4096);

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

	memset(mesh_heightmap_raw_export, 0, sizeof(char) * MAX_PATH);
	memset(mesh2_heightmap_raw_export, 0, sizeof(char) * MAX_PATH);
	memset(pc_heightmap_raw_export, 0, sizeof(char) * MAX_PATH);
	memset(l1_heightmap_raw_export, 0, sizeof(char) * MAX_PATH);
	memset(short_height_map_export, 0, sizeof(char) * MAX_PATH);
	memset(double_height_map_exportout, 0, sizeof(char) * MAX_PATH);
	memset(height_slope_map_exportout, 0, sizeof(char) * MAX_PATH);
	memset(angle_slope_map_exportout, 0, sizeof(char) * MAX_PATH);

#if __APPLE__
#if USE_OUTPUT_HEIGHT_MAP_CSV
	snprintf(mesh_heightmap_raw_export, MAX_PATH, "%s/mesh_heightmap_raw_export.csv", m_outputDir.c_str());
	snprintf(mesh2_heightmap_raw_export, MAX_PATH, "%s/mesh2_heightmap_raw_export.csv", m_outputDir.c_str());
	snprintf(pc_heightmap_raw_export, MAX_PATH, "%s/pc_heightmap_raw_export.csv", m_outputDir.c_str());
	snprintf(l1_heightmap_raw_export, MAX_PATH, "%s/l1_heightmap_raw_export.csv", m_outputDir.c_str());
	snprintf(short_height_map_export, MAX_PATH, "%s/short_height_map_export.csv", m_outputDir.c_str());
#else
	snprintf(mesh_heightmap_raw_export, MAX_PATH, "%s/mesh_heightmap_raw_export.xyz", m_outputDir.c_str());
	snprintf(mesh2_heightmap_raw_export, MAX_PATH, "%s/mesh2_heightmap_raw_export.xyz", m_outputDir.c_str());
	snprintf(pc_heightmap_raw_export, MAX_PATH, "%s/pc_heightmap_raw_export.xyz", m_outputDir.c_str());
	snprintf(l1_heightmap_raw_export, MAX_PATH, "%s/11_heightmap_raw_export.xyz", m_outputDir.c_str());
	snprintf(short_height_map_export, MAX_PATH, "%s/short_height_map_export.xyz", m_outputDir.c_str());
#endif
	snprintf(double_height_map_exportout, MAX_PATH, "%s/double_height_map_exportout.xyz", m_outputDir.c_str());
	snprintf(height_slope_map_exportout, MAX_PATH, "%s/height_slope_map_exportout.xyz", m_outputDir.c_str());
	snprintf(angle_slope_map_exportout, MAX_PATH, "%s/angle_slope_map_exportout.xyz", m_outputDir.c_str());
	
#else
#if USE_OUTPUT_HEIGHT_MAP_CSV
	sprintf_s(mesh_heightmap_raw_export, MAX_PATH, "%s\\mesh_heightmap_raw_export.csv", m_outputDir.c_str());
	sprintf_s(mesh2_heightmap_raw_export, MAX_PATH, "%s\\mesh2_heightmap_raw_export.csv", m_outputDir.c_str());
	sprintf_s(pc_heightmap_raw_export, MAX_PATH, "%s\\pc_heightmap_raw_export.csv", m_outputDir.c_str());
	sprintf_s(l1_heightmap_raw_export, MAX_PATH, "%s\\1l_heightmap_raw_export.csv", m_outputDir.c_str());
	sprintf_s(short_height_map_export, MAX_PATH, "%s\\short_height_map_export.csv", m_outputDir.c_str());
#else
	sprintf_s(mesh_heightmap_raw_export, MAX_PATH, "%s\\mesh_heightmap_raw_export.xyz", m_outputDir.c_str());
	sprintf_s(mesh2_heightmap_raw_export, MAX_PATH, "%s\\mesh2_heightmap_raw_export.xyz", m_outputDir.c_str());
	sprintf_s(pc_heightmap_raw_export, MAX_PATH, "%s\\pc_heightmap_raw_export.xyz", m_outputDir.c_str());
	sprintf_s(l1_heightmap_raw_export, MAX_PATH, "%s\\l1_heightmap_raw_export.xyz", m_outputDir.c_str());
	sprintf_s(short_height_map_export, MAX_PATH, "%s\\short_height_map_export.xyz", m_outputDir.c_str());
#endif

	sprintf_s(double_height_map_exportout, MAX_PATH, "%s\\double_height_map_exportout.xyz", m_outputDir.c_str());
	sprintf_s(height_slope_map_exportout, MAX_PATH, "%s\\height_slope_map_exportout.xyz", m_outputDir.c_str());
	sprintf_s(angle_slope_map_exportout, MAX_PATH, "%s\\angle_slope_map_exportout.xyz", m_outputDir.c_str());
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
	ExportShortHeightMap(meshHeightMapShort4096, mesh_heightmap_raw_export, 0x00FF0000, false, true);
	ExportShortHeightMap(mesh2HeightMapShort4096, mesh2_heightmap_raw_export, 0x00FF0000, false, true);
	ExportShortHeightMap(pcHeightMapShort4096, pc_heightmap_raw_export, 0x0000FF00, false, true);
	ExportShortHeightMap(l1HeightMapShort4096, l1_heightmap_raw_export, 0x0000FF00, false, true);
	ExportShortHeightMap(heightMapShort4096, short_height_map_export, 0x000000FF, false, true);
#endif
	ExportDoubleHeightMap(heightMapDouble4096, double_height_map_exportout, 0x00FFFF00, false);
	ExportShortHeightSlopeMap(slopeShort4096, height_slope_map_exportout, 0x0000FF00, false);
	ExportAngleSlopeMap(slopeDouble4096, angle_slope_map_exportout, 0x00FF0000, false);
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
 	
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
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
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
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
	
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
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
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
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
	ret = LoadInputHeightMap();
	if (!ret) {
		DeInitialize();
		return ret;
	}
	return ret;
}

//std::vector<std::vector<unsigned short>> heightMap300 = Read2DShortArray(m_heightMapFile, width, height);

//std::vector<std::vector<unsigned short>> heightMap4096 = ScaleArray(heightMap300, newWidth, newHeight);
//std::vector<std::vector<unsigned short>> slope4096 = ComputeSlopeMap(heightMap4096);

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

	cout << "Forest xSize is : " << m_pForest->xSize << endl;
	cout << "Forest zSize is : " << m_pForest->zSize << endl;

	m_pForest->loadTreeClasses();
	m_pForest->loadMasks();
	m_pForest->loadGlobalMasks();

	return true;
}

bool CPlantsSimulation::BuildForest()
{
	if (!m_pForest) {
		return false;
	}

	string title = "Build Forest";
	CTimeCounter timeCounter(title);

	float forestAge = 500;
	int iteration = 100;
	cout << "Forest Age is : " << forestAge << endl;
	cout << "Toatal iteration count is : " << iteration << endl;
	m_pForest->generate(forestAge, iteration);

	return true;
}

bool CPlantsSimulation::OutputResults()
{
	bool output = m_pForest->outputCSVTreeInstanceResults(m_outputFile);
	if (output)
	{
		output = m_pForest->outputCSVFullTreeInstanceResults(m_fullOutputFile);
	}
	if (output)
	{
		output = m_pForest->outputPointsCloudFullTreeInstanceResults(m_pcFullOutputFile);
	}
	//std::string pcFullOutputFileWithRatio = m_pcFullOutputFile + ".ratio.xyz";
	//output = m_pForest->outputPointsCloudFullTreeInstanceResultsWithRatio(pcFullOutputFileWithRatio);
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
		output = m_pForest->outputSubfiles(subFullOutput_Dir);
	}
	return output;
}
