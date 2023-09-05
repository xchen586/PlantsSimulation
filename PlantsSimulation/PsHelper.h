#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

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


InputImageDataInfo* LoadInputImageFile(const string& inputImageFile);

std::vector<std::vector<unsigned short>> Read2DShortArray(const std::string& filePath, int width, int height);

double BilinearInterpolation(double x, double y, const std::vector<std::vector<unsigned short>>& inputArray);
double BilinearInterpolation2(double x, double y, const std::vector<std::vector<unsigned short>>& inputArray);
double BilinearInterpolation3(double x, double y, const std::vector<std::vector<unsigned short>>& inputArray);

std::vector<std::vector<unsigned short>> ScaleArray(const std::vector<std::vector<unsigned short>>& inputArray, int p, int q);

std::vector<std::vector<unsigned short>> ComputeSlopeMap(const std::vector<std::vector<unsigned short>>& heightmap);