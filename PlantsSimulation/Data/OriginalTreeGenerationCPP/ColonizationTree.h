#pragma once
#include "SegmentLayer.h"
#include "PointLayer.h"
#include "HeightmapLayer.h"
#include "perlin.h"
#include <map>

using namespace std;

extern Perlin noise;

class TreeParamRange
{
public:
	double minV;
	double maxV;

public:
	TreeParamRange() :
	  minV(0.0), maxV(0.0) {};
	TreeParamRange(double minValue, double maxValue) : 
	  minV(minValue), maxV(maxValue) {};
	TreeParamRange& operator= (const TreeParamRange& other)
	{
		minV = other.minV;
		maxV = other.maxV;
		return *this;
	};

public:
	double getValue(double x, double y, double z) 
	{
		return minV + ((maxV - minV)*rand())/RAND_MAX;
		/*
		float n = noise.Get(x, z);
		n = max(0.0, min(1.0, abs((1.0 + n)*0.5)));
		return minV + (maxV - minV)*n;
		*/
	}
};

class DensityMap
{
public:
	double minval;
	double maxval;
	double ease;
	bool invert;
	bool useForThinning;
	int blur;
};

class TreeClass
{
public:
	TreeClass() :
		color(0x00000000),
		angleStrength(0.6, 0.6), 
		pipeRatio(0.45, 0.45), 
		branchWidth(1.0, 1.0),
		veinMinWidth(120.0, 120.0),
		veinHorzNoise(0.5, 0.5),
		veinVertNoise(0.0, 0.0),
		veinAngleStrength(0.5, 0.5),
		veinWidth(1.0, 1.0),
		veinPipeRatio(0.4, 0.4),
		rootAeriation(0.1, 0.1),
		rootHorzNoise(0.0, 0.0),
		rootVertNoise(0.5, 0.5),
		rootAngleStrength(0.2, 0.2),
		rootWidth(1.0, 1.0),
		rootPipeRatio(0.45, 0.45),
		matureAge(10.0),
		maxAge(80.0),
		probGenesis(1.0),
		probSeed(1.0),
		seedRange(1000.0)

	  {};
	TreeParamRange xShift; 
	TreeParamRange yShift; 
	TreeParamRange zShift; 
	TreeParamRange radius; 
	TreeParamRange xRadius; 
	TreeParamRange yRadius; 
	TreeParamRange zRadius;
	TreeParamRange crownShift;
	TreeParamRange sD;
	TreeParamRange crownStart;
	TreeParamRange maxClumpRadius;
	TreeParamRange clumpStrength;
	TreeParamRange clumpShift;
	TreeParamRange clumpKill;
	TreeParamRange horzNoise;
	TreeParamRange vertNoise;
	TreeParamRange angleStrength;
	TreeParamRange branchWidth;
	TreeParamRange pipeRatio;
	TreeParamRange veinMinWidth;
	TreeParamRange veinHorzNoise;
	TreeParamRange veinVertNoise;
	TreeParamRange veinAngleStrength;
	TreeParamRange veinWidth;
	TreeParamRange veinPipeRatio;
	TreeParamRange rootAeriation;
	TreeParamRange rootHorzNoise;
	TreeParamRange rootVertNoise;
	TreeParamRange rootAngleStrength;
	TreeParamRange rootWidth;
	TreeParamRange rootPipeRatio;

	int color;
	double matureAge;
	double maxAge;
	double probGenesis;
	double probSeed;
	double seedRange;
	map<std::string, DensityMap*> masks;

	TreeClass& operator= (const TreeClass& other)
	{
		xShift = other.xShift;
		yShift = other.yShift;
		zShift = other.zShift; 
		radius = other.radius; 
		xRadius = other.xRadius;
		yRadius = other.yRadius;
		zRadius = other.zRadius;
		crownShift = other.crownShift;
		sD = other.sD;
		crownStart = other.crownStart;
		maxClumpRadius = other.maxClumpRadius;
		clumpStrength = other.clumpStrength;
		clumpKill = other.clumpKill;
		clumpShift = other.clumpShift;
		horzNoise = other.horzNoise;
		vertNoise = other.vertNoise;
		angleStrength = other.angleStrength;
		branchWidth = other.branchWidth;
		pipeRatio = other.pipeRatio;
		veinMinWidth = other.veinMinWidth;
		veinHorzNoise = other.veinHorzNoise;
		veinVertNoise = other.veinVertNoise;
		veinAngleStrength = other.veinAngleStrength;
		veinWidth = other.veinWidth;
		veinPipeRatio = other.veinPipeRatio;
		rootAeriation = other.rootAeriation;
		rootHorzNoise = other.rootHorzNoise;
		rootVertNoise = other.rootVertNoise;
		rootAngleStrength = other.rootAngleStrength;
		rootWidth = other.rootWidth;
		rootPipeRatio = other.rootPipeRatio;

		probGenesis = other.probGenesis;
		probSeed = other.probSeed;
		seedRange = other.seedRange;
		matureAge = other.matureAge;
		maxAge = other.maxAge;

		masks = other.masks;

		return *this;
	};
};

class ColonizationTree
{
public:
	ColonizationTree(CSegmentLayer* aSegmentVolume, CSegmentLayer* aRootSegmentVolume, CPointLayer* aCrownVolume, CHeightmapLayer* aGround);
public:
	~ColonizationTree(void);
public:
	void addTree(
		double x, double y, double z, 
		double xShift, double yShift, double zShift, 
		double radius, double xRadius, double yRadius, double zRadius,
		double& trunkMassCenterX,
		double& trunkMassCenterZ,
		double crownShift = 0.9,
		double sD = 2.0,
		double crownStart = 0.75,
		double maxClumpRadius = 100.0,
		double clumpStrength = 0.3,
		double clumpKill = 0.1,
		double clumpShift = 0.0,
		double horzNoise = 0.7,
		double vertNoise = 0.7,
		double angleStrength = 0.6,
		double branchWidth = 1.0,
		double pipeRatio = 0.45,
		double veinMinWidth = 120,
		double veinHorzNoise = 0.5,
		double veinVertNoise = 0.0,
		double veinAngleStrength = 0.5,
		double veinWidth = 1.0,
		double veinPipeRatio = 0.45,
		double rootAeriation = 0.1,
		double rootHorzNoise = 0.0,
		double rootVertNoise = 0.5,
		double rootAngleStrength = 0.6,
		double rootWidth = 1.0,
		double rootPipeRatio = 0.45);
	double addTree(
		double x, double y, double z, 
		TreeClass& treeClass,
		double& trunkMassCenterX,
		double& trunkMassCenterZ);
private:
	CHeightmapLayer* ground;
	CSegmentLayer* segmentVolume;
	CSegmentLayer* rootSegmentVolume;
	CPointLayer* crownVolume;
};

double mod(double x1, double y1, double z1, double x2, double y2, double z2);

