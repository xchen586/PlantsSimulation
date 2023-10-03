#pragma once
#include "PsHelper.h"
#include "CCellInfo.h"
#include "CForest.h"
using namespace std;

class CPlantsSimulation
{
public:
	CPlantsSimulation(const string& inputImageFile, const string& inputImageMataFile, const string& mesh_HeightMapFile, const string& pc_HeightMapFile, const string& outputFile, const string& fullOutputFile, const string& pcFullOutputFile )
		: m_inputImageFile(inputImageFile)
		, m_inputImageMetaFile(inputImageMataFile)
		, m_meshHeightMapFile(mesh_HeightMapFile)
		, m_pcHeightMapFile(pc_HeightMapFile)
		, m_outputFile(outputFile)
		, m_fullOutputFile(fullOutputFile)
		, m_pcFullOutputFile(pcFullOutputFile)
		, m_topLayerImage(nullptr)
		, m_topLayerMeta(nullptr)
		, m_pCellTable(nullptr)
		, m_pForest(nullptr)
	{

	}

private:
	string m_inputImageFile;
	string m_inputImageMetaFile;
	string m_meshHeightMapFile;
	string m_pcHeightMapFile;
	string m_outputFile;
	string m_fullOutputFile;
	string m_pcFullOutputFile;

	InputImageDataInfo* m_topLayerImage;
	InputImageMetaInfo* m_topLayerMeta;

	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
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


