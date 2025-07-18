#pragma once

#include "CRoadAttributeDensityMap.h"
#include "CMoistureDensityMap.h"
#include "CRoughnessDensityMap.h"
#include "CHeightDensity.h"
#include "CSlopeDensityMap.h"
#include "CSunLightAffinityDensityMap.h"

class COakTreeClass : public TreeClass
{
public:
	COakTreeClass() : TreeClass()
	{
		//typeId = static_cast<std::underlying_type<PlantType>::type>(PlantType::TREE_OAK);
		typeId = static_cast<unsigned int>(PlantType::TREE_OAK);
		treeTypeName = PlantTypeToString(static_cast<PlantType>(typeId));
		color = 0x00FF0000;
		matureAge = 35;
		maxAge = 350;
		seedRange = 300;

		DensityMap* roadAttributeDensity = new CRoadAttributeDensityMap();
		pair<string, DensityMap*> roadAttributePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), roadAttributeDensity->type, roadAttributeDensity);
		
		DensityMap* moistureDensity = new COakMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), moistureDensity->type, moistureDensity);
		
		DensityMap* roughnessDensity = new COakRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), roughnessDensity->type, roughnessDensity);

		DensityMap* heightDensity = new COakHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), heightDensity->type, heightDensity);
		
		DensityMap* slopeDensity = new COakSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), slopeDensity->type, slopeDensity);
		
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
		typeId = static_cast<unsigned int>(PlantType::TREE_MAPLE);
		treeTypeName = PlantTypeToString(static_cast<PlantType>(typeId));
		color = 0x0000FF00;
		//matureAge = 25;
		//maxAge = 200;
		//seedRange = 750;

		matureAge = 35;
		maxAge = 350;
		seedRange = 300;

		DensityMap* roadAttributeDensity = new CRoadAttributeDensityMap();
		pair<string, DensityMap*> roadAttributePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), roadAttributeDensity->type, roadAttributeDensity);
		DensityMap* moistureDensity = new CMapleMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), moistureDensity->type, moistureDensity);
		DensityMap* roughnessDensity = new CMapleRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), roughnessDensity->type, roughnessDensity);
		DensityMap* heightDensity = new CMapleHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), heightDensity->type, heightDensity);
		DensityMap* slopeDensity = new CMapleSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), slopeDensity->type, slopeDensity);

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
		typeId = static_cast<unsigned int>(PlantType::TREE_BIRCH);
		treeTypeName = PlantTypeToString(static_cast<PlantType>(typeId));
		color = 0x00FFFF00;
		matureAge = 30;
		maxAge = 150;
		seedRange = 500;

		DensityMap* roadAttributeDensity = new CRoadAttributeDensityMap();
		pair<string, DensityMap*> roadAttributePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), roadAttributeDensity->type, roadAttributeDensity);
		
		DensityMap* moistureDensity = new CBirchMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), moistureDensity->type, moistureDensity);
		
		DensityMap* roughnessDensity = new CBirchRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), roughnessDensity->type, roughnessDensity);
		masks.insert(roughnessPair);

		DensityMap* heightDensity = new CBirchHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), heightDensity->type, heightDensity);
		
		DensityMap* slopeDensity = new CBirchSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), slopeDensity->type, slopeDensity);
			
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
		typeId = static_cast<unsigned int>(PlantType::TREE_FIR);
		treeTypeName = PlantTypeToString(static_cast<PlantType>(typeId));
		color = 0x00D2B48C;
		matureAge = 40;
		maxAge = 300;
		seedRange = 500;

		DensityMap* roadAttributeDensity = new CRoadAttributeDensityMap();
		pair<string, DensityMap*> roadAttributePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), roadAttributeDensity->type, roadAttributeDensity);
		
		DensityMap* moistureDensity = new CFirMoistureDensityMap();
		pair<string, DensityMap*> moisturePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), moistureDensity->type, moistureDensity);
		
		DensityMap* roughnessDensity = new CFirRoughnessDensityMap();
		pair<string, DensityMap*> roughnessPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), roughnessDensity->type, roughnessDensity);
		
		DensityMap* heightDensity = new CFirHeightDensityMap();
		pair<string, DensityMap*> heightPair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), heightDensity->type, heightDensity);
		
		DensityMap* slopeDensity = new CFirSlopeDensityMap();
		pair<string, DensityMap*> slopePair = GetDensityKeyPairFromPlantTypeWithDensityMapType(static_cast<PlantType>(typeId), slopeDensity->type, slopeDensity);

		masks.insert(roadAttributePair);
		masks.insert(moisturePair);
		masks.insert(roughnessPair);
		masks.insert(heightPair);
		masks.insert(slopePair);
	}
};