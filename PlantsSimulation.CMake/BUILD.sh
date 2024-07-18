#!/bin/sh

echo Building PlantsSimulation..

mkdir -p build
cd build
cmake -G Xcode ..
cmake --build . --config Debug 
cmake --build . --config Release 

cd ..

echo Done.
