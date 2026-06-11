#pragma once
#include <functional>
#include <vector>

#include "I2DMask.h"

// Refactor (Phase 2.2): normalized to forward-slash includes; dropped #if __APPLE__ blocks.
#include "../Common/include/CCellInfo.h"
#include "../Common/include/PsMarco.h"

// Refactor (Phase 2.1): 6 CCellXxxI2DMask subclasses collapsed into one data-driven class.
// Each variant is created via a named factory function at the bottom of this file.
class CCellDataI2DMask : public I2DMask
{
public:
	CCellDataI2DMask(std::vector<std::vector<CCellInfo*>>* pCellTable, double xRatio, double yRatio,
	                 std::function<double(CCellInfo*)> getter)
		: m_pCellTable(pCellTable)
		, m_tableRowsCount(0)
		, m_tableColsCount(0)
		, m_xRatio(xRatio)
		, m_yRatio(yRatio)
		, m_getter(std::move(getter))
	{
		if (pCellTable) {
			m_tableRowsCount = static_cast<int>((*pCellTable).size());
			m_tableColsCount = static_cast<int>((*pCellTable)[0].size());
		}
	}
	virtual double get2DMaskValue(double x, double z, int blur) const override {
		return m_getter(GetCellData(x, z));
	}
private:
	CCellInfo* GetCellData(double x, double z) const {
		int rowIdx = static_cast<int>(x / m_xRatio);
		int colIdx = static_cast<int>(z / m_yRatio);
		if (m_pCellTable
			&& rowIdx >= 0 && rowIdx < m_tableRowsCount
			&& colIdx >= 0 && colIdx < m_tableColsCount) {
			return (*m_pCellTable)[rowIdx][colIdx];
		}
		return nullptr;
	}
	std::vector<std::vector<CCellInfo*>>* m_pCellTable;
	int m_tableRowsCount;
	int m_tableColsCount;
	double m_xRatio;
	double m_yRatio;
	std::function<double(CCellInfo*)> m_getter;
};

inline I2DMask* MakeCellHeightMask(std::vector<std::vector<CCellInfo*>>* t, double xr, double yr) {
	return new CCellDataI2DMask(t, xr, yr, [](CCellInfo* c) -> double {
		if (c && c->GetHasHeight()) return c->GetHeight();
		return UNAVAILBLE_NEG_HEIGHT;
	});
}

inline I2DMask* MakeCellSlopeMask(std::vector<std::vector<CCellInfo*>>* t, double xr, double yr) {
	return new CCellDataI2DMask(t, xr, yr, [](CCellInfo* c) -> double {
		if (c && c->GetHasHeight()) {
#if USE_SCOPE_ANGLE
			return c->GetSlopeAngle();
#else
			return c->GetSlopeHeight();
#endif
		}
		return UNAVAILBLE_NEG_HEIGHT;
	});
}

inline I2DMask* MakeCellMoistureMask(std::vector<std::vector<CCellInfo*>>* t, double xr, double yr) {
	return new CCellDataI2DMask(t, xr, yr, [](CCellInfo* c) -> double {
		return c ? c->GetMoisture() : 0.0;
	});
}

inline I2DMask* MakeCellRoughnessMask(std::vector<std::vector<CCellInfo*>>* t, double xr, double yr) {
	return new CCellDataI2DMask(t, xr, yr, [](CCellInfo* c) -> double {
		return c ? c->GetRoughness() : 0.0;
	});
}

inline I2DMask* MakeCellRoadAttributeMask(std::vector<std::vector<CCellInfo*>>* t, double xr, double yr) {
	return new CCellDataI2DMask(t, xr, yr, [](CCellInfo* c) -> double {
		return c ? c->GetRoadAttribute() : 0.0;
	});
}

inline I2DMask* MakeCellSunLightAffinityMask(std::vector<std::vector<CCellInfo*>>* t, double xr, double yr) {
	// Fixed: original CCellSunLightAffinityID2Mask had missing return when pCellData!=null but !GetHasSunlightAffinity().
	return new CCellDataI2DMask(t, xr, yr, [](CCellInfo* c) -> double {
		if (c && c->GetHasSunlightAffinity()) return c->GetSunlightAffinity();
		return 0.0;
	});
}
