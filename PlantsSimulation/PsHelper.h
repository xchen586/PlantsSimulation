#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

struct InputImageDataInfo
{
	int input_image_width;
	int input_image_height;
	int input_image_comp;
	unsigned char* input_image_data;
	InputImageDataInfo()
		: input_image_height(0)
		, input_image_width(0)
		, input_image_comp(0)
		, input_image_data(nullptr)
	{

	}
	~InputImageDataInfo() 
	{
		if (input_image_data) {
			delete input_image_data;
		}
	}
};

struct PixelRGB {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

InputImageDataInfo* LoadInputImageFile(const string& inputImageFile);

std::vector<std::vector<short>> Read2DShortArray(const std::string& filePath, int width, int height);

double BilinearInterpolation(double x, double y, const std::vector<std::vector<short>>& inputArray);
double BilinearInterpolation2(double x, double y, const std::vector<std::vector<short>>& inputArray);
double BilinearInterpolation3(double x, double y, const std::vector<std::vector<short>>& inputArray);

double GetNormalLinearAttribute(double value, double min, double max);
double GetInvertLinearAttribute(double value, double min, double max);
double GetColorLinearNormallizedAttribute(short value);

std::vector<std::vector<short>> ScaleArray(const std::vector<std::vector<short>>& inputArray, int p, int q);

std::vector<std::vector<short>> ComputeSlopeMap(const std::vector<std::vector<short>>& heightmap);

std::vector<std::vector<PixelRGB>>* ConvertRGBArrayTo2DVector(const unsigned char* rgbData, int width, int height);

std::string GetFileExtension(const std::string& filePath);

