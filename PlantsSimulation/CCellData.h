#pragma once

#include <cstdint>

class CCellData
{
public:
	CCellData(const uint8_t& roadAttributeValue, const uint8_t& moistureValue, const uint8_t& roughnessValue)
		: m_roadAttributeValue(roadAttributeValue)
		, m_moistureValue(moistureValue)
		, m_roughnessValue(roughnessValue)
	{

	}

public:
	uint8_t m_roadAttributeValue;
	uint8_t m_moistureValue;
	uint8_t m_roughnessValue;
	
	short m_height;
	short m_slope;
};

