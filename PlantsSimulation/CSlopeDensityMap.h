#pragma once
#include "CColonizationTree.h"

class CSlopeDensityMap : public DensityMap
{
public:
	CSlopeDensityMap() : DensityMap()
	{
		invert = true;
	}
};

class COakSlopeDensityMap : public CSlopeDensityMap
{
public:
	COakSlopeDensityMap() : CSlopeDensityMap()
	{
		minval = 5;
		maxval = 30;
		ease = 5;
	}
};

class CMapleSlopeDensityMap : public CSlopeDensityMap
{
public:
	CMapleSlopeDensityMap() : CSlopeDensityMap()
	{
		minval = 5;
		maxval = 30;
		ease = 5;
	}
};

class CBirchSlopeDensityMap : public CSlopeDensityMap
{
public:
	CBirchSlopeDensityMap() : CSlopeDensityMap()
	{
		minval = 0;
		maxval = 25;
		ease = 5;
	}
};

class CFirSlopeDensityMap : public CSlopeDensityMap
{
public:
	CFirSlopeDensityMap() : CSlopeDensityMap()
	{
		minval = 0;
		maxval = 40;
		ease = 10;
	}
};