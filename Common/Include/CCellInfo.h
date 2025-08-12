#pragma once

#include <cstdint>

class CCellInfo
{
public:
	CCellInfo(const uint8_t& roadAttributeValue, const uint8_t& moistureValue, const uint8_t& roughnessValue);
		
	double GetRoadAttribute();
	double GetHeight();
	double GetSlopeHeight();
	double GetLevel1Height();
	double GetMoisture();
	double GetRoughness();
	double GetSlopeAngle();
	bool GetHasHeight();
	bool GetHasLevel1Height();

	bool GetHasSunlightAffinity() const;
	void SetHasSunlightAffinity(bool hasSunlightAffinity);
	double GetSunlightAffinity() const { return m_sunlightAffinity; }
	void SetSunlightAffinity(double sunlightAffinity) { m_sunlightAffinity = sunlightAffinity; }

	void SetHeightValue(short heightValue);
	void SetSlopeHeightValue(short slopeValue);
	void SetLevel1HeightValue(short level1HeightValue);
	
	void SetSlopeAngleValue(double angle);
	
	void SetHasHeightValue(short hasHeightValue);
	void SetHasLevel1HeightValue(short hasLevel1HeightValue);

private:
	short m_heightValue;
	short m_slopeHeightValue;
	short m_level1HeightValue;
	uint8_t m_roadAttributeValue;
	uint8_t m_moistureValue;
	uint8_t m_roughnessValue;
	double m_slopeAngleValue;
	short m_hasHeightValue;
	short m_hasLevel1HeightValue;

	double m_roadAttribute;
	double m_height;
	double m_slopeHeight;
	double m_level1Height;
	double m_moisture;
	double m_roughness;
	bool m_hasHeight;
	bool m_hasLevel1Height;

	double m_sunlightAffinity;
	bool m_hasSunlightAffinity;
};

