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
	virtual double get2DMaskValue(double x, double z, int blur) {return 1.0;};
};

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
public:
	vector<TreeClass*> classes;
	map<string, I2DMask*> masks;
	map<string, DensityMap*> globalMasks;
	vector<CTreeInstance> trees;
	std::vector<std::vector<CCellData*>> * m_pCellTable;
	int xo;
	int zo;
	int xSize;
	int zSize;
	int* grid;
};

