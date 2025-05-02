#include "CPsInstanceExporter.h"
#include "CAppFuncs.h"

#include <iostream>
#include <fstream>
#include <filesystem> 
#include <thread>
#include <future>

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

void CPsInstanceExporter::DeInitialize()
{
	for (auto& pair : m_outputMap)
	{
		pair.second.reset(); // Release the shared_ptr to InstanceSubOutputVector
	}
	m_outputMap.clear(); // Now the map itself is empty	
}
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

bool CPsInstanceExporter::loadPointInstanceFromCSV(const string& filePath, const string& outputSubDir, InstanceSubOutputMap& outputMap, CAffineTransform transform, double cellSize, int32_t lod, InstanceType instanceType, bool canRemovedFromCave = true)
{
	char delimiter = ',';
	int columnCount = countColumnsInCSV(filePath, delimiter);
	std::cout << "The PointInstance csv file " << filePath << " has " << columnCount << " columns" << std::endl;

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
	int originalCount = 0;

	unsigned int variant = 0;

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::string field;

		originalCount++;

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
		if (columnCount >= 7) //Has regionId column
		{
			if (std::getline(lineStream, field, ',')) {

			}
		}
		if (columnCount >= 8) //Has resource type (or variant) column
		{
			if (std::getline(lineStream, field, ',')) {
				variant = std::stoi(field);
			}
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

		bool beRemovedFromCave = false;
		if (m_p2dCaveLevel0Nodes && canRemovedFromCave)
		{
			Point p(xPos, yPos);
			double distance = GetDistanceToCaveNodes(p, m_p2dCaveLevel0Nodes, CAVE_DISTANCE_LIMIT_POI);
			beRemovedFromCave = (distance < CAVE_DISTANCE_LIMIT_POI) ? true : false;
			if (beRemovedFromCave)
			{
				//std::cout << "Remove the POI from Cave x : " << xPos << "  y : " << yPos << std::endl;
			}
		}
		if (hasHeight && (!beRemovedFromCave))
		{
			std::shared_ptr<PointInstanceSubOutput> sub = std::make_shared<PointInstanceSubOutput>();
			SetupInstanceSubOutput(posX, posY, posZ, transform, cellSize, lod, sub);
			sub->instanceType = static_cast<unsigned int>(instanceType);
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

	std::cout << "The original count of POI in " << filePath << " is : " << originalCount << std::endl;
	std::cout << "After the reomve from Caves Node count of POI in " << filePath << " is : " << index << std::endl;
	
	double percentageCount = static_cast<double>(100 * index / originalCount);
	std::cout << "For file " << filePath << " After Cave removal the rest of POI has pencentage of " << percentageCount << " before POI count!" << std::endl;

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
		InstanceType instanceType = m_isLevel1Instances ? InstanceType::IntanceType_Tree_level1 : InstanceType::InstanceType_Tree;
		sub->instanceType = static_cast<unsigned int>(instanceType);
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
	if (!m_isLevel1Instances)
	{
		//unsigned int mostTravelledVariant = static_cast<unsigned int>(PointType::Point_MostTravelled);
		bool getMostTravelledPoint = loadPointInstanceFromCSV(m_mostTravelledPointFilePath, outputSubsDir, m_outputMap, transform, cellSize, m_lod, InstanceType::InstanceType_NPC);
		//unsigned int mostDistantVariant = static_cast<unsigned int>(PointType::Point_MostDistant);
		bool getMostDistantPoint = loadPointInstanceFromCSV(m_mostDistantPointFilePath, outputSubsDir, m_outputMap, transform, cellSize, m_lod, InstanceType::InstanceType_Resource);
		bool getCentroidPoint = loadPointInstanceFromCSV(m_centroidPointFilePath, outputSubsDir, m_outputMap, transform, cellSize, m_lod, InstanceType::InstanceType_spawn_Point, false);
	}
	
	std::filesystem::path outputSubsDirPath = outputSubsDir;
	std::filesystem::path outputDirPath = outputSubsDirPath.parent_path();

	string geofolder = "GeoChemical";
	int level = m_isLevel1Instances ? 1 : 0;
	std::string allinstances_file = std::format("{}\\{}_{}_{}_allinstances_level{}.csv", outputDirPath.string(), m_tiles, m_tileIndexX, m_tileIndexY, level);
	std::string allinstancesGeo_folder = std::format("{}\\{}", outputDirPath.string(), geofolder);
	std::string allinstancesGeo_Csv = std::format("{}\\{}\\{}_{}_{}_{}_geo_merged.csv", outputDirPath.string(), geofolder, m_tiles, m_tileIndexX, m_tileIndexY, level);
	bool outputAll = OutputAllInstance(allinstances_file, m_outputMap);

	/*std::vector<std::thread> workers;
	for (const auto& pair : m_outputMap)
	{
		workers.emplace_back(std::thread(OutputCSVFileForSubInstances, pair.first, pair.second));
	}
	// Join all the threads to wait for them to finish
	for (std::thread& t : workers) {
		t.join();
	}
	// Use the function results as needed
	bool allOk = true;
	for (const std::thread& t : workers) {
		bool result = t.joinable(); // Replace with actual result retrieval logic: 
		//std::thread::joinable() only checks if a thread can still be joined (i.e., it hasn't been joined or detached yet). It does not return the function's result. After joining, all threads in workers are no longer joinable.
		if (result) {
			// Do something when the function returns true
		}
		else {
			// Do something when the function returns false
			std::cout << "Wait for join threads of OutputCSVFileForSubInstances is failed!" << std::endl;
			allOk = false;
		}
	}*/

	std::vector<std::future<bool>> results;

	// Launch threads using std::async
	for (const auto& pair : m_outputMap) {
		results.emplace_back(std::async(std::launch::async, OutputCSVFileForSubInstances, pair.first, pair.second));
	}

	// Check results
	bool allOk = true;
	for (auto& future : results) {
		if (!future.get()) {  // .get() waits for the result and retrieves it
			std::cout << "OutputCSVFileForSubInstances Processing failed!" << std::endl;
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
	std::cout << "End OutputAllInstanceGeoChem : " << allinstancesGeo_Csv << std::endl;

	std::cout << "End to CPsInstanceExporter::outputSubfiles to : " << outputSubsDir << std::endl;

	return allOk && outputGeo; 
	/*std::ofstream outputFile(subFilePath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the subFilePath file " << subFilePath << std::endl;
		return false;
	}*/

	return true;
}