#include "CPlantsSimulation.h"
#include "CCellData.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool CPlantsSimulation::loadInputImageFile(const string& inputImageFile)
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
}