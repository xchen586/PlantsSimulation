#pragma once
#include "PsHelper.h"
#include "CCellData.h"
#include "CForest.h"
using namespace std;

class CPlantsSimulation
{
public:
	CPlantsSimulation(const string& inputImageFile, const string& inputImageMataFile, const string& heightMapFile, const string outputFile, const string fullOutputFile)
		: m_inputImageFile(inputImageFile)
		, m_inputImageMetaFile(inputImageMataFile)
		, m_heightMapFile(heightMapFile)
		, m_outputFile(outputFile)
		, m_fullOutputFile(fullOutputFile)
		, m_topLayerImage(nullptr)
		, m_pCellTable(nullptr)
		, m_pForest(nullptr)
	{

	}

private:
	string m_inputImageFile;
	string m_inputImageMetaFile;
	string m_heightMapFile;
	string m_outputFile;
	string m_fullOutputFile;

	InputImageDataInfo* m_topLayerImage;
	InputImageMetaInfo* m_topLayerMeta;

	std::vector<std::vector<CCellData*>>* m_pCellTable;
	CForest* m_pForest;

private:
	void DeInitialize();
	bool LoadInputImage();
	bool LoadInputHeightMap();
	bool LoadImageMetaFile();
public:
	bool LoadInputData();
	bool LoadForest();
	bool BuildForest();
	bool OutputResults();
};


