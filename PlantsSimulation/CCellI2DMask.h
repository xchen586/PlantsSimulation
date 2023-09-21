#pragma once
#include "I2DMask.h"
#include "CCellData.h"

#include <vector>

class CCellDataI2DMask : public I2DMask
{
public:
	CCellDataI2DMask(std::vector<std::vector<CCellData*>>* pCellTable)
		: m_pCellTable(pCellTable)
		, rowsCount(0)
		, colsCount(0)
	{
		if (pCellTable) {
			rowsCount = (*pCellTable).size();
			colsCount = (*pCellTable)[0].size();
		}
	}
protected:
	CCellData* GetCellData(double x, double z)
	{
		CCellData* ret = nullptr;
		if (m_pCellTable) {
			if (((x >= 0) && (x < rowsCount))
				&& ((z >= 0) && (z < colsCount))) {
				ret = (*m_pCellTable)[x][z];
			}
		}
		return ret;
	}
	std::vector<std::vector<CCellData*>>* m_pCellTable;
	int rowsCount;
	int colsCount;
};

class CCellHeightI2DMask : public CCellDataI2DMask
{
public:
	CCellHeightI2DMask(std::vector<std::vector<CCellData*>>* pCellTable)
		: CCellDataI2DMask(pCellTable)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellData* pCellData = GetCellData(x, z);
		if (pCellData) {
			return pCellData->GetHeight();
		}
		else {
			return 0;
		}
	}
};

class CCellSlopeI2DMask : public CCellDataI2DMask
{
public:
	CCellSlopeI2DMask(std::vector<std::vector<CCellData*>>* pCellTable)
		: CCellDataI2DMask(pCellTable)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellData* pCellData = GetCellData(x, z);
		if (pCellData) {
#if USE_SCOPE_ANGLE
			return pCellData->GetSlopeAngle();
#else
			return pCellData->GetSlopeHeight();
#endif
		}
		else {
			return 0;
		}
	}
};

class CCellMoistureI2DMask : public CCellDataI2DMask
{
public:
	CCellMoistureI2DMask(std::vector<std::vector<CCellData*>>* pCellTable)
		: CCellDataI2DMask(pCellTable)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellData* pCellData = GetCellData(x, z);
		if (pCellData) {
			return pCellData->GetMoisture();
		}
		else {
			return 0;
		}
	}
};

class CCellRoughnessI2DMask : public CCellDataI2DMask
{
public:
	CCellRoughnessI2DMask(std::vector<std::vector<CCellData*>>* pCellTable)
		: CCellDataI2DMask(pCellTable)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellData* pCellData = GetCellData(x, z);
		if (pCellData) {
			return pCellData->GetRoughness();
		}
		else {
			return 0;
		}
	}
};

class CCellRoadAttributeI2DMask : public CCellDataI2DMask
{
public:
	CCellRoadAttributeI2DMask(std::vector<std::vector<CCellData*>>* pCellTable)
		: CCellDataI2DMask(pCellTable)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellData* pCellData = GetCellData(x, z);
		if (pCellData) {
			return pCellData->GetRoadAttribute();
		}
		else {
			return 0;
		}
	}
};