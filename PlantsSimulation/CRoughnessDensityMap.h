#pragma once
#include "CColonizationTree.h"

class CRoughnessDensityMap : public DensityMap
{
public:
	CRoughnessDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_Roughness;
		invert = true;
		ease = 0.1;
	}
};

class COakRoughnessDensityMap : public CRoughnessDensityMap
{
public:
	COakRoughnessDensityMap() : CRoughnessDensityMap()
	{
		minval = 0.1;
		maxval = 0.8;
	}
};

class CMapleRoughnessDensityMap : public CRoughnessDensityMap
{
public:
	CMapleRoughnessDensityMap() : CRoughnessDensityMap()
	{
		minval = 0.2;
		maxval = 0.7;
	}
};

class CBirchRoughnessDensityMap : public CRoughnessDensityMap
{
public:
	CBirchRoughnessDensityMap() : CRoughnessDensityMap()
	{
		minval = 0.1;
		maxval = 0.7;
	}
};

class CFirRoughnessDensityMap : public CRoughnessDensityMap
{
public:
	CFirRoughnessDensityMap() : CRoughnessDensityMap()
	{
		minval = 0.35;
		maxval = 0.9;
	}
};