#include "CCellData.h"
#include "PsHelper.h"

CCellData::CCellData(const uint8_t& roadAttributeValue, const uint8_t& moistureValue, const uint8_t& roughnessValue)
	: m_roadAttributeValue(roadAttributeValue)
	, m_moistureValue(moistureValue)
	, m_roughnessValue(roughnessValue)
	, m_heightValue(0)
	, m_slopeValue(0)
	, m_roadAttribute(0)
	, m_moisture(0)
	, m_roughness(0)
	, m_height(0)
	, m_slope(0)
{
	m_roadAttribute = GetColorLinearNormallizedAttribute(m_roadAttributeValue);
	m_moisture = GetColorLinearNormallizedAttribute(m_moistureValue);
	m_roughness = GetColorLinearNormallizedAttribute(m_roughnessValue);
}

void CCellData::SetHeightValue(short heightValue)
{
	m_heightValue = heightValue;
	m_height = static_cast<double>(m_heightValue);
}

void CCellData::SetSlopeValue(short slopeValue)
{
	m_slopeValue = slopeValue;
	m_slope = static_cast<double>(m_slopeValue);
}

double CCellData::GetRoadAttribute()
{
	return m_roadAttribute;
}

double CCellData::GetHeight()
{
	return m_height;
}

double CCellData::GetSlope()
{
	return m_slope;
}

double CCellData::GetMoisture()
{
	return m_moisture;
}

double CCellData::GetRoughness()
{
	return m_roughness;
}
