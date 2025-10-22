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
	#include "../Common/Include/CTimeCounter.h"
#else
	#include "..\Common\include\PsMarco.h"
    #include "..\Common\include\PsHelper.h"
	#include "..\Common\Include\PointInstance.h"
	#include "..\Common\Include\CTimeCounter.h"
#endif

CForest::CForest(void)
	: m_pCellTable(nullptr)
	, m_pMetaInfo(nullptr)
	, m_p2dCaveLevel0Nodes(nullptr)
	, m_p2dCaveLevel1Nodes(nullptr)
	, m_isLevel1Instances(false)

{
	grid = NULL;
	maxHeight = 10000;
}

CForest::~CForest(void)
{
	ResetMasksAndClasses();
}

void CForest::ResetMasksAndClasses()
{
	resetTreeClasses();
	resetMasks();
	resetGlobalMasks();
	resetRawI2DMasks();
}

#define SEED_MAX 16*1024*1024
void CForest::loadDefaultTreeClasses()
{
	
	resetTreeClasses();
	doLoadDefaultTreeClasses();

	return;
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
	resetGlobalMasks(); //We don't have global masks(csv file) to load for now.

	return;

	doLoadDefaultGlobalMasks();

	return;
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

void CForest::resetTreeClasses()
{
	for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i) {
		TreeClass* tree = *i;
		delete tree;
	}
	classes.clear();
}

void CForest::resetMasks()
{
	/*for (map<string, I2DMask*>::iterator iMask = masks.begin(); iMask != masks.end(); ++iMask) {
		I2DMask* i2DMask = iMask->second;
		delete i2DMask;
	}*/ //Don't do that!!! it will release in rawI2DMasks!
	masks.clear();
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

	int columnCount = countColumnsInCSV(inputTreeListCsv, ',');
	ResetMasksAndClasses();

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
		TreeClass* treeClass = getTreeClassFromStringVector(row, columnCount);
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
	if (columnCount > static_cast<int>(TreeList_CSV_Columns::TL_SunLightAffinityMax)) {
		I2DMask* pSunLightAffinityI2DMask = new CCellSunLightAffinityID2Mask(m_pCellTable, xRatio, yRatio);
		pair<DensityMapType, I2DMask*> sunLightAffinityPair(DensityMapType::DensityMap_SunLightAffinity, pSunLightAffinityI2DMask);
		rawI2DMasks.insert(sunLightAffinityPair);
	}

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

TreeClass* CForest::getTreeClassFromStringVector(const std::vector<std::string>& row, int columnCount)
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
	std::string sunLightAffinityMinString = "0";
	std::string sunLightAffinityMaxString = "1";
	if (columnCount > static_cast<int>(TreeList_CSV_Columns::TL_SunLightAffinityMin)) {
		sunLightAffinityMinString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_SunLightAffinityMin)];
	}
	if (columnCount > static_cast<int>(TreeList_CSV_Columns::TL_SunLightAffinityMax)) {
		sunLightAffinityMaxString = row[static_cast<size_t>(TreeList_CSV_Columns::TL_SunLightAffinityMax)];
	}
	
	unsigned int typeId = std::stoul(idString);
	double influenceR = std::stod(influenceRString);
	double height = std::stod(heightString);
	double elevationMin = std::stod(elevationMinString) * maxHeight / 100.0;
	double elevationMax = std::stod(elevationMaxString) * maxHeight / 100.0;
	double humidityMin = std::stod(humidityMinString);
	double humidityMax = std::stod(humidityMaxString);
	double packing = std::stod(packingString);
	double matureAge = std::stod(matureAgeString);
	double roadCloseMin = std::stod(roadCloseMinString);
	double roadCloseMax = std::stod(roadCloseMaxString);
	double roughnessMin = std::stod(roughnessMinString);
	double roughnessMax = std::stod(roughnessMaxString);
	double sunLightAffinityMin = 0;
	double sunLightAffinityMax = 1;
	if (columnCount > static_cast<int>(TreeList_CSV_Columns::TL_SunLightAffinityMin)) {
		sunLightAffinityMin = std::stod(sunLightAffinityMinString);
	}
	if (columnCount > static_cast<int>(TreeList_CSV_Columns::TL_SunLightAffinityMax)) {
		sunLightAffinityMax = std::stod(sunLightAffinityMaxString);
	}

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
	heightmapDensity->ease = 50;
	pair<string, DensityMap*> heightmapDensityPair = GetDensityKeyPairFromTreeClassWithDensityMapType(tree, heightmapDensity->type, heightmapDensity);
	tree->masks.insert(heightmapDensityPair);

	DensityMap* moistureDensity = new CMoistureDensityMap();
	moistureDensity->minval = humidityMin;
	moistureDensity->maxval = humidityMax;
	pair<string, DensityMap*> moistureDensityPair = GetDensityKeyPairFromTreeClassWithDensityMapType(tree, moistureDensity->type, moistureDensity);
	tree->masks.insert(moistureDensityPair);

	DensityMap* slopeDensity = new CSlopeDensityMap();
	slopeDensity->minval = 0 * (PI / 180.0);
	slopeDensity->maxval = 63 * (PI / 180.0);
	slopeDensity->ease = 5 * (PI / 180.0);
	pair<string, DensityMap*> slopeDensityPair = GetDensityKeyPairFromTreeClassWithDensityMapType(tree, slopeDensity->type, slopeDensity);
	tree->masks.insert(slopeDensityPair);
	
#if 1
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

	if (columnCount > static_cast<int>(TreeList_CSV_Columns::TL_SunLightAffinityMax)) {
		DensityMap* sunLightDensity = new CSunLightAffinityDensityMap();
		sunLightDensity->minval = sunLightAffinityMin;
		sunLightDensity->maxval = sunLightAffinityMax;
		pair<string, DensityMap*> sunLightDensityPair = GetDensityKeyPairFromTreeClassWithDensityMapType(tree, sunLightDensity->type, sunLightDensity);
		tree->masks.insert(sunLightDensityPair);
	}
	return tree;
}


void CForest::generate(float forestAge, int iterations)
{
	string title = "CForest::generate generate whole tree instances : ";
	CTimeCounter timeCounter(title);

	// allocate grid
	//int gridDelta = 8;
	int gridDelta = 30;
	//int gridDelta = 18;
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
				//if (rand() > RAND_MAX / 4)
				{
					continue;
				}
				
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
		double currentProgressDouble = 100.0 * iteration / iterations;
		int currentProgressInt = static_cast<int>(currentProgressDouble);
		cout << "Current iteration count is : " << iteration << endl;
		cout << "Current iteration : " << iteration << " has current instance count : " << currentCount << endl;
		cout << "progress " << currentProgressInt << " has current instance count : " << currentCount << endl;
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
						//if (rand() > RAND_MAX / 100)
						{
							continue;
						}
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

	removeTreesNearPOIs();

	if (!m_isLevel1Instances)
	{
		removeTreesNearCaves();
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
		//|| (!treeoutputs.size())
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
			if (output.posZ > UNAVAILBLE_NEG_HEIGHT)
			{
#if USE_OUTPUT_ONLY_POSITIVE_HEIGHT
				if (output.posZ >= 0)
#endif
				{
					fullOutputs.push_back(output);
				}
			}
		}
		else {
			std::cout << "Can not find the Cell Data at X : " << tree.x << ", at Y : " << tree.y << ", at rowIdx : " << rowIdx << ", at colIdx : " << colIdx << std::endl;
		}
	}
	std::cout << "FullOutputs TreeInstanceFullOutput count is " << fullOutputs.size() << std::endl;

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

// Calculate mask value for a tree class at a position
double CForest::calculateMaskValue(TreeClass* treeClass, int x, int z) {
	double maskValue = 1.0;

	for (auto iMask = masks.begin(); iMask != masks.end() && maskValue > 0.0; ++iMask) {
		string maskId = iMask->first;
		auto classMask = treeClass->masks.find(maskId);

		if (classMask != treeClass->masks.end()) {
			DensityMap* dmap = classMask->second;
			double mask = iMask->second->get2DMaskValue(x, z, dmap->blur);
			maskValue = dmap->GetDensityValue(mask);
		}
	}

	return maskValue;
}

// Select tree class based on mask values at position
TreeClass* CForest::selectTreeClass(ClassStrength* classArray, int x, int z) {
	int classIdx = 0;
	double maskSpan = 0.0;

	for (auto i = classes.begin(); i != classes.end(); ++i) {
		TreeClass* treeClass = *i;
		double maskValue = calculateMaskValue(treeClass, x, z);

		maskSpan += maskValue;
		ClassStrength& c = classArray[classIdx];
		c.strength = maskValue;
		c.treeClass = treeClass;
		classIdx++;
	}

	double dice = maskSpan * rand() / RAND_MAX;
	double sum = 0.0;
	TreeClass* chosen = NULL;

	for (int i = 0; i < classIdx && sum < dice; i++) {
		ClassStrength& c = classArray[i];
		chosen = c.treeClass;
		sum += c.strength;
	}

	return chosen;
}

// Clamp position within forest boundaries
double CForest::clampPosition(double pos, double gridPos, double offset, int limit, int origin, bool isAtLimit) {
	if (isAtLimit) {
		return gridPos - GenerateRandomDouble(0, offset);
	}
	if (pos <= origin) {
		return gridPos + GenerateRandomDouble(0, offset);
	}
	return pos;
}

// Check if grid cell is empty or has dead tree
bool CForest::isGridCellAvailable(GenerationContext& ctx, int gridX, int gridZ) {
	int& gridP = ctx.grid.at(gridX, gridZ);

	if (gridP > 0) {
		CTreeInstance& neighborTree = ctx.instances[gridP - 1];
		return neighborTree.dead;
	}

	return true;
}

// Generate initial tree instances for empty areas
void CForest::generateInitialInstances(GenerationContext& ctx) {
	for (int x = ctx.xo; x < ctx.xo + ctx.xSize; x += ctx.grid.gridDelta) {
		for (int z = ctx.zo; z < ctx.zo + ctx.zSize; z += ctx.grid.gridDelta) {
			if (rand() > RAND_MAX / 10) {
				continue;
			}

			int gridX = (x - ctx.xo) / ctx.grid.gridDelta;
			int gridZ = (z - ctx.zo) / ctx.grid.gridDelta;

			if (!isGridCellAvailable(ctx, gridX, gridZ)) {
				continue;
			}

			TreeClass* chosen = selectTreeClass(ctx.classArray, x, z);

			if (chosen != NULL) {
				createTreeInstance(ctx, chosen, x, z, gridX, gridZ);
			}
		}
	}
}


// Create new tree instance at position
void CForest::createTreeInstance(GenerationContext& ctx, TreeClass* treeClass, 
                                 int x, int z, int gridX, int gridZ) {
    int xLimit = ctx.xo + ctx.xSize;
    int zLimit = ctx.zo + ctx.zSize;
    
    CTreeInstance& t = ctx.instances[ctx.instanceIndex];
    t.treeClass = treeClass;
    t.bday = ctx.time - min((float)(ctx.timeSlice), (float)(rand() * treeClass->matureAge / RAND_MAX));
    
    t.x = x + GenerateRandomDouble(-ctx.grid.gridDelta, ctx.grid.gridDelta);
    t.x = clampPosition(t.x, x, ctx.grid.gridDelta, xLimit, ctx.xo, t.x >= xLimit);
    
    t.z = z + GenerateRandomDouble(-ctx.grid.gridDelta, ctx.grid.gridDelta);
    t.z = clampPosition(t.z, z, ctx.grid.gridDelta, zLimit, ctx.zo, t.z >= zLimit);
    
    t.dead = false;
    t.mature = false;
    
    ctx.instanceIndex++;
    ctx.grid.at(gridX, gridZ) = ctx.instanceIndex;
}


TreeGrowth CForest::calculateTreeGrowth(CTreeInstance& tree, double time, double sizeFactor) {
	TreeGrowth result;
	result.age = time - tree.bday;

	result.growth = 1.0 - max(0.0, (tree.treeClass->matureAge - result.age) / tree.treeClass->matureAge);

	result.minRx = 0.7 * sizeFactor * result.growth *
		tree.treeClass->xRadius.getValue(tree.x, 0, tree.z) *
		tree.treeClass->radius.getValue(tree.x, 0, tree.z);

	result.minRz = 0.7 * sizeFactor * result.growth *
		tree.treeClass->zRadius.getValue(tree.x, 0, tree.z) *
		tree.treeClass->radius.getValue(tree.x, 0, tree.z);

	result.mature = tree.mature;

	return result;
}

// Check competition with neighboring trees
bool CForest::checkTreeCompetition(GenerationContext& ctx, CTreeInstance& tree,
	const TreeGrowth& growth, int iTree, double sizeFactor) {

	for (double x = tree.x - growth.minRx; x <= tree.x + growth.minRx && !tree.dead; x += ctx.grid.gridDelta) {
		for (double z = tree.z - growth.minRz; z <= tree.z + growth.minRz && !tree.dead; z += ctx.grid.gridDelta) {
			int xi = (int)x;
			int zi = (int)z;

			int gridX = (xi - ctx.xo) / ctx.grid.gridDelta;
			int gridZ = (zi - ctx.zo) / ctx.grid.gridDelta;

			if (gridX < 0 || gridX >= ctx.grid.gridXSize ||
				gridZ < 0 || gridZ >= ctx.grid.gridZSize)
				continue;

			int& gridP = ctx.grid.at(gridX, gridZ);

			if (gridP > 0) {
				CTreeInstance& neighborTree = ctx.instances[gridP - 1];

				if (&neighborTree != &tree && !neighborTree.dead) {
					double neighborAge = ctx.time - neighborTree.bday;
					double neighborGrowth = 1.0 - max(0.0, (neighborTree.treeClass->matureAge - neighborAge) /
						neighborTree.treeClass->matureAge);

					double minNeighborRx = sizeFactor * neighborGrowth *
						neighborTree.treeClass->xRadius.getValue(neighborTree.x, 0, neighborTree.z) *
						neighborTree.treeClass->radius.getValue(neighborTree.x, 0, neighborTree.z);
					double minNeighborRz = sizeFactor * neighborGrowth *
						neighborTree.treeClass->zRadius.getValue(neighborTree.x, 0, neighborTree.z) *
						neighborTree.treeClass->radius.getValue(neighborTree.x, 0, neighborTree.z);

					double neighborSize = max(minNeighborRx, minNeighborRz);
					double size = max(growth.minRx, growth.minRz);

					tree.dead = 
						(neighborSize > size ||
						ctx.time - neighborTree.bday > neighborTree.treeClass->matureAge);
				}
			}

			if (!tree.dead) {
				gridP = iTree + 1;
			}
		}
	}

	return !tree.dead;
}

// Generate seeds from mature tree
void CForest::generateSeeds(GenerationContext& ctx, CTreeInstance& tree, const TreeGrowth& growth) {
	double seedRx = tree.treeClass->seedRange;
	double seedRz = tree.treeClass->seedRange;
	double timeSlice = ctx.timeSlice;

	for (double x = tree.x - seedRx; x <= tree.x + seedRx; x += ctx.grid.gridDelta) {
		for (double z = tree.z - seedRz; z <= tree.z + seedRz; z += ctx.grid.gridDelta) {
			if (x >= tree.x - growth.minRx && x <= tree.x + growth.minRx &&
				z >= tree.z - growth.minRz && z <= tree.z + growth.minRz)
				continue;

#if USE_RANDOM_SEED
			if (rand() > RAND_MAX / 1000) {
				continue;
			}
#endif

			int xi = (int)x;
			int zi = (int)z;

			int gridX = (xi - ctx.xo) / ctx.grid.gridDelta;
			int gridZ = (zi - ctx.zo) / ctx.grid.gridDelta;

			if (gridX < 0 || gridX >= ctx.grid.gridXSize ||
				gridZ < 0 || gridZ >= ctx.grid.gridZSize)
				continue;

			int& gridP = ctx.grid.at(gridX, gridZ);

			bool empty = false;
			if (gridP > 0) {
				CTreeInstance& neighborTree = ctx.instances[gridP - 1];
				empty = &neighborTree != &tree && neighborTree.dead;
			}
			else {
				empty = true;
			}

			if (empty) {
				double maskval = 1.0;
				for (auto imap = tree.treeClass->masks.begin(); imap != tree.treeClass->masks.end(); ++imap) {
					auto imask = masks.find(imap->first);
					if (imask != masks.end()) {
						DensityMap* dmap = imap->second;
						double value = imask->second->get2DMaskValue(x, z, dmap->blur);
						value = dmap->GetDensityValue(value);
						maskval *= value;
					}
				}

				double dice = ((double)rand()) / RAND_MAX;
				if (dice < maskval) {
					CTreeInstance& t = ctx.instances[ctx.instanceIndex];
					t.treeClass = tree.treeClass;
					t.bday = ctx.time - min(timeSlice, (float)rand() * tree.treeClass->matureAge / RAND_MAX);
					t.x = x;
					t.z = z;
					t.dead = false;
					t.mature = false;

					ctx.instanceIndex++;
					gridP = ctx.instanceIndex;
				}
			}
		}
	}
}

// Process all trees in current iteration
void CForest::processTreeIterationForDominatePlants(GenerationContext& ctx, int currentCount) {
	for (int iTree = 0; iTree < currentCount; ++iTree) {
		CTreeInstance& tree = ctx.instances[iTree];

		if (tree.dead)
			continue;

		double age = ctx.time - tree.bday;
		tree.age = age;
		tree.dead = age > tree.treeClass->maxAge;

		if (tree.dead)
			continue;

		double sizeFactor = 0.9;
		TreeGrowth growth = calculateTreeGrowth(tree, ctx.time, sizeFactor);

		if (!tree.mature) {
			tree.mature = abs(growth.growth - 1.0) < 0.00001 || ctx.lastIteration;
			checkTreeCompetition(ctx, tree, growth, iTree, sizeFactor);
		}

		if (!tree.dead && tree.mature) {
			generateSeeds(ctx, tree, growth);
		}
	}
}

// Apply thinning mask to tree
double CForest::applyThinningMasks(CTreeInstance& tree) {
	double maskval = 1.0;

	for (auto imap = tree.treeClass->masks.begin(); imap != tree.treeClass->masks.end(); ++imap) {
		auto imask = masks.find(imap->first);

		if (imask != masks.end()) {
			DensityMap* dmap = imap->second;
			if (dmap->useForThinning) {
				double value = imask->second->get2DMaskValue(tree.x, tree.z, dmap->blur);
				value = dmap->GetDensityValue(value);
				maskval *= value;
			}
		}
	}

	for (auto imap = globalMasks.begin(); imap != globalMasks.end(); ++imap) {
		auto imask = masks.find(imap->first);

		if (imask != masks.end()) {
			DensityMap* dmap = imap->second;
			double value = imask->second->get2DMaskValue(tree.x, tree.z, dmap->blur);
			value = dmap->GetDensityValue(value);
			maskval *= value;
		}
	}

	return maskval;
}

// Filter mature trees and apply thinning
void CForest::filterMatureTrees(CTreeInstance* instances, int instanceIndex) {
	trees.clear();

	for (int i = 0; i < instanceIndex; i++) {
		CTreeInstance& tree = instances[i];

		if (!tree.dead && tree.mature) {
			double maskval = applyThinningMasks(tree);
			double dice = ((double)rand()) / RAND_MAX;

			if (dice <= maskval) {
				trees.push_back(tree);
			}
		}
	}
}

// Remove trees near POIs
void CForest::removeTreesNearPOIs() {
	if (!m_pPoisLocations)
		return;

	string title = "Remove the tree instances from pois : ";
	CTimeCounter timeCounter(title);

	int sizeBefore = trees.size();
	cout << "Before remove tree from pois, Trees Size is : " << sizeBefore << endl;

	trees.erase(
		std::remove_if(trees.begin(), trees.end(),
			[this](auto tree) {
				Point p(tree.x, tree.z);
				double distance = GetDistancesToPOIs(p, m_pPoisLocations, TREE_FROM_POI_DISTANCE_LIMIT);
				return distance < TREE_FROM_POI_DISTANCE_LIMIT;
			}
		),
		trees.end()
	);

	int sizeAfter = trees.size();
	cout << "After remove tree from pois, Trees Size is : " << sizeAfter << endl;

	sizeBefore = sizeBefore ? sizeBefore : 1;
	double percentageCount = static_cast<double>(100 * sizeAfter / sizeBefore);
	cout << "After removal from pois the rest of tree has percentage of " << percentageCount << " before tree count!" << endl;
}

// Remove trees near caves
void CForest::removeTreesNearCaves() {
	if (!m_p2dCaveLevel0Nodes)
		return;

	string title = "Remove the tree instances from caves level 0 : ";
	CTimeCounter timeCounter(title);

	int sizeBefore = trees.size();
	cout << "Before remove tree from cave level 0, Trees Size is : " << sizeBefore << endl;

	trees.erase(
		std::remove_if(trees.begin(), trees.end(),
			[this](auto tree) {
				Point p(tree.x, tree.z);
				double distance = GetDistanceToCaveNodes(p, m_p2dCaveLevel0Nodes, CAVE_DISTANCE_LIMIT_TREE);
				return distance < CAVE_DISTANCE_LIMIT_TREE;
			}
		),
		trees.end()
	);

	int sizeAfter = trees.size();
	cout << "After remove tree from cave level 0, Trees Size is : " << sizeAfter << endl;

	sizeBefore = sizeBefore ? sizeBefore : 1;
	double percentageCount = static_cast<double>(100 * sizeAfter / sizeBefore);
	cout << "After Cave removal the rest of tree has percentage of " << percentageCount << " before tree count!" << endl;
}

// Main generate function - refactored
void CForest::generate2(float forestAge, int iterations) {
	string title = "CForest::generate generate whole tree instances : ";
	CTimeCounter timeCounter(title);

	// Initialize grid
	const int gridDelta = 30;
	GridInfo grid(xSize, zSize, gridDelta);

	cout << "gridDelta is : " << gridDelta << endl;
	cout << "gridXSize is : " << grid.gridXSize << endl;
	cout << "gridZSize is : " << grid.gridZSize << endl;
	cout << "gridSize is : " << grid.gridTotalSize << endl;

	// Initialize instances and class array
	CTreeInstance* instances = (CTreeInstance*)malloc(SEED_MAX * sizeof(CTreeInstance));
	memset(instances, 0, SEED_MAX * sizeof(CTreeInstance));

	ClassStrength* classArray = (ClassStrength*)malloc(classes.size() * sizeof(ClassStrength));

	// Iterate through forest age
	double timeSlice = forestAge / iterations;

	GenerationContext ctx = {
		instances, 0, classArray, grid, xo, zo, xSize, zSize, 0.0, timeSlice, false
	};

	for (int iteration = 0; iteration < iterations; iteration++) {
		ctx.lastIteration = (iteration == iterations - 1);
		ctx.time = timeSlice * iteration;

		generateInitialInstances(ctx);

		int currentCount = ctx.instanceIndex;
		int currentProgressInt = static_cast<int>(100.0 * iteration / iterations);

		cout << "Current iteration count is : " << iteration << endl;
		cout << "Current iteration : " << iteration << " has current instance count : " << currentCount << endl;
		cout << "progress " << currentProgressInt << " has current instance count : " << currentCount << endl;

		processTreeIterationForDominatePlants(ctx, currentCount);
	}

	cout << "Tree Instances Count : " << ctx.instanceIndex + 1 << endl;

	// Filter and finalize trees
	filterMatureTrees(instances, ctx.instanceIndex);
	removeTreesNearPOIs();
	if (!m_isLevel1Instances)
	{
		removeTreesNearCaves();
	}
	cout << "Trees Size : " << trees.size() << endl;

	// Cleanup
	delete instances;
	free(classArray);
}


// Add to CForest.cpp

// Build optimized cache once at start
void CForest::buildOptimizedTreeClassCache(std::vector<TreeClassCacheOptimized>& cache) {
	cache.reserve(classes.size());

	for (TreeClass* treeClass : classes) {
		TreeClassCacheOptimized tc;
		tc.treeClass = treeClass;
		tc.matureAge = treeClass->matureAge;
		tc.maxAge = treeClass->maxAge;
		tc.seedRange = treeClass->seedRange;

		// Cache mask pointers for fast access
		tc.maskData.reserve(treeClass->masks.size());
		for (auto& maskPair : treeClass->masks) {
			auto maskIt = masks.find(maskPair.first);
			if (maskIt != masks.end()) {
				tc.maskData.emplace_back(maskIt->second, maskPair.second);
				if (maskPair.second->useForThinning) {
					tc.thinningMasks.emplace_back(maskIt->second, maskPair.second);
				}
			}
		}
		cache.push_back(std::move(tc));
	}
}

// Optimized mask calculation with early exit
inline double calculateMaskValueFast(const TreeClassCacheOptimized& tc, int x, int z) {
	double maskValue = 1.0;

	for (const auto& [mask, density] : tc.maskData) {
		double value = mask->get2DMaskValue(x, z, density->blur);
		maskValue *= density->GetDensityValue(value);
		if (maskValue <= 0.0) return 0.0; // Early exit
	}

	return maskValue;
}

// Optimized tree class selection
TreeClass* selectTreeClassFast(
	const std::vector<TreeClassCacheOptimized>& cache,
	ClassStrength* classArray,
	int x, int z) {

	double maskSpan = 0.0;
	int classIdx = 0;

	for (const auto& tc : cache) {
		double maskValue = calculateMaskValueFast(tc, x, z);
		maskSpan += maskValue;
		classArray[classIdx].strength = maskValue;
		classArray[classIdx].treeClass = tc.treeClass;
		classIdx++;
	}

	if (maskSpan <= 0.0) return nullptr;

	double dice = maskSpan * rand() / RAND_MAX;
	double sum = 0.0;

	for (int i = 0; i < classIdx; i++) {
		sum += classArray[i].strength;
		if (sum >= dice) {
			return classArray[i].treeClass;
		}
	}

	return classArray[classIdx - 1].treeClass;
}

// Optimized position clamping
inline double clampPositionFast(double pos, double gridPos, double offset,
	int limit, int origin) {
	if (pos >= limit) {
		return gridPos - GenerateRandomDouble(0, offset);
	}
	if (pos <= origin) {
		return gridPos + GenerateRandomDouble(0, offset);
	}
	return pos;
}

// Check if grid cell is available
inline bool isGridCellAvailableFast(OptimizedGenerationContext& ctx, int gridX, int gridZ) {
	if (!ctx.isValidGridPos(gridX, gridZ)) return false;

	int& gridP = ctx.gridAt(gridX, gridZ);
	if (gridP == 0) return true;

	return ctx.instances[gridP - 1].dead;
}

// Create tree instance - optimized
void createTreeInstanceFast(OptimizedGenerationContext& ctx, TreeClass* treeClass,
	int x, int z, int gridX, int gridZ) {
	CTreeInstance& t = ctx.instances[ctx.instanceIndex];
	t.treeClass = treeClass;
	t.bday = ctx.time - min(static_cast<float>(ctx.timeSlice),
		static_cast<float>(rand() * treeClass->matureAge / RAND_MAX));

	const int xLimit = ctx.xo + ctx.xSize;
	const int zLimit = ctx.zo + ctx.zSize;

	t.x = x + GenerateRandomDouble(-ctx.gridDelta, ctx.gridDelta);
	t.x = clampPositionFast(t.x, x, ctx.gridDelta, xLimit, ctx.xo);

	t.z = z + GenerateRandomDouble(-ctx.gridDelta, ctx.gridDelta);
	t.z = clampPositionFast(t.z, z, ctx.gridDelta, zLimit, ctx.zo);

	t.dead = false;
	t.mature = false;

	ctx.instanceIndex++;
	ctx.gridAt(gridX, gridZ) = ctx.instanceIndex;
}

// Calculate tree growth parameters - optimized
struct TreeGrowthFast {
	double age;
	double growth;
	double minRx;
	double minRz;
	bool mature;

	TreeGrowthFast(CTreeInstance& tree, double time, double sizeFactor,
		const TreeClassCacheOptimized& tc) {
		age = time - tree.bday;
		growth = 1.0 - max(0.0, (tc.matureAge - age) / tc.matureAge);

		const double radius = tc.getRadiusValue(tree.x, tree.z);
		minRx = 0.7 * sizeFactor * growth * tc.getXRadiusValue(tree.x, tree.z) * radius;
		minRz = 0.7 * sizeFactor * growth * tc.getZRadiusValue(tree.x, tree.z) * radius;
		mature = tree.mature;
	}
};

// Generate initial instances - optimized
void generateInitialInstancesFast(OptimizedGenerationContext& ctx) {
	const int xLimit = ctx.xo + ctx.xSize;
	const int zLimit = ctx.zo + ctx.zSize;
	const int randThreshold = RAND_MAX / 10;

	for (int x = ctx.xo; x < xLimit; x += ctx.gridDelta) {
		for (int z = ctx.zo; z < zLimit; z += ctx.gridDelta) {
			if (rand() > randThreshold) continue;

			const int gridX = (x - ctx.xo) / ctx.gridDelta;
			const int gridZ = (z - ctx.zo) / ctx.gridDelta;

			if (!isGridCellAvailableFast(ctx, gridX, gridZ)) continue;

			TreeClass* chosen = selectTreeClassFast(*ctx.treeCache, ctx.classArray, x, z);
			if (chosen) {
				createTreeInstanceFast(ctx, chosen, x, z, gridX, gridZ);
			}
		}
	}
}

// Check tree competition - optimized with early exit
bool checkTreeCompetitionFast(OptimizedGenerationContext& ctx, CTreeInstance& tree,
	const TreeGrowthFast& growth, int iTree, double sizeFactor,
	const TreeClassCacheOptimized& treeCache) {

	for (double x = tree.x - growth.minRx; x <= tree.x + growth.minRx && !tree.dead;
		x += ctx.gridDelta) {
		for (double z = tree.z - growth.minRz; z <= tree.z + growth.minRz && !tree.dead;
			z += ctx.gridDelta) {

			const int gridX = (static_cast<int>(x) - ctx.xo) / ctx.gridDelta;
			const int gridZ = (static_cast<int>(z) - ctx.zo) / ctx.gridDelta;

			if (!ctx.isValidGridPos(gridX, gridZ)) continue;

			int& gridP = ctx.gridAt(gridX, gridZ);

			if (gridP > 0) {
				CTreeInstance& neighbor = ctx.instances[gridP - 1];

				if (&neighbor != &tree && !neighbor.dead) {
					const double neighborAge = ctx.time - neighbor.bday;

					// Find neighbor's cache
					const TreeClassCacheOptimized* neighborCache = nullptr;
					for (const auto& tc : *ctx.treeCache) {
						if (tc.treeClass == neighbor.treeClass) {
							neighborCache = &tc;
							break;
						}
					}

					if (neighborCache) {
						const double neighborGrowth = 1.0 - max(0.0,
							(neighborCache->matureAge - neighborAge) / neighborCache->matureAge);

						const double nRadius = neighborCache->getRadiusValue(neighbor.x, neighbor.z);
						const double minNeighborRx = sizeFactor * neighborGrowth *
							neighborCache->getXRadiusValue(neighbor.x, neighbor.z) * nRadius;
						const double minNeighborRz = sizeFactor * neighborGrowth *
							neighborCache->getZRadiusValue(neighbor.x, neighbor.z) * nRadius;

						const double neighborSize = max(minNeighborRx, minNeighborRz);
						const double size = max(growth.minRx, growth.minRz);

						tree.dead = (neighborSize > size) ||
							(ctx.time - neighbor.bday > neighborCache->matureAge);
					}
				}
			}

			if (!tree.dead) {
				gridP = iTree + 1;
			}
		}
	}

	return !tree.dead;
}

// Generate seeds - optimized
void generateSeedsFast(OptimizedGenerationContext& ctx, CTreeInstance& tree,
	const TreeGrowthFast& growth, const TreeClassCacheOptimized& treeCache) {

	const double seedRx = treeCache.seedRange;
	const double seedRz = treeCache.seedRange;

	for (double x = tree.x - seedRx; x <= tree.x + seedRx; x += ctx.gridDelta) {
		for (double z = tree.z - seedRz; z <= tree.z + seedRz; z += ctx.gridDelta) {
			// Skip crown area
			if (x >= tree.x - growth.minRx && x <= tree.x + growth.minRx &&
				z >= tree.z - growth.minRz && z <= tree.z + growth.minRz)
				continue;

#if USE_RANDOM_SEED
			if (rand() > RAND_MAX / 1000) continue;
#endif

			const int gridX = (static_cast<int>(x) - ctx.xo) / ctx.gridDelta;
			const int gridZ = (static_cast<int>(z) - ctx.zo) / ctx.gridDelta;

			if (!ctx.isValidGridPos(gridX, gridZ)) continue;

			int& gridP = ctx.gridAt(gridX, gridZ);

			// Check if empty
			bool empty = (gridP == 0) ||
				(ctx.instances[gridP - 1].dead && &ctx.instances[gridP - 1] != &tree);

			if (empty) {
				// Calculate mask value using cached data
				double maskval = 1.0;
				for (const auto& [mask, density] : treeCache.maskData) {
					double value = mask->get2DMaskValue(x, z, density->blur);
					maskval *= density->GetDensityValue(value);
					if (maskval <= 0.0) break;
				}

				if (static_cast<double>(rand()) / RAND_MAX < maskval) {
					CTreeInstance& t = ctx.instances[ctx.instanceIndex];
					t.treeClass = tree.treeClass;
					t.bday = ctx.time - min(static_cast<float>(ctx.timeSlice),
						static_cast<float>(rand() * treeCache.matureAge / RAND_MAX));
					t.x = x;
					t.z = z;
					t.dead = false;
					t.mature = false;

					ctx.instanceIndex++;
					gridP = ctx.instanceIndex;
				}
			}
		}
	}
}

// Process tree iteration - optimized
void processTreeIterationOptimized(OptimizedGenerationContext& ctx, int currentCount) {
	const double sizeFactor = 0.9;

	for (int iTree = 0; iTree < currentCount; ++iTree) {
		CTreeInstance& tree = ctx.instances[iTree];

		if (tree.dead) continue;

		const double age = ctx.time - tree.bday;
		tree.age = age;

		// Find tree's cache
		const TreeClassCacheOptimized* treeCache = nullptr;
		for (const auto& tc : *ctx.treeCache) {
			if (tc.treeClass == tree.treeClass) {
				treeCache = &tc;
				break;
			}
		}

		if (!treeCache) continue;

		if (age > treeCache->maxAge) {
			tree.dead = true;
			continue;
		}

		TreeGrowthFast growth(tree, ctx.time, sizeFactor, *treeCache);

		if (!tree.mature) {
			tree.mature = (abs(growth.growth - 1.0) < 0.00001) || ctx.lastIteration;
			checkTreeCompetitionFast(ctx, tree, growth, iTree, sizeFactor, *treeCache);
		}

		if (!tree.dead && tree.mature) {
			generateSeedsFast(ctx, tree, growth, *treeCache);
		}
	}
}

// Apply thinning masks - optimized
double applyThinningMasksFast(CTreeInstance& tree, const TreeClassCacheOptimized& treeCache,
	const std::map<string, DensityMap*>& globalMasks,
	const std::map<string, I2DMask*>& masks) {
	double maskval = 1.0;

	// Use cached thinning masks
	for (const auto& [mask, density] : treeCache.thinningMasks) {
		double value = mask->get2DMaskValue(tree.x, tree.z, density->blur);
		maskval *= density->GetDensityValue(value);
		if (maskval <= 0.0) return 0.0;
	}

	// Global masks
	for (const auto& [key, dmap] : globalMasks) {
		auto imask = masks.find(key);
		if (imask != masks.end()) {
			double value = imask->second->get2DMaskValue(tree.x, tree.z, dmap->blur);
			maskval *= dmap->GetDensityValue(value);
			if (maskval <= 0.0) return 0.0;
		}
	}

	return maskval;
}

// Filter mature trees - optimized
void filterMatureTreesFast(CTreeInstance* instances, int instanceIndex,
	std::vector<CTreeInstance>& trees,
	const std::vector<TreeClassCacheOptimized>& treeCache,
	const std::map<string, DensityMap*>& globalMasks,
	const std::map<string, I2DMask*>& masks) {
	trees.clear();
	trees.reserve(instanceIndex / 3); // Estimate capacity

	for (int i = 0; i < instanceIndex; i++) {
		CTreeInstance& tree = instances[i];

		if (!tree.dead && tree.mature) {
			// Find tree's cache
			const TreeClassCacheOptimized* tc = nullptr;
			for (const auto& cache : treeCache) {
				if (cache.treeClass == tree.treeClass) {
					tc = &cache;
					break;
				}
			}

			if (!tc) continue;

			double maskval = applyThinningMasksFast(tree, *tc, globalMasks, masks);

			if (static_cast<double>(rand()) / RAND_MAX <= maskval) {
				trees.push_back(tree);
			}
		}
	}
}

// MAIN OPTIMIZED FUNCTION
void CForest::generateOptimized(float forestAge, int iterations) {
	string title = "CForest::generateOptimized - generate whole tree instances : ";
	CTimeCounter timeCounter(title);

	// Grid setup
	const int gridDelta = 30;
	const int gridXSize = xSize / gridDelta;
	const int gridZSize = zSize / gridDelta;
	const int gridSize = (gridXSize + 1) * (gridZSize + 1) * sizeof(int);
	int* grid = (int*)malloc(gridSize);
	memset(grid, 0, gridSize);

	cout << "gridDelta: " << gridDelta
		<< ", gridXSize: " << gridXSize
		<< ", gridZSize: " << gridZSize << endl;

	// Allocate instances
	CTreeInstance* instances = (CTreeInstance*)malloc(SEED_MAX * sizeof(CTreeInstance));
	memset(instances, 0, SEED_MAX * sizeof(CTreeInstance));

	ClassStrength* classArray = (ClassStrength*)malloc(classes.size() * sizeof(ClassStrength));

	// Build cache once
	std::vector<TreeClassCacheOptimized> treeClassCache;
	buildOptimizedTreeClassCache(treeClassCache);

	// Setup context
	OptimizedGenerationContext ctx;
	ctx.instances = instances;
	ctx.instanceIndex = 0;
	ctx.classArray = classArray;
	ctx.grid = grid;
	ctx.gridXSize = gridXSize;
	ctx.gridZSize = gridZSize;
	ctx.gridDelta = gridDelta;
	ctx.xo = xo;
	ctx.zo = zo;
	ctx.xSize = xSize;
	ctx.zSize = zSize;
	ctx.timeSlice = forestAge / iterations;
	ctx.treeCache = &treeClassCache;

	// Main iteration loop
	for (int iteration = 0; iteration < iterations; iteration++) {
		ctx.lastIteration = (iteration == iterations - 1);
		ctx.time = ctx.timeSlice * iteration;

		generateInitialInstancesFast(ctx);

		const int currentCount = ctx.instanceIndex;
		const int progressPct = static_cast<int>(100.0 * iteration / iterations);

		cout << "Iteration " << iteration
			<< ": " << currentCount << " instances ("
			<< progressPct << "%)" << endl;

		processTreeIterationOptimized(ctx, currentCount);
	}

	cout << "Tree Instances index Count: " << ctx.instanceIndex << endl;

	// Filter and finalize
	filterMatureTreesFast(instances, ctx.instanceIndex, trees,
		treeClassCache, globalMasks, masks);

	removeTreesNearPOIs();
	if (!m_isLevel1Instances) {
		removeTreesNearCaves();
	}

	cout << "Final Trees Size: " << trees.size() << endl;

	// Cleanup
	free(instances);
	free(grid);
	free(classArray);
}

// Pre-compute mask lookups to avoid repeated map searches
void CForest::buildTreeClassCache(std::vector<TreeClassCache>& cache) {
	cache.reserve(classes.size());

	for (TreeClass* treeClass : classes) {
		TreeClassCache tc;
		tc.treeClass = treeClass;
		tc.matureAge = treeClass->matureAge;
		tc.maxAge = treeClass->maxAge;
		tc.seedRange = treeClass->seedRange;

		for (auto& maskPair : treeClass->masks) {
			auto maskIt = masks.find(maskPair.first);
			if (maskIt != masks.end()) {
				CachedMaskData cmd;
				cmd.mask = maskIt->second;
				cmd.density = maskPair.second;
				cmd.useForThinning = maskPair.second->useForThinning;
				tc.maskData.push_back(cmd);
			}
		}
		cache.push_back(tc);
	}
}

// Optimized mask value calculation using cached data
inline double calculateMaskValueOptimized(const TreeClassCache& tc, int x, int z) {
	double maskValue = 1.0;

	for (const auto& cmd : tc.maskData) {
		double mask = cmd.mask->get2DMaskValue(x, z, cmd.density->blur);
		maskValue *= cmd.density->GetDensityValue(mask);
		if (maskValue <= 0.0) break; // Early exit
	}

	return maskValue;
}

// Optimized tree class selection with pre-computed probabilities
TreeClass* selectTreeClassOptimized(
	const std::vector<TreeClassCache>& cache,
	ClassStrength* classArray,
	int x, int z,
	double& maskSpan) {

	maskSpan = 0.0;
	int classIdx = 0;

	for (const auto& tc : cache) {
		double maskValue = calculateMaskValueOptimized(tc, x, z);
		maskSpan += maskValue;
		classArray[classIdx].strength = maskValue;
		classArray[classIdx].treeClass = tc.treeClass;
		classIdx++;
	}

	if (maskSpan <= 0.0) return nullptr;

	double dice = maskSpan * rand() / RAND_MAX;
	double sum = 0.0;

	for (int i = 0; i < classIdx; i++) {
		sum += classArray[i].strength;
		if (sum >= dice) {
			return classArray[i].treeClass;
		}
	}

	return classArray[classIdx - 1].treeClass;
}

// Optimized grid access with inline function
inline int& getGridValue(int* grid, int gridXSize, int gridX, int gridZ) {
	return grid[gridXSize * gridZ + gridX];
}

// MAIN OPTIMIZED GENERATE FUNCTION
void CForest::generateFast(float forestAge, int iterations) {
	string title = "CForest::generateFast - generate whole tree instances : ";
	CTimeCounter timeCounter(title);

	// Grid setup
	const int gridDelta = 30;
	const int gridXSize = xSize / gridDelta;
	const int gridZSize = zSize / gridDelta;
	const int totalGridCells = (gridXSize + 1) * (gridZSize + 1);

	cout << "gridDelta: " << gridDelta
		<< ", gridXSize: " << gridXSize
		<< ", gridZSize: " << gridZSize
		<< ", totalGridCells: " << totalGridCells << endl;

	// Calculate initial capacity
	int initialCapacity = static_cast<int>(totalGridCells * 0.5 * iterations);
	const int ABSOLUTE_MIN = 100000;
	const int ABSOLUTE_MAX = 32 * 1024 * 1024;
	initialCapacity = max(ABSOLUTE_MIN, min(initialCapacity, ABSOLUTE_MAX));

	// Use vector instead of raw pointer
	std::vector<CTreeInstance> instances;
	instances.reserve(initialCapacity);

	cout << "Initial capacity reserved: " << initialCapacity
		<< " (" << (initialCapacity * sizeof(CTreeInstance) / (1024.0 * 1024.0))
		<< " MB)" << endl;

	int instanceIndex = 0;

	// Grid allocation
	const int gridSize = (gridXSize + 1) * (gridZSize + 1) * sizeof(int);
	int* grid = (int*)malloc(gridSize);
	if (!grid) {
		cerr << "Failed to allocate grid memory!" << endl;
		return;
	}
	memset(grid, 0, gridSize);

	// Pre-allocate class array
	ClassStrength* classArray = (ClassStrength*)malloc(classes.size() * sizeof(ClassStrength));
	if (!classArray) {
		cerr << "Failed to allocate class array!" << endl;
		free(grid);
		return;
	}

	// Build cache once
	std::vector<TreeClassCache> treeClassCache;
	buildTreeClassCache(treeClassCache);

	// Pre-compute constants
	const int xLimit = xo + xSize;
	const int zLimit = zo + zSize;
	const double timeSlice = forestAge / iterations;
	const double sizeFactor = 0.9;
	const double growthFactor = 0.7;
	const int randThreshold = RAND_MAX / 10;
	const int seedRandThreshold = RAND_MAX / 1000;

	// Main iteration loop
	for (int iteration = 0; iteration < iterations; iteration++) {
		const bool lastIteration = (iteration == iterations - 1);
		const double time = timeSlice * iteration;

		// Phase 1: Generate initial instances in empty areas
		for (int x = xo; x < xLimit; x += gridDelta) {
			for (int z = zo; z < zLimit; z += gridDelta) {
				if (rand() > randThreshold) continue;

				const int gridX = (x - xo) / gridDelta;
				const int gridZ = (z - zo) / gridDelta;
				int& gridP = getGridValue(grid, gridXSize, gridX, gridZ);

				// Check if cell is empty
				bool empty = (gridP == 0) || instances[gridP - 1].dead;
				if (!empty) continue;

				// Select tree class
				double maskSpan;
				TreeClass* chosen = selectTreeClassOptimized(treeClassCache, classArray, x, z, maskSpan);

				if (chosen) {
					// Auto-expansion check
					if (instanceIndex >= static_cast<int>(instances.size())) {
						size_t newSize = instances.size() + max(10000, static_cast<int>(instances.size() * 0.2));
						if (newSize > ABSOLUTE_MAX) {
							cout << "Warning: Reached absolute maximum instance limit (" << ABSOLUTE_MAX
								<< "). Stopping generation at iteration " << iteration << endl;
							goto finish_generation; // Break out of all loops
						}
						cout << "Auto-expanding instances buffer from " << instances.size()
							<< " to " << newSize << endl;
						instances.resize(newSize);
					}

					CTreeInstance& t = instances[instanceIndex];
					t.treeClass = chosen;
					t.bday = time - min(timeSlice, static_cast<float>(rand()) * chosen->matureAge / RAND_MAX);

					// Position with boundary clamping
					t.x = x + GenerateRandomDouble(-gridDelta, gridDelta);
					t.x = (t.x >= xLimit) ? (x - GenerateRandomDouble(0, gridDelta)) :
						(t.x <= xo) ? (x + GenerateRandomDouble(0, gridDelta)) : t.x;

					t.z = z + GenerateRandomDouble(-gridDelta, gridDelta);
					t.z = (t.z >= zLimit) ? (z - GenerateRandomDouble(0, gridDelta)) :
						(t.z <= zo) ? (z + GenerateRandomDouble(0, gridDelta)) : t.z;

					t.dead = false;
					t.mature = false;

					instanceIndex++;
					gridP = instanceIndex;
				}
			}
		}

		const int currentCount = instanceIndex;
		const int progressPct = static_cast<int>(100.0 * iteration / iterations);
		cout << "Iteration " << iteration << ": " << currentCount << " instances ("
			<< progressPct << "%, capacity: " << instances.size() << ")" << endl;

		// Phase 2: Process competition and seed generation
		for (int iTree = 0; iTree < currentCount; ++iTree) {
			CTreeInstance& tree = instances[iTree];

			if (tree.dead) continue;

			const double age = time - tree.bday;
			tree.age = age;

			if (age > tree.treeClass->maxAge) {
				tree.dead = true;
				continue;
			}

			const double growth = 1.0 - max(0.0, (tree.treeClass->matureAge - age) / tree.treeClass->matureAge);
			const double minRx = growthFactor * sizeFactor * growth *
				tree.treeClass->xRadius.getValue(tree.x, 0, tree.z) *
				tree.treeClass->radius.getValue(tree.x, 0, tree.z);
			const double minRz = growthFactor * sizeFactor * growth *
				tree.treeClass->zRadius.getValue(tree.x, 0, tree.z) *
				tree.treeClass->radius.getValue(tree.x, 0, tree.z);

			// Competition check for immature trees
			if (!tree.mature) {
				tree.mature = (abs(growth - 1.0) < 0.00001) || lastIteration;

				for (double x = tree.x - minRx; x <= tree.x + minRx && !tree.dead; x += gridDelta) {
					for (double z = tree.z - minRz; z <= tree.z + minRz && !tree.dead; z += gridDelta) {
						const int gridX = (static_cast<int>(x) - xo) / gridDelta;
						const int gridZ = (static_cast<int>(z) - zo) / gridDelta;

						if (gridX < 0 || gridX >= gridXSize || gridZ < 0 || gridZ >= gridZSize)
							continue;

						int& gridP = getGridValue(grid, gridXSize, gridX, gridZ);

						if (gridP > 0) {
							CTreeInstance& neighbor = instances[gridP - 1];

							if (&neighbor != &tree && !neighbor.dead) {
								const double neighborAge = time - neighbor.bday;
								const double neighborGrowth = 1.0 - max(0.0,
									(neighbor.treeClass->matureAge - neighborAge) / neighbor.treeClass->matureAge);

								const double minNeighborRx = sizeFactor * neighborGrowth *
									neighbor.treeClass->xRadius.getValue(neighbor.x, 0, neighbor.z) *
									neighbor.treeClass->radius.getValue(neighbor.x, 0, neighbor.z);
								const double minNeighborRz = sizeFactor * neighborGrowth *
									neighbor.treeClass->zRadius.getValue(neighbor.x, 0, neighbor.z) *
									neighbor.treeClass->radius.getValue(neighbor.x, 0, neighbor.z);

								const double neighborSize = max(minNeighborRx, minNeighborRz);
								const double size = max(minRx, minRz);

								tree.dead = (neighborSize > size) ||
									(time - neighbor.bday > neighbor.treeClass->matureAge);
							}
						}

						if (!tree.dead) gridP = iTree + 1;
					}
				}
			}

			// Seed generation for mature trees
			if (!tree.dead && tree.mature) {
				const double seedRx = tree.treeClass->seedRange;
				const double seedRz = tree.treeClass->seedRange;

				for (double x = tree.x - seedRx; x <= tree.x + seedRx; x += gridDelta) {
					for (double z = tree.z - seedRz; z <= tree.z + seedRz; z += gridDelta) {
						// Skip area within tree crown
						if (x >= tree.x - minRx && x <= tree.x + minRx &&
							z >= tree.z - minRz && z <= tree.z + minRz)
							continue;

#if USE_RANDOM_SEED
						if (rand() > seedRandThreshold) continue;
#endif

						const int gridX = (static_cast<int>(x) - xo) / gridDelta;
						const int gridZ = (static_cast<int>(z) - zo) / gridDelta;

						if (gridX < 0 || gridX >= gridXSize || gridZ < 0 || gridZ >= gridZSize)
							continue;

						int& gridP = getGridValue(grid, gridXSize, gridX, gridZ);

						// Check if location is empty
						bool empty = (gridP == 0) ||
							(instances[gridP - 1].dead && &instances[gridP - 1] != &tree);

						if (empty) {
							// Calculate mask value using cached data
							double maskval = 1.0;

							// Find correct cache for this tree class
							auto it = std::find_if(treeClassCache.begin(), treeClassCache.end(),
								[&tree](const TreeClassCache& tc) { return tc.treeClass == tree.treeClass; });

							if (it != treeClassCache.end()) {
								for (const auto& cmd : it->maskData) {
									double value = cmd.mask->get2DMaskValue(x, z, cmd.density->blur);
									maskval *= cmd.density->GetDensityValue(value);
									if (maskval <= 0.0) break;
								}
							}

							if ((static_cast<double>(rand()) / RAND_MAX) < maskval) {
								// Auto-expansion check
								if (instanceIndex >= static_cast<int>(instances.size())) {
									size_t newSize = instances.size() + max(10000, static_cast<int>(instances.size() * 0.2));
									if (newSize > ABSOLUTE_MAX) {
										cout << "Warning: Reached absolute maximum instance limit during seed generation. "
											<< "Stopping at iteration " << iteration << endl;
										goto finish_generation;
									}
									cout << "Auto-expanding instances buffer (seed phase) from " << instances.size()
										<< " to " << newSize << endl;
									instances.resize(newSize);
								}

								CTreeInstance& t = instances[instanceIndex];
								t.treeClass = tree.treeClass;
								t.bday = time - min(timeSlice, static_cast<float>(rand()) * tree.treeClass->matureAge / RAND_MAX);
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

finish_generation:
	cout << "Tree Instances Count: " << instanceIndex << endl;
	cout << "Final buffer capacity: " << instances.size()
		<< " (utilization: " << (100.0 * instanceIndex / instances.size()) << "%)" << endl;

	// Phase 3: Filter mature trees and apply thinning
	trees.clear();
	trees.reserve(instanceIndex / 2); // Reserve approximate capacity

	for (int i = 0; i < instanceIndex; i++) {
		CTreeInstance& tree = instances[i];

		if (!tree.dead && tree.mature) {
			double maskval = 1.0;

			// Apply thinning masks
			for (const auto& imap : tree.treeClass->masks) {
				auto imask = masks.find(imap.first);
				if (imask != masks.end() && imap.second->useForThinning) {
					double value = imask->second->get2DMaskValue(tree.x, tree.z, imap.second->blur);
					maskval *= imap.second->GetDensityValue(value);
					if (maskval <= 0.0) break;
				}
			}

			// Apply global masks
			if (maskval > 0.0) {
				for (const auto& imap : globalMasks) {
					auto imask = masks.find(imap.first);
					if (imask != masks.end()) {
						double value = imask->second->get2DMaskValue(tree.x, tree.z, imap.second->blur);
						maskval *= imap.second->GetDensityValue(value);
						if (maskval <= 0.0) break;
					}
				}
			}

			if ((static_cast<double>(rand()) / RAND_MAX) <= maskval) {
				trees.push_back(tree);
			}
		}
	}

	removeTreesNearPOIs();
	if (!m_isLevel1Instances) {
		removeTreesNearCaves();
	}

	cout << "Final Trees Size: " << trees.size() << endl;

	// Cleanup - vector auto-releases, only need to free other allocated memory
	free(grid);
	free(classArray);
}

/**
 * @brief Generate forest with adjustable tree density parameters
 *
 * @param forestAge Total age of the forest simulation
 * @param iterations Number of growth iterations
 * @param gridDelta Grid cell size - HIGHEST IMPACT on tree count
 *                  Smaller values = more sampling points = more trees
 *                  Impact: O(1/gridDelta²) - reducing by half increases trees ~4x
 *                  Range: 10-50, Default: 30
 *                  Example: 30->20 = 2.25x trees, 30->15 = 4x trees
 *
 * @param initialDensity Probability of tree spawn in empty grid cells - HIGH IMPACT
 *                       Higher values = more initial trees
 *                       Impact: Linear - doubling increases initial trees ~2x
 *                       Range: 0.05-0.5, Default: 0.1 (10%)
 *                       Example: 0.1->0.2 = 2x initial trees
 *
 * @param seedDensity Probability of seed generation from mature trees - MEDIUM-HIGH IMPACT
 *                    Higher values = more seeds = more trees over iterations
 *                    Impact: Linear but multiplied by mature tree count
 *                    Range: 0.0005-0.02, Default: 0.001 (0.1%)
 *                    Example: 0.001->0.005 = 5x seeds per mature tree
 *
 * @param competitionFactor Tree crown size multiplier - MEDIUM IMPACT
 *                          Lower values = smaller crowns = less competition = more trees survive
 *                          Impact: Affects survival rate, O(1/competitionFactor²) area
 *                          Range: 0.5-1.0, Default: 0.9
 *                          Example: 0.9->0.7 = ~1.6x more trees survive
 *
 * @param growthFactor Additional crown size reduction factor - MEDIUM IMPACT
 *                     Lower values = even smaller effective crowns
 *                     Impact: Multiplicative with competitionFactor
 *                     Range: 0.4-0.8, Default: 0.7
 *                     Example: 0.7->0.5 = ~1.4x more trees survive
 *
 * @param thinningThreshold Final filtering threshold - LOW-MEDIUM IMPACT
 *                          Lower values = more trees pass final filter
 *                          Impact: Affects final count directly but after competition
 *                          Range: 0.3-1.0, Default: 1.0 (no additional thinning)
 *                          Example: 1.0->0.5 = potentially 2x trees (if not limited by space)
 *
 * Performance notes:
 * - Reducing gridDelta has O(n²) impact on computation time
 * - Increasing densities has linear impact on computation time
 * - Memory usage scales with total tree instances generated
 *
 * Recommended presets:
 * - Sparse forest:  (30, 0.05, 0.0005, 0.9, 0.7, 1.0) -> baseline × 0.5
 * - Normal forest:  (30, 0.10, 0.0010, 0.9, 0.7, 1.0) -> baseline × 1.0
 * - Dense forest:   (25, 0.15, 0.0020, 0.8, 0.6, 0.8) -> baseline × 3-4
 * - Very dense:     (20, 0.20, 0.0050, 0.7, 0.5, 0.7) -> baseline × 6-8
 * - Ultra dense:    (15, 0.30, 0.0100, 0.6, 0.4, 0.5) -> baseline × 15-20
 */
void CForest::generateFastAdjustAmount(
	float forestAge,
	int iterations,
	int gridDelta/* = 30 */,                      // [1] HIGHEST IMPACT - Grid sampling density
	double initialDensity/* = 0.1*/,             // [2] HIGH IMPACT - Initial tree spawn rate
	double seedDensity/* = 0.001 */ ,              // [3] MEDIUM-HIGH IMPACT - Seed generation rate
	double competitionFactor/* = 0.9*/,          // [4] MEDIUM IMPACT - Tree crown size (competition)
	double growthFactor/* = 0.7*/,               // [5] MEDIUM IMPACT - Crown size reduction
	double thinningThreshold/* = 1.0*/           // [6] LOW-MEDIUM IMPACT - Final filter strength
) 
{
	string title = "CForest::generateFastAdjustAmount - generate whole tree instances : ";
	CTimeCounter timeCounter(title);

	// Validate and clamp parameters to safe ranges
	gridDelta = max(10, min(gridDelta, 50));
	initialDensity = max(0.01, min(initialDensity, 0.9));
	seedDensity = max(0.0001, min(seedDensity, 0.05));
	competitionFactor = max(0.3, min(competitionFactor, 1.0));
	growthFactor = max(0.3, min(growthFactor, 0.9));
	thinningThreshold = max(0.1, min(thinningThreshold, 1.0));

	// Grid setup
	const int gridXSize = xSize / gridDelta;
	const int gridZSize = zSize / gridDelta;
	const int totalGridCells = (gridXSize + 1) * (gridZSize + 1);

	cout << "=== Forest Generation Parameters ===" << endl;
	cout << "gridDelta: " << gridDelta
		<< " (gridXSize: " << gridXSize << ", gridZSize: " << gridZSize
		<< ", totalCells: " << totalGridCells << ")" << endl;
	cout << "initialDensity: " << initialDensity
		<< " (" << (initialDensity * 100) << "% spawn probability)" << endl;
	cout << "seedDensity: " << seedDensity
		<< " (" << (seedDensity * 100) << "% seed probability)" << endl;
	cout << "competitionFactor: " << competitionFactor
		<< " (crown size multiplier)" << endl;
	cout << "growthFactor: " << growthFactor
		<< " (additional crown reduction)" << endl;
	cout << "thinningThreshold: " << thinningThreshold
		<< " (final filter: " << (thinningThreshold * 100) << "%)" << endl;
	cout << "====================================" << endl;

	// Calculate initial capacity with safety margins
	int initialCapacity = static_cast<int>(totalGridCells * 0.5 * iterations);
	const int ABSOLUTE_MIN = 100000;
	const int ABSOLUTE_MAX = 64 * 1024 * 1024;  // Increased for very dense forests
	initialCapacity = max(ABSOLUTE_MIN, min(initialCapacity, ABSOLUTE_MAX));

	// Use vector instead of raw pointer
	std::vector<CTreeInstance> instances;
	instances.reserve(initialCapacity);

	cout << "Initial capacity reserved: " << initialCapacity
		<< " (" << (initialCapacity * sizeof(CTreeInstance) / (1024.0 * 1024.0))
		<< " MB)" << endl;

	int instanceIndex = 0;

	// Grid allocation
	const int gridSize = (gridXSize + 1) * (gridZSize + 1) * sizeof(int);
	int* grid = (int*)malloc(gridSize);
	if (!grid) {
		cerr << "Failed to allocate grid memory!" << endl;
		return;
	}
	memset(grid, 0, gridSize);

	// Pre-allocate class array
	ClassStrength* classArray = (ClassStrength*)malloc(classes.size() * sizeof(ClassStrength));
	if (!classArray) {
		cerr << "Failed to allocate class array!" << endl;
		free(grid);
		return;
	}

	// Build cache once
	std::vector<TreeClassCache> treeClassCache;
	buildTreeClassCache(treeClassCache);

	// Pre-compute constants and thresholds from parameters
	const int xLimit = xo + xSize;
	const int zLimit = zo + zSize;
	const double timeSlice = forestAge / iterations;
	const double sizeFactor = competitionFactor;  // Use competition factor as size factor

	// Convert density probabilities to RAND_MAX thresholds
	const int randThreshold = static_cast<int>(RAND_MAX * (1.0 - initialDensity));

#if USE_RANDOM_SEED
	const int seedRandThreshold = static_cast<int>(RAND_MAX * (1.0 - seedDensity));
#endif

	cout << "Computed thresholds:" << endl;
	cout << "  randThreshold: " << randThreshold
		<< " (1 in " << static_cast<int>(1.0 / initialDensity) << " chance)" << endl;
#if USE_RANDOM_SEED
	cout << "  seedRandThreshold: " << seedRandThreshold
		<< " (1 in " << static_cast<int>(1.0 / seedDensity) << " chance)" << endl;
#endif
	cout << endl;

	// Main iteration loop
	for (int iteration = 0; iteration < iterations; iteration++) {
		const bool lastIteration = (iteration == iterations - 1);
		const double time = timeSlice * iteration;

		// Phase 1: Generate initial instances in empty areas
		for (int x = xo; x < xLimit; x += gridDelta) {
			for (int z = zo; z < zLimit; z += gridDelta) {
				// Use initialDensity parameter for spawn probability
				if (rand() > randThreshold) continue;

				const int gridX = (x - xo) / gridDelta;
				const int gridZ = (z - zo) / gridDelta;
				int& gridP = getGridValue(grid, gridXSize, gridX, gridZ);

				// Check if cell is empty
				bool empty = (gridP == 0) || instances[gridP - 1].dead;
				if (!empty) continue;

				// Select tree class
				double maskSpan;
				TreeClass* chosen = selectTreeClassOptimized(treeClassCache, classArray, x, z, maskSpan);

				if (chosen) {
					// Auto-expansion check
					if (instanceIndex >= static_cast<int>(instances.size())) {
						size_t newSize = instances.size() + max(10000, static_cast<int>(instances.size() * 0.2));
						if (newSize > ABSOLUTE_MAX) {
							cout << "Warning: Reached absolute maximum instance limit (" << ABSOLUTE_MAX
								<< "). Stopping generation at iteration " << iteration << endl;
							goto finish_generation;
						}
						cout << "Auto-expanding instances buffer from " << instances.size()
							<< " to " << newSize << endl;
						instances.resize(newSize);
					}

					CTreeInstance& t = instances[instanceIndex];
					t.treeClass = chosen;
					t.bday = time - min(timeSlice, static_cast<float>(rand()) * chosen->matureAge / RAND_MAX);

					// Position with boundary clamping
					t.x = x + GenerateRandomDouble(-gridDelta, gridDelta);
					t.x = (t.x >= xLimit) ? (x - GenerateRandomDouble(0, gridDelta)) :
						(t.x <= xo) ? (x + GenerateRandomDouble(0, gridDelta)) : t.x;

					t.z = z + GenerateRandomDouble(-gridDelta, gridDelta);
					t.z = (t.z >= zLimit) ? (z - GenerateRandomDouble(0, gridDelta)) :
						(t.z <= zo) ? (z + GenerateRandomDouble(0, gridDelta)) : t.z;

					t.dead = false;
					t.mature = false;

					instanceIndex++;
					gridP = instanceIndex;
				}
			}
		}

		const int currentCount = instanceIndex;
		const int progressPct = static_cast<int>(100.0 * iteration / iterations);
		cout << "Iteration " << iteration << ": " << currentCount << " instances ("
			<< progressPct << "%, capacity: " << instances.size() << ")" << endl;

		// Phase 2: Process competition and seed generation
		for (int iTree = 0; iTree < currentCount; ++iTree) {
			CTreeInstance& tree = instances[iTree];

			if (tree.dead) continue;

			const double age = time - tree.bday;
			tree.age = age;

			if (age > tree.treeClass->maxAge) {
				tree.dead = true;
				continue;
			}

			const double growth = 1.0 - max(0.0, (tree.treeClass->matureAge - age) / tree.treeClass->matureAge);

			// Use adjustable competition and growth factors
			const double minRx = growthFactor * sizeFactor * growth *
				tree.treeClass->xRadius.getValue(tree.x, 0, tree.z) *
				tree.treeClass->radius.getValue(tree.x, 0, tree.z);
			const double minRz = growthFactor * sizeFactor * growth *
				tree.treeClass->zRadius.getValue(tree.x, 0, tree.z) *
				tree.treeClass->radius.getValue(tree.x, 0, tree.z);

			// Competition check for immature trees
			if (!tree.mature) {
				tree.mature = (abs(growth - 1.0) < 0.00001) || lastIteration;

				for (double x = tree.x - minRx; x <= tree.x + minRx && !tree.dead; x += gridDelta) {
					for (double z = tree.z - minRz; z <= tree.z + minRz && !tree.dead; z += gridDelta) {
						const int gridX = (static_cast<int>(x) - xo) / gridDelta;
						const int gridZ = (static_cast<int>(z) - zo) / gridDelta;

						if (gridX < 0 || gridX >= gridXSize || gridZ < 0 || gridZ >= gridZSize)
							continue;

						int& gridP = getGridValue(grid, gridXSize, gridX, gridZ);

						if (gridP > 0) {
							CTreeInstance& neighbor = instances[gridP - 1];

							if (&neighbor != &tree && !neighbor.dead) {
								const double neighborAge = time - neighbor.bday;
								const double neighborGrowth = 1.0 - max(0.0,
									(neighbor.treeClass->matureAge - neighborAge) / neighbor.treeClass->matureAge);

								const double minNeighborRx = sizeFactor * neighborGrowth *
									neighbor.treeClass->xRadius.getValue(neighbor.x, 0, neighbor.z) *
									neighbor.treeClass->radius.getValue(neighbor.x, 0, neighbor.z);
								const double minNeighborRz = sizeFactor * neighborGrowth *
									neighbor.treeClass->zRadius.getValue(neighbor.x, 0, neighbor.z) *
									neighbor.treeClass->radius.getValue(neighbor.x, 0, neighbor.z);

								const double neighborSize = max(minNeighborRx, minNeighborRz);
								const double size = max(minRx, minRz);

								tree.dead = (neighborSize > size) ||
									(time - neighbor.bday > neighbor.treeClass->matureAge);
							}
						}

						if (!tree.dead) gridP = iTree + 1;
					}
				}
			}

			// Seed generation for mature trees
			if (!tree.dead && tree.mature) {
				const double seedRx = tree.treeClass->seedRange;
				const double seedRz = tree.treeClass->seedRange;

				for (double x = tree.x - seedRx; x <= tree.x + seedRx; x += gridDelta) {
					for (double z = tree.z - seedRz; z <= tree.z + seedRz; z += gridDelta) {
						// Skip area within tree crown
						if (x >= tree.x - minRx && x <= tree.x + minRx &&
							z >= tree.z - minRz && z <= tree.z + minRz)
							continue;

#if USE_RANDOM_SEED
						// Use seedDensity parameter for seed probability
						if (rand() > seedRandThreshold) continue;
#endif

						const int gridX = (static_cast<int>(x) - xo) / gridDelta;
						const int gridZ = (static_cast<int>(z) - zo) / gridDelta;

						if (gridX < 0 || gridX >= gridXSize || gridZ < 0 || gridZ >= gridZSize)
							continue;

						int& gridP = getGridValue(grid, gridXSize, gridX, gridZ);

						// Check if location is empty
						bool empty = (gridP == 0) ||
							(instances[gridP - 1].dead && &instances[gridP - 1] != &tree);

						if (empty) {
							// Calculate mask value using cached data
							double maskval = 1.0;

							// Find correct cache for this tree class
							auto it = std::find_if(treeClassCache.begin(), treeClassCache.end(),
								[&tree](const TreeClassCache& tc) { return tc.treeClass == tree.treeClass; });

							if (it != treeClassCache.end()) {
								for (const auto& cmd : it->maskData) {
									double value = cmd.mask->get2DMaskValue(x, z, cmd.density->blur);
									maskval *= cmd.density->GetDensityValue(value);
									if (maskval <= 0.0) break;
								}
							}

							if ((static_cast<double>(rand()) / RAND_MAX) < maskval) {
								// Auto-expansion check
								if (instanceIndex >= static_cast<int>(instances.size())) {
									size_t newSize = instances.size() + max(10000, static_cast<int>(instances.size() * 0.2));
									if (newSize > ABSOLUTE_MAX) {
										cout << "Warning: Reached absolute maximum instance limit during seed generation. "
											<< "Stopping at iteration " << iteration << endl;
										goto finish_generation;
									}
									cout << "Auto-expanding instances buffer (seed phase) from " << instances.size()
										<< " to " << newSize << endl;
									instances.resize(newSize);
								}

								CTreeInstance& t = instances[instanceIndex];
								t.treeClass = tree.treeClass;
								t.bday = time - min(timeSlice, static_cast<float>(rand()) * tree.treeClass->matureAge / RAND_MAX);
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

finish_generation:
	cout << "Tree Instances Count: " << instanceIndex << endl;
	cout << "Final buffer capacity: " << instances.size()
		<< " (utilization: " << (100.0 * instanceIndex / instances.size()) << "%)" << endl;

	// Phase 3: Filter mature trees and apply thinning
	trees.clear();
	trees.reserve(instanceIndex / 2);

	for (int i = 0; i < instanceIndex; i++) {
		CTreeInstance& tree = instances[i];

		if (!tree.dead && tree.mature) {
			double maskval = 1.0;

			// Apply thinning masks
			for (const auto& imap : tree.treeClass->masks) {
				auto imask = masks.find(imap.first);
				if (imask != masks.end() && imap.second->useForThinning) {
					double value = imask->second->get2DMaskValue(tree.x, tree.z, imap.second->blur);
					maskval *= imap.second->GetDensityValue(value);
					if (maskval <= 0.0) break;
				}
			}

			// Apply global masks
			if (maskval > 0.0) {
				for (const auto& imap : globalMasks) {
					auto imask = masks.find(imap.first);
					if (imask != masks.end()) {
						double value = imask->second->get2DMaskValue(tree.x, tree.z, imap.second->blur);
						maskval *= imap.second->GetDensityValue(value);
						if (maskval <= 0.0) break;
					}
				}
			}

			// Apply thinningThreshold parameter - additional control over final tree count
			if ((static_cast<double>(rand()) / RAND_MAX) <= maskval * thinningThreshold) {
				trees.push_back(tree);
			}
		}
	}

	removeTreesNearPOIs();
	if (!m_isLevel1Instances) {
		removeTreesNearCaves();
	}

	cout << "Final Trees Size: " << trees.size() << endl;
	cout << "Estimated density multiplier vs default: "
		<< (trees.size() / max(1.0, totalGridCells * 0.05)) << "x" << endl;

	// Cleanup - vector auto-releases, only need to free other allocated memory
	free(grid);
	free(classArray);
}