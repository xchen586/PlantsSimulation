#pragma once

#include "CColonizationTree.h"
#include "I2DMask.h"
#include <vector>
#include <map>
#include <string>

#include "Utils.h"

using namespace std;
class CCellInfo;
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
	double age;
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
	double age;
	TreeInstanceOutput()
		: x(0)
		, y(0)
		, z(0)
		, red(0)
		, green(0)
		, blue(0)
		, treeType(0)
		, age(0)
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
		, age(0)
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
		age = instance.age;
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
		age = other.age;
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
		age = other.age;
		return *this;
	}
};

struct InstanceSubOutput
{
	int xIdx;
	int yIdx;
	double xOffsetW;
	double yOffsetW;
	//double rPosX;
	//double rPosY;
	double rPosZ;
};

struct TreeInstanceSubOutput : public InstanceSubOutput
{
	unsigned int typeId;
	int age;

};

typedef std::vector<TreeInstanceSubOutput> TreeInsSubOutputVector;
typedef std::map <std::string, std::shared_ptr<TreeInsSubOutputVector>> TreeInsSubOutputMap;

struct TreeInstanceFullOutput
{
	double posX;
	double posY;
	double posZ;
	TreeInstanceOutput m_instance;
	CCellInfo* m_pCellData;
	InputImageMetaInfo* m_pMetaInfo;

	TreeInstanceFullOutput(const TreeInstanceOutput& instance, CCellInfo* pCellData, InputImageMetaInfo* pMetaInfo);
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
	void setCellTable(std::vector<std::vector<CCellInfo*>>* pCellTable) {
		m_pCellTable = pCellTable;
	}
	void setMetaInfo(InputImageMetaInfo* metaInfo)
	{
		m_pMetaInfo = metaInfo;
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

	bool outputSubfiles(const std::string& outputSubsDir);
public:
	vector<TreeClass*> classes;
	map<string, I2DMask*> masks;
	map<string, DensityMap*> globalMasks;
	vector<CTreeInstance> trees;
	vector<TreeInstanceOutput> outputs;
	vector<TreeInstanceFullOutput> fullOutputs;
	std::vector<std::vector<CCellInfo*>> * m_pCellTable;
	InputImageMetaInfo * m_pMetaInfo;
	int xo;
	int zo;
	int xSize;
	int zSize;
	int* grid;
};

