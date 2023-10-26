#pragma once
#include "I2DMask.h"
#include "..\Common\include\CCellInfo.h"
#include "..\Common\include\PsMarco.h"

#include <vector>

class CCellDataI2DMask : public I2DMask
{
public:
	CCellDataI2DMask(std::vector<std::vector<CCellInfo*>>* pCellTable, double xRatio, double yRatio)
		: m_pCellTable(pCellTable)
		, m_tableRowsCount(0)
		, m_tableColsCount(0)
		, m_xRatio(xRatio)
		, m_yRatio(yRatio)
	{
		if (pCellTable) {
			m_tableRowsCount = (*pCellTable).size();
			m_tableColsCount = (*pCellTable)[0].size();
		}
	}
protected:
	CCellInfo* GetCellData(double x, double z)
	{
		CCellInfo* ret = nullptr;
		int rowIdx = static_cast<int>(x / m_xRatio);
		int colIdx = static_cast<int>(z / m_yRatio);
		if (m_pCellTable) {
			if (((rowIdx >= 0) && (rowIdx < m_tableRowsCount))
				&& ((colIdx >= 0) && (colIdx < m_tableColsCount))) {
				ret = (*m_pCellTable)[rowIdx][colIdx];
			}
		}
		return ret;
	}
	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	int m_tableRowsCount;
	int m_tableColsCount;
	double m_xRatio;
	double m_yRatio;
};

class CCellHeightI2DMask : public CCellDataI2DMask
{
public:
	CCellHeightI2DMask(std::vector<std::vector<CCellInfo*>>* pCellTable, double xRatio, double yRatio)
		: CCellDataI2DMask(pCellTable, xRatio, yRatio)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellInfo* pCellData = GetCellData(x, z);
		if (pCellData) {
			if (pCellData->GetHasHeight())
			{
				return pCellData->GetHeight();
			}
		}
		return -1000;
	}
};

class CCellSlopeI2DMask : public CCellDataI2DMask
{
public:
	CCellSlopeI2DMask(std::vector<std::vector<CCellInfo*>>* pCellTable, double xRatio, double yRatio)
		: CCellDataI2DMask(pCellTable, xRatio, yRatio)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellInfo* pCellData = GetCellData(x, z);
		if (pCellData) {
			if (pCellData->GetHasHeight())
			{
#if USE_SCOPE_ANGLE
				return pCellData->GetSlopeAngle();
#else
				return pCellData->GetSlopeHeight();
#endif
			}
		}
		return -1000;
	}
};

class CCellMoistureI2DMask : public CCellDataI2DMask
{
public:
	CCellMoistureI2DMask(std::vector<std::vector<CCellInfo*>>* pCellTable, double xRatio, double yRatio)
		: CCellDataI2DMask(pCellTable, xRatio, yRatio)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellInfo* pCellData = GetCellData(x, z);
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
	CCellRoughnessI2DMask(std::vector<std::vector<CCellInfo*>>* pCellTable, double xRatio, double yRatio)
		: CCellDataI2DMask(pCellTable, xRatio, yRatio)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellInfo* pCellData = GetCellData(x, z);
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
	CCellRoadAttributeI2DMask(std::vector<std::vector<CCellInfo*>>* pCellTable, double xRatio, double yRatio)
		: CCellDataI2DMask(pCellTable, xRatio, yRatio)
	{
	}
	virtual double get2DMaskValue(double x, double z, int blur)
	{
		CCellInfo* pCellData = GetCellData(x, z);
		if (pCellData) {
			return pCellData->GetRoadAttribute();
		}
		else {
			return 0;
		}
	}
};