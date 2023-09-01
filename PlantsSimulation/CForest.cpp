#include "CForest.h"

CForest::CForest(void)
{
	grid = NULL;
}

CForest::~CForest(void)
{
	for (map<string, DensityMap*>::iterator i = globalMasks.begin(); i != globalMasks.end(); ++i)
		delete i->second;
}

struct ClassStrength
{
	double strength;
	TreeClass* treeClass;
};

#define SEED_MAX 4*1024*1024

void CForest::generate(float forestAge, int iterations)
{
	// allocate grid
	int gridDelta = 40;
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


