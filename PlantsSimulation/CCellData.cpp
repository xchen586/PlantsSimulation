#include "CCellData.h"
#include "PsHelper.h"

CCellData::CCellData(const uint8_t& roadAttributeValue, const uint8_t& moistureValue, const uint8_t& roughnessValue)
	: m_roadAttributeValue(roadAttributeValue)
	, m_moistureValue(moistureValue)
	, m_roughnessValue(roughnessValue)
	, m_heightValue(0)
	, m_slopeHeightValue(0)
	, m_roadAttribute(0)
	, m_moisture(0)
	, m_roughness(0)
	, m_height(0)
	, m_slopeHeight(0)
	
{
	m_roadAttribute = GetColorLinearNormallizedAttribute(m_roadAttributeValue);
	m_moisture = GetColorLinearNormallizedAttribute(m_moistureValue);
	m_roughness = GetColorLinearNormallizedAttribute(m_roughnessValue);
}

void CCellData::SetHeightValue(short heightValue)
{
	m_heightValue = heightValue;
	m_height = static_cast<double>(m_heightValue);
	/*if (heightValue != 0)
	{
		std::cout << "height value is : " << heightValue << ", height is : " << m_height << std::endl;
	}*/
}

void CCellData::SetSlopeHeightValue(short slopeValue)
{
	m_slopeHeightValue = slopeValue;
	m_slopeHeight = static_cast<double>(m_slopeHeightValue);
}

void CCellData::SetSlopeAngleValue(double angle)
{
	m_slopeAngleValue = angle;
}

double CCellData::GetRoadAttribute()
{
	return m_roadAttribute;
}

double CCellData::GetHeight()
{
	return m_height;
}

double CCellData::GetSlopeHeight()
{
	return m_slopeHeight;
}

double CCellData::GetMoisture()
{
	return m_moisture;
}

double CCellData::GetRoughness()
{
	return m_roughness;
}

double CCellData::GetSlopeAngle()
{
	return m_slopeAngleValue;
}
