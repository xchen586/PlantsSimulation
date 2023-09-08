#pragma once

#include "CColonizationTree.h"
#include <vector>
#include <map>
#include <string>

using namespace std;
class CCellData;

class I2DMask
{
public:
	virtual double get2DMaskValue(double x, double z, int blur) {return 0.6;};
};

pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithIndex(PlantType type, int index, I2DMask* i2dMask);

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

struct TreeOutput {
	float x;
	float y;
	float z;
	int red;
	int green;
	int blue;
	TreeOutput()
		: x(0)
		, y(0)
		, z(0)
		, red(0)
		, green(0)
		, blue(0)
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

	TreeOutput GetTreeOutputFromInstance(const CTreeInstance& instance);
	bool exportToCSV(const std::vector<TreeOutput>& data, const std::string& filename);
	bool outputResults(const std::string& csvFileName);
public:
	vector<TreeClass*> classes;
	map<string, I2DMask*> masks;
	map<string, DensityMap*> globalMasks;
	vector<CTreeInstance> trees;
	vector<TreeOutput> outputs;
	std::vector<std::vector<CCellData*>> * m_pCellTable;
	int xo;
	int zo;
	int xSize;
	int zSize;
	int* grid;
};

