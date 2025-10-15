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

class CForest
{
public:
	CForest(void);
	~CForest(void);
public:
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

