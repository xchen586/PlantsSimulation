#include "CPlantsSimulation.h"
#include "CCellData.h"
#include "CTimeCounter.h"

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
				CCellData* cell = (*m_pCellTable)[i][j];
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

		m_pCellTable = new std::vector<std::vector<CCellData*>>(newWidth, std::vector<CCellData*>(newHeight, nullptr));
		if (!m_pCellTable) {
			return false;
		}

		for (auto i = 0; i < newWidth; i++) {
			for (auto j = 0; j < newHeight; j++) {
				int idx = (newWidth * j + i) * 3;
				uint8_t redValue = m_topLayerImage->input_image_data[idx + 0];
				uint8_t greenValue = m_topLayerImage->input_image_data[idx + 0];
				uint8_t blueValue = m_topLayerImage->input_image_data[idx + 0];
				CCellData* cell = new CCellData(redValue, greenValue, blueValue);
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

bool CPlantsSimulation::LoadInputHeightMap()
{
	if (!m_topLayerImage || !m_pCellTable)
	{
		return false;
	}
	const int width = m_topLayerImage->input_image_width;
	const int height = m_topLayerImage->input_image_height;
	std::vector<std::vector<short>> heightMap4096 = Read2DShortArray(m_heightMapFile, width, height);
	std::vector<std::vector<short>> slope4096 = ComputeSlopeMap(heightMap4096);

	for (auto i = 0; i < width; i++) {
		for (auto j = 0; j < height; j++) {
			CCellData* cell = (*m_pCellTable)[i][j];
			if (!cell)
			{
				std::cout << "Fail to get CCellData at i = " << i << " j = " << j << std::endl;
			}
			else
			{
				cell->m_height = heightMap4096[i][j];
				cell->m_slope = slope4096[i][j];
			}
		}
	}
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

	if (!m_topLayerImage) {
		return false;
	}

	const int forestXSize = m_topLayerImage->input_image_width;
	const int forestZSize = m_topLayerImage->input_image_width;
	//const int forestXSize = 4096;
	//const int forestZSize = 4096;

	m_pForest->xSize = forestXSize;
	m_pForest->zSize = forestZSize;
	m_pForest->xo = 0;
	m_pForest->zo = 0;

	cout << "Forest xSize is : " << forestXSize << endl;
	cout << "Forest zSize is : " << forestZSize << endl;

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
	bool outputCSV = m_pForest->outputResults(m_outputFile);
	return outputCSV;
}
