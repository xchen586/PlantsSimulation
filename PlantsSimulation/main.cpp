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

int commandlineMain(int argc, const char* argv[])
{
    const int t = atoi(argv[1]);
    const int x = atoi(argv[2]);
    const int y = atoi(argv[3]);

    const char* assets_path = argv[4];
    const char* output_path = argv[5];
    const char* input_image_name = argv[6];
    const char* input_meta_name = argv[7];
    const char* mesh_heightmap_raw_name = argv[8];
    const char* mesh2_heightmap_raw_name = argv[9];
    const char* pc_heightmap_raw_name = argv[10];
    const char* l1_heightmap_raw_name = argv[11];
    const char* mesh_heightmap_masks_name = argv[12];
    const char* mesh2_heightmap_masks_name = argv[13];
    const char* pc_heightmap_masks_name = argv[14];
    const char* l1_heightmap_masks_name = argv[15];
    const char* point_most_travelled_name = argv[16];
    const char* point_most_distant_name = argv[17];
    const char* lod_str = argv[18];
    const char* tree_list_csv_name = argv[19];
    const char* forest_age_str = argv[20];
    const char* tree_iteration_str = argv[21];
    const char* regions_raw_str = argv[22];

    string tree_list_csv = tree_list_csv_name ? tree_list_csv_name : "";

    std::string lod_string = lod_str;
    int lod = std::stoi(lod_string);    

    const float forestAge = (float)atof(forest_age_str);
    const int iteration = atoi(tree_iteration_str);
    

    std::cout << "Command line is : " << std::endl;
    std::cout << "Tiles count is : " << t << std::endl;
    std::cout << "Tiles x index is : " << x << std::endl;
    std::cout << "Tiles y index is : " << y << std::endl;
    std::cout << "Assert path is : " << assets_path << std::endl;
    std::cout << "Output path is : " << output_path << std::endl;
    std::cout << "Input image name is : " << input_image_name << std::endl;
    std::cout << "Input meta file name is : " << input_meta_name << std::endl;
    std::cout << "Mesh 0 height map file name is : " << mesh_heightmap_raw_name << std::endl;
    std::cout << "Mesh 1 height map file name is : " << mesh2_heightmap_raw_name << std::endl;
    std::cout << "Point cloud top level height map file name is : " << pc_heightmap_raw_name << std::endl;
    std::cout << "Point cloud level 1 height map file name is : " << l1_heightmap_raw_name << std::endl;
    std::cout << "Mesh 0 height map mask file name is : " << mesh_heightmap_masks_name << std::endl;
    std::cout << "Mesh 1 height map mask file name is : " << mesh2_heightmap_masks_name << std::endl;
    std::cout << "Point cloud top level height mask map file name is : " << pc_heightmap_masks_name << std::endl;
    std::cout << "Point cloud level 1 height mask map file name is : " << l1_heightmap_masks_name << std::endl;
    std::cout << "Most travelled point file name is : " << point_most_travelled_name << std::endl;
    std::cout << "Most distant  point file name is : " << point_most_distant_name << std::endl;
    std::cout << "Tree list csv file name is : " << tree_list_csv << std::endl;
    std::cout << "Regions Raw file name is : " << regions_raw_str << std::endl;
    std::cout << "Los is  : " << lod_str << std::endl;
    std::cout << "Forest Age is : " << forestAge << std::endl;
    std::cout << "Tree iteration count is : " << iteration << std::endl;
    std::cout << std::endl;

    int tiles = t;
    int tileX = x;
    int tileY = y;

    const int MAX_PATH = 250;

    char assets_final_path[MAX_PATH];
    char output_final_path[MAX_PATH];
    char input_image_file[MAX_PATH];
    char input_meta_file[MAX_PATH];
    char mesh_heightmap_raw_file[MAX_PATH];
    char mesh2_heightmap_raw_file[MAX_PATH];
    char pc_heightmap_raw_file[MAX_PATH];
    char l1_heightmap_raw_file[MAX_PATH];
    char mesh_heightmap_masks_file[MAX_PATH];
    char mesh2_heightmap_masks_file[MAX_PATH];
    char pc_heightmap_masks_file[MAX_PATH];
    char l1_heightmap_masks_file[MAX_PATH];
    char point_most_travelled_file[MAX_PATH];
    char point_most_distant_file[MAX_PATH];
    char output_file[MAX_PATH];
    char fullOutput_file[MAX_PATH];
    char pcFullOutput_file[MAX_PATH];
    memset(assets_final_path, 0, sizeof(char) * MAX_PATH);
    memset(output_final_path, 0, sizeof(char) * MAX_PATH);
    memset(input_image_file, 0, sizeof(char) * MAX_PATH);
    memset(mesh_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(mesh2_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(pc_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(l1_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(mesh_heightmap_masks_file, 0, sizeof(char) * MAX_PATH);
    memset(mesh2_heightmap_masks_file, 0, sizeof(char) * MAX_PATH);
    memset(pc_heightmap_masks_file, 0, sizeof(char) * MAX_PATH);
    memset(l1_heightmap_masks_file, 0, sizeof(char) * MAX_PATH);
    memset(point_most_travelled_file, 0, sizeof(char) * MAX_PATH);
    memset(point_most_distant_file, 0, sizeof(char) * MAX_PATH);
    memset(output_file, 0, sizeof(char) * MAX_PATH);
    memset(fullOutput_file, 0, sizeof(char) * MAX_PATH);
    memset(pcFullOutput_file, 0, sizeof(char) * MAX_PATH);

#if __APPLE__
    snprintf(assets_final_path, MAX_PATH, "%s/%d_%d_%d", assets_path, tiles, tileX, tileY);
    snprintf(output_final_path, MAX_PATH, "%s/%d_%d_%d", output_path, tiles, tileX, tileY);
    snprintf(input_image_file, MAX_PATH, "%s/%s", assets_final_path, input_image_name);
    snprintf(input_meta_file, MAX_PATH, "%s/%s", assets_final_path, input_meta_name);
    snprintf(mesh_heightmap_raw_file, MAX_PATH, "%s/%s", assets_final_path, mesh_heightmap_raw_name);
    snprintf(mesh2_heightmap_raw_file, MAX_PATH, "%s/%s", assets_final_path, mesh2_heightmap_raw_name);
    snprintf(pc_heightmap_raw_file, MAX_PATH, "%s/%s", assets_final_path, pc_heightmap_raw_name);
    snprintf(l1_heightmap_raw_file, MAX_PATH, "%s/%s", assets_final_path, l1_heightmap_raw_name);
    snprintf(mesh_heightmap_masks_file, MAX_PATH, "%s/%s", assets_final_path, mesh_heightmap_masks_name);
    snprintf(mesh2_heightmap_masks_file, MAX_PATH, "%s/%s", assets_final_path, mesh2_heightmap_masks_name);
    snprintf(pc_heightmap_masks_file, MAX_PATH, "%s/%s", assets_final_path, pc_heightmap_masks_name);
    snprintf(l1_heightmap_masks_file, MAX_PATH, "%s/%s", assets_final_path, l1_heightmap_masks_name);
    snprintf(point_most_travelled_file, MAX_PATH, "%s/%s", assets_final_path, point_most_distant_name);
    snprintf(point_most_distant_file, MAX_PATH, "%s/%s", assets_final_path, l1_heightmap_masks_name);
    snprintf(output_file, MAX_PATH, "%s/%d_%d_%d_plants.csv", output_final_path, tiles, tileX, tileY);
    snprintf(fullOutput_file, MAX_PATH, "%s/%d_%d_%d_plantsfulloutput.csv", output_final_path, tiles, tileX, tileY);
    snprintf(pcFullOutput_file, MAX_PATH, "%s/points_%d_%d_%d_tree.xyz", output_final_path, tiles, tileX, tileY);
#else
    sprintf_s(assets_final_path, MAX_PATH, "%s\\%d_%d_%d", assets_path, tiles, tileX, tileY);
    sprintf_s(output_final_path, MAX_PATH, "%s\\%d_%d_%d", output_path, tiles, tileX, tileY);
    sprintf_s(input_image_file, MAX_PATH, "%s\\%s", assets_final_path, input_image_name);
    sprintf_s(input_meta_file, MAX_PATH, "%s\\%s", assets_final_path, input_meta_name);
    sprintf_s(mesh_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_final_path, mesh_heightmap_raw_name);
    sprintf_s(mesh2_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_final_path, mesh2_heightmap_raw_name);
    sprintf_s(pc_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_final_path, pc_heightmap_raw_name);
    sprintf_s(l1_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_final_path, l1_heightmap_raw_name);
    sprintf_s(mesh_heightmap_masks_file, MAX_PATH, "%s\\%s", assets_final_path, mesh_heightmap_masks_name);
    sprintf_s(mesh2_heightmap_masks_file, MAX_PATH, "%s\\%s", assets_final_path, mesh_heightmap_masks_name);
    sprintf_s(pc_heightmap_masks_file, MAX_PATH, "%s\\%s", assets_final_path, pc_heightmap_masks_name);
    sprintf_s(l1_heightmap_masks_file, MAX_PATH, "%s\\%s", assets_final_path, l1_heightmap_masks_name);
    sprintf_s(point_most_travelled_file, MAX_PATH, "%s\\%s", assets_final_path, point_most_travelled_name);
    sprintf_s(point_most_distant_file, MAX_PATH, "%s\\%s", assets_final_path, point_most_distant_name);
    sprintf_s(output_file, MAX_PATH, "%s\\%d_%d_%d_plants.csv", output_final_path, tiles, tileX, tileY);
    sprintf_s(fullOutput_file, MAX_PATH, "%s\\%d_%d_%d_plantsfulloutput.csv", output_final_path, tiles, tileX, tileY);
    sprintf_s(pcFullOutput_file, MAX_PATH, "%s\\points_%d_%d_%d_tree.xyz", output_final_path, tiles, tileX, tileY);
#endif

    // Check if the directory exists, and if not, create it
    if (!std::filesystem::exists(assets_path)) {
        if (!std::filesystem::create_directory(assets_path)) {
            std::cerr << "Failed to create the directory of assets_path!" << std::endl;
            return -1;
        }
    }
    if (!std::filesystem::exists(assets_final_path)) {
        if (!std::filesystem::create_directory(assets_final_path)) {
            std::cerr << "Failed to create the directory of assets_path!" << std::endl;
            return -1;
        }
    }
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

    CPlantsSimulation ps(output_final_path, tree_list_csv, input_image_file, input_meta_file, mesh_heightmap_raw_file, mesh2_heightmap_raw_file, pc_heightmap_raw_file, l1_heightmap_raw_file
        , mesh_heightmap_masks_file, mesh2_heightmap_masks_file, pc_heightmap_masks_file, l1_heightmap_masks_file
        , point_most_travelled_file, point_most_distant_file, regions_raw_str, output_file, fullOutput_file, pcFullOutput_file, lod, forestAge, iteration, tiles, tileX, tileY);
    
    bool isLoad = ps.LoadInputData();
    if (!isLoad)
    {
        return -1;
    }
    bool loadForest = ps.LoadForest();
    bool buildForest = ps.BuildForest();
    bool results = ps.OutputResults();

    return 0;
}

int iniRelativePathMain(int argc, const char* argv[])
{
    const char* ini_Path = argv[1];
    if (!std::filesystem::exists(ini_Path))
    {
        std::cerr << "Failed to find relative ini file in " << ini_Path << " !" << std::endl;
        return -1;
    }
    
    PsIniParser iniParser;
    bool parseIni = iniParser.Parse(ini_Path);
    if (!parseIni)
    {
        std::cerr << "Failed to parse relative ini file in " << ini_Path << " !" << std::endl;
        return -1;
    }
    const string Title_Section = "Tiles";
    const string Input_Section = "Input";
    const string Output_Section = "Output";
    const string Others_Section = "Others";

    const char* t_str = GetIniValue(iniParser, Title_Section, "Tiles_Count");
    const char* x_str = GetIniValue(iniParser, Title_Section, "Tiles_X_Index");
    const char* y_str = GetIniValue(iniParser, Title_Section, "Tiles_Y_Index");
    const char* assets_path = GetIniValue(iniParser, Input_Section,"Asset_Dir");
    const char* output_path = GetIniValue(iniParser, Output_Section, "Output_Dir");
    const char* input_image_name = GetIniValue(iniParser, Input_Section, "Toplayer_Image");
    const char* input_meta_name = GetIniValue(iniParser, Input_Section, "Toplayer_Image_Meta");
    const char* mesh_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_0_HeightMap");
    const char* mesh2_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_1_HeightMap");
    const char* pc_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "TopLayer_HeightMap");
    const char* l1_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "Level1Layer_heightMap");;
    const char* mesh_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_0_HeightMap_Mask");
    const char* mesh2_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_1_HeightMap_Mask");
    const char* pc_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "TopLayer_HeightMap_Mask");
    const char* l1_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "Level1Layer_heightMap_Mask");
    const char* point_most_travelled_name = GetIniValue(iniParser, Input_Section, "Most_Travelled_Points");
    const char* point_most_distant_name = GetIniValue(iniParser, Input_Section, "Most_Distant_Points");
    const char* tree_list_csv_name = GetIniValue(iniParser, Input_Section, "Tree_List");
    string tree_list_csv = tree_list_csv_name ? tree_list_csv_name : "";
    const char* regions_raw_name = GetIniValue(iniParser, Input_Section, "Regions_Raw");
    string regions_raw_raw = regions_raw_name ? regions_raw_name : "";
   
    const char* lod_str = GetIniValue(iniParser, Others_Section, "Lod");
    const char* forest_age_str = GetIniValue(iniParser, Others_Section, "Forest_Age");
    const char* tree_iteration_str = GetIniValue(iniParser, Others_Section, "Tree_Iteration");

    const int t = atoi(t_str);
    const int x = atoi(x_str);
    const int y = atoi(y_str);
    const int lod = atoi(lod_str);
    const float forestAge = (float)atof(forest_age_str);
    const int iteration = atoi(tree_iteration_str);

    //std::string lod_string = lod_str;
    //int lod = std::stoi(lod_string);

    std::cout << "Command line is : " << std::endl;
    std::cout << "Tiles count is : " << t << std::endl;
    std::cout << "Tiles x index is : " << x << std::endl;
    std::cout << "Tiles y index is : " << y << std::endl;
    std::cout << "Assert path is : " << assets_path << std::endl;
    std::cout << "Output path is : " << output_path << std::endl;
    std::cout << "Input image name is : " << input_image_name << std::endl;
    std::cout << "Input meta file name is : " << input_meta_name << std::endl;
    std::cout << "Mesh 0 height map file name is : " << mesh_heightmap_raw_name << std::endl;
    std::cout << "Mesh 1 height map file name is : " << mesh2_heightmap_raw_name << std::endl;
    std::cout << "Point cloud top level height map file name is : " << pc_heightmap_raw_name << std::endl;
    std::cout << "Point cloud level 1 height map file name is : " << l1_heightmap_raw_name << std::endl;
    std::cout << "Mesh 0 height map mask file name is : " << mesh_heightmap_masks_name << std::endl;
    std::cout << "Mesh 1 height map mask file name is : " << mesh2_heightmap_masks_name << std::endl;
    std::cout << "Point cloud top level height mask map file name is : " << pc_heightmap_masks_name << std::endl;
    std::cout << "Point cloud level 1 height mask map file name is : " << l1_heightmap_masks_name << std::endl;
    std::cout << "Most travelled point file name is : " << point_most_travelled_name << std::endl;
    std::cout << "Most distant  point file name is : " << point_most_distant_name << std::endl;
    std::cout << "Tree list csv file name is : " << tree_list_csv << std::endl;
    std::cout << "Regions Raw file name is : " << regions_raw_raw << std::endl;
    std::cout << "Los is  : " << lod_str << std::endl;
    std::cout << "Forest Age is : " << forestAge << std::endl;
    std::cout << "Tree iteration count is : " << iteration << std::endl;
    std::cout << std::endl;

    int tiles = t;
    int tileX = x;
    int tileY = y;

    const int MAX_PATH = 250;

    char assets_final_path[MAX_PATH];
    char output_final_path[MAX_PATH];
    char input_image_file[MAX_PATH];
    char input_meta_file[MAX_PATH];
    char mesh_heightmap_raw_file[MAX_PATH];
    char mesh2_heightmap_raw_file[MAX_PATH];
    char pc_heightmap_raw_file[MAX_PATH];
    char l1_heightmap_raw_file[MAX_PATH];
    char mesh_heightmap_masks_file[MAX_PATH];
    char mesh2_heightmap_masks_file[MAX_PATH];
    char pc_heightmap_masks_file[MAX_PATH];
    char l1_heightmap_masks_file[MAX_PATH];
    char point_most_travelled_file[MAX_PATH];
    char point_most_distant_file[MAX_PATH];
    char output_file[MAX_PATH];
    char fullOutput_file[MAX_PATH];
    char pcFullOutput_file[MAX_PATH];
    memset(assets_final_path, 0, sizeof(char) * MAX_PATH);
    memset(output_final_path, 0, sizeof(char) * MAX_PATH);
    memset(input_image_file, 0, sizeof(char) * MAX_PATH);
    memset(mesh_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(mesh2_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(pc_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(l1_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(mesh_heightmap_masks_file, 0, sizeof(char) * MAX_PATH);
    memset(mesh2_heightmap_masks_file, 0, sizeof(char) * MAX_PATH);
    memset(pc_heightmap_masks_file, 0, sizeof(char) * MAX_PATH);
    memset(l1_heightmap_masks_file, 0, sizeof(char) * MAX_PATH);
    memset(point_most_travelled_file, 0, sizeof(char) * MAX_PATH);
    memset(point_most_distant_file, 0, sizeof(char) * MAX_PATH);
    memset(output_file, 0, sizeof(char) * MAX_PATH);
    memset(fullOutput_file, 0, sizeof(char) * MAX_PATH);
    memset(pcFullOutput_file, 0, sizeof(char) * MAX_PATH);

#if __APPLE__
    snprintf(assets_final_path, MAX_PATH, "%s/%d_%d_%d", assets_path, tiles, tileX, tileY);
    snprintf(output_final_path, MAX_PATH, "%s/%d_%d_%d", output_path, tiles, tileX, tileY);
    snprintf(input_image_file, MAX_PATH, "%s/%s", assets_final_path, input_image_name);
    snprintf(input_meta_file, MAX_PATH, "%s/%s", assets_final_path, input_meta_name);
    snprintf(mesh_heightmap_raw_file, MAX_PATH, "%s/%s", assets_final_path, mesh_heightmap_raw_name);
    snprintf(mesh2_heightmap_raw_file, MAX_PATH, "%s/%s", assets_final_path, mesh2_heightmap_raw_name);
    snprintf(pc_heightmap_raw_file, MAX_PATH, "%s/%s", assets_final_path, pc_heightmap_raw_name);
    snprintf(l1_heightmap_raw_file, MAX_PATH, "%s/%s", assets_final_path, l1_heightmap_raw_name);
    snprintf(mesh_heightmap_masks_file, MAX_PATH, "%s/%s", assets_final_path, mesh_heightmap_masks_name);
    snprintf(mesh2_heightmap_masks_file, MAX_PATH, "%s/%s", assets_final_path, mesh2_heightmap_masks_name);
    snprintf(pc_heightmap_masks_file, MAX_PATH, "%s/%s", assets_final_path, pc_heightmap_masks_name);
    snprintf(l1_heightmap_masks_file, MAX_PATH, "%s/%s", assets_final_path, l1_heightmap_masks_name);
    snprintf(point_most_travelled_file, MAX_PATH, "%s/%s", assets_final_path, point_most_distant_name);
    snprintf(point_most_distant_file, MAX_PATH, "%s/%s", assets_final_path, l1_heightmap_masks_name);
    snprintf(output_file, MAX_PATH, "%s/%d_%d_%d_plants.csv", output_final_path, tiles, tileX, tileY);
    snprintf(fullOutput_file, MAX_PATH, "%s/%d_%d_%d_plantsfulloutput.csv", output_final_path, tiles, tileX, tileY);
    snprintf(pcFullOutput_file, MAX_PATH, "%s/points_%d_%d_%d_tree.xyz", output_final_path, tiles, tileX, tileY);
#else
    sprintf_s(assets_final_path, MAX_PATH, "%s\\%d_%d_%d", assets_path, tiles, tileX, tileY);
    sprintf_s(output_final_path, MAX_PATH, "%s\\%d_%d_%d", output_path, tiles, tileX, tileY);
    sprintf_s(input_image_file, MAX_PATH, "%s\\%s", assets_final_path, input_image_name);
    sprintf_s(input_meta_file, MAX_PATH, "%s\\%s", assets_final_path, input_meta_name);
    sprintf_s(mesh_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_final_path, mesh_heightmap_raw_name);
    sprintf_s(mesh2_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_final_path, mesh2_heightmap_raw_name);
    sprintf_s(pc_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_final_path, pc_heightmap_raw_name);
    sprintf_s(l1_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_final_path, l1_heightmap_raw_name);
    sprintf_s(mesh_heightmap_masks_file, MAX_PATH, "%s\\%s", assets_final_path, mesh_heightmap_masks_name);
    sprintf_s(mesh2_heightmap_masks_file, MAX_PATH, "%s\\%s", assets_final_path, mesh_heightmap_masks_name);
    sprintf_s(pc_heightmap_masks_file, MAX_PATH, "%s\\%s", assets_final_path, pc_heightmap_masks_name);
    sprintf_s(l1_heightmap_masks_file, MAX_PATH, "%s\\%s", assets_final_path, l1_heightmap_masks_name);
    sprintf_s(point_most_travelled_file, MAX_PATH, "%s\\%s", assets_final_path, point_most_travelled_name);
    sprintf_s(point_most_distant_file, MAX_PATH, "%s\\%s", assets_final_path, point_most_distant_name);
    sprintf_s(output_file, MAX_PATH, "%s\\%d_%d_%d_plants.csv", output_final_path, tiles, tileX, tileY);
    sprintf_s(fullOutput_file, MAX_PATH, "%s\\%d_%d_%d_plantsfulloutput.csv", output_final_path, tiles, tileX, tileY);
    sprintf_s(pcFullOutput_file, MAX_PATH, "%s\\points_%d_%d_%d_tree.xyz", output_final_path, tiles, tileX, tileY);
#endif

    // Check if the directory exists, and if not, create it
    if (!std::filesystem::exists(assets_path)) {
        if (!std::filesystem::create_directory(assets_path)) {
            std::cerr << "Failed to create the directory of assets_path!" << std::endl;
            return -1;
        }
    }
    if (!std::filesystem::exists(assets_final_path)) {
        if (!std::filesystem::create_directory(assets_final_path)) {
            std::cerr << "Failed to create the directory of assets_path!" << std::endl;
            return -1;
        }
    }
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

    CPlantsSimulation ps(output_final_path, tree_list_csv, input_image_file, input_meta_file, mesh_heightmap_raw_file, mesh2_heightmap_raw_file, pc_heightmap_raw_file, l1_heightmap_raw_file
        , mesh_heightmap_masks_file, mesh2_heightmap_masks_file, pc_heightmap_masks_file, l1_heightmap_masks_file
        , point_most_travelled_file, point_most_distant_file, regions_raw_raw, output_file, fullOutput_file, pcFullOutput_file, lod, forestAge, iteration, tiles, tileX, tileY);

    bool isLoad = ps.LoadInputData();
    if (!isLoad)
    {
        return -1;
    }
    bool loadForest = ps.LoadForest();
    bool buildForest = ps.BuildForest();
    bool results = ps.OutputResults();

    if (t_str) delete t_str;
    if (x_str) delete x_str;
    if (y_str) delete y_str;
    if (assets_path) delete assets_path;
    if (output_path) delete output_path;
    if (input_image_name) delete input_image_name;
    if (input_meta_name) delete input_meta_name;
    if (mesh_heightmap_raw_name) delete mesh_heightmap_raw_name;
    if (mesh2_heightmap_raw_name) delete mesh2_heightmap_raw_name;
    if (pc_heightmap_raw_name) delete pc_heightmap_raw_name;
    if (l1_heightmap_raw_name) delete l1_heightmap_raw_name;
    if (mesh_heightmap_masks_name) delete mesh_heightmap_masks_name;
    if (mesh2_heightmap_masks_name) delete mesh2_heightmap_masks_name;
    if (pc_heightmap_masks_name) delete pc_heightmap_masks_name;
    if (l1_heightmap_masks_name) delete l1_heightmap_masks_name;
    if (point_most_travelled_name) delete point_most_travelled_name;
    if (point_most_distant_name) delete point_most_distant_name;
    if (lod_str) delete lod_str;
    
    return 0;
}

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
    const char* l1_heightmap_raw_name = GetIniValue(iniParser, Input_Section, "Level1Layer_heightMap");;
    const char* mesh_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_0_HeightMap_Mask");
    const char* mesh2_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "BaseMeshes_Level_1_HeightMap_Mask");
    const char* pc_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "TopLayer_HeightMap_Mask");
    const char* l1_heightmap_masks_name = GetIniValue(iniParser, Input_Section, "Level1Layer_heightMap_Mask");
    const char* point_most_travelled_name = GetIniValue(iniParser, Input_Section, "Most_Travelled_Points");
    const char* point_most_distant_name = GetIniValue(iniParser, Input_Section, "Most_Distant_Points");
    const char* regions_raw_name = GetIniValue(iniParser, Input_Section, "Regions_Raw");
    const char* tree_list_csv_name = GetIniValue(iniParser, Input_Section, "Tree_List");
    string tree_list_csv = tree_list_csv_name ? tree_list_csv_name : "";
    const char* lod_str = GetIniValue(iniParser, Others_Section, "Lod");
    const char* forest_age_str = GetIniValue(iniParser, Others_Section, "Forest_Age");
    const char* tree_iteration_str = GetIniValue(iniParser, Others_Section, "Tree_Iteration");

    const char* only_road_data_str = GetIniValue(iniParser, Options_Section, "Only_Road_Data");

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
    std::cout << "Mesh 0 height map mask file name is : " << (mesh_heightmap_masks_name ? mesh_heightmap_masks_name : "") << std::endl;
    std::cout << "Mesh 1 height map mask file name is : " << (mesh2_heightmap_masks_name ? mesh2_heightmap_masks_name : "") << std::endl;
    std::cout << "Point cloud top level height mask map file name is : " << (pc_heightmap_masks_name ? pc_heightmap_masks_name : "") << std::endl;
    std::cout << "Point cloud level 1 height mask map file name is : " << (l1_heightmap_masks_name ? l1_heightmap_masks_name : "") << std::endl;
    std::cout << "Most travelled point file name is : " << (point_most_travelled_name ? point_most_travelled_name : "") << std::endl;
    std::cout << "Most distant point file name is : " << (point_most_distant_name ? point_most_distant_name : "") << std::endl;
    std::cout << "Regions Raw file name is : " << (regions_raw_name ? regions_raw_name : "") << std::endl;
    std::cout << "Tree list csv file name is : " << (tree_list_csv_name ? tree_list_csv_name : "") << std::endl;
    std::cout << "Los is  : " << (lod_str ? lod_str : "") << std::endl;
    std::cout << "Forest Age is : " << (forest_age_str ? forest_age_str : "") << std::endl;
    std::cout << "Tree iteration count is : " << (tree_iteration_str ? tree_iteration_str : "") << std::endl;
    std::cout << "Only generate road data : " << (only_road_data_str ? only_road_data_str : "") << std::endl;
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

    CPlantsSimulation ps(output_final_path, tree_list_csv, input_image_name, input_meta_name, mesh_heightmap_raw_name, mesh2_heightmap_raw_name, pc_heightmap_raw_name, l1_heightmap_raw_name
        , mesh_heightmap_masks_name, mesh2_heightmap_masks_name, pc_heightmap_masks_name, l1_heightmap_masks_name
        , point_most_travelled_name, point_most_distant_name, regions_raw_name, output_file, fullOutput_file, pcFullOutput_file, lod, forestAge, iteration, tiles, tileX, tileY);

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
    if (mesh_heightmap_masks_name) delete mesh_heightmap_masks_name;
    if (mesh2_heightmap_masks_name) delete mesh2_heightmap_masks_name;
    if (pc_heightmap_masks_name) delete pc_heightmap_masks_name;
    if (l1_heightmap_masks_name) delete l1_heightmap_masks_name;
    if (point_most_travelled_name) delete point_most_travelled_name;
    if (regions_raw_name) delete regions_raw_name;
    if (point_most_distant_name) delete point_most_distant_name;
    if (lod_str) delete lod_str;
    if (only_road_data_str) delete only_road_data_str;

    return 0;
}
int main(int argc, const char* argv[])
{
    //return commandlineMain(argc, argv);
    //return iniRelativePathMain(argc, argv);
    return iniAbsolutePathMain(argc, argv);
}
