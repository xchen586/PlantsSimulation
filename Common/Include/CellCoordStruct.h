#pragma once

#include "PsHelper.h"

struct CellCoordStruct
{

	int cellXIdx;
	int cellYIdx;
	int cellZIdx;
	double posX;
	double posY;
	double posZ;
	double vX;
	double vY;
	double vZ;
	double xOffsetW;
	double yOffsetW;
	double zOffsetW;

	int lod;
	VoxelFarm::CellId cellId;

    bool isInit;

	CellCoordStruct()
		: lod(0)
		, cellXIdx(0)
		, cellYIdx(0)
		, cellZIdx(0)
		, posX(0.0)
		, posY(0.0)
		, posZ(0.0)
		, vX(0.0)
		, vY(0.0)
		, vZ(0.0)
		, xOffsetW(0.0)
		, yOffsetW(0.0)
		, zOffsetW(0.0)
		, cellId(0)
        , isInit(false)
	{

	}

    bool SetupCellCoordinate(double posX, double posY, double posZ, const CAffineTransform& transform, int32_t lod)
    {
        this->isInit = false;

        const double cellScale = (1 << lod) * VoxelFarm::CELL_SIZE;

        const auto vfPosition = transform.WC_TO_VF(CAffineTransform::sAffineVector(posX, posY, posZ));

        //cell min
        int cellX = (int)(vfPosition.X / cellScale);
        int cellY = (int)(vfPosition.Y / cellScale);
        int cellZ = (int)(vfPosition.Z / cellScale);

        //cell max
        int cellX1 = cellX + 1;
        int cellY1 = cellY + 1;
        int cellZ1 = cellZ + 1;

        //vf point 0
        double vfPointX = (cellX * cellScale);
        double vfPointY = (cellY * cellScale);
        double vfPointZ = (cellZ * cellScale);

        //vf point 1
        double vfPoint1X = (cellX1 * cellScale);
        double vfPoint1Y = (cellY1 * cellScale);
        double vfPoint1Z = (cellZ1 * cellScale);

        //vf bounds size
        double vfBoundsSizeX = std::abs(vfPoint1X - vfPointX);
        double vfBoundsSizeY = std::abs(vfPoint1Y - vfPointY);
        double vfBoundsSizeZ = std::abs(vfPoint1Z - vfPointZ);

        //vf min
        double vfMinX = min(vfPointX, vfPoint1X);
        double vfMinY = min(vfPointY, vfPoint1Y);
        double vfMinZ = min(vfPointZ, vfPoint1Z);

        //vf max
        double vfMaxX = max(vfPointX, vfPoint1X);
        double vfMaxY = max(vfPointY, vfPoint1Y);
        double vfMaxZ = max(vfPointZ, vfPoint1Z);

        //world point 0
        auto worldPoint0 = transform.VF_TO_WC(CAffineTransform::sAffineVector{ vfMinX, vfMinY, vfMinZ });

        //world point 1
        auto worldPoint1 = transform.VF_TO_WC(CAffineTransform::sAffineVector{ vfMaxX, vfMaxY, vfMaxZ });

        //world min
        double worldMinX = min(worldPoint0.X, worldPoint1.X);
        double worldMinY = min(worldPoint0.Y, worldPoint1.Y);
        double worldMinZ = min(worldPoint0.Z, worldPoint1.Z);

        //world max
        double worldMaxX = max(worldPoint0.X, worldPoint1.X);
        double worldMaxY = max(worldPoint0.Y, worldPoint1.Y);
        double worldMaxZ = max(worldPoint0.Z, worldPoint1.Z);

        //world bounds size
        double worldBoundsSizeX = std::abs(worldMaxX - worldMinX);
        double worldBoundsSizeY = std::abs(worldMaxY - worldMinY);
        double worldBoundsSizeZ = std::abs(worldMaxZ - worldMinZ);

        //world offset
        double worldOffsetX = (posX - worldMinX);
        double worldOffsetY = (posY - worldMinY);
        double worldOffsetZ = (posZ - worldMinZ);

        if ((posX < worldMinX) ||
            (posY < worldMinY) ||
            (posZ < worldMinZ) ||
            (posX > worldMaxX) ||
            (posY > worldMaxY) ||
            (posZ > worldMaxZ))
        {
            std::cout << "pos is not in the cell" << std::endl;
        }

        if (worldOffsetX < 0 ||
            worldOffsetY < 0 ||
            worldOffsetZ < 0)
        {
            std::cout << "offset is not in the cell" << std::endl;
        }

        //cellY = 0; //Because I only 2D cellX and CellZ;

        this->lod = lod;
        this->cellXIdx = cellX;
        this->cellYIdx = cellY;
        this->cellZIdx = cellZ;

        this->xOffsetW = worldOffsetX;
        this->yOffsetW = worldOffsetY;
        this->zOffsetW = worldOffsetZ;

        this->posX = posX;
        this->posY = posY;
        this->posZ = posZ;

        this->vX = vfPosition.X;
        this->vY = vfPosition.Y;
        this->vZ = vfPosition.Z;

        this->cellId = VoxelFarm::packCellId(lod, cellX, cellY, cellZ);

        this->isInit = true;

        return this->isInit;
    }
};