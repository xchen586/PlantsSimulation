#pragma once

#if __APPLE__
    #include "../Common/include/CColonizationTree.h"
#else
    #include "..\Common\include\CColonizationTree.h"
#endif

const double WaterLevel = 0;

class CHeightDensityMap : public DensityMap
{
public:
	CHeightDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_Height;
		invert = false;
		blur = 0;
	}
};

class COakHeightDensityMap : public CHeightDensityMap
{
public:
	COakHeightDensityMap() : CHeightDensityMap()
	{
		minval = WaterLevel + 1;
		maxval = 1500;
		ease = 200;
	}
};

class CMapleHeightDensityMap : public CHeightDensityMap
{
public:
	CMapleHeightDensityMap() : CHeightDensityMap()
	{
		minval = WaterLevel + 1;
		maxval = 1800;
		ease = 200;
	}
};

class CBirchHeightDensityMap : public CHeightDensityMap
{
public:
	CBirchHeightDensityMap() : CHeightDensityMap()
	{
		minval = 600; // 600;
		maxval = 2400;
		ease = 800;
	}
};

class CFirHeightDensityMap : public CHeightDensityMap
{
public:
	CFirHeightDensityMap() : CHeightDensityMap()
	{
		minval = 600;
		maxval = 3300;
		ease = 600;
	}
};
