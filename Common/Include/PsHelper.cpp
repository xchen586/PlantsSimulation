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

// Function to calculate the squared distance between two points
double DistanceSquared(const Point& a, const Point& b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

// Function to calculate the distance from a point to a line segment
double PointToSegmentDistance(const Point& p, const Point& a, const Point& b, double distanceLimit/* = 0.0*/) {

    // Quick rejection: check if endpoints are within distance limit
    double distToA = std::sqrt(DistanceSquared(p, a));
    if (distToA < distanceLimit) return distToA;

    double distToB = std::sqrt(DistanceSquared(p, b));
    if (distToB < distanceLimit) return distToB;

    // Vector AB
    double abx = b.x - a.x;
    double aby = b.y - a.y;

    // Vector AP
    double apx = p.x - a.x;
    double apy = p.y - a.y;

    // Dot product of AB and AP
    double dot = apx * abx + apy * aby;

    // Squared length of AB
    double lengthSquared = abx * abx + aby * aby;

    // Parameter of the projection of P onto AB
    double t = (lengthSquared > 0) ? dot / lengthSquared : 0;

    // Clamp t to the range [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    // Closest point on the segment
    Point closest = { a.x + t * abx, a.y + t * aby };

    // Calculate the squared distance to the closest point
    double distanceSquared = DistanceSquared(p, closest);

    return std::sqrt(distanceSquared);
}

// Function to calculate the distance from a point to a polyline
double PointToPolylineDistance(const Point& p, const std::vector<Point>& polyline, double distanceLimit/* = 0.0*/) {
    if (polyline.empty()) {
        throw std::invalid_argument("Polyline must have at least one point.");
    }

    // Special case: Degenerate polyline with one point
    if (polyline.size() == 1) {
        return std::sqrt(DistanceSquared(p, polyline[0]));
    }

    double minDistance = std::numeric_limits<double>::max();
   
    // Iterate through each segment of the polyline
    for (size_t i = 0; i < polyline.size() - 1; ++i) {
        // Calculate the squared distance to the current segment
        double segmentDistance = PointToSegmentDistance(p, polyline[i], polyline[i + 1], distanceLimit);
        minDistance = std::min(minDistance, segmentDistance);

        // Early exit if the distance is within the limit
        if (minDistance < distanceLimit) {
            return minDistance;
        }
    }

    return minDistance;
}

// Function to calculate the minimum distance from a point to a group of lines
double GetDistanceToCaveNodes(const Point& p, const std::vector<std::pair<std::vector<Point>, int>>* lines, double distanceLimit/* = 0.0*/) {
    double minDistance = std::numeric_limits<double>::max();

    for (const auto& [points, generation] : *lines) {

        double distance = PointToPolylineDistance(p, points, distanceLimit);
        minDistance = std::min(minDistance, distance);

        // Early exit if we're already below the distance limit
        if (minDistance < distanceLimit) {
            return minDistance;
        }
    }

    return minDistance;
}

// Function to calculate the minimum distance from a point to a list of POIs
double GetDistancesToPOIs(const Point& p, const std::vector<Point>* poisLocations, double distanceLimit/* = 0.0*/)
{
	double minDistance = std::numeric_limits<double>::max();
	for (const auto& poi : *poisLocations) {
		double distance = std::sqrt(DistanceSquared(p, poi));
		minDistance = std::min(minDistance, distance);
		// Early exit if we're already below the distance limit
		if (minDistance < distanceLimit) {
			return minDistance;
		}
	}
	return minDistance;
}

std::vector<std::pair<std::vector<Point>, int>>* ConvertCaveInfoToCaveNodes(
    const std::vector<CavesPointInfo>& points) {
    try {
        // Dynamically allocate with exception handling
        auto* result = new std::vector<std::pair<std::vector<Point>, int>>();
        result->reserve(points.size());  // Reserve space to avoid reallocations

        // Group points by index and seq
        std::map<int, std::map<int, std::vector<CavesPointInfo>>> groupedData;
        for (const auto& point : points) {
            groupedData[point.index][point.generation].push_back(point);
        }

        // Process grouped data
        for (const auto& [index, generationMap] : groupedData) {
            int generationCount = generationMap.size();
            if (generationCount > 1)
            {
                std::cout << "There are more than 1 generation in cave segment." << std::endl;
            }
            for (const auto& [generation, pointsVec] : generationMap) {
                // Sort by order
                std::vector<CavesPointInfo> sortedGroup = pointsVec;
                std::sort(sortedGroup.begin(), sortedGroup.end(),
                    [](const CavesPointInfo& a, const CavesPointInfo& b) {
                        return a.order < b.order;
                    });

                // Convert to Points
                std::vector<Point> pointGroup;
                pointGroup.reserve(sortedGroup.size());
                for (const auto& point : sortedGroup) {
                    pointGroup.emplace_back(point.x, point.y);
                }

                result->emplace_back(std::move(pointGroup), generation);
            }
        }

        return result;

    }
    catch (const std::exception& e) {
        // Handle any exceptions (memory allocation, etc.)
        return nullptr;  // Or handle error differently based on your needs
    }
}

VoxelFarm::CellId GetVFCellIDFromVF(double vfX, double vfY, double vfZ, const CAffineTransform& transform, int32_t lod)
{
    const double cellScale = (1 << lod) * VoxelFarm::CELL_SIZE;

    int cellX = (int)((vfX) / cellScale);
    int cellY = (int)((vfY) / cellScale);
    int cellZ = (int)((vfZ) / cellScale);

    return VoxelFarm::packCellId(lod, cellX, cellY, cellZ);
}

VoxelFarm::CellId GetVFCellIDFromWorld(double posX, double posY, double posZ, const CAffineTransform& transform, int32_t lod)
{
    const auto vf = transform.WC_TO_VF(CAffineTransform::sAffineVector(posX, posY, posZ));
    return GetVFCellIDFromVF(vf.X, vf.Y, vf.Z, transform, lod);
}

std::vector<std::vector<short>> Read2DShortArray(const std::string& filePath, int width, int height, bool invert/* = true*/)
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
            if (invert)
            {
                array[y][x] = value;  //mirror issue !!!!!!!
            }
            else
            {
                array[x][y] = value;
            }
        }
    }
    std::cout << "Short Height Map minHeight = " << minHeight << " , maxHeight = " << maxHeight << std::endl;
    return array;
}

std::vector<std::vector<int>> Read2DIntArray(const std::string& filePath, int width, int height, bool invert/* = false*/)
{
    std::ifstream fs(filePath, std::ios::binary);
    if (!fs.is_open())
    {
        throw std::runtime_error("Failed to open file");
    }

    int minHeight = std::numeric_limits<int>::max();
    int maxHeight = std::numeric_limits<int>::min();

    // Check if the file size matches the expected size
    fs.seekg(0, std::ios::end);
    std::streampos fileSize = fs.tellg();
    if (fileSize != static_cast<std::streampos>(width) * height * sizeof(int))
    {
        throw std::runtime_error("File size doesn't match expected dimensions");
    }
    fs.seekg(0, std::ios::beg);

    std::vector<std::vector<int>> array(width, std::vector<int>(height));

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            unsigned char bytes[4]; // Reading 4 bytes for an int
            fs.read(reinterpret_cast<char*>(bytes), 4);
            if (!fs)
            {
                throw std::runtime_error("Failed to read data from file");
            }

            // Reconstruct the int from 4 bytes (assuming little-endian format)
            int value = (static_cast<unsigned char>(bytes[3]) << 24) |
                (static_cast<unsigned char>(bytes[2]) << 16) |
                (static_cast<unsigned char>(bytes[1]) << 8) |
                static_cast<unsigned char>(bytes[0]);

            minHeight = std::min(minHeight, value);
            maxHeight = std::max(maxHeight, value);
            if (invert)
            {
                array[y][x] = value;  // Store the int value correctly
            }
            else
            {
                array[x][y] = value;  // Store the int value correctly
            }
        }
    }

    std::cout << "Int Height Map minHeight = " << minHeight << " , maxHeight = " << maxHeight << std::endl;
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

/*double BilinearInterpolation3(double x, double y, const std::vector<std::vector<short>>& inputArray)
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
}*/

double GetNormalLinearAttribute(double value, double min, double max)
{
    value = std::clamp(value, min, max);
    double normalized = (value - min) / (max - min);
    return std::lerp(0.0, 1.0, normalized);
}

double GetValueFromNormalized(double normalized, double min, double max)
{
    normalized = std::clamp(normalized, 0.0, 1.0);
    return min + normalized * (max - min);
}

double GetInvertLinearAttribute(double value, double min, double max)
{
    value = std::clamp(value, min, max);
    double normalized = (value - min) / (max - min);
    return std::lerp(0.0, 1.0, normalized);
}

double GetValueFromInvertedNormalized(double invertedNormalized, double min, double max)
{
    invertedNormalized = std::clamp(invertedNormalized, 0.0, 1.0);
    return min + (1.0 - invertedNormalized) * (max - min);
}

double GetColorLinearNormallizedAttribute(short value)
{
    double doublevalue = static_cast<double>(value);
    return GetNormalLinearAttribute(doublevalue, 0.0, 255.0);
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

bool CheckExistFolderAndRemoveSubFiles(const std::string& outputSubsDir)
{
    if (!std::filesystem::exists(outputSubsDir)) {
        if (!std::filesystem::create_directory(outputSubsDir)) {
            std::cerr << "Failed to create the directory of outputSubsDir!" << std::endl;
            return false;
        }
    }
    bool removeFiles = RemoveAllFilesInFolder(outputSubsDir);
}

std::vector<std::vector<short>> resampl2DShortMask(const std::vector<std::vector<short>>& originalMask,
    int newWidth, int newHeight) {
    int originalWidth = originalMask.size();
    int originalHeight = originalMask[0].size();

    // Create the new resized mask
    std::vector<std::vector<short>> resizedMask(newWidth, std::vector<short>(newHeight, 0));

    // Calculate scaling factors
    double scaleX = static_cast<double>(originalWidth) / newWidth;
    double scaleY = static_cast<double>(originalHeight) / newHeight;

    // For each cell in the new mask
    for (int newX = 0; newX < newWidth; ++newX) {
        for (int newY = 0; newY < newHeight; ++newY) {
            // Calculate the corresponding region in the original mask
            int startX = static_cast<int>(newX * scaleX);
            int startY = static_cast<int>(newY * scaleY);
            int endX = static_cast<int>((newX + 1) * scaleX);
            int endY = static_cast<int>((newY + 1) * scaleY);

            // Make sure we don't exceed boundaries
            endX = std::min(endX, originalWidth);
            endY = std::min(endY, originalHeight);

            // Count how many lake cells we have in this region
            int lakeCount = 0;
            int totalCells = 0;

            for (int x = startX; x < endX; ++x) {
                for (int y = startY; y < endY; ++y) {
                    if (originalMask[x][y] != 0) {
                        lakeCount++;
                    }
                    totalCells++;
                }
            }

            // If more than half of the cells in the region are lakes,
            // mark this cell as a lake in the resized mask
            if (totalCells > 0 && lakeCount > 0) {
                // Use the most common non-zero value, or simply 1 if we just want to preserve binary nature
                // Here, we're using the average value of the non-zero cells
                resizedMask[newX][newY] = static_cast<short>(lakeCount > 0 ?
                    (totalCells > 0 ? 1 : 0) : 0);
            }
        }
    }

    return resizedMask;
}

std::vector<std::vector<unsigned char>> resample2DShortMaskToByte(const std::vector<std::vector<short>>& originalMask,
    int newWidth, int newHeight) {
    int originalWidth = originalMask.size();
    int originalHeight = originalMask[0].size();

    // Create the new resized mask using bytes (uint8_t)
    std::vector<std::vector<unsigned char>> resizedMask(newWidth, std::vector<unsigned char>(newHeight, 0));

    // Calculate scaling factors
    double scaleX = static_cast<double>(originalWidth) / newWidth;
    double scaleY = static_cast<double>(originalHeight) / newHeight;

    // For each cell in the new mask
    for (int newX = 0; newX < newWidth; ++newX) {
        for (int newY = 0; newY < newHeight; ++newY) {
            // Calculate the corresponding region in the original mask
            int startX = static_cast<int>(newX * scaleX);
            int startY = static_cast<int>(newY * scaleY);
            int endX = static_cast<int>((newX + 1) * scaleX);
            int endY = static_cast<int>((newY + 1) * scaleY);

            // Make sure we don't exceed boundaries
            endX = std::min(endX, originalWidth);
            endY = std::min(endY, originalHeight);

            // Count how many lake cells we have in this region
            int lakeCount = 0;
            int totalCells = 0;

            for (int x = startX; x < endX; ++x) {
                for (int y = startY; y < endY; ++y) {
                    if (originalMask[x][y] != 0) {
                        lakeCount++;
                    }
                    totalCells++;
                }
            }

            // If there are lake cells in the region, mark this cell in the resized mask
            if (totalCells > 0 && lakeCount > 0) {
                // Convert to byte, ensuring we don't exceed byte range (0-255)
                resizedMask[newX][newY] = static_cast<uint8_t>(lakeCount > 0 ? 1 : 0);
            }
        }
    }

    return resizedMask;
}

std::vector<std::vector<short>> resample2DShortWithAverage(const std::vector<std::vector<short>>& original,
    int new_rows, int new_cols) {

    int original_rows = original.size();
    if (original_rows == 0) throw std::invalid_argument("Original grid has no rows");
    int original_cols = original[0].size();
    if (original_cols == 0) throw std::invalid_argument("Original grid has no columns");

    std::vector<std::vector<short>> resampled(new_rows, std::vector<short>(new_cols, 0));

    // Use floating-point factors for scaling
    double row_scale = static_cast<double>(original_rows) / new_rows;
    double col_scale = static_cast<double>(original_cols) / new_cols;

    for (int i = 0; i < new_rows; ++i) {
        for (int j = 0; j < new_cols; ++j) {
            long long sum = 0;
            int count = 0;

            // Calculate the bounds of the current region in the original grid
            int row_start = static_cast<int>(i * row_scale);
            int row_end = static_cast<int>((i + 1) * row_scale);
            int col_start = static_cast<int>(j * col_scale);
            int col_end = static_cast<int>((j + 1) * col_scale);

            // Ensure the bounds stay within the original grid dimensions
            row_end = std::min(row_end, original_rows);
            col_end = std::min(col_end, original_cols);

            for (int x = row_start; x < row_end; ++x) {
                for (int y = col_start; y < col_end; ++y) {
                    sum += original[x][y];
                    ++count;
                }
            }

            // Avoid division by zero, though this shouldn't happen with valid inputs
            if (count > 0) {
                resampled[i][j] = static_cast<short>(sum / count);
            }
        }
    }

    return resampled;
}

std::vector<std::vector<unsigned char>> resample2DUCharWithAverage(const std::vector<std::vector<unsigned char>>& original,
    int new_rows, int new_cols) {

    int original_rows = original.size();
    if (original_rows == 0) throw std::invalid_argument("Original grid has no rows");
    int original_cols = original[0].size();
    if (original_cols == 0) throw std::invalid_argument("Original grid has no columns");

    std::vector<std::vector<unsigned char>> resampled(new_rows, std::vector<unsigned char>(new_cols, 0));

    // Use floating-point scaling factors for row and column
    double row_scale = static_cast<double>(original_rows) / new_rows;
    double col_scale = static_cast<double>(original_cols) / new_cols;

    for (int i = 0; i < new_rows; ++i) {
        for (int j = 0; j < new_cols; ++j) {
            long long sum = 0;
            int count = 0;

            // Calculate the bounds of the current region in the original grid
            int row_start = static_cast<int>(i * row_scale);
            int row_end = static_cast<int>((i + 1) * row_scale);
            int col_start = static_cast<int>(j * col_scale);
            int col_end = static_cast<int>((j + 1) * col_scale);

            // Ensure the boundaries stay within the original grid dimensions
            row_end = std::min(row_end, original_rows);
            col_end = std::min(col_end, original_cols);

            // Sum the values in the defined region of the original grid
            for (int x = row_start; x < row_end; ++x) {
                for (int y = col_start; y < col_end; ++y) {
                    sum += original[x][y];
                    ++count;
                }
            }

            // Avoid division by zero and compute the average
            if (count > 0) {
                resampled[i][j] = static_cast<unsigned char>(sum / count);
            }
        }
    }

    return resampled;
}

bool stringToBool(const std::string& str) {
    std::string lowerStr = str;

    // Convert string to lowercase
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);

    // Check for true values
    if (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "y") {
        return true;
    }

    // Check for false values
    if (lowerStr == "false" || lowerStr == "0" || lowerStr == "no" || lowerStr == "n") {
        return false;
    }

    // If the string is neither, throw an error or return a default value
    //throw std::invalid_argument("Invalid string for conversion to bool: " + str);
    // or you can return false as a default
    return false;
}

// Helper function to trim whitespace
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Function to count columns in a CSV file
int countColumnsInCSV(const std::string& filePath, char delimiter = ',') {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filePath << "'." << std::endl;
        return -1;
    }

    std::string line;
    if (std::getline(file, line)) { // Read the first line
        std::stringstream ss(line);
        std::string column;
        int columnCount = 0;

        // Count columns separated by the specified delimiter
        while (std::getline(ss, column, delimiter)) {
            column = trim(column); // Trim whitespace from each column
            columnCount++;
        }

        file.close();
        return columnCount;
    }

    file.close();
    std::cerr << "Error: File is empty or only contains whitespace." << std::endl;
    return -1;
}