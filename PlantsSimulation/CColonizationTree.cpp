#include "CColonizationTree.h"

string PlantTypeToString(PlantType type) {
    switch (type) {
    case PlantType::TREE_OAK:
        return "TREE_OAK";
    case PlantType::TREE_MAPLE:
        return "TREE_MAPLE";
    case PlantType::TREE_PINE:
        return "TREE_PINE";
    case PlantType::TREE_BIRCH:
        return "TREE_BIRCH";
    case PlantType::TREE_WILLOW:
        return "TREE_WILLOW";
    case PlantType::TREE_CHERRY:
        return "TREE_CHERRY";
    case PlantType::TREE_APPLE:
        return "TREE_APPLE";
    case PlantType::TREE_ORANGE:
        return "TREE_ORANGE";
    case PlantType::TREE_LEMON:
        return "TREE_LEMON";
    case PlantType::TREE_PALM:
        return "TREE_PALM";
    case PlantType::TREE_SPRUCE:
        return "TREE_SPRUCE";
    case PlantType::TREE_CEDAR:
        return "TREE_CEDAR";
    case PlantType::TREE_FIR:
        return "TREE_FIR";

    case PlantType::FLOWER_ROSE:
        return "FLOWER_ROSE";
    case PlantType::FLOWER_TULIP:
        return "FLOWER_TULIP";
    case PlantType::FLOWER_DAISY:
        return "Daisy Flower";
    case PlantType::SHRUB_AZALEA:
        return "FLOWER_DAISY";
    case PlantType::SHRUB_RHODODENDRON:
        return "SHRUB_RHODODENDRON";
    case PlantType::VEGETABLE_TOMATO:
        return "VEGETABLE_TOMATO";
    case PlantType::VEGETABLE_CARROT:
        return "VEGETABLE_CARROT";
    case PlantType::HERB_BASIL:
        return "HERB_BASIL";
    case PlantType::HERB_MINT:
        return "HERB_MINT";

    case PlantType::LAST_PLANT_TYPE:
        return "LAST_PLANT_TYPE";
    default:
        return "Unknown_Plant";
    }
}

string PlantTypeToMaskString(PlantType type, int index)
{
    string typeName = PlantTypeToString(type);
    string ret = typeName + "_" + to_string(index);
    return ret;
}

pair<string, DensityMap*> GetDensityKeyPairFromPlantTypeWithIndex(PlantType type, int index, DensityMap* density)
{
    string keyString = PlantTypeToMaskString(type, index);
    pair<string, DensityMap*> ret(keyString, density);
    return ret;
}

PlantType RandomPlantType() {
    // Get the number of enum values (count from the first enum to the last)
    int numEnumValues = static_cast<int>(PlantType::LAST_PLANT_TYPE) + 1;

    // Create a random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, numEnumValues - 1);

    // Generate a random number
    int randomNum = distribution(gen);

    // Map the random number to a PlantType enum value
    return static_cast<PlantType>(randomNum);
}