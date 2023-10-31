#include "CPsInstanceExporter.h"

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

bool OutputCSVFileForSubInstances(const string& filePath, std::shared_ptr<InstanceSubOutputVector> subVector)
{
	std::ofstream outputFile(filePath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the sub csv file " << filePath << std::endl;
		return false;
	}

	outputFile << "X,Y,Z,ScaleX,ScaleY,ScaleZ,RoationX,RotationY,RotaionZ,InstanceType,Variant,Age" << std::endl;

	int fullOutputItemCount = 12;

	for (const std::shared_ptr<InstanceSubOutput>& sub : *subVector)
	{
		int outputItemCount = sub->outputItemCount;

		outputFile
#if USE_OFFSET_FOR_INSTANCE_OUTPUT
			<< sub->xOffsetW << ","
			<< sub->yOffsetW << ","
			<< sub->zOffsetW << ","
#else
			<< sub->posX << ","
			<< sub->posY << ","
			<< sub->posZ << ","
#endif
			<< sub->scaleX << ","
			<< sub->scaleY << ","
			<< sub->scaleZ << ","
			<< sub->rotationX << ","
			<< sub->rotationY << ","
			<< sub->rotationZ << ","
			<< sub->instanceType << ","
			<< sub->variant << ","
			<< sub->age << std::endl;

		/*if (outputItemCount != fullOutputItemCount)
		{
			outputFile << 1.0 << std::endl;
		}
		else
		{
			std::shared_ptr<TreeInstanceSubOutput> tree = std::dynamic_pointer_cast<TreeInstanceSubOutput>(sub);
			if (tree != nullptr)
			{
				outputFile << tree->age << std::endl;
			}
			else
			{
				std::shared_ptr<PointInstanceSubOutput> point = std::dynamic_pointer_cast<PointInstanceSubOutput>(sub);
				if (point != nullptr)
				{
					outputFile << point->age << std::endl;
				}
				else
				{
					outputFile << 0 << std::endl;
				}
			}

		}*/
	}

	outputFile.close();
	return true;
}

void SetupInstanceSubOutput(double posX, double posY, double posZ, CAffineTransform transform, double cellSize, std::shared_ptr<InstanceSubOutput> sub)
{
	if (sub == nullptr)
	{
		return;
	}

	auto posWorldToVF = transform.WC_TO_VF(CAffineTransform::sAffineVector(posX, posY, posZ));

	double doubleXIdx = posWorldToVF.X / cellSize;
	double doubleYIdx = posWorldToVF.Y / cellSize;
	double doubleZIdx = posWorldToVF.Z / cellSize;

	int intXIdx = static_cast<int>(std::floor(doubleXIdx));
	int intYIdx = static_cast<int>(std::floor(doubleYIdx));
	int intZIdx = static_cast<int>(std::floor(doubleZIdx));

	double VF_X = static_cast<double>(intXIdx * cellSize);// + worldOriginVF.X;
	double VF_Y = static_cast<double>(intYIdx * cellSize);// + worldOriginVF.Y;
	double VF_Z = static_cast<double>(intZIdx * cellSize);// + worldOriginVF.Z;

	auto cellOrgVFToWorld = transform.VF_TO_WC(CAffineTransform::sAffineVector{ VF_X, VF_Y, VF_Z + cellSize }); // + cellSize : maybe it is a hack!!!

	double relativeOffsetXWorld = posX - cellOrgVFToWorld.X;
	double relativeOffsetYWorld = posY - cellOrgVFToWorld.Y;
	double relativeOffsetZWorld = posZ - cellOrgVFToWorld.Z;

	sub->xIdx = intXIdx;
	sub->yIdx = intYIdx;
	sub->zIdx = intZIdx;

	sub->xOffsetW = relativeOffsetXWorld;
	sub->yOffsetW = relativeOffsetYWorld;
	sub->zOffsetW = relativeOffsetZWorld;

	sub->posX = posX;
	sub->posY = posY;
	sub->posZ = posZ;
}

std::string GetKeyStringForInstance(const string& outputDir, int intXIdx, int intZIdx)
{
	const int MAX_PATH = 250;
	char subFileName[MAX_PATH];
	char subFilePath[MAX_PATH];
	memset(subFileName, 0, sizeof(char) * MAX_PATH);
	memset(subFilePath, 0, sizeof(char) * MAX_PATH);
#if __APPLE__
	snprintf(subFileName, MAX_PATH, "instances_%d_%d.csv", intXIdx, intZIdx);
	snprintf(subFilePath, MAX_PATH, "%s/%s", outputDir.c_str(), subFileName);
#else
	sprintf_s(subFileName, MAX_PATH, "instances_%d_%d.csv", intXIdx, intZIdx);
	sprintf_s(subFilePath, MAX_PATH, "%s\\%s", outputDir.c_str(), subFileName);
#endif
	string ret = subFilePath;
	return ret;
}

bool CPsInstanceExporter::loadPointInstanceFromCSV(const string& filePath, const string& outputSubDir, InstanceSubOutputMap& outputMap, unsigned int variant, CAffineTransform transform, double cellSize)
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
		if ((zPos < 0) && hasHeight);
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
			SetupInstanceSubOutput(posX, posY, posZ, transform, cellSize, sub);
			sub->instanceType = static_cast<unsigned int>(InstanceType::InstanceType_Point);
			sub->variant = variant;
			sub->age = 1.0;

			string keyString = GetKeyStringForInstance(outputSubDir, sub->xIdx, sub->zIdx);
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
}

bool CPsInstanceExporter::outputSubfiles(const std::string& outputSubsDir)
{
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
	auto lod = VoxelFarm::LOD_0 + 3;

	const double cellSize = (1 << lod) * VoxelFarm::CELL_SIZE;
	const double voxelSize = cellSize / VoxelFarm::BLOCK_DIMENSION;

	CAffineTransform transform(
		CAffineTransform::sAffineVector{
			0.0, 0.0, 0.0
		}, voxelSize, CAffineTransform::eTransformMode::TM_YZ_ROTATE);
	auto worldOriginVF = transform.WC_TO_VF(CAffineTransform::sAffineVector{ 0.0, 0.0, 0.0 });

	InstanceSubOutputMap outputMap;

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
		SetupInstanceSubOutput(instance.posX, instance.posY, instance.posZ, transform, cellSize, sub);

		sub->instanceType = static_cast<unsigned int>(InstanceType::InstanceType_Tree);
		sub->variant = instance.m_instance.treeType;
		sub->age = static_cast<double>(instance.m_instance.age / instance.m_instance.maxAge);

		string keyString = GetKeyStringForInstance(outputSubsDir, sub->xIdx, sub->yIdx);
		InstanceSubOutputMap::iterator iter = outputMap.find(keyString);
		if (outputMap.end() == iter)
		{
			outputMap[keyString] = std::make_shared<InstanceSubOutputVector>();
		}

		std::shared_ptr<InstanceSubOutputVector> subVector = outputMap[keyString];
		subVector->push_back(sub);
	}
	std::cout << "The trees of negative height count is : " << negativeHeightCount << std::endl;
	unsigned int mostTravelledVariant = static_cast<unsigned int>(PointType::Point_MostTravelled);
	bool getMostTravelledPoint = loadPointInstanceFromCSV(m_mostTravelledPointFilePath, outputSubsDir, outputMap, mostTravelledVariant, transform, cellSize);
	unsigned int mostDistantVariant = static_cast<unsigned int>(PointType::Point_MostDistant);
	bool getMostDistantPoint = loadPointInstanceFromCSV(m_mostDistantPointFilePath, outputSubsDir, outputMap, mostDistantVariant, transform, cellSize);

	std::vector<std::thread> workers;
	for (const auto& pair : outputMap)
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

	return allOk;
	/*std::ofstream outputFile(subFilePath);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the subFilePath file " << subFilePath << std::endl;
		return false;
	}*/

	return true;
}