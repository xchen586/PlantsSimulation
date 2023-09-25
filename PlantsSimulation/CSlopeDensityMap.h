#pragma once
#include "CColonizationTree.h"
#include "PsMarco.h"

#if USE_SCOPE_ANGLE
class CSlopeDensityMap : public DensityMap
{
public:
	CSlopeDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_Slope;
		invert = false;
	}
};

class COakSlopeDensityMap : public CSlopeDensityMap
{
public:
	COakSlopeDensityMap() : CSlopeDensityMap()
	{
		//minval = 5 * (PI / 180.0);
		minval = 0 * (PI / 180.0);
		maxval = 30 * (PI / 180.0);
		ease = 5 * (PI / 180.0);
	}
};

class CMapleSlopeDensityMap : public CSlopeDensityMap
{
public:
	CMapleSlopeDensityMap() : CSlopeDensityMap()
	{
		//minval = 5 * (PI / 180.0);
		minval = 0 * (PI / 180.0);
		maxval = 30 * (PI / 180.0);
		ease = 5 * (PI / 180.0);
	}
};

class CBirchSlopeDensityMap : public CSlopeDensityMap
{
public:
	CBirchSlopeDensityMap() : CSlopeDensityMap()
	{
		minval = 0 * (PI / 180.0);
		maxval = 25 * (PI / 180.0);
		ease = 5 * (PI / 180.0);
	}
};

class CFirSlopeDensityMap : public CSlopeDensityMap
{
public:
	CFirSlopeDensityMap() : CSlopeDensityMap()
	{
		minval = 0 * (PI / 180.0);
		maxval = 40 * (PI / 180.0);
		ease = 10 * (PI / 180.0);
	}
};
#else

class CSlopeDensityMap : public DensityMap
{
public:
	CSlopeDensityMap() : DensityMap()
	{
		type = DensityMapType::DensityMap_Slope;
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

#endif