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

bool CPsInstanceExporter::ShouldKeepOldTreeInstances()
{
	return m_isOnlyPoIs && m_isKeepOldTreeFiles;
}

void CPsInstanceExporter::DeInitialize()
{
	for (auto& pair : m_outputTreeMap)
	{
		pair.second.reset(); // Release the shared_ptr to InstanceSubOutputVector
	}
	m_outputTreeMap.clear(); // Now the map itself is empty	
	for (auto& pair : m_outputPoiMap)
	{
		pair.second.reset(); // Release the shared_ptr to InstanceSubOutputVector
	}
	m_outputPoiMap.clear(); // Now the map itself is empty	
}
bool CPsInstanceExporter::OutputAllInstanceGeoChem(string outputFilePath, const InstanceSubOutputMap* pTreeInstances, const InstanceSubOutputMap* pPoiInstances)
{
	std::cout << "Start to OutputAllInstanceGeoChem to : " << outputFilePath << std::endl;

	std::ofstream outputFile(outputFilePath);
	double batch_min_x = m_pMetaInfo->batch_min_x;
	double batch_min_y = m_pMetaInfo->batch_min_y;
	double x0 = m_pMetaInfo->x0;
	double y0 = m_pMetaInfo->y0;

	outputFile << "VX,VY,VZ,InstanceType,Variant,Slope,Index" << std::endl;

	if (pTreeInstances)
	{
		for (auto pair : *pTreeInstances)
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
						<< sub->slopeValue << ","
						<< sub->index << std::endl;
				}
			}
		}
	}
	
	if (pPoiInstances)
	{
		for (auto pair : *pPoiInstances)
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
						<< sub->slopeValue << ","
						<< sub->index << std::endl;
				}
			}
		}
	}
	
	outputFile.close();
	std::cout << "End to OutputAllInstanceGeoChem to : " << outputFilePath << std::endl;

	return true;
}

bool CPsInstanceExporter::loadDungeonsPoiFromCSV(const string& filePath, const string& outputSubDir, InstanceSubOutputMap& outputMap, CAffineTransform transform, double cellSize, int32_t lod)
{
	char delimiter = ',';
	int columnCount = countColumnsInCSV(filePath, delimiter);
	std::cout << "The Dungeon Poi csv file " << filePath << " has " << columnCount << " columns" << std::endl;

	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Failed to open the dungeon csv file :" << filePath << std::endl;
		return false;
	}

	std::string header;
	std::getline(file, header);

	std::string line;

	int instanceValue = -1;
	bool hasInstanceType = false;

	const double xRatio = m_pMetaInfo->xRatio;
	const double yRatio = m_pMetaInfo->yRatio;
	const double batch_min_x = m_pMetaInfo->batch_min_x;
	const double batch_min_y = m_pMetaInfo->batch_min_y;
	const double x0 = m_pMetaInfo->x0;
	const double y0 = m_pMetaInfo->y0;

	const int tableRowsCount = (*m_pCellTable).size();
	const int tableColsCount = (*m_pCellTable)[0].size();

	int negativeHeightCount = 0;
	int index = 0;
	int originalCount = 0;

	unsigned int variant = 0;
	unsigned int slopeValue = 0;

	bool useCellHeight = false;

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::string field;

		originalCount++;

		double xPos = 0.0;
		double yPos = 0.0;
		double zPos = 0.0;

		if (std::getline(lineStream, field, ',')) {
			xPos = std::stod(field);
			//yPos = std::stod(field);
		}
		if (std::getline(lineStream, field, ',')) {
			yPos = std::stod(field);
			//xPos = std::stod(field);
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
		if (columnCount >= 7) //Has dungeon poi type
		{
			if (std::getline(lineStream, field, ',')) {
				instanceValue = std::stoi(field);
				if (instanceValue > 0) {
					hasInstanceType = true;
				}
				else {
					hasInstanceType = false;
					std::cout << "The dungeon poi instance type is not valid : " << instanceValue << std::endl;
				}
			}
		}
		if (columnCount >= 8) //dungeon poi level
		{
			if (std::getline(lineStream, field, ',')) {
				variant = std::stoi(field);
			}
		}
		if (columnCount >= 9) //Has dungeon instance type
		{
			if (std::getline(lineStream, field, ',')) {
				
			}
		}
		if (columnCount >= 10) //Has dungeon instance id
		{
			if (std::getline(lineStream, field, ',')) {
				
			}
		}
		
#if USE_CELLINFO_HEIGHT_FOR_POINT_INSTANCE
		int rowIdx = static_cast<int>((xPos - batch_min_x - x0) / xRatio);
		int colIdx = static_cast<int>((yPos - batch_min_y - y0) / yRatio);

		CCellInfo* pCell = nullptr;
		if (((rowIdx >= 0) && (rowIdx < tableRowsCount))
			&& ((colIdx >= 0) && (colIdx < tableColsCount))) {
			pCell = (*m_pCellTable)[rowIdx][colIdx];
		}
		if ((pCell != nullptr) && (pCell->GetHasHeight()))
		{
			if (useCellHeight) {
				zPos = pCell->GetHeight();
			}
			
			slopeValue = pCell->GetSlopeHeight();
		}

		bool negativeZPos = false;
		if ((zPos < 0))
		{
			negativeZPos = true;
			negativeHeightCount++;
		}
#endif
		//double posX = xPos + batch_min_x + x0;
		//double posY = yPos + batch_min_y + y0;
		double posX = xPos; //Because the dungeon pois are already in the world coordinate system
		double posY = yPos;
		double posZ = zPos;
		
		std::shared_ptr<PointInstanceSubOutput> sub = std::make_shared<PointInstanceSubOutput>();
		SetupInstanceSubOutput(posX, posY, posZ, transform, cellSize, lod, sub);
		sub->instanceType = static_cast<unsigned int>(instanceValue);
		sub->variant = variant;
		sub->age = 1.0;
		index++;
		sub->index = index;
		sub->MakeIdString();
		sub->slopeValue = slopeValue;
		
		string keyString = GetKeyStringForInstance(outputSubDir, sub->cellXIdx, sub->cellZIdx);
		InstanceSubOutputMap::iterator iter = outputMap.find(keyString);
		if (outputMap.end() == iter)
		{
			outputMap[keyString] = std::make_shared<InstanceSubOutputVector>();
		}

		std::shared_ptr<InstanceSubOutputVector> subVector = outputMap[keyString];
		subVector->push_back(sub);
		
	}

	std::cout << "The dungeons pois of negative height count is : " << negativeHeightCount << std::endl;

	file.close();
	return true;
}

bool CPsInstanceExporter::loadPointInstanceFromCSV(const string& filePath, const string& outputSubDir, InstanceSubOutputMap& outputMap, CAffineTransform transform, double cellSize, int32_t lod, InstanceType instanceType, bool canRemovedFromCave = false, bool isLevel1POI = false)
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

	const double xRatio = m_pMetaInfo->xRatio;
	const double yRatio = m_pMetaInfo->yRatio;
	const double batch_min_x = m_pMetaInfo->batch_min_x;
	const double batch_min_y = m_pMetaInfo->batch_min_y;
	const double x0 = m_pMetaInfo->x0;
	const double y0 = m_pMetaInfo->y0;

	const int tableRowsCount = (*m_pCellTable).size();
	const int tableColsCount = (*m_pCellTable)[0].size();

	int negativeHeightCount = 0;
	int index = 0;
	int originalCount = 0;

	unsigned int variant = 0;
	unsigned int slopeValue = 0;
	PointType pointType = PointType::Point_None; // 0 for most travelled, 1 for most distant, 2 for centroid
	int nPointType = static_cast<int>(pointType);

	while (std::getline(file, line)) {
		std::stringstream lineStream(line);
		std::string field;

		originalCount++;

		double xPos = 0.0;
		double yPos = 0.0;
		double zPos = 0.0;

		double roadDistance = 0.0;

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
		if (columnCount >= 9) //Has slope value column
		{
			if (std::getline(lineStream, field, ',')) {
				slopeValue = std::stoi(field);
			}
		}
		if (columnCount >= 10) //Has poi type column
		{
			if (std::getline(lineStream, field, ',')) {
				nPointType = std::stoi(field);
				pointType = static_cast<PointType>(nPointType);
			}
		}
		if (columnCount >= 11) //Has road instance column
		{
			if (std::getline(lineStream, field, ',')) {
				roadDistance = std::stod(field);
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
		double posX = xPos + batch_min_x + x0;//Because the position in the pois(road) csv is local coordinate system, it neeeds to be converted to world coordinate system
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
			sub->slopeValue = slopeValue;
			if ((pointType != PointType::Point_None) && (static_cast<unsigned int>(instanceType) != static_cast<unsigned int>(pointType)))
			{
				std::cout << "The instanceType is not equal to pointType for POI at index : " << index << "  instanceType : " << static_cast<unsigned int>(instanceType) << "  pointType : " << static_cast<unsigned int>(pointType) << std::endl;
			}
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
			std::cerr << "Failed to create the outputSubsDir directory of " << outputSubsDir << std::endl;
			return false;
		}
	}
	if (!ShouldKeepOldTreeInstances())
	{
		bool removeFiles = RemoveAllFilesInFolder(outputSubsDir);
	}

	const int MAX_PATH = 250;
	char subFullOutput_Dir_Tree[MAX_PATH];
	memset(subFullOutput_Dir_Tree, 0, sizeof(char) * MAX_PATH);
	char subFullOutput_Dir_Poi[MAX_PATH];
	memset(subFullOutput_Dir_Poi, 0, sizeof(char) * MAX_PATH);
#if __APPLE__ 
	snprintf(subFullOutput_Dir_Tree, MAX_PATH, "%s/Trees", outputSubsDir.c_str());
	snprintf(subFullOutput_Dir_Poi, MAX_PATH, "%s/POIs", outputSubsDir.c_str());
#else
	sprintf_s(subFullOutput_Dir_Tree, MAX_PATH, "%s/Trees", outputSubsDir.c_str());
	sprintf_s(subFullOutput_Dir_Poi, MAX_PATH, "%s/POIs", outputSubsDir.c_str());
#endif

	if (!std::filesystem::exists(subFullOutput_Dir_Tree)) {
		if (!std::filesystem::create_directory(subFullOutput_Dir_Tree)) {
			std::cerr << "Failed to create the subFullOutput_Dir_Tree directory of " << subFullOutput_Dir_Tree << std::endl;
			return false;
		}
	}
	if (!ShouldKeepOldTreeInstances())
	{
		bool removeTreeFiles = RemoveAllFilesInFolder(subFullOutput_Dir_Tree);
	}
	
	if (!std::filesystem::exists(subFullOutput_Dir_Poi)) {
		if (!std::filesystem::create_directory(subFullOutput_Dir_Poi)) {
			std::cerr << "Failed to create the subFullOutput_Dir_Poi directory of " << subFullOutput_Dir_Poi << std::endl;
			return false;
		}
	}
	bool removePoiFiles = RemoveAllFilesInFolder(subFullOutput_Dir_Poi);

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

	if (!m_isOnlyPoIs)
	{
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
			InstanceType instanceType = m_isLevel1Instances ? InstanceType::InstanceType_Tree_Level1 : InstanceType::InstanceType_Tree;
			sub->instanceType = static_cast<unsigned int>(instanceType);
			sub->variant = instance.m_instance.treeType;
			sub->age = static_cast<double>(instance.m_instance.age / instance.m_instance.maxAge);
			sub->slopeValue = instance.slopeValue;
			sub->MakeIdString();

			string keyString = GetKeyStringForInstance(subFullOutput_Dir_Tree, sub->cellXIdx, sub->cellZIdx);
			InstanceSubOutputMap::iterator iter = m_outputTreeMap.find(keyString);
			if (m_outputTreeMap.end() == iter)
			{
				m_outputTreeMap[keyString] = std::make_shared<InstanceSubOutputVector>();
			}

			std::shared_ptr<InstanceSubOutputVector> subVector = m_outputTreeMap[keyString];
			subVector->push_back(sub);
		}
		std::cout << "The trees of negative height count is : " << negativeHeightCount << std::endl;
	}
	
	if (!m_isLevel1Instances)
	{
		//unsigned int mostTravelledVariant = static_cast<unsigned int>(PointType::Point_MostTravelled);
		bool getMostTravelledPoint = loadPointInstanceFromCSV(m_mostTravelledPointFilePath, subFullOutput_Dir_Poi, m_outputPoiMap, transform, cellSize, m_lod, InstanceType::InstanceType_NPC);
		//unsigned int mostDistantVariant = static_cast<unsigned int>(PointType::Point_MostDistant);
		bool getMostDistantPoint = loadPointInstanceFromCSV(m_mostDistantPointFilePath, subFullOutput_Dir_Poi, m_outputPoiMap, transform, cellSize, m_lod, InstanceType::InstanceType_Resource);
		bool getCentroidPoint = loadPointInstanceFromCSV(m_centroidPointFilePath, subFullOutput_Dir_Poi, m_outputPoiMap, transform, cellSize, m_lod, InstanceType::InstanceType_Spawn_Point, false);
		int beforeMergeDungeonPoiCount = GetInstancesCountFromInstanceSubOutputMap(m_outputPoiMap);
		std::cout << "Before merge dungeon poi count is : " << beforeMergeDungeonPoiCount << std::endl;
		bool getDungeonPoi = loadDungeonsPoiFromCSV(m_dungeonsPoiCsvLevel0Path, subFullOutput_Dir_Poi, m_outputPoiMap, transform, cellSize, m_lod);
		int afterMergeDungeonPoiCount = GetInstancesCountFromInstanceSubOutputMap(m_outputPoiMap);
		std::cout << "loadDungeonsPoiFromCSV is : " << getDungeonPoi << std::endl;
		std::cout << "After merge dungeon poi count is : " << afterMergeDungeonPoiCount << std::endl;
	}
	
	std::filesystem::path outputSubsDirPath = outputSubsDir;
	std::filesystem::path outputDirPath = outputSubsDirPath.parent_path();

	int level = m_isLevel1Instances ? 1 : 0;
	std::string geoLevelFolder = std::format("GeoChemical_Level_{}", level);
	string geoLevelFolderTree = std::format("GeoChemical_Level_{}_Trees", level);
	string geoLevelFolderPoi = std::format("GeoChemical_Level_{}_POIs", level);
	
#if __APPLE__
	std::string allinstances_csv_file = std::format("{}//{}_{}_{}_allinstances_level{}.csv", outputDirPath.string(), m_tiles, m_tileIndexX, m_tileIndexY, level);
	std::string allinstancesGeo_folder = std::format("{}//{}", outputDirPath.string(), geofolder);
	std::string allinstancesGeo_Csv = std::format("{}//{}//{}_{}_{}_geo_merged.csv", outputDirPath.string(), geofolder, m_tiles, m_tileIndexX, m_tileIndexY);
	std::string allinstancesGeo_Tree_folder = std::format("{}//{}", outputDirPath.string(), geofolderTree);
	std::string allinstancesGeo_Tree_Csv = std::format("{}//{}//{}_{}_{}_geo_merged.csv", outputDirPath.string(), geofolderTree, m_tiles, m_tileIndexX, m_tileIndexY);
	std::string allinstancesGeo_Poi_folder = std::format("{}//{}", outputDirPath.string(), geofolderPoi);
	std::string allinstancesGeo_Poi_Csv = std::format("{}//{}//{}_{}_{}_geo_merged.csv", outputDirPath.string(), geofolderPoi, m_tiles, m_tileIndexX, m_tileIndexY);
#else
	std::string allinstances_csv_file = std::format("{}\\{}_{}_{}_allinstances_level{}.csv", outputDirPath.string(), m_tiles, m_tileIndexX, m_tileIndexY, level);
	std::string allinstancesGeo_folder = std::format("{}\\{}", outputDirPath.string(), geoLevelFolder);
	std::string allinstancesGeo_Csv = std::format("{}\\{}\\{}_{}_{}_geo_merged.csv", outputDirPath.string(), geoLevelFolder, m_tiles, m_tileIndexX, m_tileIndexY);
	std::string allinstancesGeo_Tree_folder = std::format("{}\\{}", outputDirPath.string(), geoLevelFolderTree);
	std::string allinstancesGeo_Tree_Csv = std::format("{}\\{}\\{}_{}_{}_geo_merged.csv", outputDirPath.string(), geoLevelFolderTree, m_tiles, m_tileIndexX, m_tileIndexY);
	std::string allinstancesGeo_Poi_folder = std::format("{}\\{}", outputDirPath.string(), geoLevelFolderPoi);
	std::string allinstancesGeo_Poi_Csv = std::format("{}\\{}\\{}_{}_{}_geo_merged.csv", outputDirPath.string(), geoLevelFolderPoi, m_tiles, m_tileIndexX, m_tileIndexY);
#endif

	if (!std::filesystem::exists(allinstancesGeo_Tree_folder)) {
		if (!std::filesystem::create_directory(allinstancesGeo_Tree_folder)) {
			std::cerr << "Failed to create the directory of allinstancesGeo_Tree_folder: " << allinstancesGeo_Tree_folder << std::endl;
			return false;
		}
	}
	if (!ShouldKeepOldTreeInstances())
	{
		bool removeTreeChemicalFiles = RemoveAllFilesInFolder(allinstancesGeo_Tree_folder);
	}
	if (!std::filesystem::exists(allinstancesGeo_Poi_folder)) {
		if (!std::filesystem::create_directory(allinstancesGeo_Poi_folder)) {
			std::cerr << "Failed to create the directory of allinstancesGeo_Poi_folder: " << allinstancesGeo_Poi_folder << std::endl;
			return false;
		}
	}

	bool outputAll = OutputAllInstance(allinstances_csv_file, &m_outputTreeMap, &m_outputPoiMap);

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

	bool allTreeOk = true;
	if (!m_isOnlyPoIs) {
		std::vector<std::future<bool>> treeResults;
		// Launch threads using std::async for trees
		for (const auto& pair : m_outputTreeMap) {
			treeResults.emplace_back(std::async(std::launch::async, OutputCSVFileForSubInstances, pair.first, pair.second));
		}
		// Check results
		for (auto& future : treeResults) {
			if (!future.get()) {  // .get() waits for the result and retrieves it
				std::cout << "OutputCSVFileForSubInstances Tree Processing failed!" << std::endl;
				allTreeOk = false;
			}
		}
	}
	
	std::vector<std::future<bool>> poiResults;
	// Launch threads for POIs
	for (const auto& pair : m_outputPoiMap) {
		poiResults.emplace_back(std::async(std::launch::async, OutputCSVFileForSubInstances, pair.first, pair.second));
	}
	// Check results
	bool allPoiOk = true;
	for (auto& future : poiResults) {
		if (!future.get()) {  // .get() waits for the result and retrieves it
			std::cout << "OutputCSVFileForSubInstances POI Processing failed!" << std::endl;
			allPoiOk = false;
		}
	}

	if (!std::filesystem::exists(allinstancesGeo_folder)) {
		if (!std::filesystem::create_directory(allinstancesGeo_folder)) {
			std::cerr << "Failed to create the directory of allinstancesGeo_folder: " << allinstancesGeo_folder << std::endl;
			return false;
		}
	}

	bool outputGeoTree = true;
	if (!m_isOnlyPoIs)
	{
		std::cout << "Start OutputAllInstanceGeoChem For Tree : " << allinstancesGeo_Tree_Csv << std::endl;
		outputGeoTree = OutputAllInstanceGeoChem(allinstancesGeo_Tree_Csv, &m_outputTreeMap, nullptr);
		std::cout << "End OutputAllInstanceGeoChem For Tree : " << allinstancesGeo_Tree_Csv << std::endl;
	}

	std::cout << "Start OutputAllInstanceGeoChem For POIs : " << allinstancesGeo_Poi_Csv << std::endl;
	bool outputGeoPoi = OutputAllInstanceGeoChem(allinstancesGeo_Poi_Csv, nullptr, &m_outputPoiMap);
	std::cout << "End OutputAllInstanceGeoChem For POIs : " << allinstancesGeo_Poi_Csv << std::endl;

	bool outputGeo = outputGeoTree && outputGeoPoi;
	/*std::cout << "Start OutputAllInstanceGeoChem : " << allinstancesGeo_Csv << std::endl;
	outputGeo = OutputAllInstanceGeoChem(allinstancesGeo_Csv, &m_outputTreeMap, &m_outputPoiMap);
	std::cout << "End OutputAllInstanceGeoChem : " << allinstancesGeo_Csv << std::endl;*/

	std::cout << "End to CPsInstanceExporter::outputSubfiles to : " << outputSubsDir << std::endl;

	return allTreeOk && allPoiOk && outputGeo; 
	/*std::ofstream outputFile(subFilePath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the subFilePath file " << subFilePath << std::endl;
		return false;
	}*/

	return true;
}