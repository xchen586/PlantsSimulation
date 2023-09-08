#include "CForest.h"
#include <iostream>
#include <fstream>

CForest::CForest(void)
{
	grid = NULL;
}

CForest::~CForest(void)
{
	for (map<string, DensityMap*>::iterator i = globalMasks.begin(); i != globalMasks.end(); ++i)
		delete i->second;
}

#define SEED_MAX 16*1024*1024
void CForest::loadTreeClasses()
{
	for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i) {
		TreeClass* tree = *i;
		delete tree;
	}
	classes.clear();

	TreeClass* treeClass1 = new TreeClass();
	treeClass1->type = PlantType::TREE_OAK;
	treeClass1->color = 0x00FF0000;
	treeClass1->matureAge = 10;
	treeClass1->maxAge = 80;

	DensityMap*  densityTree1_0 = new DensityMap();
	densityTree1_0->minval = 0.00001;
	densityTree1_0->maxval = 0.99999;
	densityTree1_0->ease = 0.3333;
	densityTree1_0->blur = 1;
	densityTree1_0->invert = false;
	densityTree1_0->useForThinning = true;
	pair<string, DensityMap*> densityPair1_0 = GetDensityKeyPairFromPlantTypeWithIndex(PlantType::TREE_OAK, 0, densityTree1_0);
	treeClass1->masks.insert(densityPair1_0);

	classes.push_back(treeClass1);

	TreeClass* treeClass2 = new TreeClass();
	treeClass2->type = PlantType::TREE_MAPLE;
	treeClass2->matureAge = 20;
	treeClass2->maxAge = 100;
	treeClass2->color = 0x0000FF00;

	DensityMap* densityTree2_0 = new DensityMap();
	densityTree2_0->minval = 0.0002;
	densityTree2_0->maxval = 0.9999;
	densityTree2_0->ease = 0.5555;
	densityTree2_0->blur = 1;
	densityTree2_0->invert = false;
	densityTree2_0->useForThinning = true;
	pair<string, DensityMap*> densityPair2_0 = GetDensityKeyPairFromPlantTypeWithIndex(PlantType::TREE_MAPLE, 0, densityTree2_0);
	treeClass2->masks.insert(densityPair2_0);

	classes.push_back(treeClass2);

	TreeClass* treeClass3 = new TreeClass();
	treeClass2->type = PlantType::TREE_FIR;
	treeClass3->matureAge = 50;
	treeClass3->maxAge = 200;
	treeClass3->color = 0x000000FF;

	DensityMap* densityTree3_0 = new DensityMap();
	densityTree3_0->minval = 0.0056;
	densityTree3_0->maxval = 1.6666;
	densityTree3_0->ease = 0.7777;
	densityTree3_0->blur = 1;
	densityTree3_0->invert = true;
	densityTree3_0->useForThinning = true;
	pair<string, DensityMap*> densityPair3_0 = GetDensityKeyPairFromPlantTypeWithIndex(PlantType::TREE_FIR, 0, densityTree3_0);
	treeClass3->masks.insert(densityPair3_0);

	classes.push_back(treeClass3);

	return;
}

void CForest::loadMasks()
{
	for (map<string, I2DMask*>::iterator iMask = masks.begin(); iMask != masks.end(); ++iMask) {
		I2DMask* i2DMask = iMask->second;
		delete i2DMask;
	}
	masks.clear();

	int typeIndex = 0;
	I2DMask* pI2DMask = new I2DMask();
	pair<string, I2DMask*> treeType1Pair = GetI2DMaskKeyPairFromPlantTypeWithIndex(PlantType::TREE_OAK, typeIndex, pI2DMask);
	masks.insert(treeType1Pair);
	pair<string, I2DMask*> treeType2Pair = GetI2DMaskKeyPairFromPlantTypeWithIndex(PlantType::TREE_MAPLE, typeIndex, pI2DMask);
	masks.insert(treeType2Pair);
	pair<string, I2DMask*> treeType3Pair = GetI2DMaskKeyPairFromPlantTypeWithIndex(PlantType::TREE_FIR, typeIndex, pI2DMask);
	masks.insert(treeType3Pair);

	return;
}

void CForest::loadGlobalMasks()
{
	for (map<string, DensityMap*>::iterator imap = globalMasks.begin(); imap != globalMasks.end(); ++imap) {
		DensityMap* density = imap->second;
		delete density;
	}
	globalMasks.clear();

	DensityMap* densityTree1_0 = new DensityMap();
	densityTree1_0->minval = 0.00001;
	densityTree1_0->maxval = 0.99999;
	densityTree1_0->ease = 0.3333;
	densityTree1_0->blur = 1;
	densityTree1_0->invert = false;
	densityTree1_0->useForThinning = true;
	pair<string, DensityMap*> densityPair1_0 = GetDensityKeyPairFromPlantTypeWithIndex(PlantType::TREE_OAK, 0, densityTree1_0);

	globalMasks.insert(densityPair1_0);

	DensityMap* densityTree2_0 = new DensityMap();
	densityTree2_0->minval = 0.0002;
	densityTree2_0->maxval = 0.88888;
	densityTree2_0->ease = 0.5555;
	densityTree2_0->blur = 1;
	densityTree2_0->invert = false;
	densityTree2_0->useForThinning = true;
	pair<string, DensityMap*> densityPair2_0 = GetDensityKeyPairFromPlantTypeWithIndex(PlantType::TREE_MAPLE, 0, densityTree2_0);

	globalMasks.insert(densityPair2_0);

	DensityMap* densityTree3_0 = new DensityMap();
	densityTree3_0->minval = 0.0056;
	densityTree3_0->maxval = 1.6666;
	densityTree3_0->ease = 0.7777;
	densityTree3_0->blur = 1;
	densityTree3_0->invert = true;
	densityTree3_0->useForThinning = true;
	pair<string, DensityMap*> densityPair3_0 = GetDensityKeyPairFromPlantTypeWithIndex(PlantType::TREE_FIR, 0, densityTree3_0);
	
	globalMasks.insert(densityPair3_0);

	return;
}

void CForest::generate(float forestAge, int iterations)
{
	// allocate grid
	int gridDelta = 64;
	int gridXSize = xSize/gridDelta;
	int gridZSize = zSize/gridDelta;
	int gridSize = (gridXSize + 1)*(gridZSize + 1)*sizeof(int);
	int* grid = (int*)malloc(gridSize);
	memset(grid, 0, gridSize);

	CTreeInstance* instances = (CTreeInstance*)malloc(SEED_MAX*sizeof(CTreeInstance));
	memset(instances, 0, SEED_MAX*sizeof(CTreeInstance));
	int instanceIndex = 0;

	ClassStrength* classArray = (ClassStrength*)malloc(classes.size()*sizeof(ClassStrength));

	/*
	for (int x = xo; x < xo + xSize; x += gridDelta)
		for (int z = zo; z < zo + zSize; z += gridDelta)
		{
			int gridX = (x - xo)/gridDelta;
			int gridZ = (z - zo)/gridDelta;
			int gridIndex = gridXSize*gridZ + gridX;
			int& gridP = grid[gridIndex];

			int classIdx = 0;
			double maskSpan = 0.0;

			for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i)
			{
				TreeClass* treeClass = *i;

				double maskValue = 1.0;
				for (map<string, I2DMask*>::iterator iMask = masks.begin(); iMask != masks.end() && maskValue > 0.0; ++iMask)
				{
					double mask = iMask->second->get2DMaskValue(x, z); 

					string maskId = iMask->first;
					map<string, TreeParamRange>::iterator classMask = treeClass->masks.find(maskId);

					if (classMask != treeClass->masks.end())
					{
						TreeParamRange& param = classMask->second;
						if (mask < param.minV || mask > param.maxV)
							maskValue = 0.0;
					}
				}
				maskSpan += maskValue;
				ClassStrength& c = classArray[classIdx];
				c.strength = maskValue;
				c.treeClass = treeClass;
				classIdx++;
			}

			double dice = maskSpan*rand()/RAND_MAX;
			double sum = 0.0;
			TreeClass* chosen = NULL;			
			for (int i = 0; i < classIdx && sum < dice; i++)
			{
				ClassStrength& c = classArray[i];
				chosen = c.treeClass;
				sum += c.strength;
			}

			if (chosen != NULL)
			{
				CTreeInstance& t = instances[instanceIndex];
				t.treeClass = chosen;
				t.bday = -(float)rand()*chosen->matureAge/RAND_MAX;
				t.x = x;
				t.z = z;
				t.dead = false;
				t.mature = false;

				instanceIndex++;
				gridP = instanceIndex;
			}

		}
	*/

	double timeSlice = forestAge/iterations;
	for (int iteration = 0; iteration < iterations; iteration++)
	{
		bool lastIteration = iteration == iterations - 1;
		double time = timeSlice*iteration;

		for (int x = xo; x < xo + xSize; x += gridDelta)
			for (int z = zo; z < zo + zSize; z += gridDelta)
			{
				if (rand() > RAND_MAX/10)
					continue;


				int gridX = (x - xo)/gridDelta;
				int gridZ = (z - zo)/gridDelta;
				int gridIndex = gridXSize*gridZ + gridX;
				int& gridP = grid[gridIndex];

				bool empty = false;
				if (gridP > 0)
				{
					CTreeInstance& neighborTree = instances[gridP - 1];
					empty = neighborTree.dead;
				}
				else
					empty = true;

				if (!empty)
					continue;

				int classIdx = 0;
				double maskSpan = 0.0;

				for (vector<TreeClass*>::iterator i = classes.begin(); i != classes.end(); ++i)
				{
					TreeClass* treeClass = *i;

					double maskValue = 1.0;
					for (map<string, I2DMask*>::iterator iMask = masks.begin(); iMask != masks.end() && maskValue > 0.0; ++iMask)
					{
						string maskId = iMask->first;
						map<string, DensityMap*>::iterator classMask = treeClass->masks.find(maskId);

						if (classMask != treeClass->masks.end())
						{
							DensityMap* dmap = classMask->second;

							double mask = iMask->second->get2DMaskValue(x, z, dmap->blur); 

							if (dmap->invert)
								mask = 1.0 - mask;
							if (mask < dmap->minval || mask > dmap->maxval)
								maskValue = 0.0;
							else
							{
								double scope = abs(dmap->minval - dmap->maxval);
								if (scope > 0.000001)
								{
									maskValue = (mask - dmap->minval)/scope;
								}
								else
									maskValue = 0.0;
							}
						}
					}
					maskSpan += maskValue;
					ClassStrength& c = classArray[classIdx];
					c.strength = maskValue;
					c.treeClass = treeClass;
					classIdx++;
				}

				double dice = maskSpan*rand()/RAND_MAX;
				double sum = 0.0;
				TreeClass* chosen = NULL;			
				for (int i = 0; i < classIdx && sum < dice; i++)
				{
					ClassStrength& c = classArray[i];
					chosen = c.treeClass;
					sum += c.strength;
				}

				if (chosen != NULL)
				{
					CTreeInstance& t = instances[instanceIndex];
					t.treeClass = chosen;
					t.bday = time - min(timeSlice, (float)rand()*chosen->matureAge/RAND_MAX);
					t.x = x;
					t.z = z;
					t.dead = false;
					t.mature = false;

					instanceIndex++;
					gridP = instanceIndex;
				}

			}

		int currentCount = instanceIndex;
		for (int iTree = 0; iTree < currentCount; ++iTree)
		{
			CTreeInstance& tree = instances[iTree];

			if (tree.dead)
				continue;

			double age = time - tree.bday;
			tree.dead = age > tree.treeClass->maxAge;

			if (tree.dead)
				continue;

			double sizeFactor = 0.9;

			double growth = 1.0 - max(0.0, (tree.treeClass->matureAge - age)/tree.treeClass->matureAge);
			/*
			double minRx = sizeFactor*growth*tree.treeClass->xRadius.minV*tree.treeClass->radius.minV;
			double minRz = sizeFactor*growth*tree.treeClass->zRadius.minV*tree.treeClass->radius.minV;
			*/
			double minRx = 0.7*sizeFactor*growth*tree.treeClass->xRadius.getValue(tree.x, 0, tree.z)*tree.treeClass->radius.getValue(tree.x, 0, tree.z);
			double minRz = 0.7*sizeFactor*growth*tree.treeClass->zRadius.getValue(tree.x, 0, tree.z)*tree.treeClass->radius.getValue(tree.x, 0, tree.z);
			if (!tree.mature)
			{
				tree.mature = abs(growth - 1.0) < 0.00001 || lastIteration;
				for (double x = tree.x - minRx; x <= tree.x + minRx && !tree.dead; x += gridDelta)
					for (double z = tree.z - minRz; z <= tree.z + minRz && !tree.dead; z += gridDelta)
					{
						int xi = (int)x;
						int zi = (int)z;

						int gridX = (xi - xo)/gridDelta;
						int gridZ = (zi - zo)/gridDelta;

						if (gridX < 0 || gridX >= gridXSize ||
							gridZ < 0 || gridZ >= gridZSize)
							continue;

						int gridIndex = gridXSize*gridZ + gridX;
						int& gridP = grid[gridIndex];

						if (gridP > 0)
						{
							CTreeInstance& neighborTree = instances[gridP - 1];
							double neighborAge = time - neighborTree.bday;
							double neighborGrowth = 1.0 - max(0.0, (neighborTree.treeClass->matureAge - neighborAge)/neighborTree.treeClass->matureAge);
							double minNeighborRx = sizeFactor*neighborGrowth*neighborTree.treeClass->xRadius.getValue(neighborTree.x, 0, neighborTree.z)*neighborTree.treeClass->radius.getValue(neighborTree.x, 0, neighborTree.z);
							double minNeighborRz = sizeFactor*neighborGrowth*neighborTree.treeClass->zRadius.getValue(neighborTree.x, 0, neighborTree.z)*neighborTree.treeClass->radius.getValue(neighborTree.x, 0, neighborTree.z);
							double neighborSize = max(minNeighborRx, minNeighborRz);
							double size = max(minRx, minRz);
							tree.dead = 
								&neighborTree != &tree &&
								!neighborTree.dead && 
								(/*neighborTree.bday < tree.bday || */
								neighborSize > size ||
								time - neighborTree.bday > neighborTree.treeClass->matureAge);
						}

						if (!tree.dead)
							gridP = iTree + 1;
					}
			}
			if (!tree.dead && tree.mature)
			{
				double seedRx = tree.treeClass->seedRange;
				double seedRz = tree.treeClass->seedRange;

				for (double x = tree.x - seedRx; x <= tree.x + seedRx; x += gridDelta)
					for (double z = tree.z - seedRx; z <= tree.z + seedRx; z += gridDelta)
					{
						if (x >= tree.x - minRx && x <= tree.x + minRx &&
							z >= tree.z - minRz && z <= tree.z + minRz)
							continue;

						if (rand() > RAND_MAX/1000)
							continue;

						int xi = (int)x;
						int zi = (int)z;

						int gridX = (xi - xo)/gridDelta;
						int gridZ = (zi - zo)/gridDelta;

						if (gridX < 0 || gridX >= gridXSize ||
							gridZ < 0 || gridZ >= gridZSize)
							continue;

						int gridIndex = gridXSize*gridZ + gridX;
						int& gridP = grid[gridIndex];

						bool empty = false;
						if (gridP > 0)
						{
							CTreeInstance& neighborTree = instances[gridP - 1];
							empty = &neighborTree != &tree && neighborTree.dead;
						}
						else
							empty = true;

						if (empty)
						{
							double maskval = 1.0;
							for (map<string, DensityMap*>::iterator imap = tree.treeClass->masks.begin();
								imap != tree.treeClass->masks.end(); ++imap)
							{

								map<string, I2DMask*>::iterator imask = masks.find(imap->first);
								if (imask != masks.end())
								{
									DensityMap* dmap = imap->second;

									double value = imask->second->get2DMaskValue(x, z, dmap->blur);

									if (dmap->invert)
										value = 1.0 - value;
									if (value < dmap->minval || value > dmap->maxval)
										value = 0.0;
									else
									{
										double scope = abs(dmap->minval - dmap->maxval);
										if (scope > 0.000001)
										{
											value = (value - dmap->minval)/scope;
										}
										else
											value = 0.0;
									}

									maskval *= value;
								}
							}

							double dice = ((double)rand())/RAND_MAX;
							if (dice < maskval)
							{
								CTreeInstance& t = instances[instanceIndex];
								t.treeClass = tree.treeClass;
								t.bday = time - min(timeSlice, (float)rand()*tree.treeClass->matureAge/RAND_MAX);
								t.x = x;
								t.z = z;
								t.dead = false;
								t.mature = false;

								instanceIndex++;
								gridP = instanceIndex;
							}
						}
					}
			}
		}

	}
	trees.clear();
	for (int i = 0; i < instanceIndex; i++)
	{
		CTreeInstance& tree = instances[i];
		if (!tree.dead && tree.mature)
		{
			double maskval = 1.0;
			for (map<string, DensityMap*>::iterator imap = tree.treeClass->masks.begin();
				imap != tree.treeClass->masks.end(); ++imap)
			{
				map<string, I2DMask*>::iterator imask = masks.find(imap->first);

				if (imask != masks.end())
				{
					DensityMap* dmap = imap->second;
					if (dmap->useForThinning)
					{
						double value = imask->second->get2DMaskValue(tree.x, tree.z, dmap->blur);
						if (dmap->invert)
							value = 1.0 - value;
						if (value < dmap->minval || value > dmap->maxval)
							value = 0.0;
						else
						{
							double scope = abs(dmap->minval - dmap->maxval);
							if (scope > 0.000001)
							{
								value = (value - dmap->minval)/scope;
							}
							else
								value = 0.0;
						}
						maskval *= value;
					}
				}
			}

			// global masks
			for (map<string, DensityMap*>::iterator imap = globalMasks.begin();
				imap != globalMasks.end(); ++imap)
			{
				map<string, I2DMask*>::iterator imask = masks.find(imap->first);

				if (imask != masks.end())
				{
					DensityMap* dmap = imap->second;
					double value = imask->second->get2DMaskValue(tree.x, tree.z, dmap->blur);
					if (dmap->invert)
						value = 1.0 - value;
					if (value < dmap->minval || value > dmap->maxval)
						value = 0.0;
					else
					{
						double scope = abs(dmap->minval - dmap->maxval);
						if (scope > 0.000001)
						{
							value = (value - dmap->minval)/scope;
						}
						else
							value = 0.0;
					}
					maskval *= value;
				}
			}

			double dice = ((double)rand())/RAND_MAX;
			if (dice <= maskval)
				trees.push_back(tree);
		}
	}

	delete instances;
	free(grid);

}

TreeOutput CForest::GetTreeOutputFromInstance(const CTreeInstance& instance)
{
	TreeOutput output;
	output.x = instance.x;
	output.y = instance.z;

	int rgbColor = instance.treeClass->color;
	int red = (rgbColor >> 16) & 0xFF;
	int green = (rgbColor >> 8) & 0xFF;
	int blue = rgbColor & 0xFF;
	
	output.red = red;
	output.green = green;
	output.blue = blue;

	return output;
}

bool CForest::exportToCSV(const std::vector<TreeOutput>& data, const std::string& filename) {
	std::ofstream outputFile(filename);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open the file " << filename << std::endl;
		return false;
	}

	// Write header row
	outputFile << "X,Y,Z,Red,Green,Yellow" << std::endl;

	// Write data rows
	for (const TreeOutput& tree : data) {
		outputFile << tree.x << ","
			<< tree.y << ","
			<< tree.z << ","
			<< tree.red << ","
			<< tree.green << ","
			<< tree.blue << std::endl;
	}

	outputFile.close();

	return true;
}

bool CForest::outputResults(const std::string& csvFileName)
{
	outputs.clear();
	for (const CTreeInstance& instance : trees)
	{
		TreeOutput output = GetTreeOutputFromInstance(instance);
		outputs.push_back(output);
	}

	bool exportCSV = exportToCSV(outputs, csvFileName);
	return true;
}

pair<string, I2DMask*> GetI2DMaskKeyPairFromPlantTypeWithIndex(PlantType type, int index, I2DMask* pI2dMask)
{
	string keyString = PlantTypeToMaskString(type, index);
	pair<string, I2DMask*> ret(keyString, pI2dMask);
	return ret;
}