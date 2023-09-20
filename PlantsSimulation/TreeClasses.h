#pragma once
#include "CHeightDensity.h"
#include "CSlopeDensityMap.h"
#include "CMoistureDensityMap.h"
#include "CRoughnessDensityMap.h"

class COakTreeClass : public TreeClass
{
public:
	COakTreeClass() : TreeClass()
	{
		type = PlantType::TREE_OAK;
		color = 0x00FF0000;
		matureAge = 35;
		maxAge = 350;
		seedRange = 300;

		DensityMap* heightDensity = new COakHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, heightDensity->type, heightDensity);
		masks.insert(heightPair);
		DensityMap* slopeDensity = new COakSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, slopeDensity->type, slopeDensity);
		masks.insert(slopePair);
		DensityMap* moistureDensity = new COakMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, moistureDensity->type, moistureDensity);
		masks.insert(moisturePair);
		DensityMap* roughnessDensity = new COakRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roughnessDensity->type, roughnessDensity);
		masks.insert(roughnessPair);
	}
};

class CMapleTreeClass : public TreeClass
{
public:
	CMapleTreeClass() : TreeClass()
	{
		type = PlantType::TREE_MAPLE;
		color = 0x0000FF00;
		matureAge = 25;
		maxAge = 200;
		seedRange = 750;

		DensityMap* heightDensity = new CMapleHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, heightDensity->type, heightDensity);
		masks.insert(heightPair);
		DensityMap* slopeDensity = new CMapleSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, slopeDensity->type, slopeDensity);
		masks.insert(slopePair);
		DensityMap* moistureDensity = new CMapleMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, moistureDensity->type, moistureDensity);
		masks.insert(moisturePair);
		DensityMap* roughnessDensity = new CMapleRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roughnessDensity->type, roughnessDensity);
		masks.insert(roughnessPair);
	}
};

class CBirchTreeClass : public TreeClass
{
public:
	CBirchTreeClass() : TreeClass()
	{
		type = PlantType::TREE_BIRCH;
		color = 0x00FFFF00;
		matureAge = 30;
		maxAge = 150;
		seedRange = 500;

		DensityMap* heightDensity = new CBirchHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, heightDensity->type, heightDensity);
		masks.insert(heightPair);
		DensityMap* slopeDensity = new CBirchSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, slopeDensity->type, slopeDensity);
		masks.insert(slopePair);
		DensityMap* moistureDensity = new CBirchMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, moistureDensity->type, moistureDensity);
		masks.insert(moisturePair);
		DensityMap* roughnessDensity = new CBirchRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roughnessDensity->type, roughnessDensity);
		masks.insert(roughnessPair);
	}
};

class CFirTreeClass : public TreeClass
{
public:
	CFirTreeClass() : TreeClass()
	{
		type = PlantType::TREE_FIR;
		color = 0x00D2B48C;
		matureAge = 40;
		maxAge = 300;
		seedRange = 500;

		DensityMap* heightDensity = new CFirHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, heightDensity->type, heightDensity);
		masks.insert(heightPair);
		DensityMap* slopeDensity = new CFirSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, slopeDensity->type, slopeDensity);
		masks.insert(slopePair);
		DensityMap* moistureDensity = new CFirMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, moistureDensity->type, moistureDensity);
		masks.insert(moisturePair);
		DensityMap* roughnessDensity = new CFirRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roughnessDensity->type, roughnessDensity);
		masks.insert(roughnessPair);
	}
};