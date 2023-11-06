#pragma once

#include "CForest.h"
#include "CPsInstanceExporter.h"

#if __APPLE__
    #include "../Common/include/PsHelper.h"
    #include "../Common/include/CCellInfo.h"
#else
    #include "..\Common\include\PsHelper.h"
    #include "..\Common\include\CCellInfo.h"
#endif

using namespace std;

class CPlantsSimulation
{
public:
	CPlantsSimulation(const string& outputDir, const string& inputImageFile, const string& inputImageMataFile, const string& mesh_HeightMapFile, const string& mesh2_HeightMapFile, const string& pc_HeightMapFile, const string& l1_HeightMapFile,
		const string& mesh_HeightMasksFile, const string& mesh2_HeightMasksFile, const string& pc_HeightMasksFile, const string& l1_HeightMasksFile, const string& mostTravelledPointFile, const string& mostDistantPointFile, 
		const string& outputFile, const string& fullOutputFile, const string& pcFullOutputFile, int32_t lod )
		: m_outputDir(outputDir)
		, m_inputImageFile(inputImageFile)
		, m_inputImageMetaFile(inputImageMataFile)
		, m_meshHeightMapFile(mesh_HeightMapFile)
		, m_mesh2HeightMapFile(mesh2_HeightMapFile)
		, m_pcHeightMapFile(pc_HeightMapFile)
		, m_l1HeightMapFile(l1_HeightMapFile)
		, m_meshHeightMasksFile(mesh_HeightMasksFile)
		, m_mesh2HeightMasksFile(mesh2_HeightMasksFile)
		, m_pcHeightMasksFile(pc_HeightMasksFile)
		, m_l1HeightMasksFile(l1_HeightMasksFile)
		, m_mostTravelledPointFile(mostTravelledPointFile)
		, m_mostDistantPointFile(mostDistantPointFile)
		, m_outputFile(outputFile)
		, m_fullOutputFile(fullOutputFile)
		, m_pcFullOutputFile(pcFullOutputFile)
		, m_topLayerImage(nullptr)
		, m_topLayerMeta(nullptr)
		, m_pCellTable(nullptr)
		, m_pForest(nullptr)
		, m_currentLod(lod)
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
	string m_mesh2HeightMapFile;
	string m_pcHeightMapFile;
	string m_l1HeightMapFile;
	string m_meshHeightMasksFile;
	string m_mesh2HeightMasksFile;
	string m_pcHeightMasksFile;
	string m_l1HeightMasksFile;
	string m_mostTravelledPointFile;
	string m_mostDistantPointFile;
	string m_outputFile;
	string m_fullOutputFile;
	string m_pcFullOutputFile;

	int32_t m_currentLod;

	InputImageDataInfo* m_topLayerImage;
	InputImageMetaInfo* m_topLayerMeta;

	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	CForest* m_pForest;
	CPsInstanceExporter* m_pInstanceExporter;

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
	bool LoadInstanceExporter();
	bool BuildForest();
	bool OutputResults();
};


