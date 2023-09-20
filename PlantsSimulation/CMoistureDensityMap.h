#pragma once
#include "CColonizationTree.h"

class CMoistureDensityMap : public DensityMap
{
public:
	CMoistureDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_Moisture;
		invert = false;
		ease = 0.1;
	}
};

class COakMoistureDensityMap : public CMoistureDensityMap
{
public:
	COakMoistureDensityMap() : CMoistureDensityMap()
	{
		minval = 0.15;
		maxval = 0.7;
	}
};

class CMapleMoistureDensityMap : public CMoistureDensityMap
{
public:
	CMapleMoistureDensityMap() : CMoistureDensityMap()
	{
		minval = 0.35;
		maxval = 0.8;
	}
};

class CBirchMoistureDensityMap : public CMoistureDensityMap
{
public:
	CBirchMoistureDensityMap() : CMoistureDensityMap()
	{
		minval = 0.25;
		maxval = 0.8;
	}
};

class CFirMoistureDensityMap : public CMoistureDensityMap
{
public:
	CFirMoistureDensityMap() : CMoistureDensityMap()
	{
		minval = 0.5;
		maxval = 0.9;
	}
};