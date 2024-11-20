#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>
#include <random>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "Utils.h"

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

VoxelFarm::CellId GetVFCellIDFromVF(double vfX, double vfY, double vfZ, const CAffineTransform& transform, int32_t lod);
VoxelFarm::CellId GetVFCellIDFromWorld(double posX, double posY, double posZ, const CAffineTransform& transform, int32_t lod);

InputImageDataInfo* LoadInputImageFile(const string& inputImageFile);

std::vector<std::vector<short>> Read2DShortArray(const std::string& filePath, int width, int height, bool invert = true);
bool Write2DShortArrayAsRaw(const std::string& filePath, std::vector<std::vector<short>> array);

std::vector<std::vector<int>> Read2DIntArray(const std::string& filePath, int width, int height, bool invert = false);

double BilinearInterpolation(double x, double y, const std::vector<std::vector<short>>& inputArray);
double BilinearInterpolation2(double x, double y, const std::vector<std::vector<short>>& inputArray);
//double BilinearInterpolation3(double x, double y, const std::vector<std::vector<short>>& inputArray);

//std::vector<std::vector<short>> ScaleArray(const std::vector<std::vector<short>>& inputArray, int p, int q);

double GetNormalLinearAttribute(double value, double min, double max);
double GetInvertLinearAttribute(double value, double min, double max);
double GetColorLinearNormallizedAttribute(short value);
double GetValueFromNormalized(double normalized, double min, double max);
double GetValueFromInvertedNormalized(double invertedNormalized, double min, double max);



std::vector<std::vector<short>> ComputeSlopeMap(const std::vector<std::vector<short>>& heightmap);
std::vector<std::vector<short>> ComputeAbsMaxHeightSlopeMap(const std::vector<std::vector<short>>& heightmap);
vector<vector<double>> ComputeAbsMaxSlopeAngle(const vector<vector<double>>& heightMap, double dx);
vector<vector<double>> ComputeAbsAverageNeighborSlopeAngle(const vector<vector<double>>& heightMap, double dx);

std::vector<std::vector<PixelRGB>>* ConvertRGBArrayTo2DVector(const unsigned char* rgbData, int width, int height);

std::string GetFileExtension(const std::string& filePath);

std::vector<std::vector<double>> ConvertUnsignedShortToDouble(const std::vector<std::vector<unsigned short>>& inputArray);
std::vector<std::vector<double>> ConvertShortMatrixToDouble1(const std::vector<std::vector<short>>& shortMatrix);
std::vector<std::vector<double>> ConvertShortMatrixToDouble2(const std::vector<std::vector<short>>& shortMatrix);
std::vector<std::vector<unsigned short>> ConvertShortMatrixToUShort(const std::vector<std::vector<short>>& shortMatrix);

double GenerateRandomDouble(double min, double max);

bool stringToBool(const std::string& str);

std::vector<std::vector<short>> resample2DShortWithAverage(const std::vector<std::vector<short>>& original, int new_rows, int new_cols);
std::vector<std::vector<unsigned char>> resample2DUCharWithAverage(const std::vector<std::vector<unsigned char>>& original, int new_rows, int new_cols);

template <typename T>
T GenerateRandomT(T min, T max) {
	// Initialize a random number generator
	std::random_device rd;  // Seed generator
	std::mt19937 gen(rd()); // Mersenne Twister engine
	std::uniform_real_distribution<T> distribution(min, max);

	// Generate a random double within the specified range and return it
	return distribution(gen);
}

bool RemoveAllFilesInFolder(const std::string& folderPath);
bool CheckExistFolderAndRemoveSubFiles(const std::string& outputSubsDir);

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

template <typename T>
T FindMaxIn5(T num1, T num2, T num3, T num4, T num5) {
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

	if (num5 > max_num) {
		max_num = num5;
	}

	return max_num;
}

template <typename T>
bool Output2DVectorToRawFile(const std::vector<std::vector<T>>& data, const std::string& filePath) {
	
	std::cout << "start to Output2DVectorToRawFile to : " << filePath << std::endl;
	
	std::ofstream outputFile(filePath, std::ios::binary);

	if (!outputFile.is_open()) {
		std::cerr << "Failed to open the file for writing: " << filePath << std::endl;
		return false;
	}

	try {
		for (const auto& row : data) {
			for (const T& value : row) {
				if (!outputFile.write(reinterpret_cast<const char*>(&value), sizeof(value))) {
					std::cerr << "Failed to write data to file: " << filePath << std::endl;
					return false;
				}
			}
		}
	}
	catch (const std::ofstream::failure& ex) {
		std::cerr << "File write error: " << ex.what() << std::endl;
		return false;
	}

	outputFile.close();

	std::cout << "end to Output2DVectorToRawFile to : " << filePath << std::endl;
	return true;
}

template <typename T>
T BilinearInterpolation3(double x, double y, const std::vector<std::vector<T>>& inputArray)
{
	int x0 = static_cast<int>(x);
	int y0 = static_cast<int>(y);
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	double dx = x - x0;
	double dy = y - y0;

	if (x0 >= 0 && x1 < inputArray.size() && y0 >= 0 && y1 < inputArray[0].size())
	{
		double f00 = static_cast<double>(inputArray[x0][y0]);
		double f01 = static_cast<double>(inputArray[x0][y1]);
		double f10 = static_cast<double>(inputArray[x1][y0]);
		double f11 = static_cast<double>(inputArray[x1][y1]);

		double interpolatedDouble = ((1.0 - dx) * (1.0 - dy) * f00 +
			dx * (1.0 - dy) * f10 +
			(1.0 - dx) * dy * f01 +
			dx * dy * f11);
		T interpolatedValue = static_cast<T>(interpolatedDouble);

		return interpolatedValue;
	}

	return static_cast<T>(0); // Handle out-of-bounds cases gracefully
}

template <typename T>
std::vector<std::vector<T>> ScaleArray(const std::vector<std::vector<T>>& inputArray, int p, int q)
{
	int rows = inputArray.size();
	int cols = inputArray[0].size();

	std::vector<std::vector<T>> scaledArray(p, std::vector<T>(q));

	double scaleX = static_cast<double>(rows) / p;
	double scaleY = static_cast<double>(cols) / q;

	for (int x = 0; x < p; x++)
	{
		for (int y = 0; y < q; y++)
		{
			double originalX = x * scaleX;
			double originalY = y * scaleY;

			scaledArray[x][y] = BilinearInterpolation3<T>(originalX, originalY, inputArray);
		}
	}

	return scaledArray;
}

// Generic function to find the minimum value in an N*N block of any numeric type
template <typename T>
T findMinInBlock(const std::vector<std::vector<T>>& input, int x_start, int y_start, int block_width, int block_height, int orig_width, int orig_height) {
	T min_value = std::numeric_limits<T>::max();  // Initialize with the maximum possible value for type T
	for (int y = y_start; y < std::min(y_start + block_height, orig_height); ++y) {
		for (int x = x_start; x < std::min(x_start + block_width, orig_width); ++x) {
			min_value = std::min(min_value, input[y][x]);
		}
	}
	return min_value;
}

// Generic function to find the minimum value in an N*N block of any numeric type
template <typename T>
T findMinInBlock(const std::vector<std::vector<T>>& input, int x_start, int y_start, int block_width, int block_height) {
	T min_value = std::numeric_limits<T>::max();  // Initialize with the maximum possible value for type T
	int orig_height = input.size();
	int orig_width = input[0].size();

	for (int y = y_start; y < std::min(y_start + block_height, orig_height); ++y) {
		for (int x = x_start; x < std::min(x_start + block_width, orig_width); ++x) {
			min_value = std::min(min_value, input[y][x]);
		}
	}
	return min_value;
}

// Generic function to find the maximum value in an N*N block of any numeric type
template <typename T>
T findMaxInBlock(const std::vector<std::vector<T>>& input, int x_start, int y_start, int block_width, int block_height) {
	T max_value = std::numeric_limits<T>::min();  // Initialize with the minimum possible value for type T
	int orig_height = input.size();
	int orig_width = input[0].size();

	for (int y = y_start; y < std::min(y_start + block_height, orig_height); ++y) {
		for (int x = x_start; x < std::min(x_start + block_width, orig_width); ++x) {
			max_value = std::max(max_value, input[y][x]);
		}
	}
	return max_value;
}

// Generic function to find the average value in an N*N block of any numeric type
template <typename T>
T findAverageInBlock(const std::vector<std::vector<T>>& input, int x_start, int y_start, int block_width, int block_height) {
	T sum = 0;
	int count = 0;
	int orig_height = input.size();
	int orig_width = input[0].size();

	for (int y = y_start; y < std::min(y_start + block_height, orig_height); ++y) {
		for (int x = x_start; x < std::min(x_start + block_width, orig_width); ++x) {
			sum += input[y][x];
			count++;
		}
	}
	return (count > 0) ? sum / count : 0;  // Avoid division by zero
}

// Template function to resample the 2D vector using the specified aggregation function (min, max, or average)
template <typename T, typename Func>
std::vector<std::vector<T>> resample2DArrayByFunc(const std::vector<std::vector<T>>& input, int new_width, int new_height, Func aggregationFunc) {
	int orig_height = input.size();
	int orig_width = input[0].size();

	std::vector<std::vector<T>> output(new_height, std::vector<T>(new_width));

	// Calculate scaling factors
	float x_scale = static_cast<float>(orig_width) / new_width;
	float y_scale = static_cast<float>(orig_height) / new_height;

	// Loop through each output pixel
	for (int j = 0; j < new_height; ++j) {
		for (int i = 0; i < new_width; ++i) {
			// Determine the block of the high-resolution array that maps to the current output pixel
			int x_start = static_cast<int>(i * x_scale);
			int y_start = static_cast<int>(j * y_scale);
			int block_width = static_cast<int>(std::ceil(x_scale));
			int block_height = static_cast<int>(std::ceil(y_scale));

			// Apply the specified aggregation function (min, max, or average)
			output[j][i] = aggregationFunc(input, x_start, y_start, block_width, block_height);
		}
	}

	return output;
}

// Template function to return an inverted 2D vector
template <typename T>
std::vector<std::vector<T>> invert2DArray(const std::vector<std::vector<T>>& matrix) {
	if (matrix.empty() || matrix[0].empty()) return {}; // Handle empty matrix

	int numRows = matrix.size();
	int numCols = matrix[0].size();

	// Create a new 2D vector with swapped dimensions
	std::vector<std::vector<T>> inverted(numCols, std::vector<T>(numRows));

	for (int i = 0; i < numRows; ++i) {
		for (int j = 0; j < numCols; ++j) {
			inverted[j][i] = matrix[i][j];
		}
	}

	return inverted;
}

template <typename T>
bool Write2DArrayAsRaw(const std::string& filePath, const std::vector<std::vector<T>>& array) {
	bool ret = false;
	size_t width = array.size();
	size_t height = array[0].size();

	std::ofstream outputArrayFile(filePath, std::ios::binary | std::ios::trunc);
	size_t arraySize = width * height * sizeof(T);  // Calculate size based on the generic type T

	if (outputArrayFile.is_open()) {
		for (const auto& row : array) {
			for (T value : row) {
				outputArrayFile.write(reinterpret_cast<const char*>(&value), sizeof(T));  // Write values of type T
			}
		}
		outputArrayFile.close();
		std::cout << "Array of type " << typeid(T).name() << " saved to array file successfully." << std::endl;
		ret = true;
	}
	else {
		std::cerr << "Unable to open array file for saving." << std::endl;
		ret = false;
	}

	return ret;
}