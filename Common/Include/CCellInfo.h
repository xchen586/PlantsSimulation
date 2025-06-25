#pragma once

#include <cstdint>

class CCellInfo
{
public:
	CCellInfo(const uint8_t& roadAttributeValue, const uint8_t& moistureValue, const uint8_t& roughnessValue);
		
	double GetRoadAttribute();
	double GetHeight();
	double GetSlopeHeight();
	double GetMoisture();
	double GetRoughness();
	double GetSlopeAngle();
	bool GetHasHeight();

	bool GetHasSunlightAffinity() const;
	void SetHasSunlightAffinity(bool hasSunlightAffinity);
	double GetSunlightAffinity() const { return m_sunlightAffinity; }
	void SetSunlightAffinity(double sunlightAffinity) { m_sunlightAffinity = sunlightAffinity; }

	void SetHeightValue(short heightValue);
	void SetSlopeHeightValue(short slopeValue);
	
	void SetSlopeAngleValue(double angle);
	
	void SetHasHeightValue(short hasHeightValue);
private:
	short m_heightValue;
	short m_slopeHeightValue;
	uint8_t m_roadAttributeValue;
	uint8_t m_moistureValue;
	uint8_t m_roughnessValue;
	double m_slopeAngleValue;
	short m_hasHeightValue;

	double m_roadAttribute;
	double m_height;
	double m_slopeHeight;
	double m_moisture;
	double m_roughness;
	bool m_hasHeight;

	double m_sunlightAffinity;
	bool m_hasSunlightAffinity;
};

