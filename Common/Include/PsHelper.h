#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>
#include <random>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

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

struct InputImageMetaInfo
{
	double xRatio;
	double x0;
	double y0;
	double yRatio;
	double batch_min_x;
	double batch_min_y;
	InputImageMetaInfo()
		: xRatio(1)
		, x0(0)
		, y0(0)
		, yRatio(1)
		, batch_min_x(0)
		, batch_min_y(0)
	{

	}
	InputImageMetaInfo& operator= (const InputImageMetaInfo& other)
	{
		xRatio = other.xRatio;
		x0 = other.x0;
		y0 = other.y0;
		yRatio = other.yRatio;
		batch_min_x = other.batch_min_x;
		batch_min_y = other.batch_min_y;
		return *this;
	};

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
std::vector<std::vector<short>> ComputeAbsMaxHeightSlopeMap(const std::vector<std::vector<short>>& heightmap);
vector<vector<double>> ComputeAbsMaxSlopeAngle(const vector<vector<double>>& heightMap, double dx);
vector<vector<double>> ComputeAbsAverageNeighborSlopeAngle(const vector<vector<double>>& heightMap, double dx);

std::vector<std::vector<PixelRGB>>* ConvertRGBArrayTo2DVector(const unsigned char* rgbData, int width, int height);

std::string GetFileExtension(const std::string& filePath);

std::vector<std::vector<double>> ConvertUnsignedShortToDouble(const std::vector<std::vector<unsigned short>>& inputArray);
std::vector<std::vector<double>> ConvertShortMatrixToDouble1(const std::vector<std::vector<short>>& shortMatrix);
std::vector<std::vector<double>> ConvertShortMatrixToDouble2(const std::vector<std::vector<short>>& shortMatrix);

double GenerateRandomDouble(double min, double max);

template <typename T>
T FindMinIn3(const T& num1, const T& num2, const T& num3) {
	T min_num = num1;

	if (num2 < min_num) {
		min_num = num2;
	}
	if (num3 < min_num) {
		min_num = num3;
	}
	return min_num;
}

template <typename T>
T FindMaxIn3(const T& num1, const T& num2, const T& num3) {
	T max_num = num1;

	if (num2 > max_num) {
		max_num = num2;
	}
	if (num3 > max_num) {
		max_num = num3;
	}
	return max_num;
}

template <typename T>
T FindMinIn4(T num1, T num2, T num3, T num4) {
	T min_num = num1; // Assume num1 is the minimum

	if (num2 < min_num) {
		min_num = num2;
	}

	if (num3 < min_num) {
		min_num = num3;
	}

	if (num4 < min_num) {
		min_num = num4;
	}

	return min_num;
}

template <typename T>
T FindMaxIn4(T num1, T num2, T num3, T num4) {
	T max_num = num1; // Assume num1 is the maximum

	if (num2 > max_num) {
		max_num = num2;
	}

	if (num3 > max_num) {
		max_num = num3;
	}

	if (num4 > max_num) {
		max_num = num4;
	}

	return max_num;
}