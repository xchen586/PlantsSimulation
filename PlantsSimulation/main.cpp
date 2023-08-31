#include <iostream>
#include <fstream>
#include <random>

#include "CPlantsSimulation.h"

int main(int argc, const char* argv[]) 
{
    const char* assets_path = argv[1];
    const char* output_path = argv[2];
    const char* input_image_name = argv[3];
    const char* heightmap_raw_name = argv[4];

    const int MAX_PATH = 250;

    char input_image_file[MAX_PATH];
    char heightmap_raw_file[MAX_PATH];
    char output_file[MAX_PATH];

#if __APPLE__
    snprintf(input_image_file, MAX_PATH, "%s\\%s", assets_path, input_image_name);
    snprintf(heightmap_raw_file, MAX_PATH, "%s\\%s", assets_path, heightmap_raw_name);
    snprintf(output_file, MAX_PATH, "%s\\plants.csv", output_path);
#else
    sprintf_s(input_image_file, MAX_PATH, "%s\\%s", assets_path, input_image_name);
    sprintf_s(heightmap_raw_file, MAX_PATH, "%s\\%s", assets_path, heightmap_raw_name);
    sprintf_s(output_file, MAX_PATH, "%s\\plants.csv", output_path);
#endif

    return 0;
}