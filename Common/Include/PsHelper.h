#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <limits>
#include <random>

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>

#include <cstdlib>  // Contains strtod function
#include <cctype>   // Contains isspace function

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "Utils.h"
#include "XcDualLogging.h"

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

struct CavesPointInfo
{
	double x;
	double y;
	double z;
	int generation;
	int index;
	int order;

	CavesPointInfo()
		: x(0.0)
		, y(0.0)
		, z(0.0)
		, generation(0)
		, index(0)
		, order(0)
	{

	}
	CavesPointInfo& operator= (const CavesPointInfo& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		generation = other.generation;
		index = other.index;
		order = other.order;
		return *this;
	};
};

struct PixelRGB {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

struct Point {
	double x, y;
	Point()
		: x(0.0)
		, y(0.0)
	{

	}
	Point(double x_, double y_)
	{
		x = x_;
		y = y_;
	}
};

double DistanceSquared(const Point& a, const Point& b);
double PointToSegmentDistance(const Point& p, const Point& a, const Point& b, double distanceLimit = 0.0);
double PointToPolylineDistance(const Point& p, const std::vector<Point>& polyline, double distanceLimit = 0.0);
double GetDistanceToCaveNodes(const Point& p, const std::vector<std::pair<std::vector<Point>, int>>* lines, double distanceLimit = 0.0);
double GetDistancesToPOIs(const Point& p, const std::vector<Point>* poisLocations, double distanceLimit = 0.0);
std::vector<std::pair<std::vector<Point>, int>>* ConvertCaveInfoToCaveNodes(const std::vector<CavesPointInfo>& points);

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

std::vector<double> generateGaussianKernel(int radius, double sigma);
std::vector<std::vector<short>> NormalGaussianBlurHeightmap(
	const std::vector<std::vector<short>>& heightmap,
	int radius = 8,      // Default: 5 pixels for 4096x4096
	double sigma = 3.0   // Default: moderate smoothing
);
std::vector<std::vector<short>> IIRGaussianBlurHeightmap(
	const std::vector<std::vector<short>>& heightmap,
	int radius = 0,      // Unused in IIR version (kept for interface compatibility)
	double sigma = 5.0   // Default: larger smoothing for efficiency
);

double GenerateRandomDouble(double min, double max);

bool stringToBool(const std::string& str);
bool safe_strtod(const char* str, double& result);

std::vector<std::vector<short>> resampl2DShortMask(const std::vector<std::vector<short>>& originalMask, int newWidth, int newHeight);
std::vector<std::vector<unsigned char>> resample2DShortMaskToByte(const std::vector<std::vector<short>>& originalMask, int newWidth, int newHeight);

std::vector<std::vector<short>> resample2DShortWithAverage(const std::vector<std::vector<short>>& original, int new_rows, int new_cols);
std::vector<std::vector<unsigned char>> resample2DUCharWithAverage(const std::vector<std::vector<unsigned char>>& original, int new_rows, int new_cols);

std::string trim(const std::string& str);
int countColumnsInCSV(const std::string& filePath, char delimiter/* = ','*/);

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

class ThreadPool {
private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queueMutex;
	std::condition_variable condition;
	bool stop = false;

public:
	ThreadPool(size_t threads = std::thread::hardware_concurrency()) {
		for (size_t i = 0; i < threads; ++i) {
			workers.emplace_back([this] {
				for (;;) {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(this->queueMutex);
						this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
						if (this->stop && this->tasks.empty()) return;
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}
					task();
				}
				});
		}
	}

	template<class F>
	auto enqueue(F&& f) -> std::future<typename std::invoke_result<F()>::type> {
		using return_type = typename std::invoke_result<F>::type;
		auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
			tasks.emplace([task] { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread& worker : workers) worker.join();
	}
};

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

template <typename T>
bool Write2DArrayAsCSV(const std::string& filePath, const std::vector<std::vector<T>>& array) {
	bool ret = false;

	std::cout << "Start to exported 2D arrary to CSV file " << filePath << std::endl;

	std::ofstream outputCSVFile(filePath, std::ios::trunc);

	if (outputCSVFile.is_open()) {
		// Write header
		outputCSVFile << "X,Y,Value\n";

		// Write data rows (skipping zeros)
		for (size_t x = 0; x < array.size(); ++x) {
			const auto& row = array[x];
			for (size_t y = 0; y < row.size(); ++y) {
				const T& value = row[y];

				// Skip if value is zero
				if (value == static_cast<T>(0)) {
					continue;
				}

				// Write non-zero entries as X,Y,Value
				outputCSVFile << x << "," << y << "," << value << "\n";
			}
		}

		outputCSVFile.close();
		std::cout << "Start to exported 2D arrary to  CSV file " << filePath << " successfully." << std::endl;
		ret = true;
	}
	else {
		std::cerr << "Unable to open CSV file "<< filePath <<" for writing." << std::endl;
		ret = false;
	}

	return ret;
}

template <typename SourceType, typename TargetType>
std::vector<std::vector<TargetType>> Read2DArrayAsType(const std::string& filePath, int width, int height, bool transposed = true)
{
	static_assert(std::is_trivially_copyable<SourceType>::value, "SourceType must be trivially copyable");
	static_assert(std::is_trivially_copyable<TargetType>::value, "TargetType must be trivially copyable");

	std::ifstream fs(filePath, std::ios::binary);
	if (!fs.is_open())
	{
		throw std::runtime_error("Failed to open file");
	}

	SourceType minVal = std::numeric_limits<SourceType>::max();
	SourceType maxVal = std::numeric_limits<SourceType>::min();

	// Check file size
	fs.seekg(0, std::ios::end);
	std::streampos fileSize = fs.tellg();
	if (fileSize != static_cast<std::streampos>(width) * height * sizeof(SourceType))
	{
		throw std::runtime_error("File size doesn't match expected dimensions");
	}
	fs.seekg(0, std::ios::beg);

	std::vector<std::vector<TargetType>> array(width, std::vector<TargetType>(height));

	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			SourceType value;
			fs.read(reinterpret_cast<char*>(&value), sizeof(SourceType));
			if (!fs)
			{
				throw std::runtime_error("Failed to read data from file");
			}

			minVal = std::min(minVal, value);
			maxVal = std::max(maxVal, value);

			if (transposed)
			{
				array[y][x] = static_cast<TargetType>(value);  // Invert axis
			}
			else
			{
				array[x][y] = static_cast<TargetType>(value);
			}
		}
	}

	std::cout << "Height Map min = " << static_cast<TargetType>(minVal)
		<< " , max = " << static_cast<TargetType>(maxVal) << std::endl;

	return array;
}

template<typename T>
std::vector<std::vector<T>> resample2DArrayWithAverage(
	const std::vector<std::vector<T>>& original,
	int new_rows,
	int new_cols) {

	// Static assertion to ensure T is an arithmetic type or std::byte
	static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, std::byte>,
		"T must be an arithmetic type or std::byte");

	// Input validation
	if (new_rows <= 0 || new_cols <= 0) {
		throw std::invalid_argument("New dimensions must be positive");
	}

	const int original_rows = static_cast<int>(original.size());
	if (original_rows == 0) {
		throw std::invalid_argument("Original grid has no rows");
	}

	const int original_cols = static_cast<int>(original[0].size());
	if (original_cols == 0) {
		throw std::invalid_argument("Original grid has no columns");
	}

	// Check if all rows have the same number of columns
	for (size_t i = 1; i < original.size(); ++i) {
		if (static_cast<int>(original[i].size()) != original_cols) {
			throw std::invalid_argument("All rows must have the same number of columns");
		}
	}

	// Initialize output grid
	std::vector<std::vector<T>> resampled(new_rows, std::vector<T>(new_cols));

	// Calculate scaling factors
	const double row_scale = static_cast<double>(original_rows) / new_rows;
	const double col_scale = static_cast<double>(original_cols) / new_cols;

	// Parallelization-friendly loop structure
	for (int i = 0; i < new_rows; ++i) {
		for (int j = 0; j < new_cols; ++j) {
			// Calculate bounds of current region in original grid
			const double row_start_f = i * row_scale;
			const double row_end_f = (i + 1) * row_scale;
			const double col_start_f = j * col_scale;
			const double col_end_f = (j + 1) * col_scale;

			// Convert to integer bounds
			const int row_start = static_cast<int>(std::floor(row_start_f));
			const int row_end = std::min(static_cast<int>(std::ceil(row_end_f)), original_rows);
			const int col_start = static_cast<int>(std::floor(col_start_f));
			const int col_end = std::min(static_cast<int>(std::ceil(col_end_f)), original_cols);

			// Use more precise accumulation type to avoid overflow
			if constexpr (std::is_same_v<T, std::byte>) {
				// Special handling for std::byte - convert to unsigned long long for arithmetic
				unsigned long long sum = 0;
				int count = 0;

				// Accumulate all values in the region
				for (int x = row_start; x < row_end; ++x) {
					for (int y = col_start; y < col_end; ++y) {
						sum += static_cast<unsigned long long>(static_cast<unsigned char>(original[x][y]));
						++count;
					}
				}

				// Calculate average and assign
				if (count > 0) {
					// Use rounding for byte values
					unsigned char avg = static_cast<unsigned char>((sum + count / 2) / count);
					resampled[i][j] = static_cast<std::byte>(avg);
				}
				else {
					// Should not happen theoretically, but for safety
					resampled[i][j] = std::byte{ 0 };
				}
			}
			else {
				// Original handling for arithmetic types
				using AccumType = std::conditional_t<std::is_floating_point_v<T>,
					long double,
					long long>;
				AccumType sum = 0;
				int count = 0;

				// Accumulate all values in the region
				for (int x = row_start; x < row_end; ++x) {
					for (int y = col_start; y < col_end; ++y) {
						sum += static_cast<AccumType>(original[x][y]);
						++count;
					}
				}

				// Calculate average and assign
				if (count > 0) {
					if constexpr (std::is_floating_point_v<T>) {
						resampled[i][j] = static_cast<T>(sum / count);
					}
					else {
						// Use rounding for integer types
						resampled[i][j] = static_cast<T>((sum + count / 2) / count);
					}
				}
				else {
					// Should not happen theoretically, but for safety
					resampled[i][j] = T{};
				}
			}
		}
	}

	return resampled;
}

// Nearest neighbor resampling implementation
template<typename T>
std::vector<std::vector<T>> resample2DArrayWithNearestNeighbor(
	const std::vector<std::vector<T>>& original,
	int new_rows,
	int new_cols) {

	static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, std::byte>,
		"T must be an arithmetic type or std::byte");

	if (new_rows <= 0 || new_cols <= 0) {
		throw std::invalid_argument("New dimensions must be positive");
	}

	const int original_rows = static_cast<int>(original.size());
	const int original_cols = static_cast<int>(original[0].size());

	if (original_rows == 0 || original_cols == 0) {
		throw std::invalid_argument("Original grid cannot be empty");
	}

	std::vector<std::vector<T>> resampled(new_rows, std::vector<T>(new_cols));

	const double row_scale = static_cast<double>(original_rows) / new_rows;
	const double col_scale = static_cast<double>(original_cols) / new_cols;

	for (int i = 0; i < new_rows; ++i) {
		for (int j = 0; j < new_cols; ++j) {
			const int orig_row = std::min(static_cast<int>(i * row_scale + 0.5), original_rows - 1);
			const int orig_col = std::min(static_cast<int>(j * col_scale + 0.5), original_cols - 1);
			resampled[i][j] = original[orig_row][orig_col];
		}
	}

	return resampled;
}

// Bilinear interpolation resampling implementation (for floating-point types only)
template<typename T>
std::vector<std::vector<T>> resample2DArrayWithBilinear(
	const std::vector<std::vector<T>>& original,
	int new_rows,
	int new_cols) {

	static_assert(std::is_floating_point_v<T>,
		"Bilinear interpolation requires floating-point type");

	if (new_rows <= 0 || new_cols <= 0) {
		throw std::invalid_argument("New dimensions must be positive");
	}

	const int original_rows = static_cast<int>(original.size());
	const int original_cols = static_cast<int>(original[0].size());

	if (original_rows == 0 || original_cols == 0) {
		throw std::invalid_argument("Original grid cannot be empty");
	}

	std::vector<std::vector<T>> resampled(new_rows, std::vector<T>(new_cols));

	const double row_scale = static_cast<double>(original_rows - 1) / (new_rows - 1);
	const double col_scale = static_cast<double>(original_cols - 1) / (new_cols - 1);

	for (int i = 0; i < new_rows; ++i) {
		for (int j = 0; j < new_cols; ++j) {
			const double orig_row = i * row_scale;
			const double orig_col = j * col_scale;

			const int row0 = static_cast<int>(std::floor(orig_row));
			const int row1 = std::min(row0 + 1, original_rows - 1);
			const int col0 = static_cast<int>(std::floor(orig_col));
			const int col1 = std::min(col0 + 1, original_cols - 1);

			const double row_weight = orig_row - row0;
			const double col_weight = orig_col - col0;

			const T val00 = original[row0][col0];
			const T val01 = original[row0][col1];
			const T val10 = original[row1][col0];
			const T val11 = original[row1][col1];

			const T interpolated = static_cast<T>(
				val00 * (1 - row_weight) * (1 - col_weight) +
				val01 * (1 - row_weight) * col_weight +
				val10 * row_weight * (1 - col_weight) +
				val11 * row_weight * col_weight
				);

			resampled[i][j] = interpolated;
		}
	}

	return resampled;
}

// Convenience function: supports different resampling strategies
enum class ResampleMethod {
	Average,
	NearestNeighbor,
	Bilinear
};

template<typename T>
std::vector<std::vector<T>> resample2DArray(
	const std::vector<std::vector<T>>& original,
	int new_rows,
	int new_cols,
	ResampleMethod method = ResampleMethod::Average) {

	switch (method) {
	case ResampleMethod::Average:
		return resample2DArrayWithAverage(original, new_rows, new_cols);
	case ResampleMethod::NearestNeighbor:
		return resample2DArrayWithNearestNeighbor(original, new_rows, new_cols);
	case ResampleMethod::Bilinear:
		// Add compile-time check for bilinear interpolation
		if constexpr (std::is_floating_point_v<T>) {
			return resample2DArrayWithBilinear(original, new_rows, new_cols);
		}
		else {
			throw std::invalid_argument("Bilinear interpolation requires floating-point type");
		}
	default:
		throw std::invalid_argument("Unsupported resampling method");
	}
}
