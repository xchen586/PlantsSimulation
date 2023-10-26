#pragma once

enum struct InstanceType
{
	InstanceType_None,
	InstanceType_Tree,
	InstanceType_End,
};

struct InstanceSubOutput
{
	int xIdx;
	int yIdx;
	double xOffsetW;
	double yOffsetW;
	//double posX;
	//double posY;
	double posZ;
	double scaleX;
	double scaleY;
	double scaleZ;
	double rotationX;
	double rotationY;
	double rotationZ;
	unsigned int instanceType;

	InstanceSubOutput()
		: xIdx(0)
		, yIdx(0)
		, xOffsetW(0.0)
		, yOffsetW(0.0)
		, posZ(0.0)
		, scaleX(1.0)
		, scaleY(1.0)
		, scaleZ(1.0)
		, rotationX(0.0)
		, rotationY(0.0)
		, rotationZ(0.0)
		, instanceType(0)
	{

	}

};
