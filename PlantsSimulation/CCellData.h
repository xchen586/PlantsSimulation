#pragma once

#include <cstdint>

class CCellData
{
public:
	CCellData(const uint8_t& roadAttributeValue, const uint8_t& moistureValue, const uint8_t& roughnessValue);
		
	double GetRoadAttribute();
	double GetHeight();
	double GetSlope();
	double GetMoisture();
	double GetRoughness();

	void SetHeightValue(short heightValue);
	void SetSlopeValue(short slopeValue);
private:
	short m_heightValue;
	short m_slopeValue;
	uint8_t m_roadAttributeValue;
	uint8_t m_moistureValue;
	uint8_t m_roughnessValue;

	double m_roadAttribute;
	double m_height;
	double m_slope;
	double m_moisture;
	double m_roughness;
};

