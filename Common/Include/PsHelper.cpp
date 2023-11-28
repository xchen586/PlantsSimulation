#include "PsHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

InputImageDataInfo* LoadInputImageFile(const string& inputImageFile)
{
	InputImageDataInfo* pDataInfo = new InputImageDataInfo();
	pDataInfo->input_image_data = stbi_load(inputImageFile.c_str(), &pDataInfo->input_image_width, &pDataInfo->input_image_height, &pDataInfo->input_image_comp, 3);
	if (!pDataInfo->input_image_data) {
		delete pDataInfo;
		return nullptr;
	}
	else {
		return pDataInfo;
	}
}

std::vector<std::vector<short>> Read2DShortArray(const std::string& filePath, int width, int height)
{
    std::ifstream fs(filePath, std::ios::binary);
    if (!fs.is_open())
    {
        throw std::runtime_error("Failed to open file");
    }

    short minHeight = std::numeric_limits<short>::max();
    short maxHeight = std::numeric_limits<short>::min();
    // Check if the file size matches the expected size
    fs.seekg(0, std::ios::end);
    std::streampos fileSize = fs.tellg();
    if (fileSize != static_cast<std::streampos>(width) * height * sizeof(short))
    {
        throw std::runtime_error("File size doesn't match expected dimensions");
    }
    fs.seekg(0, std::ios::beg);

    std::vector<std::vector<short>> array(width, std::vector<short>(height));

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            unsigned char bytes[2];
            fs.read(reinterpret_cast<char*>(bytes), 2);
            if (!fs)
            {
                throw std::runtime_error("Failed to read data from file");
            }
            short value = (static_cast<unsigned char>(bytes[1]) << 8) | static_cast<unsigned char>(bytes[0]);
            //short value = (static_cast<unsigned short>(bytes[1]) << 8) | bytes[0];
            minHeight = std::min(minHeight, value);
            maxHeight = std::max(maxHeight, value);
            array[y][x] = value;  //mirror issue !!!!!!!
        }
    }
    std::cout << "Short Height Map minHeight = " << minHeight << " , maxHeight = " << maxHeight << std::endl;
    return array;
}

bool Write2DShortArrayAsRaw(const std::string& filePath, std::vector<std::vector<short>> array)
{
    bool ret = false;
    int width = array.size();
    int height = array[0].size();
        
    std::ofstream outputArrayFile(filePath, std::ios::binary | std::ios::trunc);
    int arraySize = width * height * sizeof(short);
    if (outputArrayFile.is_open()) {
        for (const auto& row : array) {
            for (short value : row) {
                outputArrayFile.write(reinterpret_cast<char*>(&value), sizeof(short));
            }
        }
        outputArrayFile.close();
        std::cout << "Array of short save to array file successfully." << std::endl;
        ret = true;
    }
    else {
        std::cerr << "Unable to open array file for saving." << std::endl;
        ret = false;
    }
    return ret;
}

double BilinearInterpolation(double x, double y, const std::vector<std::vector<short>>& inputArray)
{
    int width = inputArray.size();
    int height = inputArray[0].size();

    // Check boundaries
    if (x < 0 || x >= width - 1 || y < 0 || y >= height - 1)
    {
        // Return the default value when coordinates are out of bounds
        return 0.0;
    }

    int x0 = static_cast<int>(x);
    int y0 = static_cast<int>(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    double dx = x - x0;
    double dy = y - y0;

    double value = (1 - dx) * (1 - dy) * inputArray[x0][y0] +
        dx * (1 - dy) * inputArray[x1][y0] +
        (1 - dx) * dy * inputArray[x0][y1] +
        dx * dy * inputArray[x1][y1];

    return value;
}

double BilinearInterpolation2(double x, double y, const std::vector<std::vector<short>>& inputArray)
{
    int width = inputArray.size();
    int height = inputArray[0].size();

    // Check boundaries
    if (x < 0 || x >= width - 1 || y < 0 || y >= height - 1)
    {
        // Return the default value when coordinates are out of bounds
        return 0.0;
    }

    int x0 = static_cast<int>(x);
    int y0 = static_cast<int>(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    double dx = x - x0;
    double dy = y - y0;
    double dx1 = 1.0 - dx;
    double dy1 = 1.0 - dy;

    double result =
        inputArray[x0][y0] * (dx1 * dy1) +
        inputArray[x1][y0] * (dx * dy1) +
        inputArray[x0][y1] * (dx1 * dy) +
        inputArray[x1][y1] * (dx * dy);

    return result;
}

double BilinearInterpolation3(double x, double y, const std::vector<std::vector<short>>& inputArray)
{
    int x0 = static_cast<int>(x);
    int y0 = static_cast<int>(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    double dx = x - x0;
    double dy = y - y0;

    if (x0 >= 0 && x1 < inputArray.size() && y0 >= 0 && y1 < inputArray[0].size())
    {
        double f00 = inputArray[x0][y0];
        double f01 = inputArray[x0][y1];
        double f10 = inputArray[x1][y0];
        double f11 = inputArray[x1][y1];

        double interpolatedValue = (1.0 - dx) * (1.0 - dy) * f00 +
            dx * (1.0 - dy) * f10 +
            (1.0 - dx) * dy * f01 +
            dx * dy * f11;

        return interpolatedValue;
    }

    return 0.0; // Handle out-of-bounds cases gracefully
}

double GetNormalLinearAttribute(double value, double min, double max)
{
    value = std::clamp(value, min, max);
    double normalized = (value - min) / (max - min);
    return std::lerp(0.0, 1.0, normalized);
}

double GetInvertLinearAttribute(double value, double min, double max)
{
    value = std::clamp(value, min, max);
    double normalized = (value - min) / (max - min);
    return std::lerp(0.0, 1.0, normalized);
}

double GetColorLinearNormallizedAttribute(short value)
{
    double doublevalue = static_cast<double>(value);
    return GetNormalLinearAttribute(doublevalue, 0.0, 255.0);
}

std::vector<std::vector<short>> ScaleArray(const std::vector<std::vector<short>>& inputArray, int p, int q)
{
    int rows = inputArray.size();
    int cols = inputArray[0].size();

    std::vector<std::vector<short>> scaledArray(p, std::vector<short>(q));

    double scaleX = static_cast<double>(rows) / p;
    double scaleY = static_cast<double>(cols) / q;

    for (int x = 0; x < p; x++)
    {
        for (int y = 0; y < q; y++)
        {
            double originalX = x * scaleX;
            double originalY = y * scaleY;

            scaledArray[x][y] = static_cast<short>(BilinearInterpolation3(originalX, originalY, inputArray));
        }
    }

    return scaledArray;
}

std::vector<std::vector<double>> ConvertUnsignedShortToDouble(const std::vector<std::vector<unsigned short>>& inputArray)
{
    int m = inputArray.size();
    int n = inputArray[0].size();

    std::vector<std::vector<double>> doubleArray(m, std::vector<double>(n));

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            doubleArray[i][j] = static_cast<double>(inputArray[i][j]);
        }
    }

    return doubleArray;
}

// Function to convert a 2D vector of shorts to a 2D vector of doubles
std::vector<std::vector<double>> ConvertShortMatrixToDouble1(const std::vector<std::vector<short>>& shortMatrix) {
    std::vector<std::vector<double>> doubleMatrix(shortMatrix.size(), std::vector<double>(shortMatrix[0].size()));

    for (size_t i = 0; i < shortMatrix.size(); ++i) {
        for (size_t j = 0; j < shortMatrix[i].size(); ++j) {
            doubleMatrix[i][j] = static_cast<double>(shortMatrix[i][j]);
        }
    }

    return doubleMatrix;
}

// Function to convert a 2D vector of shorts to a 2D vector of doubles
std::vector<std::vector<double>> ConvertShortMatrixToDouble2(const std::vector<std::vector<short>>& shortMatrix) {
    std::vector<std::vector<double>> doubleMatrix;

    for (const auto& shortRow : shortMatrix) {
        std::vector<double> doubleRow;
        for (const short& value : shortRow) {
            doubleRow.push_back(static_cast<double>(value));
        }
        doubleMatrix.push_back(doubleRow);
    }

    return doubleMatrix;
}

std::vector<std::vector<unsigned short>> ConvertShortMatrixToUShort(const std::vector<std::vector<short>>& shortMatrix) {
    std::vector<std::vector<unsigned short>> ushortMatrix(shortMatrix.size(), std::vector<unsigned short>(shortMatrix[0].size()));

    for (size_t i = 0; i < shortMatrix.size(); ++i) {
        for (size_t j = 0; j < shortMatrix[i].size(); ++j) {
            if (shortMatrix[i][j] < 0)
            {
                ushortMatrix[i][j] = 0;
            }
            else
            {
                ushortMatrix[i][j] = static_cast<unsigned short>(shortMatrix[i][j]);
            }
        }
    }

    return ushortMatrix;
}

std::vector<std::vector<short>> ComputeSlopeMap(const std::vector<std::vector<short>>& heightmap)
{
    int width = heightmap.size();
    int height = heightmap[0].size();

    short minSlopeHeight = numeric_limits<short>::max();
    short maxSlopeHeight = numeric_limits<short>::min();

    std::vector<std::vector<short>> slopeMap(width, std::vector<short>(height, 0));

    int dx[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    int dy[] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            short maxHeightDifference = numeric_limits<short>::min();
            for (int i = 0; i < 8; i++)
            {
                int newX = x + dx[i];
                int newY = y + dy[i];

                if (newX >= 0 && newX < width && newY >= 0 && newY < height)
                {
                    short heightDifference = static_cast<short>(std::abs(heightmap[x][y] - heightmap[newX][newY]));
                    if (heightDifference != 0) {
                        //std::cout << "heightDifference is : " << heightDifference << ", old is : " << heightmap[x][y] << ", new is : " << heightmap[newX][newY] << std::endl;
                    }
                    if (heightDifference > maxHeightDifference)
                    {
                        maxHeightDifference = static_cast<unsigned short>(heightDifference);;   
                    }
                }
            }
            short value = std::min(maxHeightDifference, static_cast<short>(SHRT_MAX));
            //short value = maxHeightDifference;
            slopeMap[x][y] = value;
            minSlopeHeight = std::min(minSlopeHeight, value);
            maxSlopeHeight = std::max(maxSlopeHeight, value);
        }
    }
    std::cout << "ComputeSlopeMap minSlopeHeight is : " << minSlopeHeight << ", maxSlopeHeight is : " << maxSlopeHeight << std::endl;
    return slopeMap;

}

std::vector<std::vector<short>> ComputeAbsMaxHeightSlopeMap(const std::vector<std::vector<short>>& heightmap)
{
    int width = heightmap.size();
    int height = heightmap[0].size();

    std::vector<std::vector<short>> slopeMap(width, std::vector<short>(height, 0));

    int dx[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    int dy[] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    short minSlope = SHRT_MAX;
    short maxSlope = SHRT_MIN;
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            short maxHeightDifference = 0;
            unsigned short absMaxHeightDifference = 0;

            for (int i = 0; i < 8; i++)
            {
                int newX = x + dx[i];
                int newY = y + dy[i];

                if (newX >= 0 && newX < width && newY >= 0 && newY < height)
                {
                    short heightDifference = static_cast<short>((heightmap[x][y] - heightmap[newX][newY]));
                    unsigned short absHeightDifference = std::abs(heightDifference);
                    if (heightDifference < 0)
                    {
                        //std::cout << "heightDifference abs is negative : " << heightDifference << endl;
                    }

                    if (absHeightDifference > absMaxHeightDifference)
                    {
                        maxHeightDifference = static_cast<short>(heightDifference);
                        absMaxHeightDifference = std::abs(maxHeightDifference);
                    }
                }
            }

            short value = std::min(static_cast<short>(absMaxHeightDifference), static_cast<short>(SHRT_MAX));
            slopeMap[x][y] = value;
            //std::cout << "slope value is : value : " << value << ", x : " << x << ", y : " << y << std::endl;
            if (value < 0)
            {
                //std::cout << "slope value is negative: value : " << value << ", x : " << x << ", y : " << y << std::endl;
            }
            minSlope = std::min(minSlope, value);
            maxSlope = std::max(maxSlope, value);
        }
    }
    std::cout << "Min Slope is : " << minSlope << " , Max Slope is : " << maxSlope << std::endl;
    return slopeMap;
}

vector<vector<double>> ComputeAbsMaxSlopeAngle(const vector<vector<double>>& heightMap, double dx)
{
    int m = heightMap.size();
    vector<vector<double>> slopeAngle(m, vector<double>(m, -1.0));  // Initialize with a default value.

    // Initialize min and max slope angles with extreme values.
    double minSlopeAngle = M_PI / 2.0;
    double maxSlopeAngle = 0;

    int dxdy[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    int dydx[] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            double max_slope_angle = 0;
            double min_slope_angle = M_PI / 2.0;;

            for (int k = 0; k < 8; k++) {
                int ni = i + dxdy[k];
                int nj = j + dydx[k];

                if (ni >= 0 && ni < m && nj >= 0 && nj < m) {
                    // Calculate the slope angle for the neighboring cell.
                    double dz = heightMap[ni][nj] - heightMap[i][j];  // Vertical rise.
                    double dy = sqrt(dz * dz / (dx * dx + dz * dz));  // Calculate dy based on dx and dz.

                    // Handle potential division by zero (near-zero denominator).
                    if (dz != 0.0) {
                        double slope_angle = atan2(dy, dx) * (180.0 / M_PI);  // Convert radians to degrees.

                        // Ensure the slope angle is within the range of 0-90 degrees.
                        slope_angle = max(0.0, min(90.0 * M_PI / 180 , slope_angle));

                        // Update the maximum slope angle.
                        if (slope_angle > max_slope_angle) {
                            max_slope_angle = slope_angle;
                        }

                        // Update the minimum slope angle.
                        if (slope_angle < min_slope_angle) {
                            min_slope_angle = slope_angle;
                        }
                    }
                }
            }

            // Ensure positive slope angles or their absolute values.
            max_slope_angle = abs(max_slope_angle);
            min_slope_angle = abs(min_slope_angle);

            // Store the maximum slope angle for the current cell.
            slopeAngle[i][j] = max_slope_angle;

            // Update the minimum and maximum slope angles.
            if (max_slope_angle > maxSlopeAngle) {
                maxSlopeAngle = max_slope_angle;
            }
            if (min_slope_angle < minSlopeAngle) {
                minSlopeAngle = min_slope_angle;
            }
        }
    }
    std::cout << "The min slope angle is : " << minSlopeAngle << ", max slope angle is : " << maxSlopeAngle << std::endl;
    return slopeAngle;
}

std::vector<std::vector<double>> ComputeAbsMaxSlopeAngleEx(const std::vector<std::vector<double>>& heightMap, double dx) {
    int m = heightMap.size();
    std::vector<std::vector<double>> slopeAngle(m, std::vector<double>(m, -1.0));  // Initialize with a default value.

    int dxdy[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    int dydx[] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    double min_slope_angle = M_PI / 2.0;  // Initialize as 90 degrees in radians
    double max_slope_angle = 0.0;         // Initialize as 0 degrees in radians

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            double max_slope_angle_cell = 0.0;   // Initialize as 0 degrees in radians

            for (int k = 0; k < 8; k++) {
                int ni = i + dxdy[k];
                int nj = j + dydx[k];

                if (ni >= 0 && ni < m && nj >= 0 && nj < m) {
                    double dz = heightMap[ni][nj] - heightMap[i][j];
                    double dy = std::sqrt(dz * dz / (dx * dx + dz * dz));

                    if (dz != 0.0) {
                        double slope_angle = std::atan2(dy, dx);

                        // Ensure the slope angle is within the range of 0 to p/2 radians
                        slope_angle = std::max(0.0, std::min(M_PI / 2.0, slope_angle));

                        max_slope_angle_cell = std::max(max_slope_angle_cell, slope_angle);
                        min_slope_angle = std::min(min_slope_angle, slope_angle);
                    }
                }
            }

            // Store the maximum slope angle for the current cell (in radians)
            slopeAngle[i][j] = max_slope_angle_cell;
        }
    }

    std::cout << "The minimum slope angle is: " << min_slope_angle << " radians, the maximum slope angle is: " << max_slope_angle << " radians" << std::endl;

    return slopeAngle;
}

// Function to compute the average slope angle (limited to 0-90 degrees) for a height map
vector<vector<double>> ComputeAbsAverageNeighborSlopeAngle(const vector<vector<double>>& heightMap, double dx) 
{
    int m = heightMap.size();
    vector<vector<double>> slopeAngle(m, vector<double>(m, 0.0));  // Initialize with zeros.

    // Initialize min and max slope angles with extreme values
    double minSlopeAngle = M_PI / 2.0;  // Initialize as 90 degrees in radians
    double maxSlopeAngle = 0.0;         // Initialize as 0 degrees in radians

    //double minSlopeAngle = numeric_limits<double>::infinity();
    //double maxSlopeAngle = -numeric_limits<double>::infinity();

    int dxdy[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    int dydx[] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            double total_slope_angle = 0.0;
            int neighbor_count = 0;

            for (int k = 0; k < 8; k++) {
                int ni = i + dxdy[k];
                int nj = j + dydx[k];

                if (ni >= 0 && ni < m && nj >= 0 && nj < m) {
                    // Calculate the slope angle for the neighboring cell.
                    double dz = heightMap[ni][nj] - heightMap[i][j];  // Vertical rise.
                    double dy = sqrt(dz * dz / (dx * dx + dz * dz));  // Calculate dy based on dx and dz.
                    //double dy = sqrt(dx * dx + dz * dz);
                    // Handle potential division by zero (near-zero denominator).
                    if (dz != 0.0) {
                        double slope_angle = atan2(dy, dx) * (180.0 / M_PI);  // Convert radians to degrees.

                        // Ensure the slope angle is within the range of 0-90 degrees.
                        slope_angle = max(0.0, min(90.0 * M_PI / 180, slope_angle));

                        // Add the slope angle to the total.
                        total_slope_angle += slope_angle;
                        neighbor_count++;

                        //std::cout << slope_angle << std::endl;
                    }
                    else
                    {
                        //std::cout << 0 << std::endl;

                        total_slope_angle += 0.0;
                        neighbor_count++;
                    }
                }
            }

            // Calculate the average slope angle for the current cell's neighbors.
            if (neighbor_count > 0) {
                double angle = total_slope_angle / neighbor_count;
                angle = std::max(0.0, min(90.0 * M_PI / 180, angle));
                slopeAngle[i][j] = angle;
                minSlopeAngle = std::min(minSlopeAngle, angle);
                maxSlopeAngle = std::max(maxSlopeAngle, angle);

            }
        }
    }
    std::cout << "The min slope angle is : " << minSlopeAngle << ", max slope angle is : " << maxSlopeAngle << std::endl;
    
    return slopeAngle;
}

// Function to convert a 1D RGB array to a 2D vector, returning a pointer to the vector
std::vector<std::vector<PixelRGB>>* ConvertRGBArrayTo2DVector(const unsigned char* rgbData, int width, int height) {
    auto result = new std::vector<std::vector<PixelRGB>>(height, std::vector<PixelRGB>(width));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 3; // 3 channels (RGB) per pixel

            PixelRGB pixel;
            pixel.red = rgbData[index];
            pixel.green = rgbData[index + 1];
            pixel.blue = rgbData[index + 2];

            (*result)[y][x] = pixel;
        }
    }

    return result;
}

std::string GetFileExtension(const std::string& filePath) {
    size_t dotPosition = filePath.find_last_of('.');
    if (dotPosition != std::string::npos) {
        return filePath.substr(dotPosition + 1);
    }
    return ""; // No file extension found
}

// Function to generate a random double within a given range [min, max]
double GenerateRandomDouble(double min, double max) {
    // Initialize a random number generator
    std::random_device rd;  // Seed generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<double> distribution(min, max);

    // Generate a random double within the specified range and return it
    return distribution(gen);
}

bool RemoveAllFilesInFolder(const std::string& folderPath) {
    try {
        std::filesystem::path directoryPath(folderPath);

        if (std::filesystem::is_directory(directoryPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                if (entry.is_regular_file()) {
                    std::filesystem::remove(entry);
                }
            }
            return true;  // All files removed successfully
        }
        else {
            std::cerr << "The specified path is not a directory: " << folderPath << std::endl;
            return false;  // Directory doesn't exist
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error while removing files in " << folderPath << ": " << e.what() << std::endl;
        return false;  // An error occurred
    }
}


