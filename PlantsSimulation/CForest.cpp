#include "CForest.h"

#include <iostream>
#include <fstream>
#include <filesystem> 
#include <thread>
#include <cassert>

#include "TreeClasses.h"
#include "CCellI2DMask.h"

#if __APPLE__
	#include "../Common/include/PsMarco.h"
	#include "../Common/include/PsHelper.h"
	#include "../Common/Include/PointInstance.h"
#else
	#include "..\Common\include\PsMarco.h"
    #include "..\Common\include\PsHelper.h"
	#include "..\Common\Include\PointInstance.h"
#endif

CForest::CForest(void)
{
	grid = NULL;
}

CForest::~CForest(void)
{
	for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i) {
		TreeClass* tree = *i;
		delete tree;
	}
	for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i) {
		TreeClass* tree = *i;
		delete tree;
	}
	for (map<string, DensityMap*>::iterator i = globalMasks.begin(); i != globalMasks.end(); ++i) {
		delete i->second;
	}
}

#define SEED_MAX 16*1024*1024
void CForest::loadDefaultTreeClasses()
{
	
	resetTreeClasses();
	doLoadDefaultTreeClasses();

	return;
}

void CForest::resetTreeClasses()
{
	for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i) {
		TreeClass* tree = *i;
		delete tree;
	}
	classes.clear();
}

void CForest::doLoadDefaultTreeClasses()
{
	TreeClass* treeClassOak = new COakTreeClass();
	TreeClass* treeClassMaple = new CMapleTreeClass();
	TreeClass* treeClassBirch = new CBirchTreeClass();
	TreeClass* treeClassFir = new CFirTreeClass();

	classes.push_back(treeClassOak);
	classes.push_back(treeClassMaple);
	//classes.push_back(treeClassBirch);
	//classes.push_back(treeClassFir);
}

void CForest::loadDefaultMasks()
{
	if (!m_pMetaInfo)
	{
		std::cout << "Meta info is not available in CForest::loadMasks()" << std::endl;
		return;
	}
	
	resetMasks();
	doLoadDefaultMasks();

	return;
}

void CForest::resetMasks()
{
	for (map<string, I2DMask*>::iterator iMask = masks.begin(); iMask != masks.end(); ++iMask) {
		I2DMask* i2DMask = iMask->second;
		delete i2DMask;
	}
	masks.clear();
}

void CForest::doLoadDefaultMasks()
{
	double xRatio = m_pMetaInfo->xRatio;
	double yRatio = m_pMetaInfo->yRatio;

	I2DMask* pHeightI2DMask = new CCellHeightI2DMask(m_pCellTable, xRatio, yRatio);
	I2DMask* pSlopeI2DMask = new CCellSlopeI2DMask(m_pCellTable, xRatio, yRatio);
	I2DMask* pMoistureI2DMask = new CCellMoistureI2DMask(m_pCellTable, xRatio, yRatio);
	I2DMask* pRoughnessI2DMask = new CCellRoughnessI2DMask(m_pCellTable, xRatio, yRatio);
	I2DMask* pRoadAttributeI2DMask = new CCellRoadAttributeI2DMask(m_pCellTable, xRatio, yRatio);

	pair<string, I2DMask*> treeOakHeightPair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_OAK, DensityMapType::DensityMap_Height, pHeightI2DMask);
	pair<string, I2DMask*> treeOakSlopePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_OAK, DensityMapType::DensityMap_Slope, pSlopeI2DMask);
	pair<string, I2DMask*> treeOakMoisturePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_OAK, DensityMapType::DensityMap_Moisture, pMoistureI2DMask);
	pair<string, I2DMask*> treeOakRoughnessPair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_OAK, DensityMapType::DensityMap_Roughness, pRoughnessI2DMask);
	pair<string, I2DMask*> treeOakRoadAttributePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_OAK, DensityMapType::DensityMap_RoadAttribute, pRoadAttributeI2DMask);

	pair<string, I2DMask*> treeMapleHeightPair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_MAPLE, DensityMapType::DensityMap_Height, pHeightI2DMask);
	pair<string, I2DMask*> treeMapleSlopePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_MAPLE, DensityMapType::DensityMap_Slope, pSlopeI2DMask);
	pair<string, I2DMask*> treeMapleMoisturePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_MAPLE, DensityMapType::DensityMap_Moisture, pMoistureI2DMask);
	pair<string, I2DMask*> treeMapleRoughnessPair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_MAPLE, DensityMapType::DensityMap_Roughness, pRoughnessI2DMask);
	pair<string, I2DMask*> treeMapleRoadAttributePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_MAPLE, DensityMapType::DensityMap_RoadAttribute, pRoadAttributeI2DMask);

	pair<string, I2DMask*> treeBirchHeightPair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_BIRCH, DensityMapType::DensityMap_Height, pHeightI2DMask);
	pair<string, I2DMask*> treeBirchSlopePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_BIRCH, DensityMapType::DensityMap_Slope, pSlopeI2DMask);
	pair<string, I2DMask*> treeBirchMoisturePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_BIRCH, DensityMapType::DensityMap_Moisture, pMoistureI2DMask);
	pair<string, I2DMask*> treeBrichRoughnessPair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_BIRCH, DensityMapType::DensityMap_Roughness, pRoughnessI2DMask);
	pair<string, I2DMask*> treeBirchRoadAttributePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_BIRCH, DensityMapType::DensityMap_RoadAttribute, pRoadAttributeI2DMask);

	pair<string, I2DMask*> treeFirHeightPair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_FIR, DensityMapType::DensityMap_Height, pHeightI2DMask);
	pair<string, I2DMask*> treeFirSlopePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_FIR, DensityMapType::DensityMap_Slope, pSlopeI2DMask);
	pair<string, I2DMask*> treeFirMoisturePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_FIR, DensityMapType::DensityMap_Moisture, pMoistureI2DMask);
	pair<string, I2DMask*> treeFirRoughnessPair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_FIR, DensityMapType::DensityMap_Roughness, pRoughnessI2DMask);
	pair<string, I2DMask*> treeFirRoadAttributePair = GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType::TREE_FIR, DensityMapType::DensityMap_RoadAttribute, pRoadAttributeI2DMask);

	masks.insert(treeOakHeightPair);
	masks.insert(treeOakSlopePair);
	masks.insert(treeOakMoisturePair);
	masks.insert(treeOakRoughnessPair);
	masks.insert(treeOakRoadAttributePair);

	masks.insert(treeMapleHeightPair);
	masks.insert(treeMapleSlopePair);
	masks.insert(treeMapleMoisturePair);
	masks.insert(treeMapleRoughnessPair);
	masks.insert(treeMapleRoadAttributePair);

	masks.insert(treeBirchHeightPair);
	masks.insert(treeBirchSlopePair);
	masks.insert(treeBirchMoisturePair);
	masks.insert(treeBrichRoughnessPair);
	masks.insert(treeBirchRoadAttributePair);

	masks.insert(treeFirHeightPair);
	masks.insert(treeFirSlopePair);
	masks.insert(treeFirMoisturePair);
	masks.insert(treeFirRoughnessPair);
	masks.insert(treeFirRoadAttributePair);
}

void CForest::loadDefaultGlobalMasks()
{
	resetGlobalMasks();

	return;

	doLoadDefaultGlobalMasks();

	return;
}

void CForest::resetGlobalMasks()
{
	for (map<string, DensityMap*>::iterator imap = globalMasks.begin(); imap != globalMasks.end(); ++imap) {
		DensityMap* density = imap->second;
		delete density;
	}
	globalMasks.clear();
}

void CForest::resetRawI2DMasks()
{
	for (auto it = rawI2DMasks.begin(); it != rawI2DMasks.end(); ++it)
	{
		delete it->second;
	}
	rawI2DMasks.clear();
}

void CForest::doLoadDefaultGlobalMasks()
{
	for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i) {
		TreeClass* tree = *i;
		for (map<string, DensityMap*>::iterator iMap = tree->masks.begin(); iMap != tree->masks.end(); ++iMap)
		{
			//pair<string, DensityMap*> pairMap = *iMap;
			pair<string, DensityMap*> deepCopyPair(iMap->first, new DensityMap(*(iMap->second)));
			globalMasks.insert(deepCopyPair);
		}
	}
}

bool CForest::parseTreeListCsv(const string& inputTreeListCsv)
{
	if (!std::filesystem::exists(inputTreeListCsv)) {
		return false;
	}

	std::ifstream file(inputTreeListCsv);
	if (!file.is_open()) {
		std::cerr << "Failed to open the tree list csv file :" << inputTreeListCsv << std::endl;
		return false;
	}

	if (!m_pMetaInfo)
	{
		std::cout << "Meta info is not available in CForest::loadMasks()" << std::endl;
		return false;
	}

	std::cout << "Start to parseTreeListCsv : " << inputTreeListCsv << std::endl;

	resetTreeClasses();
	resetMasks();
	resetGlobalMasks();
	resetRawI2DMasks();

	std::string header;
	std::getline(file, header);
	std::string line;

	while (std::getline(file, line)) {
		std::cout << line << std::endl;

		std::stringstream lineStream(line);
		std::string cell;
		std::vector<std::string> row;
		while (std::getline(lineStream, cell, ','))
		{
			row.push_back(cell);
		}
		// This checks for a trailing comma with no data after it.
		if (!lineStream && cell.empty())
		{
			// If there was a trailing comma then add an empty element.
			row.push_back("");
		}
		TreeClass* treeClass = getTreeClassFromStringVector(row);
		classes.push_back(treeClass);

	}

	double xRatio = m_pMetaInfo->xRatio;
	double yRatio = m_pMetaInfo->yRatio;

	I2DMask* pHeightI2DMask = new CCellHeightI2DMask(m_pCellTable, xRatio, yRatio);
	I2DMask* pSlopeI2DMask = new CCellSlopeI2DMask(m_pCellTable, xRatio, yRatio);
	I2DMask* pMoistureI2DMask = new CCellMoistureI2DMask(m_pCellTable, xRatio, yRatio);
	I2DMask* pRoughnessI2DMask = new CCellRoughnessI2DMask(m_pCellTable, xRatio, yRatio);
	I2DMask* pRoadAttributeI2DMask = new CCellRoadAttributeI2DMask(m_pCellTable, xRatio, yRatio);

	pair<DensityMapType, I2DMask*> heightPair(DensityMapType::DensityMap_Height, pHeightI2DMask);
	rawI2DMasks.insert(heightPair);
	pair<DensityMapType, I2DMask*> slopePair(DensityMapType::DensityMap_Slope, pSlopeI2DMask);
	rawI2DMasks.insert(slopePair);
	pair<DensityMapType, I2DMask*> moisturePair(DensityMapType::DensityMap_Moisture, pMoistureI2DMask);
	rawI2DMasks.insert(moisturePair);
	pair<DensityMapType, I2DMask*> roughnessPair(DensityMapType::DensityMap_Roughness, pRoughnessI2DMask);
	rawI2DMasks.insert(roughnessPair);
	pair<DensityMapType, I2DMask*> roadPair(DensityMapType::DensityMap_RoadAttribute, pRoadAttributeI2DMask);
	rawI2DMasks.insert(roadPair);

	for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i) {
		TreeClass* tree = *i;
		for (auto it = tree->masks.begin(); it != tree->masks.end(); ++it) {
			DensityMapType densityType = it->second->type;
			auto rawMaskIt = rawI2DMasks.find(densityType);
			I2DMask* mask = rawMaskIt->second;
			assert(mask != nullptr);
			pair<string, I2DMask*> i2dMaskPair = GetI2DMaskKeyPairFromTreeClassWithDensityMapType(tree, densityType, mask);
			masks.insert(i2dMaskPair);
		}
	}

	//doLoadDefaultGlobalMasks();
	std::cout << "End to parseTreeListCsv : " << inputTreeListCsv << std::endl;

	return true;
}

TreeClass* CForest::getTreeClassFromStringVector(const std::vector<std::string>& row)
{
	std::string nameString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_Name)];
	std::string idString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_TypeId)];
	std::string influenceRString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_InfluenceR)];
	std::string heightString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_Height)];
	std::string elevationMinString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_ElevationMin)];
	std::string elevationMaxString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_ElevationMax)];
	std::string humidityMinString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_HumidityMin)];
	std::string humidityMaxString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_HumidityMax)];
	std::string packingString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_Packing)];
	std::string matureAgeString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_MatureAge)];
	std::string roadCloseMinString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_RoadCloseMin)];
	std::string roadCloseMaxString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_RoadCloseMax)];
	std::string roughnessMinString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_RoughnessMin)];
	std::string roughnessMaxString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_RoughnessMax)];

	unsigned int typeId = std::stoul(idString);
	double influenceR = std::stod(influenceRString);
	double height = std::stod(heightString);
	double elevationMin = std::stod(elevationMinString);
	double elevationMax = std::stod(elevationMaxString);
	double humidityMin = std::stod(humidityMinString);
	double humidityMax = std::stod(humidityMaxString);
	double packing = std::stod(packingString);
	double matureAge = std::stod(matureAgeString);
	double roadCloseMin = std::stod(roadCloseMinString);
	double roadCloseMax = std::stod(roadCloseMaxString);
	double roughnessMin = std::stod(roughnessMinString);
	double roughnessMax = std::stod(roughnessMaxString);

	TreeClass* tree = new TreeClass();
	tree->treeTypeName = nameString;
	tree->typeId = typeId;
	tree->matureAge = matureAge;
	tree->radius = TreeParamRange(0, influenceR);
	tree->seedRange = packing;
	tree->maxAge = 360;

	DensityMap* heightmapDensity = new CHeightDensityMap();
	heightmapDensity->minval = elevationMin;
	heightmapDensity->maxval = elevationMax;
	pair<string, DensityMap*> heightmapDensityPair = GetDensityKeyPairFromTreeClassWithDensityMapType(tree, heightmapDensity->type, heightmapDensity);
	tree->masks.insert(heightmapDensityPair);

	DensityMap* moistureDensity = new CMoistureDensityMap();
	moistureDensity->minval = humidityMin;
	moistureDensity->maxval = humidityMax;
	pair<string, DensityMap*> moistureDensityPair = GetDensityKeyPairFromTreeClassWithDensityMapType(tree, moistureDensity->type, moistureDensity);
	tree->masks.insert(moistureDensityPair);
#if 0
	DensityMap* roadDensity = new CRoadAttributeDensityMap();
	roadDensity->minval = roadCloseMin;
	roadDensity->maxval = roadCloseMax;
	pair<string, DensityMap*> roadDensityPair = GetDensityKeyPairFromTreeClassWithDensityMapType(tree, roadDensity->type, roadDensity);
	tree->masks.insert(roadDensityPair);

	DensityMap* roughnessDensity = new CRoughnessDensityMap();
	roughnessDensity->minval = roughnessMin;
	roughnessDensity->maxval = roughnessMax;
	pair<string, DensityMap*> roughnessDensityPair = GetDensityKeyPairFromTreeClassWithDensityMapType(tree, roughnessDensity->type, roughnessDensity);
	tree->masks.insert(roughnessDensityPair);
#endif
	return tree;
}


void CForest::generate(float forestAge, int iterations)
{
	// allocate grid
	//int gridDelta = 8;
	int gridDelta = 30;
	int gridXSize = xSize/gridDelta;
	int gridZSize = zSize/gridDelta;
	int gridSize = (gridXSize + 1)*(gridZSize + 1)*sizeof(int);
	int* grid = (int*)malloc(gridSize);
	memset(grid, 0, gridSize);

	cout << "gridDelta is :" << gridDelta << endl;
	cout << "gridXSize is :" << gridXSize << endl;
	cout << "gridZSize is :" << gridZSize << endl;
	cout << "gridSize is :" << gridSize << endl;

	CTreeInstance* instances = (CTreeInstance*)malloc(SEED_MAX*sizeof(CTreeInstance));
	memset(instances, 0, SEED_MAX*sizeof(CTreeInstance));
	int instanceIndex = 0;

	ClassStrength* classArray = (ClassStrength*)malloc(classes.size()*sizeof(ClassStrength));

	/*
	for (int x = xo; x < xo + xSize; x += gridDelta)
		for (int z = zo; z < zo + zSize; z += gridDelta)
		{
			int gridX = (x - xo)/gridDelta;
			int gridZ = (z - zo)/gridDelta;
			int gridIndex = gridXSize*gridZ + gridX;
			int& gridP = grid[gridIndex];

			int classIdx = 0;
			double maskSpan = 0.0;

			for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i)
			{
				TreeClass* treeClass = *i;

				double maskValue = 1.0;
				for (map<string, I2DMask*>::iterator iMask = masks.begin(); iMask != masks.end() && maskValue > 0.0; ++iMask)
				{
					double mask = iMask->second->get2DMaskValue(x, z); 

					string maskId = iMask->first;
					map<string, TreeParamRange>::iterator classMask = treeClass->masks.find(maskId);

					if (classMask != treeClass->masks.end())
					{
						TreeParamRange& param = classMask->second;
						if (mask < param.minV || mask > param.maxV)
							maskValue = 0.0;
					}
				}
				maskSpan += maskValue;
				ClassStrength& c = classArray[classIdx];
				c.strength = maskValue;
				c.treeClass = treeClass;
				classIdx++;
			}

			double dice = maskSpan*rand()/RAND_MAX;
			double sum = 0.0;
			TreeClass* chosen = NULL;			
			for (int i = 0; i < classIdx && sum < dice; i++)
			{
				ClassStrength& c = classArray[i];
				chosen = c.treeClass;
				sum += c.strength;
			}

			if (chosen != NULL)
			{
				CTreeInstance& t = instances[instanceIndex];
				t.treeClass = chosen;
				t.bday = -(float)rand()*chosen->matureAge/RAND_MAX;
				t.x = x;
				t.z = z;
				t.dead = false;
				t.mature = false;

				instanceIndex++;
				gridP = instanceIndex;
			}

		}
	*/

	int xLimit = xo + xSize;
	int zLimit = zo + zSize;
	double timeSlice = forestAge/iterations;
	for (int iteration = 0; iteration < iterations; iteration++)
	{
		bool lastIteration = iteration == iterations - 1;
		double time = timeSlice*iteration;
		//Generate tree instance from empty erea.
		for (int x = xo; x < xo + xSize; x += gridDelta)
		{
			for (int z = zo; z < zo + zSize; z += gridDelta)
			{
				if (rand() > RAND_MAX / 10)
					continue;
				
				int gridX = (x - xo) / gridDelta;
				int gridZ = (z - zo) / gridDelta;
				int gridIndex = gridXSize * gridZ + gridX;
				int& gridP = grid[gridIndex];

				bool empty = false;
				if (gridP > 0)
				{
					CTreeInstance& neighborTree = instances[gridP - 1];
					empty = neighborTree.dead;
				}
				else
					empty = true;

				if (!empty)
					continue;

				int classIdx = 0;
				double maskSpan = 0.0;

				for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i)
				{
					TreeClass* treeClass = *i;
					//std::cout << "----------------------Begin TreeClass for ClassStrength : treeClass is : " << PlantTypeToString(treeClass->type) << std::endl;
					double maskValue = 1.0;
					for (map<string, I2DMask*>::iterator iMask = masks.begin(); iMask != masks.end() && maskValue > 0.0; ++iMask)
					{
						string maskId = iMask->first;
						//std::cout << "maskId is : " << maskId << std::endl;
						map<string, DensityMap*>::iterator classMask = treeClass->masks.find(maskId);

						if (classMask != treeClass->masks.end())
						{
							DensityMap* dmap = classMask->second;

							double mask = iMask->second->get2DMaskValue(x, z, dmap->blur);
							maskValue = dmap->GetDensityValue(mask);
							/*if (dmap->invert)
								mask = 1.0 - mask;
							if (mask < dmap->minval || mask > dmap->maxval)
								maskValue = 0.0;
							else
							{
								double scope = abs(dmap->minval - dmap->maxval);
								if (scope > 0.000001)
								{
									maskValue = (mask - dmap->minval) / scope;
								}
								else
									maskValue = 0.0;
							}*/
							//std::cout << "Maskspan for ClassStrength has value : maskId is : " << maskId << " maskValue is : " << maskValue << std::endl;
							//maskSpan *= maskValue;
						}
					}
					//std::cout << "----------------------End TreeClass for ClassStrength : treeClass is : " << PlantTypeToString(treeClass->type) << " final maskValue is : " << maskValue << std::endl;
					maskSpan += maskValue;
					ClassStrength& c = classArray[classIdx];
					c.strength = maskValue;
					//c.strength = maskSpan;
					c.treeClass = treeClass;
					classIdx++;
				}

				double dice = maskSpan * rand() / RAND_MAX;
				double sum = 0.0;
				TreeClass* chosen = NULL;
				for (int i = 0; i < classIdx && sum < dice; i++)
				{
					ClassStrength& c = classArray[i];
					chosen = c.treeClass;
					sum += c.strength;
				}

				if (chosen != NULL)
				{
					CTreeInstance& t = instances[instanceIndex];
					t.treeClass = chosen;
					t.bday = time - min(timeSlice, (float)rand() * chosen->matureAge / RAND_MAX);
					//t.x = x;
					//t.z = z;
					t.x = x + GenerateRandomDouble(-gridDelta, gridDelta);
					if (t.x >= xLimit)
					{
						t.x = x - GenerateRandomDouble(0, gridDelta);
					}
					if (t.x <= xo)
					{
						t.x = x + GenerateRandomDouble(0, gridDelta);
					}
					t.z = z + GenerateRandomDouble(-gridDelta, gridDelta);
					if (t.z >= zLimit)
					{
						t.z = z - GenerateRandomDouble(0, gridDelta);
					}
					if (t.z <= zo)
					{
						t.z = z + GenerateRandomDouble(0, gridDelta);
					}
					t.dead = false;
					t.mature = false;

					instanceIndex++;
					gridP = instanceIndex;
				}

			}
		}
		int currentCount = instanceIndex;
		cout << "Current iteration : " << iteration << " has current instance count : " << currentCount << endl;
		cout << "progress " << iteration << " has current instance count : " << currentCount << endl;
		//Decide the dominated plants.
		for (int iTree = 0; iTree < currentCount; ++iTree)
		{
			CTreeInstance& tree = instances[iTree];

			if (tree.dead)
				continue;

			double age = time - tree.bday;
			tree.age = age;
			tree.dead = age > tree.treeClass->maxAge;

			if (tree.dead)
				continue;

			double sizeFactor = 0.9;

			double growth = 1.0 - max(0.0, (tree.treeClass->matureAge - age)/tree.treeClass->matureAge);
			/*
			double minRx = sizeFactor*growth*tree.treeClass->xRadius.minV*tree.treeClass->radius.minV;
			double minRz = sizeFactor*growth*tree.treeClass->zRadius.minV*tree.treeClass->radius.minV;
			*/
			double minRx = 0.7*sizeFactor*growth*tree.treeClass->xRadius.getValue(tree.x, 0, tree.z)*tree.treeClass->radius.getValue(tree.x, 0, tree.z);
			double minRz = 0.7*sizeFactor*growth*tree.treeClass->zRadius.getValue(tree.x, 0, tree.z)*tree.treeClass->radius.getValue(tree.x, 0, tree.z);
			if (!tree.mature)
			{
				tree.mature = abs(growth - 1.0) < 0.00001 || lastIteration;
				for (double x = tree.x - minRx; x <= tree.x + minRx && !tree.dead; x += gridDelta)
				{
					for (double z = tree.z - minRz; z <= tree.z + minRz && !tree.dead; z += gridDelta)
					{
						int xi = (int)x;
						int zi = (int)z;

						int gridX = (xi - xo) / gridDelta;
						int gridZ = (zi - zo) / gridDelta;

						if (gridX < 0 || gridX >= gridXSize ||
							gridZ < 0 || gridZ >= gridZSize)
							continue;

						int gridIndex = gridXSize * gridZ + gridX;
						int& gridP = grid[gridIndex];

						if (gridP > 0)
						{
							CTreeInstance& neighborTree = instances[gridP - 1];
							double neighborAge = time - neighborTree.bday;
							double neighborGrowth = 1.0 - max(0.0, (neighborTree.treeClass->matureAge - neighborAge) / neighborTree.treeClass->matureAge);
							double minNeighborRx = sizeFactor * neighborGrowth * neighborTree.treeClass->xRadius.getValue(neighborTree.x, 0, neighborTree.z) * neighborTree.treeClass->radius.getValue(neighborTree.x, 0, neighborTree.z);
							double minNeighborRz = sizeFactor * neighborGrowth * neighborTree.treeClass->zRadius.getValue(neighborTree.x, 0, neighborTree.z) * neighborTree.treeClass->radius.getValue(neighborTree.x, 0, neighborTree.z);
							double neighborSize = max(minNeighborRx, minNeighborRz);
							double size = max(minRx, minRz);
							tree.dead =
								&neighborTree != &tree &&
								!neighborTree.dead &&
								(/*neighborTree.bday < tree.bday || */
									neighborSize > size ||
									time - neighborTree.bday > neighborTree.treeClass->matureAge);
						}

						if (!tree.dead)
							gridP = iTree + 1;
					}
				}
			}
			//Generate the new seed.
			if (!tree.dead && tree.mature)
			{
				double seedRx = tree.treeClass->seedRange;
				double seedRz = tree.treeClass->seedRange;

				for (double x = tree.x - seedRx; x <= tree.x + seedRx; x += gridDelta)
				{
					for (double z = tree.z - seedRx; z <= tree.z + seedRx; z += gridDelta)
					{
						if (x >= tree.x - minRx && x <= tree.x + minRx &&
							z >= tree.z - minRz && z <= tree.z + minRz)
							continue;
#if USE_RANDOM_SEED
						if (rand() > RAND_MAX / 1000)
							continue;
#endif
						int xi = (int)x;
						int zi = (int)z;

						int gridX = (xi - xo) / gridDelta;
						int gridZ = (zi - zo) / gridDelta;

						if (gridX < 0 || gridX >= gridXSize ||
							gridZ < 0 || gridZ >= gridZSize)
							continue;

						int gridIndex = gridXSize * gridZ + gridX;
						int& gridP = grid[gridIndex];

						bool empty = false;
						if (gridP > 0)
						{
							CTreeInstance& neighborTree = instances[gridP - 1];
							empty = &neighborTree != &tree && neighborTree.dead;
						}
						else
							empty = true;

						if (empty)
						{
							double maskval = 1.0;
							for (map<string, DensityMap*>::iterator imap = tree.treeClass->masks.begin();
								imap != tree.treeClass->masks.end(); ++imap)
							{

								map<string, I2DMask*>::iterator imask = masks.find(imap->first);
								if (imask != masks.end())
								{
									DensityMap* dmap = imap->second;

									double value = imask->second->get2DMaskValue(x, z, dmap->blur);

									value = dmap->GetDensityValue(value);

									maskval *= value;
								}
							}

							double dice = ((double)rand()) / RAND_MAX;
							if (dice < maskval)
							{
								CTreeInstance& t = instances[instanceIndex];
								t.treeClass = tree.treeClass;
								t.bday = time - min(timeSlice, (float)rand() * tree.treeClass->matureAge / RAND_MAX);
								t.x = x;
								t.z = z;
								t.dead = false;
								t.mature = false;

								instanceIndex++;
								gridP = instanceIndex;
							}
						}
					}
				}
			}
		}
	}

	std::cout << "Tree Instances Count :" << " " << instanceIndex + 1 << std::endl;
	trees.clear();
	for (int i = 0; i < instanceIndex; i++)
	{
		CTreeInstance& tree = instances[i];
		if (!tree.dead && tree.mature)
		{
			double maskval = 1.0;
			for (map<string, DensityMap*>::iterator imap = tree.treeClass->masks.begin();
				imap != tree.treeClass->masks.end(); ++imap)
			{
				map<string, I2DMask*>::iterator imask = masks.find(imap->first);

				if (imask != masks.end())
				{
					DensityMap* dmap = imap->second;
					if (dmap->useForThinning)
					{
						double value = imask->second->get2DMaskValue(tree.x, tree.z, dmap->blur);
						value = dmap->GetDensityValue(value);
						maskval *= value;
					}
				}
			}

			// global masks
			for (map<string, DensityMap*>::iterator imap = globalMasks.begin();
				imap != globalMasks.end(); ++imap)
			{
				map<string, I2DMask*>::iterator imask = masks.find(imap->first);

				if (imask != masks.end())
				{
					DensityMap* dmap = imap->second;
					double value = imask->second->get2DMaskValue(tree.x, tree.z, dmap->blur);
					value = dmap->GetDensityValue(value);
					maskval *= value;
				}
			}

			double dice = ((double)rand())/RAND_MAX;
			if (dice <= maskval)
				trees.push_back(tree);
		}
	}

	std::cout << "Trees Size :" << " " << trees.size() << std::endl;

	delete instances;
	free(grid);
}

TreeInstanceOutput CForest::GetTreeOutputFromInstance(const CTreeInstance& instance)
{
	TreeInstanceOutput output;
	output.x = instance.x;
	output.y = instance.z;

	int rgbColor = instance.treeClass->color;
	int red = (rgbColor >> 16) & 0xFF;
	int green = (rgbColor >> 8) & 0xFF;
	int blue = rgbColor & 0xFF;

	output.red = red;
	output.green = green;
	output.blue = blue;

	output.treeType = static_cast<unsigned int>(instance.treeClass->typeId);
	return output;
}

bool CForest::exportTreeInstanceOutput(const std::vector<TreeInstanceOutput>& data, const std::string& filename, bool hasHeader) {
	std::ofstream outputFile(filename);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the file " << filename << std::endl;
		return false;
	}

	// Write header row
	if (hasHeader)
	{
		outputFile << "X,Y,Z,Red,Green,Blue,TreeType" << std::endl;
	}
	
	// Write data rows
	for (const TreeInstanceOutput& tree : data) {
		outputFile << tree.x << ","
			<< tree.y << ","
			<< tree.z << ","
			<< tree.red << ","
			<< tree.green << ","
			<< tree.blue << ","
			<< tree.treeType << std::endl;
	}

	outputFile.close();

	return true;
}

bool CForest::exportTreeInstanceFullOutput(const std::vector<TreeInstanceFullOutput>& data, const std::string& filename, bool hasHeader, bool withRatio) 
{
	std::ofstream outputFile(filename);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the file " << filename << std::endl;
		return false;
	}

	// Write header row
	if (hasHeader)
	{
		outputFile << "X,Y,Z,Red,Green,Blue,TreeType,RoadAttribute,Moisture,Roughness,Height,Slope" << std::endl;
	}
	
	//bool withRatio = true;
	double xRatio = m_pMetaInfo->xRatio;
	double yRatio = m_pMetaInfo->yRatio;
	double batch_min_x = m_pMetaInfo->batch_min_x;
	double batch_min_y = m_pMetaInfo->batch_min_y;
	double x0 = m_pMetaInfo->x0;
	double y0 = m_pMetaInfo->y0;
	// Write data rows
	for (const TreeInstanceFullOutput& tree : data) {

		if (withRatio)
		{
			outputFile 
#if USE_POS_RELATIVE
				<< tree.posX / xRatio << ","
				<< tree.posY / yRatio << ","
#else
				<< (tree.posX - batch_min_x - x0) / xRatio << ","
				<< (tree.posY - batch_min_y - y0) / yRatio << ","
				
#endif
				<< tree.posZ / xRatio << ","
				<< tree.m_instance.red << ","
				<< tree.m_instance.green << ","
#if !USE_SIMPLE_PC_OUTPUT
				<< tree.m_instance.blue << ","
				<< tree.m_instance.treeType << ","
				<< tree.m_pCellData->GetRoadAttribute() << ","
				<< tree.m_pCellData->GetMoisture() << ","
				<< tree.m_pCellData->GetRoughness() << ","
				<< tree.m_pCellData->GetHeight() << ","
				<< tree.m_pCellData->GetSlopeAngle() << std::endl;
#else
				<< tree.m_instance.blue << std::endl;
#endif
		}
		else
		{
			outputFile << tree.posX << ","
				<< tree.posY << ","
				<< tree.posZ << ","
				<< tree.m_instance.red << ","
				<< tree.m_instance.green << ","
#if !USE_SIMPLE_PC_OUTPUT
				<< tree.m_instance.blue << ","
				<< tree.m_instance.treeType << ","
				<< tree.m_pCellData->GetRoadAttribute() << ","
				<< tree.m_pCellData->GetMoisture() << ","
				<< tree.m_pCellData->GetRoughness() << ","
				<< tree.m_pCellData->GetHeight() << ","
				<< tree.m_pCellData->GetSlopeAngle() << std::endl;
#else
				<< tree.m_instance.blue << std::endl;
#endif
		}
		
	}

	outputFile.close();

	return true;
}

bool CForest::outputTreeInstanceResults(const std::string& fileName, bool hasHeader)
{
	treeoutputs.clear();
	//map<PlantType, int> plants;
	map<std::string, int> plants;
	for (const CTreeInstance& instance : trees)
	{
		//PlantType pt = instance.treeClass->typeId;
		//map<PlantType, int>::iterator it = plants.find(pt);
		std::string pt = instance.treeClass->treeTypeName;
		map<std::string, int>::iterator it = plants.find(pt);
		if (it != plants.end())
		{
			int count = it->second;
			count++;
			plants[pt] = count;
		}
		else {
			plants[pt] = 1;
		}
		TreeInstanceOutput output = TreeInstanceOutput(instance);
		//TreeInstanceOutput output = GetTreeOutputFromInstance(instance);
		treeoutputs.push_back(output);
	}

	//for (map<PlantType, int>::iterator it = plants.begin(); it != plants.end(); ++it)
	for (map<std::string, int>::iterator it = plants.begin(); it != plants.end(); ++it)
	{
		//string typeString = PlantTypeToString(static_cast<PlantType>(it->first));
		std::string typeString = it->first;
		int count = it->second;
		cout << typeString << " count are " << count << endl;
	}

	bool exportCSV = exportTreeInstanceOutput(treeoutputs, fileName, hasHeader);
	return exportCSV;
}

bool CForest::outputCSVTreeInstanceResults(const std::string& fileName)
{
	bool output = outputTreeInstanceResults(fileName, true);
	return output;
}

bool CForest::outputPointsCloudTreeInstanceResults(const std::string& fileName)
{
	bool output = outputTreeInstanceResults(fileName, false);
	return output;
}

bool CForest::outputFullTreeInstanceResults(const std::string& fileName, bool hasHeader, bool withRatio)
{
	if ((!m_pCellTable)
		|| (!m_pMetaInfo)
		|| (!treeoutputs.size())
		)
	{
		return false;
	}
	
	fullOutputs.clear();

	int tableRowsCount = (*m_pCellTable).size();
	int tableColsCount = (*m_pCellTable)[0].size();

	double xRatio = m_pMetaInfo->xRatio;
	double yRatio = m_pMetaInfo->yRatio;
	unsigned int index = 0;

	for (const TreeInstanceOutput& tree : treeoutputs)
	{
	
		int rowIdx = static_cast<int>(tree.x / xRatio);
		int colIdx = static_cast<int>(tree.y / yRatio);
		
		CCellInfo* pCell = nullptr;
		if (((rowIdx >= 0) && (rowIdx < tableRowsCount))
			&& ((colIdx >= 0) && (colIdx < tableColsCount))) {
			pCell = (*m_pCellTable)[rowIdx][colIdx];
		}
		if (pCell != nullptr)
		{
			index++;
			TreeInstanceFullOutput output = TreeInstanceFullOutput(tree, pCell, m_pMetaInfo, index);
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
			if (output.posZ >= 0)
#endif
			{
				fullOutputs.push_back(output);
			}
		}
		else {
			std::cout << "Can not find the Cell Data at X : " << tree.x << ", at Y : " << tree.y << ", at rowIdx : " << rowIdx << ", at colIdx : " << colIdx << std::endl;
		}
	}
	bool exportCSV = exportTreeInstanceFullOutput(fullOutputs, fileName, hasHeader, withRatio);
	return true;
}

bool CForest::outputCSVFullTreeInstanceResults(const std::string& fileName)
{
	bool output = outputFullTreeInstanceResults(fileName, true, false);
	return output;
}

bool CForest::outputCSVFullTreeInstanceResultsWithRatio(const std::string& fileName)
{
	bool output = outputFullTreeInstanceResults(fileName, true, true);
	return output;
}

bool CForest::outputPointsCloudFullTreeInstanceResults(const std::string& fileName)
{
	bool output = outputFullTreeInstanceResults(fileName, false, false);
	return output;
}

bool CForest::outputPointsCloudFullTreeInstanceResultsWithRatio(const std::string& fileName)
{
	bool output = outputFullTreeInstanceResults(fileName, false, true);
	return output;
}

pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithIndex(PlantType type, int index, I2DMask* pI2dMask)
{
	string keyString = PlantTypeWithIndexToMaskString(type, index);
	pair<string, I2DMask*> ret(keyString, pI2dMask);
	return ret;
}

pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType plantType, DensityMapType densityType, I2DMask* pI2dMask)
{
	string keyString = PlantTypeWithDensityMapTypeToMaskString(plantType, densityType);
	pair<string, I2DMask*> ret(keyString, pI2dMask);
	return ret;
}

pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithDensityMapTypeIndex(PlantType plantType, DensityMapType densityType, I2DMask* pI2dMask)
{
	string keyString = PlantTypeWithDensityMapTypeIndexToMaskString(plantType, densityType);
	pair<string, I2DMask*> ret(keyString, pI2dMask);
	return ret;
}

pair<string, I2DMask*> GetI2DMaskKeyPairFromTreeClassWithDensityMapType(TreeClass* treeClass, DensityMapType densityType, I2DMask* pI2dMask)
{
	string keyString = TreeClassWithDensityMapTypeToMaskString(treeClass, densityType);
	pair<string, I2DMask*> ret(keyString, pI2dMask);
	return ret;
}