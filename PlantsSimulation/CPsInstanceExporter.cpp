#include "CPsInstanceExporter.h"
#include "CAppFuncs.h"

#include <iostream>
#include <fstream>
#include <filesystem> 
#include <thread>

#if __APPLE__
#include "../Common/include/PsMarco.h"
#include "../Common/include/PsHelper.h"
#include "../Common/include/CCellInfo.h"
#include "../Common/Include/PointInstance.h"
#else
#include "..\Common\include\PsMarco.h"
#include "..\Common\include\PsHelper.h"
#include "..\Common\include\CCellInfo.h"
#include "..\Common\Include\PointInstance.h"
#endif

bool CPsInstanceExporter::OutputAllInstanceGeoChem(string outputFilePath, const InstanceSubOutputMap& allInstances)
{
	std::cout << "Start to OutputAllInstanceGeoChem to : " << outputFilePath << std::endl;

	std::ofstream outputFile(outputFilePath);
	double batch_min_x = m_pMetaInfo->batch_min_x;
	double batch_min_y = m_pMetaInfo->batch_min_y;
	double x0 = m_pMetaInfo->x0;
	double y0 = m_pMetaInfo->y0;

	outputFile << "VX,VY,VZ,InstanceType,Variant,Index" << std::endl;

	for (auto pair : allInstances)
	{
		std::shared_ptr<InstanceSubOutputVector> subVector = pair.second;
		for (const std::shared_ptr<InstanceSubOutput>& sub : *subVector)
		{
			//if (sub->posZ > 0)
			{
				int outputItemCount = sub->outputItemCount;
				outputFile
#if 0
					<< sub->vX << ","
					<< sub->vY << ","
					<< sub->vZ << ","
#endif
					<< sub->posX << ","
					<< sub->posY << ","
					<< sub->posZ << ","
					<< sub->instanceType << ","
					<< sub->variant << ","
					<< sub->index << std::endl;
			}
		}
	}
	outputFile.close();
	std::cout << "End to OutputAllInstanceGeoChem to : " << outputFilePath << std::endl;

	return true;
}

bool CPsInstanceExporter::loadPointInstanceFromCSV(const string& filePath, const string& outputSubDir, InstanceSubOutputMap& outputMap, unsigned int variant, CAffineTransform transform, double cellSize, int32_t lod)
{
	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open the csv file :" << filePath << std::endl;
		return false;
	}

	std::string header;
	std::getline(file, header);

	std::string line;

	double xRatio = m_pMetaInfo->xRatio;
	double yRatio = m_pMetaInfo->yRatio;
	double batch_min_x = m_pMetaInfo->batch_min_x;
	double batch_min_y = m_pMetaInfo->batch_min_y;
	double x0 = m_pMetaInfo->x0;
	double y0 = m_pMetaInfo->y0;

	int tableRowsCount = (*m_pCellTable).size();
	int tableColsCount = (*m_pCellTable)[0].size();

	int negativeHeightCount = 0;
	int index = 0;

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::string field;

		double xPos = 0.0;
		double yPos = 0.0;
		double zPos = 0.0;

		bool hasHeight = true;
		if (std::getline(lineStream, field, ',')) {
			//xPos = std::stod(field);
			yPos = std::stod(field);
		}
		if (std::getline(lineStream, field, ',')) {
			//yPos = std::stod(field);
			xPos = std::stod(field);
		}
		if (std::getline(lineStream, field, ',')) {
			zPos = std::stod(field);
		}
		if (std::getline(lineStream, field, ',')) {

		}
		if (std::getline(lineStream, field, ',')) {

		}
		if (std::getline(lineStream, field, ',')) {

		}
#if USE_CELLINFO_HEIGHT_FOR_POINT_INSTANCE
		int rowIdx = static_cast<int>(xPos / xRatio);
		int colIdx = static_cast<int>(yPos / yRatio);

		CCellInfo* pCell = nullptr;
		if (((rowIdx >= 0) && (rowIdx < tableRowsCount))
			&& ((colIdx >= 0) && (colIdx < tableColsCount))) {
			pCell = (*m_pCellTable)[rowIdx][colIdx];
		}
		if ((pCell != nullptr) && (pCell->GetHasHeight()))
		{
			zPos = pCell->GetHeight();
		}
		else
		{
			hasHeight = false;
		}

		bool negativeZPos = false;
		if ((zPos < 0) && hasHeight)
		{
			negativeZPos = true;
			negativeHeightCount++;
		}
#endif
		double posX = xPos + batch_min_x + x0;
		double posY = yPos + batch_min_y + y0;
		double posZ = zPos;
		//double posZ = zPos ? zPos : 0;

		if (hasHeight)
		{
			std::shared_ptr<PointInstanceSubOutput> sub = std::make_shared<PointInstanceSubOutput>();
			SetupInstanceSubOutput(posX, posY, posZ, transform, cellSize, lod, sub);
			sub->instanceType = static_cast<unsigned int>(InstanceType::InstanceType_Point);
			sub->variant = variant;
			sub->age = 1.0;
			index++;
			sub->index = index;
			sub->MakeIdString();

			string keyString = GetKeyStringForInstance(outputSubDir, sub->cellXIdx, sub->cellZIdx);
			InstanceSubOutputMap::iterator iter = outputMap.find(keyString);
			if (outputMap.end() == iter)
			{
				outputMap[keyString] = std::make_shared<InstanceSubOutputVector>();
			}

			std::shared_ptr<InstanceSubOutputVector> subVector = outputMap[keyString];
			subVector->push_back(sub);
		}
	}

	std::cout << "The points of negative height count is : " << negativeHeightCount << std::endl;

	file.close();
    return true;
}

bool CPsInstanceExporter::outputSubfiles(const std::string& outputSubsDir)
{
	std::cout << "Start to CPsInstanceExporter::outputSubfiles to : " << outputSubsDir << std::endl;

	if (!m_pFullTreeOutputs)
	{
		return false;
	}

	if (!std::filesystem::exists(outputSubsDir)) {
		if (!std::filesystem::create_directory(outputSubsDir)) {
			std::cerr << "Failed to create the directory of outputSubsDir!" << std::endl;
			return false;
		}
	}
	bool removeFiles = RemoveAllFilesInFolder(outputSubsDir);

	//int lod = 5;
	//auto lod = VoxelFarm::LOD_0 + 3;
	//auto m_lod = VoxelFarm::LOD_0 + 6;

	const double cellSize = (1 << m_lod) * VoxelFarm::CELL_SIZE;
	//const double voxelSize = cellSize / VoxelFarm::BLOCK_DIMENSION;// 
	const double voxelSize = 2.0;// cellSize / VoxelFarm::BLOCK_DIMENSION;

	CAffineTransform transform(
		CAffineTransform::sAffineVector{
			0.0, 0.0, 0.0
		}, voxelSize, CAffineTransform::eTransformMode::TM_YZ_ROTATE);
	auto worldOriginVF = transform.WC_TO_VF(CAffineTransform::sAffineVector{ 0.0, 0.0, 0.0 });

	int negativeHeightCount = 0;
	for (const TreeInstanceFullOutput& instance : (*m_pFullTreeOutputs))
	{
		bool negativeHeight = false;
		if (instance.posZ < 0)
		{
			negativeHeight = true;
			negativeHeightCount++;
		}
		std::shared_ptr<TreeInstanceSubOutput> sub = std::make_shared<TreeInstanceSubOutput>();
		SetupInstanceSubOutput(instance.posX, instance.posY, instance.posZ, transform, cellSize, m_lod, sub);

		sub->index = instance.index;
		sub->instanceType = static_cast<unsigned int>(InstanceType::InstanceType_Tree);
		sub->variant = instance.m_instance.treeType;
		sub->age = static_cast<double>(instance.m_instance.age / instance.m_instance.maxAge);
		sub->MakeIdString();

		string keyString = GetKeyStringForInstance(outputSubsDir, sub->cellXIdx, sub->cellZIdx);
		InstanceSubOutputMap::iterator iter = m_outputMap.find(keyString);
		if (m_outputMap.end() == iter)
		{
			m_outputMap[keyString] = std::make_shared<InstanceSubOutputVector>();
		}

		std::shared_ptr<InstanceSubOutputVector> subVector = m_outputMap[keyString];
		subVector->push_back(sub);
	}
	std::cout << "The trees of negative height count is : " << negativeHeightCount << std::endl;
	unsigned int mostTravelledVariant = static_cast<unsigned int>(PointType::Point_MostTravelled);
	bool getMostTravelledPoint = loadPointInstanceFromCSV(m_mostTravelledPointFilePath, outputSubsDir, m_outputMap, mostTravelledVariant, transform, cellSize, m_lod);
	unsigned int mostDistantVariant = static_cast<unsigned int>(PointType::Point_MostDistant);
	bool getMostDistantPoint = loadPointInstanceFromCSV(m_mostDistantPointFilePath, outputSubsDir, m_outputMap, mostDistantVariant, transform, cellSize, m_lod);

	std::filesystem::path outputSubsDirPath = outputSubsDir;
	std::filesystem::path outputDirPath = outputSubsDirPath.parent_path();

	string geofolder = "GeoChemical";
	std::string allinstances_file = std::format("{}\\{}_{}_{}_allinstances.csv", outputDirPath.string(), m_tiles, m_tileIndexX, m_tileIndexY);
	std::string allinstancesGeo_folder = std::format("{}\\{}", outputDirPath.string(), geofolder);
	std::string allinstancesGeo_Csv = std::format("{}\\{}\\{}_{}_{}_geo_merged.csv", outputDirPath.string(), geofolder, m_tiles, m_tileIndexX, m_tileIndexY);
	bool outputAll = OutputAllInstance(allinstances_file, m_outputMap);

	std::vector<std::thread> workers;
	for (const auto& pair : m_outputMap)
	{
		workers.emplace_back(std::thread(OutputCSVFileForSubInstances, pair.first, pair.second));
	}
	/*for (int i = 0; i < workers.size() - 1; i++)
	{
		workers[i].join();
	}
	workers.back().join();*/
	// Join all the threads to wait for them to finish
	for (std::thread& t : workers) {

		t.join();
	}

	// Use the function results as needed
	bool allOk = true;
	for (const std::thread& t : workers) {
		bool result = t.joinable(); // Replace with actual result retrieval logic
		if (result) {
			// Do something when the function returns true
		}
		else {
			// Do something when the function returns false
			allOk = false;
		}
	}

	if (!std::filesystem::exists(allinstancesGeo_folder)) {
		if (!std::filesystem::create_directory(allinstancesGeo_folder)) {
			std::cerr << "Failed to create the directory of allinstancesGeo_folder: " << allinstancesGeo_folder << std::endl;
			return false;
		}
	}
	std::cout << "Start OutputAllInstanceGeoChem : " << allinstancesGeo_Csv << std::endl;
	bool outputGeo = true;
	outputGeo = OutputAllInstanceGeoChem(allinstancesGeo_Csv, m_outputMap);
	std::cout << "Start OutputAllInstanceGeoChem : " << allinstancesGeo_Csv << std::endl;

	std::cout << "End to CPsInstanceExporter::outputSubfiles to : " << outputSubsDir << std::endl;

	return allOk && outputGeo; 
	/*std::ofstream outputFile(subFilePath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the subFilePath file " << subFilePath << std::endl;
		return false;
	}*/

	return true;
}