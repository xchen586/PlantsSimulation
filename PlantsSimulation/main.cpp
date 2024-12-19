#include <iostream>
#include <fstream>
#include <filesystem> 
#include <random>
#include <string>

#include "CPlantsSimulation.h"

#if __APPLE__
#include "../Common/include/PsIniParser.h"
#else
#include "..\Common\include\PsIniParser.h"
#endif

int iniAbsolutePathMain(int argc, const char* argv[])
{
    const char* ini_Path = argv[1];
    if (!std::filesystem::exists(ini_Path))
    {
        std::cerr << "Failed to find ini file in " << ini_Path << " !" << std::endl;
        return -1;
    }

    PsIniParser iniParser;
    bool parseIni = iniParser.Parse(ini_Path);
    if (!parseIni)
    {
        std::cerr << "Failed to parse ini file in " << ini_Path << " !" << std::endl;
        return -1;
    }
    const string Title_Section = "Tiles";
    const string Input_Section = "Input";
    const string Output_Section = "Output";
    const string Others_Section = "Others";
    const string Options_Section = "Options";

    const char* t_str = GetIniValue(iniParser, Title_Section, "Tiles_Count");
    const char* x_str = GetIniValue(iniParser, Title_Section, "Tiles_X_Index");
    const char* y_str = GetIniValue(iniParser, Title_Section, "Tiles_Y_Index");
    //const char* assets_path = GetIniValue(iniParser, Input_Section, "Asset_Dir");
    const char* output_path = GetIniValue(iniParser, Output_Section, "Output_Dir");
    const char* input_image_name = GetIniValue(iniParser, Input_Section, "Toplayer_Image");
    const char* input_meta_name = GetIniValue(iniParser, Input_Section, "Toplayer_Image_Meta");
    const char* mesh_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_0_HeightMap");
    const char* mesh2_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_1_HeightMap");
    const char* pc_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "TopLayer_HeightMap");
    const char* l1_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "Level1Layer_heightMap");
    const char* bedrock_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "BedrokLayer_heightMap");
    const char* mesh_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_0_HeightMap_Mask");
    const char* mesh2_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_1_HeightMap_Mask");
    const char* pc_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "TopLayer_HeightMap_Mask");
    const char* l1_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "Level1Layer_heightMap_Mask");
    const char* bedrock_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "Bedrock_heightMap_Mask");
    const char* point_most_travelled_name = GetIniValue(iniParser, Input_Section, "Most_Travelled_Points");
    const char* point_most_distant_name = GetIniValue(iniParser, Input_Section, "Most_Distant_Points");
    const char* caves_point_cloud_level_0_name = GetIniValue(iniParser, Input_Section, "Caves_Point_Cloud_Level_0");
    const char* caves_point_cloud_level_1_name = GetIniValue(iniParser, Input_Section, "Caves_Point_Cloud_Level_1");
    const char* regions_raw_name = GetIniValue(iniParser, Input_Section, "Regions_Raw");
    const char* regions_info_name = GetIniValue(iniParser, Input_Section, "Regions_Info");
    const char* tree_list_csv_name = GetIniValue(iniParser, Input_Section, "Tree_List");
    string tree_list_csv = tree_list_csv_name ? tree_list_csv_name : "";
    const char* lod_str = GetIniValue(iniParser, Others_Section, "Lod");
    const char* forest_age_str = GetIniValue(iniParser, Others_Section, "Forest_Age");
    const char* tree_iteration_str = GetIniValue(iniParser, Others_Section, "Tree_Iteration");

    const char* only_road_data_str = GetIniValue(iniParser, Options_Section, "Only_Road_Data");
    const char* use_with_basemeshes_level1_str = GetIniValue(iniParser, Options_Section, "Use_With_BaseMeshes_Level1");

    const int t = atoi(t_str);
    const int x = atoi(x_str);
    const int y = atoi(y_str);
    const int lod = atoi(lod_str);
    const float forestAge = (float)atof(forest_age_str);
    const int iteration = atoi(tree_iteration_str);
    bool isOnlyRoadData = false;
    if (only_road_data_str)
    {
        bool isOnlyRoadDataValue = stringToBool(only_road_data_str);
        if (isOnlyRoadDataValue)
        {
            isOnlyRoadData = isOnlyRoadDataValue;
        }
    }
    bool useBaseMeshesLevel1 = true;
    if (use_with_basemeshes_level1_str)
    {
        bool useBaseMeshesLevel1Value = stringToBool(use_with_basemeshes_level1_str);
        if (!useBaseMeshesLevel1Value)
        {
            useBaseMeshesLevel1 = useBaseMeshesLevel1Value;
        }
    }

    std::cout << "Command line is : " << std::endl;
    std::cout << "Tiles count is : " << (t_str ? t_str : "") << std::endl;
    std::cout << "Tiles x index is : " << (x_str ? x_str : "") << std::endl;
    std::cout << "Tiles y index is : " << (y_str ? y_str : "") << std::endl;
    //std::cout << "Assert path is : " << assets_path << std::endl;
    std::cout << "Output path is : " << (output_path ? output_path : "") << std::endl;
    std::cout << "Input image name is : " << (input_image_name ? input_image_name : "") << std::endl;
    std::cout << "Input meta file name is : " << (input_meta_name ? input_meta_name : "") << std::endl;
    std::cout << "Mesh 0 height map file name is : " << (mesh_heightmap_raw_name ? mesh_heightmap_raw_name : "") << std::endl;
    std::cout << "Mesh 1 height map file name is : " << (mesh2_heightmap_raw_name ? mesh2_heightmap_raw_name : "") << std::endl;
    std::cout << "Point cloud top level height map file name is : " << (pc_heightmap_raw_name ? pc_heightmap_raw_name : "") << std::endl;
    std::cout << "Point cloud level 1 height map file name is : " << (l1_heightmap_raw_name ? l1_heightmap_raw_name : "") << std::endl;
    std::cout << "Point cloud bedrock height map file name is : " << (bedrock_heightmap_raw_name ? bedrock_heightmap_raw_name : "") << std::endl;
    std::cout << "Mesh 0 height map mask file name is : " << (mesh_heightmap_masks_name ? mesh_heightmap_masks_name : "") << std::endl;
    std::cout << "Mesh 1 height map mask file name is : " << (mesh2_heightmap_masks_name ? mesh2_heightmap_masks_name : "") << std::endl;
    std::cout << "Point cloud top level height mask map file name is : " << (pc_heightmap_masks_name ? pc_heightmap_masks_name : "") << std::endl;
    std::cout << "Point cloud level 1 height mask map file name is : " << (l1_heightmap_masks_name ? l1_heightmap_masks_name : "") << std::endl;
    std::cout << "Point cloud bedrock height mask map file name is : " << (bedrock_heightmap_masks_name ? bedrock_heightmap_masks_name : "") << std::endl;
    std::cout << "Most travelled point file name is : " << (point_most_travelled_name ? point_most_travelled_name : "") << std::endl;
    std::cout << "Most distant point file name is : " << (point_most_distant_name ? point_most_distant_name : "") << std::endl;
    std::cout << "Cave point cloud level 0 file name is : " << (caves_point_cloud_level_0_name ? caves_point_cloud_level_0_name : "") << std::endl;
    std::cout << "Cave point cloud level 1 file name is : " << (caves_point_cloud_level_1_name ? caves_point_cloud_level_1_name : "") << std::endl;
    std::cout << "Regions Raw file name is : " << (regions_raw_name ? regions_raw_name : "") << std::endl;
    std::cout << "Regions Info file name is : " << (regions_info_name ? regions_info_name : "") << std::endl;
    std::cout << "Tree list csv file name is : " << (tree_list_csv_name ? tree_list_csv_name : "") << std::endl;
    std::cout << "Los is  : " << (lod_str ? lod_str : "") << std::endl;
    std::cout << "Forest Age is : " << (forest_age_str ? forest_age_str : "") << std::endl;
    std::cout << "Tree iteration count is : " << (tree_iteration_str ? tree_iteration_str : "") << std::endl;
    std::cout << "Only generate road data : " << (only_road_data_str ? only_road_data_str : "") << std::endl;
    std::cout << "Use with base meshes level1  : " << (use_with_basemeshes_level1_str ? use_with_basemeshes_level1_str : "") << std::endl;
    std::cout << std::endl;

    int tiles = t;
    int tileX = x;
    int tileY = y;

    const int MAX_PATH = 250;

    char output_final_path[MAX_PATH];
    char output_file[MAX_PATH];
    char fullOutput_file[MAX_PATH];
    char pcFullOutput_file[MAX_PATH];
    
    memset(output_final_path, 0, sizeof(char) * MAX_PATH);
    memset(output_file, 0, sizeof(char) * MAX_PATH);
    memset(fullOutput_file, 0, sizeof(char) * MAX_PATH);
    memset(pcFullOutput_file, 0, sizeof(char) * MAX_PATH);

#if __APPLE__
    snprintf(output_final_path, MAX_PATH, "%s/%d_%d_%d", output_path, tiles, tileX, tileY);
    snprintf(output_file, MAX_PATH, "%s/%d_%d_%d_plants.csv", output_final_path, tiles, tileX, tileY);
    snprintf(fullOutput_file, MAX_PATH, "%s/%d_%d_%d_plantsfulloutput.csv", output_final_path, tiles, tileX, tileY);
    snprintf(pcFullOutput_file, MAX_PATH, "%s/points_%d_%d_%d_tree.xyz", output_final_path, tiles, tileX, tileY);
#else
    sprintf_s(output_final_path, MAX_PATH, "%s\\%d_%d_%d", output_path, tiles, tileX, tileY);
    sprintf_s(output_file, MAX_PATH, "%s\\%d_%d_%d_plants.csv", output_final_path, tiles, tileX, tileY);
    sprintf_s(fullOutput_file, MAX_PATH, "%s\\%d_%d_%d_plantsfulloutput.csv", output_final_path, tiles, tileX, tileY);
    sprintf_s(pcFullOutput_file, MAX_PATH, "%s\\points_%d_%d_%d_tree.xyz", output_final_path, tiles, tileX, tileY);
#endif

    if (!std::filesystem::exists(output_path)) {
        if (!std::filesystem::create_directory(output_path)) {
            std::cerr << "Failed to create the directory of output_path!" << std::endl;
            return -1;
        }
    }
    if (!std::filesystem::exists(output_final_path)) {
        if (!std::filesystem::create_directory(output_final_path)) {
            std::cerr << "Failed to create the directory of output_path!" << std::endl;
            return -1;
        }
    }

    CPlantsSimulation ps(output_final_path, tree_list_csv, input_image_name, input_meta_name, mesh_heightmap_raw_name, mesh2_heightmap_raw_name, pc_heightmap_raw_name, l1_heightmap_raw_name, bedrock_heightmap_raw_name
        , mesh_heightmap_masks_name, mesh2_heightmap_masks_name, pc_heightmap_masks_name, l1_heightmap_masks_name, bedrock_heightmap_masks_name
        , point_most_travelled_name, point_most_distant_name, caves_point_cloud_level_0_name, caves_point_cloud_level_1_name, regions_raw_name, regions_info_name, output_file, fullOutput_file, pcFullOutput_file, lod, forestAge, iteration, tiles, tileX, tileY, useBaseMeshesLevel1);

    bool isLoad = ps.LoadInputData();
    if (!isLoad)
    {
        return -1;
    }
    if (isOnlyRoadData)
    {
        std::cout << "The program only need generate road data without tree instance" << std::endl;
    }
    else
    {
        bool loadForest = ps.LoadForest();
        bool buildForest = ps.BuildForest();
        bool results = ps.OutputResults();

    }
    
    if (t_str) delete t_str;
    if (x_str) delete x_str;
    if (y_str) delete y_str;
    //if (assets_path) delete assets_path;
    if (output_path) delete output_path;
    if (input_image_name) delete input_image_name;
    if (input_meta_name) delete input_meta_name;
    if (mesh_heightmap_raw_name) delete mesh_heightmap_raw_name;
    if (mesh2_heightmap_raw_name) delete mesh2_heightmap_raw_name;
    if (pc_heightmap_raw_name) delete pc_heightmap_raw_name;
    if (l1_heightmap_raw_name) delete l1_heightmap_raw_name;
    if (bedrock_heightmap_raw_name) delete bedrock_heightmap_raw_name;
    if (mesh_heightmap_masks_name) delete mesh_heightmap_masks_name;
    if (mesh2_heightmap_masks_name) delete mesh2_heightmap_masks_name;
    if (pc_heightmap_masks_name) delete pc_heightmap_masks_name;
    if (l1_heightmap_masks_name) delete l1_heightmap_masks_name;
    if (bedrock_heightmap_masks_name) delete bedrock_heightmap_masks_name;
    if (point_most_travelled_name) delete point_most_travelled_name;
    if (point_most_distant_name) delete point_most_distant_name;
    if (caves_point_cloud_level_0_name) delete caves_point_cloud_level_0_name;
    if (caves_point_cloud_level_1_name) delete caves_point_cloud_level_1_name;
    if (regions_raw_name) delete regions_raw_name;
    if (regions_info_name) delete regions_info_name;
    if (lod_str) delete lod_str;
    if (only_road_data_str) delete only_road_data_str;

    return 0;
}
int main(int argc, const char* argv[])
{
    return iniAbsolutePathMain(argc, argv);
}
