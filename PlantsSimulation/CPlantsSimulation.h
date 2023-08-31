#pragma once
#include <string>

using namespace std;

class CPlantsSimulation
{
	CPlantsSimulation(const string& inputImageFile, const string& heightMapFile, const string outputFile)
		:  m_inputImageFile(inputImageFile)
		,  m_heightMapFile(heightMapFile)
		,  m_outputFile(outputFile)

	{

	}
private:
	string m_inputImageFile;
	string m_heightMapFile;
	string m_outputFile;
};

