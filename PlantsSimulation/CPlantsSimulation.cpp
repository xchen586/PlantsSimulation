#include "CPlantsSimulation.h"
#include "CCellData.h"


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

bool CPlantsSimulation::LoadInputData()
{
	bool ret = false;
	this->DeInitialize();

	const int width = 300;
	const int height = 300;
	const int newWidth = 4096;
	const int newHeight = 4096;

	try
	{
		std::vector<std::vector<unsigned short>> heightMap300 = Read2DShortArray(m_heightMapFile, width, height);

		std::vector<std::vector<unsigned short>> heightMap4096 = ScaleArray(heightMap300, 4096, 4096);
		std::vector<std::vector<unsigned short>> slope4096 = ComputeSlopeMap(heightMap4096);

		m_pCellTable = new std::vector<std::vector<CCellData*>>(newWidth, std::vector<CCellData*>(newHeight, nullptr));
		if (!m_pCellTable) {
			return false;
		}

		m_topLayerImage = LoadInputImageFile(m_inputImageFile);
		if (!m_topLayerImage)
		{
			return false;
		}

		for (auto i = 0; i < width; i++) {
			for (auto j = 0; j < height; j++) {
				int idx = (width * j + i) * 3;
				uint8_t redValue = m_topLayerImage->input_image_data[idx + 0];
				uint8_t greenValue = m_topLayerImage->input_image_data[idx + 0];
				uint8_t blueValue = m_topLayerImage->input_image_data[idx + 0];
				CCellData* cell = new CCellData(redValue, greenValue, blueValue);
				cell->m_height = heightMap4096[i][j];
				cell->m_slope = slope4096[i][j];
				(*m_pCellTable)[i][j] = cell;
			}
		}
		// You can access and work with 'result' here
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
}

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
	m_pForest->xSize = m_topLayerImage->input_image_width;
	m_pForest->zSize = m_topLayerImage->input_image_height;

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

	float forestAge = 300;
	int iteration = 40;
	m_pForest->generate(forestAge, iteration);

	return true;
};