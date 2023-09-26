#pragma once

#include "CColonizationTree.h"
#include "I2DMask.h"
#include <vector>
#include <map>
#include <string>

using namespace std;
class CCellData;

pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithIndex(PlantType type, int index, I2DMask* i2dMask);
pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithDensityMapType(PlantType plantType, DensityMapType densityType, I2DMask* pI2dMask);
pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithDensityMapTypeIndex(PlantType plantType, DensityMapType densityType, I2DMask* pI2dMask);

class CTreeInstance
{
public:
	TreeClass* treeClass;
	float bday;
	float x;
	float z;
	bool dead;
	bool mature;
};

struct ClassStrength
{
	double strength;
	TreeClass* treeClass;
};

struct TreeInstanceOutput {
	float x;
	float y;
	float z;
	int red;
	int green;
	int blue;
	unsigned int treeType;
	TreeInstanceOutput()
		: x(0)
		, y(0)
		, z(0)
		, red(0)
		, green(0)
		, blue(0)
		, treeType(0)
	{

	}
};

class CForest
{
public:
	CForest(void);
	~CForest(void);
public:
	void generate(float forestAge, int iterations);
	void loadTreeClasses();
	void loadMasks();
	void loadGlobalMasks();
	void setCellTable(std::vector<std::vector<CCellData*>>* pCellTable) {
		m_pCellTable = pCellTable;
	}

	TreeInstanceOutput GetTreeOutputFromInstance(const CTreeInstance& instance);
	bool exportToCSV(const std::vector<TreeInstanceOutput>& data, const std::string& filename);
	bool outputResults(const std::string& csvFileName);
public:
	vector<TreeClass*> classes;
	map<string, I2DMask*> masks;
	map<string, DensityMap*> globalMasks;
	vector<CTreeInstance> trees;
	vector<TreeInstanceOutput> outputs;
	std::vector<std::vector<CCellData*>> * m_pCellTable;
	int xo;
	int zo;
	int xSize;
	int zSize;
	int* grid;
};

