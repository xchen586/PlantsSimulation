#pragma once

#include "CColonizationTree.h"
#include "I2DMask.h"
#include <vector>
#include <map>
#include <string>

using namespace std;
class CCellData;
struct InputImageMetaInfo;

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

	TreeInstanceOutput(const CTreeInstance& instance) 
		: x(0)
		, y(0)
		, z(0)
		, red(0)
		, green(0)
		, blue(0)
		, treeType(0)
	{
		x = instance.x;
		y = instance.z;

		int rgbColor = instance.treeClass->color;
		int redColor = (rgbColor >> 16) & 0xFF;
		int greenColor = (rgbColor >> 8) & 0xFF;
		int blueColor = rgbColor & 0xFF;

		red = redColor;
		green = greenColor;
		blue = blueColor;

		treeType = static_cast<unsigned int>(instance.treeClass->type);
	}

	TreeInstanceOutput(const TreeInstanceOutput& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		red = other.red;
		green = other.green;
		blue = other.blue;
		treeType = other.treeType;
	}

	TreeInstanceOutput& operator= (const TreeInstanceOutput& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		red = other.red;
		green = other.green;
		blue = other.blue;
		treeType = other.treeType;
		return *this;
	}
};

struct TreeInstanceFullOutput
{
	double posX;
	double posY;
	double posZ;
	TreeInstanceOutput m_instance;
	CCellData* m_pCellData;
	InputImageMetaInfo* m_pMetaInfo;

	TreeInstanceFullOutput(const TreeInstanceOutput& instance, CCellData* pCellData, InputImageMetaInfo* pMetaInfo);
	void GetPosFromInstanceOutput();
	void GetPosFromInstanceOutputEx();
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
	void setMetaInfo(InputImageMetaInfo* metaInfo)
	{
		m_pMetaInfo = metaInfo;
	}
	TreeInstanceOutput GetTreeOutputFromInstance(const CTreeInstance& instance);
	bool exportTreeInstanceOutputToCSV(const std::vector<TreeInstanceOutput>& data, const std::string& filename);
	bool exportTreeInstanceFullOutputToCSV(const std::vector<TreeInstanceFullOutput>& data, const std::string& filename);
	bool outputTreeInstanceResults(const std::string& csvFileName);
	bool outputFullTreeInstanceResults(const std::string& csvFileName);
public:
	vector<TreeClass*> classes;
	map<string, I2DMask*> masks;
	map<string, DensityMap*> globalMasks;
	vector<CTreeInstance> trees;
	vector<TreeInstanceOutput> outputs;
	vector<TreeInstanceFullOutput> fullOutputs;
	std::vector<std::vector<CCellData*>> * m_pCellTable;
	InputImageMetaInfo * m_pMetaInfo;
	int xo;
	int zo;
	int xSize;
	int zSize;
	int* grid;
};

