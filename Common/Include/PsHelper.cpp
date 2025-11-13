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

/**
 * Safely converts a string to a double
 * @param str The string to convert
 * @param result Pointer to store the conversion result
 * @return true if conversion succeeds, false otherwise
 */
bool safe_strtod(const char* str, double& result) {
    // Handle null pointer case
    if (str == nullptr) {
        return false;
    }

    // Skip leading whitespace characters (spaces, tabs, newlines, etc.)
    const char* start = str;
    while (*start != '\0' && isspace(static_cast<unsigned char>(*start))) {
        ++start;
    }

    // Handle empty string or string with only whitespace
    if (*start == '\0') {
        return false;
    }

    char* endptr;
    // Perform the conversion
    result = std::strtod(start, &endptr);

    // Check if any valid characters were converted and all characters were processed
    if (endptr == start) {
        // No valid conversion occurred
        return false;
    }

    // Skip any trailing whitespace after the converted value
    while (*endptr != '\0' && isspace(static_cast<unsigned char>(*endptr))) {
        ++endptr;
    }

    // Conversion is considered fully successful only if all characters were processed
    return *endptr == '\0';
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

// ============================================================================
// Traditional Gaussian Blur (Convolution-based)
// ============================================================================

// Generate 1D Gaussian kernel
std::vector<double> generateGaussianKernel(int radius, double sigma) {
    int size = 2 * radius + 1;
    std::vector<double> kernel(size);
    double sum = 0.0;

    for (int i = 0; i < size; ++i) {
        int x = i - radius;
        kernel[i] = std::exp(-(x * x) / (2.0 * sigma * sigma));
        sum += kernel[i];
    }

    // Normalize the kernel
    for (int i = 0; i < size; ++i) {
        kernel[i] /= sum;
    }

    return kernel;
}

// Apply traditional Gaussian blur using separable convolution
// Best for small blur radii (radius < 10)
// Default values tuned for 4096x4096 heightmap with range [-10000, 10000]
std::vector<std::vector<short>> NormalGaussianBlurHeightmap(
    const std::vector<std::vector<short>>& heightmap,
    int radius/* = 15*/,     // Default: 15 pixels for smoother result
    double sigma/* = 5.0*/   // Default: larger sigma for smooth blending
) {
    if (heightmap.empty() || heightmap[0].empty()) {
        return heightmap;
    }

    int height = heightmap.size();
    int width = heightmap[0].size();

    // If sigma is too small relative to radius, auto-calculate it
    double effective_sigma = sigma;
    if (sigma < radius / 3.0) {
        effective_sigma = radius / 2.5;  // Good rule of thumb
    }

    // Generate Gaussian kernel
    std::vector<double> kernel = generateGaussianKernel(radius, effective_sigma);

    // Temporary buffer for horizontal pass
    std::vector<std::vector<double>> temp(height, std::vector<double>(width));

    // Horizontal pass
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double sum = 0.0;
            double weightSum = 0.0;  // Track actual weights used for normalization

            for (int k = -radius; k <= radius; ++k) {
                int nx = x + k;
                // Clamp to edges
                if (nx < 0) nx = 0;
                if (nx >= width) nx = width - 1;

                double weight = kernel[k + radius];
                sum += heightmap[y][nx] * weight;
                weightSum += weight;
            }
            // Renormalize in case of edge effects
            temp[y][x] = sum / weightSum;
        }
    }

    // Vertical pass
    std::vector<std::vector<short>> result(height, std::vector<short>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double sum = 0.0;
            double weightSum = 0.0;  // Track actual weights used for normalization

            for (int k = -radius; k <= radius; ++k) {
                int ny = y + k;
                // Clamp to edges
                if (ny < 0) ny = 0;
                if (ny >= height) ny = height - 1;

                double weight = kernel[k + radius];
                sum += temp[ny][x] * weight;
                weightSum += weight;
            }
            // Renormalize in case of edge effects
            double val = sum / weightSum;

            // Clamp result to valid range [-10000, 10000]
            if (val < -10000.0) val = -10000.0;
            if (val > 10000.0) val = 10000.0;
            result[y][x] = static_cast<short>(std::round(val));
        }
    }

    return result;
}

// ============================================================================
// IIR Gaussian Blur (Recursive filter approximation)
// ============================================================================

// Apply IIR Gaussian blur using recursive filters
// Much faster for large blur radii, O(n*m) complexity regardless of sigma
// Best for large blur radii (sigma > 5)
// Default values tuned for 4096x4096 heightmap with range [-10000, 10000]
std::vector<std::vector<short>> IIRGaussianBlurHeightmap(
    const std::vector<std::vector<short>>& heightmap,
    int radius/* = 0*/,      // Unused in IIR version (kept for interface compatibility)
    double sigma/* = 8.0*/   // Default: larger sigma for smooth terrain blending
) {
    if (heightmap.empty() || heightmap[0].empty()) {
        return heightmap;
    }

    int height = heightmap.size();
    int width = heightmap[0].size();

    // Calculate filter parameters for specified sigma
    // Use Equation 11b to determine q
    float q;
    if (sigma >= 2.5)
        q = 0.98711f * sigma - 0.96330f;
    else if (sigma >= 0.5)
        q = 3.97156f - 4.14554f * std::sqrt(1.0f - 0.26891f * sigma);
    else
        return heightmap; // Sigma too small, return original

    // Use equation 8c to determine b0, b1, b2 and b3
    float b0 = 1.57825f + 2.44413f * q + 1.4281f * q * q + 0.422205f * q * q * q;
    float b1 = 2.44413f * q + 2.85619f * q * q + 1.26661f * q * q * q;
    float b2 = -(1.4281f * q * q + 1.26661f * q * q * q);
    float b3 = 0.422205f * q * q * q;

    // Use equation 10 to determine B
    float B = 1.0f - (b1 + b2 + b3) / b0;

    // Allocate float buffer for intermediate calculations
    std::vector<std::vector<float>> buffer(height, std::vector<float>(width));

    // Horizontal forward pass
    for (int y = 0; y < height; y++) {
        float prev1 = heightmap[y][0];
        float prev2 = prev1;
        float prev3 = prev2;

        for (int x = 0; x < width; x++) {
            float val = B * heightmap[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
            buffer[y][x] = val;
            prev3 = prev2;
            prev2 = prev1;
            prev1 = val;
        }
    }

    // Horizontal backward pass
    for (int y = 0; y < height; y++) {
        float prev1 = buffer[y][width - 1];
        float prev2 = prev1;
        float prev3 = prev2;

        for (int x = width - 1; x >= 0; x--) {
            float val = B * buffer[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
            buffer[y][x] = val;
            prev3 = prev2;
            prev2 = prev1;
            prev1 = val;
        }
    }

    // Vertical forward pass
    for (int x = 0; x < width; x++) {
        float prev1 = buffer[0][x];
        float prev2 = prev1;
        float prev3 = prev2;

        for (int y = 0; y < height; y++) {
            float val = B * buffer[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
            buffer[y][x] = val;
            prev3 = prev2;
            prev2 = prev1;
            prev1 = val;
        }
    }

    // Vertical backward pass and convert back to short
    std::vector<std::vector<short>> result(height, std::vector<short>(width));

    for (int x = 0; x < width; x++) {
        float prev1 = buffer[height - 1][x];
        float prev2 = prev1;
        float prev3 = prev2;

        for (int y = height - 1; y >= 0; y--) {
            float val = B * buffer[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;

            // Clamp result to valid range [-10000, 10000]
            if (val < -10000.0f) val = -10000.0f;
            if (val > 10000.0f) val = 10000.0f;

            result[y][x] = static_cast<short>(std::round(val));
            prev3 = prev2;
            prev2 = prev1;
            prev1 = val;
        }
    }

    return result;
}

std::vector<std::vector<short>> iir_gauss_blur(const std::vector<std::vector<short>>& image, float sigma) {
    // Return empty array if input image is empty or has empty rows
    if (image.empty()) {
        return {};
    }
    const int height = static_cast<int>(image.size());
    const int width = static_cast<int>(image[0].size());

    // Check for jagged array (rows with inconsistent lengths)
    for (int y = 0; y < height; ++y) {
        if (image[y].size() != static_cast<size_t>(width)) {
            return {}; // Or handle error appropriately
        }
    }

    // Calculate filter parameters
    float q;
    if (sigma >= 2.5f) {
        q = 0.98711f * sigma - 0.96330f;
    }
    else if (sigma >= 0.5f) {
        q = 3.97156f - 4.14554f * std::sqrt(1.0f - 0.26891f * sigma);
    }
    else {
        return image; // Return original for small sigma
    }

    // Filter coefficients
    const float b0 = 1.57825f + 2.44413f * q + 1.4281f * q * q + 0.422205f * q * q * q;
    const float b1 = 2.44413f * q + 2.85619f * q * q + 1.26661f * q * q * q;
    const float b2 = -(1.4281f * q * q + 1.26661f * q * q * q);
    const float b3 = 0.422205f * q * q * q;
    const float B = 1.0f - (b1 + b2 + b3) / b0;

    // Temporary buffer
    std::vector<std::vector<float>> buffer(height, std::vector<float>(width));

    // Horizontal forward filtering
    for (int y = 0; y < height; ++y) {
        float prev1 = static_cast<float>(image[y][0]);
        float prev2 = prev1;
        float prev3 = prev2;

        for (int x = 0; x < width; ++x) {
            const float val = B * static_cast<float>(image[y][x]) + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
            buffer[y][x] = val;

            prev3 = prev2;
            prev2 = prev1;
            prev1 = val;
        }
    }

    // Horizontal backward filtering
    for (int y = height - 1; y >= 0; --y) {
        float prev1 = buffer[y][width - 1];
        float prev2 = prev1;
        float prev3 = prev2;

        for (int x = width - 1; x >= 0; --x) {
            const float val = B * buffer[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
            buffer[y][x] = val;

            prev3 = prev2;
            prev2 = prev1;
            prev1 = val;
        }
    }

    // Vertical forward filtering
    for (int x = 0; x < width; ++x) {
        float prev1 = buffer[0][x];
        float prev2 = prev1;
        float prev3 = prev2;

        for (int y = 0; y < height; ++y) {
            const float val = B * buffer[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
            buffer[y][x] = val;

            prev3 = prev2;
            prev2 = prev1;
            prev1 = val;
        }
    }

    // Vertical backward filtering and result creation
    std::vector<std::vector<short>> result(height, std::vector<short>(width));
    for (int x = width - 1; x >= 0; --x) {
        float prev1 = buffer[height - 1][x];
        float prev2 = prev1;
        float prev3 = prev2;

        for (int y = height - 1; y >= 0; --y) {
            const float val = B * buffer[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
            result[y][x] = static_cast<short>(std::max(-32768.0f, std::min(32767.0f, val)));

            prev3 = prev2;
            prev2 = prev1;
            prev1 = val;
        }
    }

    return result;
}

std::vector<std::vector<short>> iir_gauss_blur_with_mask(
    const std::vector<std::vector<short>>& image,
    const std::vector<std::vector<short>>& masks,  // Mask: 1 = valid, 0 = invalid
    float sigma) {

    // Validate input
    if (image.empty() || masks.empty() || image[0].empty() || masks[0].empty()) {
        return {};
    }
    const int height = static_cast<int>(image.size());
    const int width = static_cast<int>(image[0].size());

    // Check dimension consistency
    if (static_cast<int>(masks.size()) != height || static_cast<int>(masks[0].size()) != width) {
        return {};
    }
    for (int y = 0; y < height; ++y) {
        if (image[y].size() != static_cast<size_t>(width) || masks[y].size() != static_cast<size_t>(width)) {
            return {}; // Jagged array
        }
    }

    // Return original if sigma is too small (no blur)
    if (sigma < 0.5f) {
        return image;
    }

    // Calculate filter parameters (same as original)
    float q;
    if (sigma >= 2.5f) {
        q = 0.98711f * sigma - 0.96330f;
    }
    else {
        q = 3.97156f - 4.14554f * std::sqrt(1.0f - 0.26891f * sigma);
    }
    const float b0 = 1.57825f + 2.44413f * q + 1.4281f * q * q + 0.422205f * q * q * q;
    const float b1 = 2.44413f * q + 2.85619f * q * q + 1.26661f * q * q * q;
    const float b2 = -(1.4281f * q * q + 1.26661f * q * q * q);
    const float b3 = 0.422205f * q * q * q;
    const float B = 1.0f - (b1 + b2 + b3) / b0;

    // Initialize buffer with original values (invalid pixels stay unchanged)
    std::vector<std::vector<float>> buffer(height, std::vector<float>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            buffer[y][x] = static_cast<float>(image[y][x]);
        }
    }

    // Horizontal forward pass
    for (int y = 0; y < height; ++y) {
        // Find first valid pixel to initialize history
        float prev1 = 0.0f, prev2 = 0.0f, prev3 = 0.0f;
        bool has_valid = false;
        for (int x = 0; x < width; ++x) {
            if (masks[y][x] > 0) {
                prev1 = static_cast<float>(image[y][x]);
                prev2 = prev1;
                prev3 = prev2;
                has_valid = true;
                break;
            }
        }
        if (!has_valid) continue; // Skip rows with no valid pixels

        // Process each pixel (update history only for valid pixels)
        for (int x = 0; x < width; ++x) {
            if (masks[y][x] > 0) {
                const float val = B * static_cast<float>(image[y][x]) + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
                buffer[y][x] = val;
                // Update history with current valid value
                prev3 = prev2;
                prev2 = prev1;
                prev1 = val;
            }
            // For invalid pixels: history remains unchanged (uses last valid state)
        }
    }

    // Horizontal backward pass
    for (int y = height - 1; y >= 0; --y) {
        float prev1 = 0.0f, prev2 = 0.0f, prev3 = 0.0f;
        bool has_valid = false;
        for (int x = width - 1; x >= 0; --x) {
            if (masks[y][x] > 0) {
                prev1 = buffer[y][x];
                prev2 = prev1;
                prev3 = prev2;
                has_valid = true;
                break;
            }
        }
        if (!has_valid) continue;

        for (int x = width - 1; x >= 0; --x) {
            if (masks[y][x] > 0) {
                const float val = B * buffer[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
                buffer[y][x] = val;
                prev3 = prev2;
                prev2 = prev1;
                prev1 = val;
            }
            // History preserved for invalid pixels
        }
    }

    // Vertical forward pass
    for (int x = 0; x < width; ++x) {
        float prev1 = 0.0f, prev2 = 0.0f, prev3 = 0.0f;
        bool has_valid = false;
        for (int y = 0; y < height; ++y) {
            if (masks[y][x] > 0) {
                prev1 = buffer[y][x];
                prev2 = prev1;
                prev3 = prev2;
                has_valid = true;
                break;
            }
        }
        if (!has_valid) continue;

        for (int y = 0; y < height; ++y) {
            if (masks[y][x] > 0) {
                const float val = B * buffer[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
                buffer[y][x] = val;
                prev3 = prev2;
                prev2 = prev1;
                prev1 = val;
            }
        }
    }

    // Vertical backward pass and result creation
    std::vector<std::vector<short>> result(height, std::vector<short>(width));
    for (int x = width - 1; x >= 0; --x) {
        float prev1 = 0.0f, prev2 = 0.0f, prev3 = 0.0f;
        bool has_valid = false;
        for (int y = height - 1; y >= 0; --y) {
            if (masks[y][x] > 0) {
                prev1 = buffer[y][x];
                prev2 = prev1;
                prev3 = prev2;
                has_valid = true;
                break;
            }
        }
        if (!has_valid) {
            // Copy original values for columns with no valid pixels
            for (int y = 0; y < height; ++y) {
                result[y][x] = image[y][x];
            }
            continue;
        }

        for (int y = height - 1; y >= 0; --y) {
            if (masks[y][x] > 0) {
                const float val = B * buffer[y][x] + (b1 * prev1 + b2 * prev2 + b3 * prev3) / b0;
                result[y][x] = static_cast<short>(std::clamp(val, -32768.0f, 32767.0f));
                prev3 = prev2;
                prev2 = prev1;
                prev1 = val;
            }
            else {
                // Invalid pixels retain original value
                result[y][x] = image[y][x];
            }
        }
    }

    return result;
}


// Generate 1D Gaussian kernel
std::vector<double> generateGaussianKernelEx(int radius, double sigma) {
    size_t size = 2 * static_cast<size_t>(radius) + 1;  // Avoid overflow
    std::vector<double> kernel(size);
    double sum = 0.0;

    for (size_t i = 0; i < size; ++i) {
        int x = static_cast<int>(i) - radius;
        kernel[i] = std::exp(-(x * x) / (2.0 * sigma * sigma));
        sum += kernel[i];
    }

    // Normalize the kernel
    for (double& w : kernel) {
        w /= sum;
    }

    return kernel;
}

// Apply traditional Gaussian blur using separable convolution
std::vector<std::vector<short>> NormalGaussianBlurHeightmapEx(
    const std::vector<std::vector<short>>& heightmap,
    int radius/* = 15*/,
    double sigma/* = 5.0*/,
    short min_val/* = -10000*/,  // Configurable range
    short max_val/* = 10000*/
) {
    if (heightmap.empty() || heightmap[0].empty()) {
        return heightmap;
    }

    int height = static_cast<int>(heightmap.size());
    int width = static_cast<int>(heightmap[0].size());

    // Auto-calculate sigma using 3σ rule if too small
    double effective_sigma = sigma;
    if (sigma < radius / 3.0) {
        effective_sigma = radius / 3.0;  // Standard 3σ coverage
    }

    // Generate Gaussian kernel
    std::vector<double> kernel = generateGaussianKernelEx(radius, effective_sigma);

    // Temporary buffer for horizontal pass
    std::vector<std::vector<double>> temp(height, std::vector<double>(width));

    // Horizontal pass
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double sum = 0.0;
            double weightSum = 0.0;

            for (int k = -radius; k <= radius; ++k) {
                const int nx = std::clamp(x + k, 0, width - 1);  // Clamp with std::clamp
                const double weight = kernel[k + radius];
                sum += heightmap[y][nx] * weight;
                weightSum += weight;
            }
            temp[y][x] = sum / weightSum;  // Renormalize
        }
    }

    // Vertical pass
    std::vector<std::vector<short>> result(height, std::vector<short>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double sum = 0.0;
            double weightSum = 0.0;

            for (int k = -radius; k <= radius; ++k) {
                const int ny = std::clamp(y + k, 0, height - 1);  // Clamp with std::clamp
                const double weight = kernel[k + radius];
                sum += temp[ny][x] * weight;
                weightSum += weight;
            }
            double val = sum / weightSum;  // Renormalize

            // Clamp and round
            val = std::clamp(val, static_cast<double>(min_val), static_cast<double>(max_val));
            result[y][x] = static_cast<short>(std::round(val));
        }
    }

    return result;
}

std::vector<std::vector<short>> MaskedGaussianBlurHeightmap(
    const std::vector<std::vector<short>>& heightmap,
    const std::vector<std::vector<short>>& masks,
    int radius/* = 15*/,
    double sigma/* = 5.0*/,
    short min_val/* = -10000*/,
    short max_val/* = 10000*/
) {
    if (heightmap.empty() || masks.empty() ||
        heightmap[0].empty() || masks[0].empty() ||
        heightmap.size() != masks.size() ||
        heightmap[0].size() != masks[0].size()) {
        return {};  // Return empty on invalid input
    }

    int height = static_cast<int>(heightmap.size());
    int width = static_cast<int>(heightmap[0].size());

    // Reject jagged arrays explicitly
    for (int y = 0; y < height; ++y) {
        if (heightmap[y].size() != static_cast<size_t>(width) ||
            masks[y].size() != static_cast<size_t>(width)) {
            return {};
        }
    }

    double effective_sigma = sigma;
    if (sigma < radius / 3.0) {
        effective_sigma = radius / 3.0;
    }

    std::vector<double> kernel = generateGaussianKernelEx(radius, effective_sigma);
    std::vector<std::vector<double>> temp(height, std::vector<double>(width));

    // Horizontal pass
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (masks[y][x] <= 0) {
                temp[y][x] = static_cast<double>(heightmap[y][x]);
                continue;
            }

            double sum = 0.0;
            double weightSum = 0.0;
            for (int k = -radius; k <= radius; ++k) {
                const int nx = std::clamp(x + k, 0, width - 1);
                if (masks[y][nx] > 0) {
                    const double weight = kernel[k + radius];
                    sum += heightmap[y][nx] * weight;
                    weightSum += weight;
                }
            }

            temp[y][x] = (weightSum < 1e-9) ?
                static_cast<double>(heightmap[y][x]) :
                sum / weightSum;
        }
    }

    // Vertical pass
    std::vector<std::vector<short>> result(height, std::vector<short>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (masks[y][x] <= 0) {
                result[y][x] = heightmap[y][x];
                continue;
            }

            double sum = 0.0;
            double weightSum = 0.0;
            for (int k = -radius; k <= radius; ++k) {
                const int ny = std::clamp(y + k, 0, height - 1);
                if (masks[ny][x] > 0) {
                    const double weight = kernel[k + radius];
                    sum += temp[ny][x] * weight;
                    weightSum += weight;
                }
            }

            if (weightSum < 1e-9) {
                result[y][x] = heightmap[y][x];
            }
            else {
                double val = sum / weightSum;
                val = std::clamp(val, static_cast<double>(min_val), static_cast<double>(max_val));
                result[y][x] = static_cast<short>(std::round(val));
            }
        }
    }

    return result;
}