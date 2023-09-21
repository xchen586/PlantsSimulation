#pragma once

#include <cstdint>

class CCellData
{
public:
	CCellData(const uint8_t& roadAttributeValue, const uint8_t& moistureValue, const uint8_t& roughnessValue);
		
	double GetRoadAttribute();
	double GetHeight();
	double GetSlopeHeight();
	double GetMoisture();
	double GetRoughness();
	double GetSlopeAngle();

	void SetHeightValue(short heightValue);
	void SetSlopeHeightValue(short slopeValue);
	
	void SetSlopeAngleValue(double angle);
private:
	short m_heightValue;
	short m_slopeHeightValue;
	uint8_t m_roadAttributeValue;
	uint8_t m_moistureValue;
	uint8_t m_roughnessValue;
	double m_slopeAngleValue;

	double m_roadAttribute;
	double m_height;
	double m_slopeHeight;
	double m_moisture;
	double m_roughness;
};

