#pragma once
#include "CColonizationTree.h"

class CRoughnessDensityMap : public DensityMap
{
public:
	CRoughnessDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_Roughness;
		invert = false;
		ease = 0.1;
	}
};

class COakRoughnessDensityMap : public CRoughnessDensityMap
{
public:
	COakRoughnessDensityMap() : CRoughnessDensityMap()
	{
		minval = 0.1;
		maxval = 0.298;
	}
};

class CMapleRoughnessDensityMap : public CRoughnessDensityMap
{
public:
	CMapleRoughnessDensityMap() : CRoughnessDensityMap()
	{
		minval = 0.12;
		maxval = 0.297;
	}
};

class CBirchRoughnessDensityMap : public CRoughnessDensityMap
{
public:
	CBirchRoughnessDensityMap() : CRoughnessDensityMap()
	{
		minval = 0.1;
		maxval = 0.297;
	}
};

class CFirRoughnessDensityMap : public CRoughnessDensityMap
{
public:
	CFirRoughnessDensityMap() : CRoughnessDensityMap()
	{
		minval = 0.135;
		maxval = 0.299;
	}
};