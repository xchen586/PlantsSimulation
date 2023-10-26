#include "TreeInstance.h"
#include "PsHelper.h"
#include "CCellInfo.h"

TreeInstanceFullOutput::TreeInstanceFullOutput(const TreeInstanceOutput& instance, CCellInfo* pCellData, InputImageMetaInfo* pMetaInfo)
	: posX(0)
	, posY(0)
	, posZ(0)
	, m_instance(instance)
	, m_pCellData(pCellData)
	, m_pMetaInfo(pMetaInfo)
{
	GetPosFromInstanceOutputEx();
}

void TreeInstanceFullOutput::GetPosFromInstanceOutput()
{
	double scaleXMin = static_cast<double>(m_instance.x) * m_pMetaInfo->xRatio;
	double scaleXmax = static_cast<double>(m_instance.x + 1) * m_pMetaInfo->xRatio;
	double scaleYMin = static_cast<double>(m_instance.y) * m_pMetaInfo->yRatio;
	double scaleYmax = static_cast<double>(m_instance.y + 1) * m_pMetaInfo->yRatio;
	double localX = GenerateRandomDouble(scaleXMin, scaleXmax);
	double localY = GenerateRandomDouble(scaleYMin, scaleYmax);
	//posX = localX;
	//posY = localY;
	posX = m_pMetaInfo->batch_min_x
		+ m_pMetaInfo->x0
		+ localX;
	posY = m_pMetaInfo->batch_min_y
		+ m_pMetaInfo->y0
		+ localY;
	posZ = m_pCellData->GetHeight();
}

void TreeInstanceFullOutput::GetPosFromInstanceOutputEx()
{
	double localX = m_instance.x;
	double localY = m_instance.y;
#if USE_POS_RELATIVE
	posX = localX;
	posY = localY;
#else
	posX = m_pMetaInfo->batch_min_x
		+ m_pMetaInfo->x0
		+ localX;
	posY = m_pMetaInfo->batch_min_y
		+ m_pMetaInfo->y0
		+ localY;
#endif
	posZ = m_pCellData->GetHeight();
}
