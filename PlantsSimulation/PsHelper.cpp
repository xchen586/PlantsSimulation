#include "PsHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

InputImageDataInfo* LoadInputImageFile(const string& inputImageFile)
{
	InputImageDataInfo* pDataInfo = new InputImageDataInfo();
	pDataInfo->input_image_data = stbi_load(inputImageFile.c_str(), &pDataInfo->input_image_width, &pDataInfo->input_image_height, &pDataInfo->input_image_comp, 1);
	if (!pDataInfo->input_image_data) {
		delete pDataInfo;
		return nullptr;
	}
	else {
		return pDataInfo;
	}
}

std::vector<std::vector<unsigned short>> Read2DShortArray(const std::string& filePath, int width, int height)
{
    std::ifstream fs(filePath, std::ios::binary);
    if (!fs.is_open())
    {
        throw std::runtime_error("Failed to open file");
    }

    // Check if the file size matches the expected size
    fs.seekg(0, std::ios::end);
    std::streampos fileSize = fs.tellg();
    if (fileSize != static_cast<std::streampos>(width) * height * sizeof(unsigned short))
    {
        throw std::runtime_error("File size doesn't match expected dimensions");
    }
    fs.seekg(0, std::ios::beg);

    std::vector<std::vector<unsigned short>> array(width, std::vector<unsigned short>(height));

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

            unsigned short value = (static_cast<unsigned short>(bytes[1]) << 8) | bytes[0];
            array[x][y] = value;
        }
    }

    return array;
}

double BilinearInterpolation(double x, double y, const std::vector<std::vector<unsigned short>>& inputArray)
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

double BilinearInterpolation2(double x, double y, const std::vector<std::vector<unsigned short>>& inputArray)
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

double BilinearInterpolation3(double x, double y, const std::vector<std::vector<unsigned short>>& inputArray)
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

std::vector<std::vector<unsigned short>> ScaleArray(const std::vector<std::vector<unsigned short>>& inputArray, int p, int q)
{
    int m = inputArray.size();
    int n = inputArray[0].size();

    std::vector<std::vector<unsigned short>> scaledArray(p, std::vector<unsigned short>(q));

    double scaleX = static_cast<double>(m) / p;
    double scaleY = static_cast<double>(n) / q;

    for (int x = 0; x < p; x++)
    {
        for (int y = 0; y < q; y++)
        {
            double originalX = x * scaleX;
            double originalY = y * scaleY;

            scaledArray[x][y] = static_cast<unsigned short>(BilinearInterpolation3(originalX, originalY, inputArray));
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

std::vector<std::vector<unsigned short>> ComputeSlopeMap(const std::vector<std::vector<unsigned short>>& heightmap)
{
    int width = heightmap.size();
    int height = heightmap[0].size();

    std::vector<std::vector<unsigned short>> slopeMap(width, std::vector<unsigned short>(height, 0));

    int dx[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    int dy[] = { -1, 0, 1, 1, 1, 0, -1, -1 };

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            unsigned short maxHeightDifference = 0;

            for (int i = 0; i < 8; i++)
            {
                int newX = x + dx[i];
                int newY = y + dy[i];

                if (newX >= 0 && newX < width && newY >= 0 && newY < height)
                {
                    short heightDifference = static_cast<short>(std::abs(heightmap[x][y] - heightmap[newX][newY]));

                    if (heightDifference > maxHeightDifference)
                    {
                        maxHeightDifference = static_cast<unsigned short>(heightDifference);
                    }
                }
            }

            slopeMap[x][y] = std::min(maxHeightDifference, static_cast<unsigned short>(USHRT_MAX));
        }
    }

    return slopeMap;
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
