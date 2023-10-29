#pragma once

#include "CColonizationTree.h"
#include "Instance.h"

class CCellInfo;
struct InputImageMetaInfo;

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

struct TreeInstanceOutput {
	float x;
	float y;
	float z;
	int red;
	int green;
	int blue;
	unsigned int treeType;
	double age;
	double maxAge;
	TreeInstanceOutput()
		: x(0)
		, y(0)
		, z(0)
		, red(0)
		, green(0)
		, blue(0)
		, treeType(0)
		, age(0)
		, maxAge(0)
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
		maxAge = instance.treeClass->maxAge;
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
		maxAge = other.maxAge;
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
		maxAge = other.maxAge;
		return *this;
	}
};



struct TreeInstanceSubOutput : public InstanceSubOutput
{
	double age;
	TreeInstanceSubOutput() : InstanceSubOutput()
	{
		outputItemCount = 12;
	}
};

typedef std::vector<std::shared_ptr<InstanceSubOutput>> InstanceSubOutputVector;
typedef std::map <std::string, std::shared_ptr<InstanceSubOutputVector>> InstanceSubOutputMap;

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