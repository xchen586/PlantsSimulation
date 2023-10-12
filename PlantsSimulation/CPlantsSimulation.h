#pragma once
#include "PsHelper.h"
#include "CCellInfo.h"
#include "CForest.h"
using namespace std;

class CPlantsSimulation
{
public:
	CPlantsSimulation(const string& outputDir, const string& inputImageFile, const string& inputImageMataFile, const string& mesh_HeightMapFile, const string& pc_HeightMapFile, const string& mesh_HeightMasksFile, const string& pc_HeightMasksFile, const string& outputFile, const string& fullOutputFile, const string& pcFullOutputFile )
		: m_outputDir(outputDir)
		, m_inputImageFile(inputImageFile)
		, m_inputImageMetaFile(inputImageMataFile)
		, m_meshHeightMapFile(mesh_HeightMapFile)
		, m_pcHeightMapFile(pc_HeightMapFile)
		, m_meshHeightMasksFile(mesh_HeightMasksFile)
		, m_pcHeightMasksFile(pc_HeightMasksFile)
		, m_outputFile(outputFile)
		, m_fullOutputFile(fullOutputFile)
		, m_pcFullOutputFile(pcFullOutputFile)
		, m_topLayerImage(nullptr)
		, m_topLayerMeta(nullptr)
		, m_pCellTable(nullptr)
		, m_pForest(nullptr)
	{
		/*Tiles = 10;
		TileX = 8;
		TileY = 5;*/
	}

private:
	string m_outputDir;
	string m_inputImageFile;
	string m_inputImageMetaFile;
	string m_meshHeightMapFile;
	string m_pcHeightMapFile;
	string m_meshHeightMasksFile;
	string m_pcHeightMasksFile;
	string m_outputFile;
	string m_fullOutputFile;
	string m_pcFullOutputFile;

	InputImageDataInfo* m_topLayerImage;
	InputImageMetaInfo* m_topLayerMeta;

	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	CForest* m_pForest;

public:
	/*int Tiles;
	int TileX;
	int TileY;*/

private:
	void DeInitialize();
	bool LoadInputImage();
	bool LoadInputHeightMap();
	bool LoadImageMetaFile();
	bool ExportDoubleHeightMap(std::vector<std::vector<double>>& heightMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportShortHeightMap(std::vector<std::vector<short>>& heightMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportShortHeightSlopeMap(std::vector<std::vector<short>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
	bool ExportAngleSlopeMap(std::vector<std::vector<double>>& slopeMap, const string& outputPath, int rgbColor, bool hasHeader, bool withRatio = false);
public:
	bool LoadInputData();
	bool LoadForest();
	bool BuildForest();
	bool OutputResults();
};


