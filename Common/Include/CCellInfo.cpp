#include "CCellInfo.h"
#include "PsHelper.h"

CCellInfo::CCellInfo(const uint8_t& roadAttributeValue, const uint8_t& moistureValue, const uint8_t& roughnessValue)
	: m_roadAttributeValue(roadAttributeValue)
	, m_moistureValue(moistureValue)
	, m_roughnessValue(roughnessValue)
	, m_heightValue(0)
	, m_slopeHeightValue(0)
	, m_hasHeightValue(0)
	, m_roadAttribute(0)
	, m_moisture(0)
	, m_roughness(0)
	, m_height(0)
	, m_slopeHeight(0)
	, m_hasHeight(false)
	, m_slopeAngleValue(0)
	, m_sunlightAffinity(0)
	, m_hasSunlightAffinity(false)
	
{
	m_roadAttribute = GetColorLinearNormallizedAttribute(m_roadAttributeValue);
	m_moisture = GetColorLinearNormallizedAttribute(m_moistureValue);
	m_roughness = GetColorLinearNormallizedAttribute(m_roughnessValue);
}

void CCellInfo::SetHeightValue(short heightValue)
{
	m_heightValue = heightValue;
	m_height = static_cast<double>(m_heightValue);
	/*if (heightValue != 0)
	{
		std::cout << "height value is : " << heightValue << ", height is : " << m_height << std::endl;
	}*/
}

void CCellInfo::SetSlopeHeightValue(short slopeValue)
{
	m_slopeHeightValue = slopeValue;
	m_slopeHeight = static_cast<double>(m_slopeHeightValue);
}

void CCellInfo::SetSlopeAngleValue(double angle)
{
	m_slopeAngleValue = angle;
}

void CCellInfo::SetHasHeightValue(short hasHeightValue)
{
	m_hasHeightValue = hasHeightValue;
	m_hasHeight = (m_hasHeightValue != 0) ? true : false;
}

double CCellInfo::GetRoadAttribute()
{
	return m_roadAttribute;
}

double CCellInfo::GetHeight()
{
	return m_height;
}

double CCellInfo::GetSlopeHeight()
{
	return m_slopeHeight;
}

double CCellInfo::GetMoisture()
{
	return m_moisture;
}

double CCellInfo::GetRoughness()
{
	return m_roughness;
}

double CCellInfo::GetSlopeAngle()
{
	return m_slopeAngleValue;
}

bool CCellInfo::GetHasHeight()
{
	return m_hasHeight;
}

bool CCellInfo::GetHasSunlightAffinity() const
{
	return m_hasSunlightAffinity;
}

void CCellInfo::SetHasSunlightAffinity(bool hasSunlightAffinity)
{
	m_hasSunlightAffinity = hasSunlightAffinity;
}