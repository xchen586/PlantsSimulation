#pragma once

#if __APPLE__
#include "../Common/include/CColonizationTree.h"
#else
#include "..\Common\include\CColonizationTree.h"
#endif

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
		//minval = 0.15;
		//maxval = 0.7;
		minval = 0.1;
		maxval = 0.37;
		
		//minval = 0.0;
		//maxval = 1.0;
	}
};

class CMapleMoistureDensityMap : public CMoistureDensityMap
{
public:
	CMapleMoistureDensityMap() : CMoistureDensityMap()
	{
		//minval = 0.1;
		//maxval = 0.8;

		//minval = 0;
		//maxval = 1.0;

		minval = 0.1;
		maxval = 0.37;
	}
};

class CBirchMoistureDensityMap : public CMoistureDensityMap
{
public:
	CBirchMoistureDensityMap() : CMoistureDensityMap()
	{
		minval = 0.15;
		maxval = 0.85;
	}
};

class CFirMoistureDensityMap : public CMoistureDensityMap
{
public:
	CFirMoistureDensityMap() : CMoistureDensityMap()
	{
		minval = 0.3;
		maxval = 0.9;
	}
};