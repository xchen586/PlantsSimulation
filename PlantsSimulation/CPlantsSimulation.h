#pragma once
#include <string>

using namespace std;

class CPlantsSimulation
{
public:
	CPlantsSimulation(const string& inputImageFile, const string& heightMapFile, const string outputFile)
		: m_inputImageFile(inputImageFile)
		, m_heightMapFile(heightMapFile)
		, m_outputFile(outputFile)
		, input_image_width(0)
		, input_image_height(0)
		, input_image_data(nullptr)
		, input_image_comp(0)
	{
	
	}
private:
	string m_inputImageFile;
	string m_heightMapFile;
	string m_outputFile;

	int input_image_width;
	int input_image_height;
	unsigned char* input_image_data;
	int input_image_comp;

	bool loadInputImageFile(const string& inputImageFile);
};

