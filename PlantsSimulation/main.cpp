#include <iostream>
#include <fstream>
#include <filesystem> 
#include <random>

#include "CPlantsSimulation.h"
//#include "CForest.h"

int main(int argc, const char* argv[]) 
{
    const char* assets_path = argv[1];
    const char* output_path = argv[2];
    const char* input_image_name = argv[3];
    const char* input_meta_name = argv[4];
    const char* mesh_heightmap_raw_name = argv[5];
    const char* pc_heightmap_raw_name = argv[6];

    const int MAX_PATH = 250;

    char input_image_file[MAX_PATH];
    char input_meta_file[MAX_PATH];
    char mesh_heightmap_raw_file[MAX_PATH];
    char pc_heightmap_raw_file[MAX_PATH];
    char output_file[MAX_PATH];
    char fullOutput_file[MAX_PATH];
    char pcFullOutput_file[MAX_PATH];
    memset(input_image_file, 0, sizeof(char) * MAX_PATH);
    memset(mesh_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(pc_heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(output_file, 0, sizeof(char) * MAX_PATH);
    memset(fullOutput_file, 0, sizeof(char) * MAX_PATH);
    memset(pcFullOutput_file, 0, sizeof(char) * MAX_PATH);

#if __APPLE__
    snprintf(input_image_file, MAX_PATH, "%s/%s", assets_path, input_image_name);
    snprintf(input_meta_file, MAX_PATH, "%s/%s", assets_path, input_meta_name);
    snprintf(mesh_heightmap_raw_file, MAX_PATH, "%s/%s", assets_path, mesh_heightmap_raw_name);
    snprintf(pc_heightmap_raw_file, MAX_PATH, "%s/%s", assets_path, pc_heightmap_raw_name);
    snprintf(output_file, MAX_PATH, "%s/plants.csv", output_path);
    snprintf(fullOutput_file, MAX_PATH, "%s/plantsfulloutput.csv", output_path);
    snprintf(pcOutput_file, MAX_PATH, "%s/plantspointscloudoutput.xyz", output_path);
#else
    sprintf_s(input_image_file, MAX_PATH, "%s\\%s", assets_path, input_image_name);
    sprintf_s(input_meta_file, MAX_PATH, "%s\\%s", assets_path, input_meta_name);
    sprintf_s(mesh_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_path, mesh_heightmap_raw_name);
    sprintf_s(pc_heightmap_raw_file, MAX_PATH, "%s\\%s", assets_path, pc_heightmap_raw_name);
    sprintf_s(output_file, MAX_PATH, "%s\\plants.csv", output_path);
    sprintf_s(fullOutput_file, MAX_PATH, "%s\\plantsfulloutput.csv", output_path);
    sprintf_s(pcFullOutput_file, MAX_PATH, "%s\\plantspointscloudoutput.xyz", output_path);
#endif

    // Check if the directory exists, and if not, create it
    if (!std::filesystem::exists(assets_path)) {
        if (!std::filesystem::create_directory(assets_path)) {
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

    CPlantsSimulation ps(output_path, input_image_file, input_meta_file, mesh_heightmap_raw_file, pc_heightmap_raw_file, output_file, fullOutput_file, pcFullOutput_file);
    
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
