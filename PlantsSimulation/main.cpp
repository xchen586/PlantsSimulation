#include <iostream>
#include <fstream>
#include <random>

// Define the number of plants and the area dimensions
const int numPlants = 100;
const int areaWidth = 100;
const int areaHeight = 100;

int main() {
    // Set up random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> disX(0.0, areaWidth);
    std::uniform_real_distribution<double> disY(0.0, areaHeight);

    // Create a file to store the coordinates
    std::ofstream outFile("plant_distribution.txt");

    // Generate random x and y coordinates for each plant
    for (int i = 0; i < numPlants; ++i) {
        double xCoord = disX(gen);
        double yCoord = disY(gen);
        outFile << xCoord << " " << yCoord << std::endl;
    }

    outFile.close();

    std::cout << "Random spatial distribution of plants generated." << std::endl;

    return 0;
}