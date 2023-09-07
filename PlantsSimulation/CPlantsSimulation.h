#pragma once
#include "PsHelper.h"
#include "CCellData.h"
#include "CForest.h"
using namespace std;

class CPlantsSimulation
{
public:
	CPlantsSimulation(const string& inputImageFile, const string& heightMapFile, const string outputFile)
		: m_inputImageFile(inputImageFile)
		, m_heightMapFile(heightMapFile)
		, m_outputFile(outputFile)
		, m_topLayerImage(nullptr)
		, m_pCellTable(nullptr)
		, m_pForest(nullptr)
	{

	}

private:
	string m_inputImageFile;
	string m_heightMapFile;
	string m_outputFile;

	InputImageDataInfo* m_topLayerImage;

	std::vector<std::vector<CCellData*>> * m_pCellTable;
	CForest* m_pForest;

private:
	void DeInitialize();

public: 
	bool LoadInputData();
	bool LoadForest();
	bool BuildForest();
	bool OutputResults();
};


