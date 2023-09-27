#include <iostream>
#include <fstream>
#include <random>

#include "CPlantsSimulation.h"
//#include "CForest.h"

int main(int argc, const char* argv[]) 
{
    const char* assets_path = argv[1];
    const char* output_path = argv[2];
    const char* input_image_name = argv[3];
    const char* input_meta_name = argv[4];
    const char* heightmap_raw_name = argv[5];

    const int MAX_PATH = 250;

    char input_image_file[MAX_PATH];
    char input_meta_file[MAX_PATH];
    char heightmap_raw_file[MAX_PATH];
    char output_file[MAX_PATH];
    char fullOutput_file[MAX_PATH];
    memset(input_image_file, 0, sizeof(char) * MAX_PATH);
    memset(heightmap_raw_file, 0, sizeof(char) * MAX_PATH);
    memset(output_file, 0, sizeof(char) * MAX_PATH);
    memset(fullOutput_file, 0, sizeof(char) * MAX_PATH);

#if __APPLE__
    snprintf(input_image_file, MAX_PATH, "%s/%s", assets_path, input_image_name);
    snprintf(input_meta_file, MAX_PATH, "%s/%s", assets_path, input_meta_name);
    snprintf(heightmap_raw_file, MAX_PATH, "%s/%s", assets_path, heightmap_raw_name);
    snprintf(output_file, MAX_PATH, "%s/plants.csv", output_path);
    snprintf(fullOutput_file, MAX_PATH, "%s/plantsfulloutput.csv", output_path);
#else
    sprintf_s(input_image_file, MAX_PATH, "%s\\%s", assets_path, input_image_name);
    sprintf_s(input_meta_file, MAX_PATH, "%s\\%s", assets_path, input_meta_name);
    sprintf_s(heightmap_raw_file, MAX_PATH, "%s\\%s", assets_path, heightmap_raw_name);
    sprintf_s(output_file, MAX_PATH, "%s\\plants.csv", output_path);
    sprintf_s(fullOutput_file, MAX_PATH, "%s\\plantsfulloutput.csv", output_path);
#endif

    CPlantsSimulation ps(input_image_file, input_meta_file, heightmap_raw_file, output_file, fullOutput_file);
    
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
