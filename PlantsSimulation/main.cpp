#include <iostream>
#include <fstream>
#include <filesystem> 
#include <random>
#include <string>

#include "CPlantsSimulation.h"
//#include "CForest.h"

int main(int argc, const char* argv[]) 
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

    std::string lod_string = lod_str;
    int lod = std::stoi(lod_string);

    std::cout << "Command line is : " << std::endl;
    std::cout << "Output path is : " << assets_path << std::endl;
    std::cout << "Assert path is : " << output_path << std::endl;
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
    std::cout << "Los is  : " << lod_str << std::endl;
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

    CPlantsSimulation ps(output_final_path, input_image_file, input_meta_file, mesh_heightmap_raw_file, mesh2_heightmap_raw_file, pc_heightmap_raw_file, l1_heightmap_raw_file
        , mesh_heightmap_masks_file, mesh2_heightmap_masks_file, pc_heightmap_masks_file, l1_heightmap_masks_file
        , point_most_travelled_file, point_most_distant_file, output_file, fullOutput_file, pcFullOutput_file, lod, tiles, tileX, tileY);
    
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
