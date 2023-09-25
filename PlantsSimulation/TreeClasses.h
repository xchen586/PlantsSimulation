#pragma once

#include "CRoadAttributeDensityMap.h"
#include "CMoistureDensityMap.h"
#include "CRoughnessDensityMap.h"
#include "CHeightDensity.h"
#include "CSlopeDensityMap.h"

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

		DensityMap* roadAttributeDensity = new CRoadAttributeDensityMap();
		pair<string, DensityMap*> roadAttributePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roadAttributeDensity->type, roadAttributeDensity);
		
		DensityMap* moistureDensity = new COakMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, moistureDensity->type, moistureDensity);
		
		DensityMap* roughnessDensity = new COakRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roughnessDensity->type, roughnessDensity);

		DensityMap* heightDensity = new COakHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, heightDensity->type, heightDensity);
		
		DensityMap* slopeDensity = new COakSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, slopeDensity->type, slopeDensity);
		
		masks.insert(roadAttributePair);
		masks.insert(moisturePair);
		masks.insert(roughnessPair);
		masks.insert(heightPair);
		masks.insert(slopePair);
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

		DensityMap* roadAttributeDensity = new CRoadAttributeDensityMap();
		pair<string, DensityMap*> roadAttributePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roadAttributeDensity->type, roadAttributeDensity);
		DensityMap* moistureDensity = new CMapleMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, moistureDensity->type, moistureDensity);
		DensityMap* roughnessDensity = new CMapleRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roughnessDensity->type, roughnessDensity);
		DensityMap* heightDensity = new CMapleHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, heightDensity->type, heightDensity);
		DensityMap* slopeDensity = new CMapleSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, slopeDensity->type, slopeDensity);

		masks.insert(roadAttributePair);
		masks.insert(moisturePair);
		masks.insert(roughnessPair);
		masks.insert(heightPair);
		masks.insert(slopePair);	
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

		DensityMap* roadAttributeDensity = new CRoadAttributeDensityMap();
		pair<string, DensityMap*> roadAttributePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roadAttributeDensity->type, roadAttributeDensity);
		
		DensityMap* moistureDensity = new CBirchMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, moistureDensity->type, moistureDensity);
		
		DensityMap* roughnessDensity = new CBirchRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roughnessDensity->type, roughnessDensity);
		masks.insert(roughnessPair);

		DensityMap* heightDensity = new CBirchHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, heightDensity->type, heightDensity);
		
		DensityMap* slopeDensity = new CBirchSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, slopeDensity->type, slopeDensity);
			
		masks.insert(roadAttributePair);
		masks.insert(moisturePair);
		masks.insert(roughnessPair);
		masks.insert(heightPair);
		masks.insert(slopePair);
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

		DensityMap* roadAttributeDensity = new CRoadAttributeDensityMap();
		pair<string, DensityMap*> roadAttributePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roadAttributeDensity->type, roadAttributeDensity);
		
		DensityMap* moistureDensity = new CFirMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, moistureDensity->type, moistureDensity);
		
		DensityMap* roughnessDensity = new CFirRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, roughnessDensity->type, roughnessDensity);
		
		DensityMap* heightDensity = new CFirHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, heightDensity->type, heightDensity);
		
		DensityMap* slopeDensity = new CFirSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(type, slopeDensity->type, slopeDensity);

		masks.insert(roadAttributePair);
		masks.insert(moisturePair);
		masks.insert(roughnessPair);
		masks.insert(heightPair);
		masks.insert(slopePair);
	}
};