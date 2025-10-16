#pragma once

#include <vector>
#include <map>
#include <string>

#include "I2DMask.h"

#if __APPLE__
    #include "../Common/include/TreeInstance.h"
#else
    #include "..\Common\include\TreeInstance.h"
#endif

using namespace std;
class CCellInfo;
struct InputImageMetaInfo;

pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithIndex(PlantType type, int index, I2DMask* i2dMask);
pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType plantType, DensityMapType densityType, I2DMask* pI2dMask);
pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithDensityMapTypeIndex(PlantType plantType, DensityMapType densityType, I2DMask* pI2dMask);
pair<string, I2DMask*> GetI2DMaskKeyPairFromTreeClassWithDensityMapType(TreeClass * treeClass, DensityMapType densityType, I2DMask* pI2dMask);

struct ClassStrength
{
	double strength;
	TreeClass* treeClass;
};

// Helper structure and initialization
struct GridInfo {
	int gridDelta;
	int gridXSize;
	int gridZSize;
	int gridTotalSize;
	int* girdData;

	GridInfo(int forestXSize, int forestZSize, int gridDelta)
		: gridDelta(gridDelta),
		gridXSize(forestXSize / gridDelta),
		gridZSize(forestZSize / gridDelta) {
		gridTotalSize = (gridXSize + 1) * (gridZSize + 1) * sizeof(int);
		girdData = (int*)malloc(gridTotalSize);
		memset(girdData, 0, gridTotalSize);
	}

	~GridInfo() {
		if (girdData) free(girdData);
	}

	int& at(int gridX, int gridZ) {
		return girdData[gridXSize * gridZ + gridX];
	}
};

struct GenerationContext {
	CTreeInstance* instances;
	int instanceIndex;
	ClassStrength* classArray;
	GridInfo& grid;
	int xo, zo, xSize, zSize;
	double time;
	double timeSlice;
	bool lastIteration;
};

// Calculate tree growth parameters
struct TreeGrowth {
	double age;
	double growth;
	double minRx;
	double minRz;
	bool mature;
};

struct TreeClassCacheOptimized {
	TreeClass* treeClass;
	std::vector<std::pair<I2DMask*, DensityMap*>> maskData;
	std::vector<std::pair<I2DMask*, DensityMap*>> thinningMasks;
	double matureAge;
	double maxAge;
	double seedRange;

	// Pre-computed radius accessors
	inline double getRadiusValue(double x, double z) const {
		return treeClass->radius.getValue(x, 0, z);
	}

	inline double getXRadiusValue(double x, double z) const {
		return treeClass->xRadius.getValue(x, 0, z);
	}

	inline double getZRadiusValue(double x, double z) const {
		return treeClass->zRadius.getValue(x, 0, z);
	}
};

struct OptimizedGenerationContext {
	CTreeInstance* instances;
	int instanceIndex;
	ClassStrength* classArray;
	int* grid;
	int gridXSize;
	int gridZSize;
	int gridDelta;
	int xo, zo, xSize, zSize;
	double time;
	double timeSlice;
	bool lastIteration;
	const std::vector<TreeClassCacheOptimized>* treeCache;

	inline int& gridAt(int gridX, int gridZ) {
		return grid[gridXSize * gridZ + gridX];
	}

	inline bool isValidGridPos(int gridX, int gridZ) const {
		return gridX >= 0 && gridX < gridXSize && gridZ >= 0 && gridZ < gridZSize;
	}
};

// Optimized helper structures
struct CachedMaskData {
	I2DMask* mask;
	DensityMap* density;
	bool useForThinning;
};

struct TreeClassCache {
	TreeClass* treeClass;
	std::vector<CachedMaskData> maskData;
	double matureAge;
	double maxAge;
	double seedRange;
};

class CForest
{
public:
	CForest(void);
	~CForest(void);
public:
	void generateFast(float forestAge, int iterations);
	void generateOptimized(float forestAge, int iterations);
	void generate2(float forestAge, int iterations);
	void generate(float forestAge, int iterations);
	void loadDefaultTreeClasses();
	void loadDefaultMasks();
	void loadDefaultGlobalMasks();

	void doLoadDefaultTreeClasses();
	void doLoadDefaultMasks();
	void doLoadDefaultGlobalMasks();

	void resetTreeClasses();
	void resetMasks();
	void resetGlobalMasks();
	void resetRawI2DMasks();

	void ResetMasksAndClasses();

	void setCellTable(std::vector<std::vector<CCellInfo*>>* pCellTable) {
		m_pCellTable = pCellTable;
	}
	void setMetaInfo(InputImageMetaInfo* metaInfo)
	{
		m_pMetaInfo = metaInfo;
	}
	vector<TreeInstanceFullOutput>* getTreeInstanceFullOutput()
	{
		return &fullOutputs;
	}
	void set2dCaveLevel0Nodes(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel0Nodes)
	{
		m_p2dCaveLevel0Nodes = p2dCaveLevel0Nodes;
	}
	void set2dCaveLevel1Nodes(std::vector<std::pair<std::vector<Point>, int>>* p2dCaveLevel1Nodes)
	{
		m_p2dCaveLevel1Nodes = p2dCaveLevel1Nodes;
	}
	void setPoisLocations(std::vector<Point>* pPoisLocations)
	{
		m_pPoisLocations = pPoisLocations;
	}
	void setIsLevel1Instances(bool isLevel1Instances)
	{
		m_isLevel1Instances = isLevel1Instances;
		std::cout << "Set CForest isLevel1Instances to " << m_isLevel1Instances << std::endl;
	}
	TreeInstanceOutput GetTreeOutputFromInstance(const CTreeInstance& instance);
	bool exportTreeInstanceOutput(const std::vector<TreeInstanceOutput>& data, const std::string& filename, bool hasHeader);
	bool exportTreeInstanceFullOutput(const std::vector<TreeInstanceFullOutput>& data, const std::string& filename, bool hasHeader, bool withRatio = false);
	bool outputTreeInstanceResults(const std::string& fileName, bool hasHeader);
	bool outputCSVTreeInstanceResults(const std::string& fileName);
	bool outputPointsCloudTreeInstanceResults(const std::string& fileName);
	bool outputFullTreeInstanceResults(const std::string& fileName, bool hasHeader, bool withRatio = false);
	bool outputCSVFullTreeInstanceResults(const std::string& fileName);
	bool outputCSVFullTreeInstanceResultsWithRatio(const std::string& fileName);
	bool outputPointsCloudFullTreeInstanceResults(const std::string& fileName);
	bool outputPointsCloudFullTreeInstanceResultsWithRatio(const std::string& fileName);
	bool parseTreeListCsv(const string& inputTreeListCsv);
	TreeClass* getTreeClassFromStringVector(const std::vector<std::string>& row, int columnCount);

	void removeTreesNearPOIs();
	void removeTreesNearCaves();
	double calculateMaskValue(TreeClass* treeClass, int x, int z);
	TreeClass* selectTreeClass(ClassStrength* classArray, int x, int z);
	double clampPosition(double pos, double gridPos, double offset, int limit, int origin, bool isAtLimit);
	void createTreeInstance(GenerationContext& ctx, TreeClass* treeClass,int x, int z, int gridX, int gridZ);
	bool isGridCellAvailable(GenerationContext& ctx, int gridX, int gridZ);
	void generateInitialInstances(GenerationContext& ctx);
	TreeGrowth calculateTreeGrowth(CTreeInstance& tree, double time, double sizeFactor);
	bool checkTreeCompetition(GenerationContext& ctx, CTreeInstance& tree, const TreeGrowth& growth, int iTree, double sizeFactor);
	void generateSeeds(GenerationContext& ctx, CTreeInstance& tree, const TreeGrowth& growth);
	void processTreeIterationForDominatePlants(GenerationContext& ctx, int currentCount);
	double applyThinningMasks(CTreeInstance& tree);
	void filterMatureTrees(CTreeInstance* instances, int instanceIndex);

	void buildOptimizedTreeClassCache(std::vector<TreeClassCacheOptimized>& cache);

	void buildTreeClassCache(std::vector<TreeClassCache>& cache);

public:
	vector<TreeClass*> classes;
	map<string, I2DMask*> masks;
	map<string, DensityMap*> globalMasks;
	map<DensityMapType, I2DMask*> rawI2DMasks;
	vector<CTreeInstance> trees;
	vector<TreeInstanceOutput> treeoutputs;
	vector<TreeInstanceFullOutput> fullOutputs;

	int xo;
	int zo;
	int xSize;
	int zSize;
	int* grid;
	int maxHeight;
protected:
	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	InputImageMetaInfo* m_pMetaInfo;
	std::vector<std::pair<std::vector<Point>, int>>* m_p2dCaveLevel0Nodes;
	std::vector<std::pair<std::vector<Point>, int>>* m_p2dCaveLevel1Nodes;
	std::vector<Point>* m_pPoisLocations;
	bool m_isLevel1Instances;
};

