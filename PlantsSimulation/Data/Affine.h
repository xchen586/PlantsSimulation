/************************************************************
* (C) Voxel Farm Inc. 2023
*/

#pragma once

#include <VoxelLayer.h>
#include <mapindex.h>

class CAffineTransform
{
public:
	enum eTransformMode : char
	{
		TM_YZ_SWAP,
		TM_YZ_ROTATE
	};

	struct sAffineVector
	{
		double X, Y, Z;
	};

public:
	CAffineTransform(
		const sAffineVector& WC_CENT,
		const double VF_SCALE,
		const eTransformMode MODE
	)
	{
		// compute the scale factor
		static const double vfIntrinsicVoxelSize = ((1 << VoxelFarm::LOD_0) * VoxelFarm::CELL_SIZE) / VoxelFarm::BLOCK_DIMENSION;
		S = VF_SCALE / vfIntrinsicVoxelSize;
		iS = 1.0 / S;

		// compute the R3 term
		switch (MODE)
		{
			case eTransformMode::TM_YZ_SWAP:	R3 = +1.0;	break;
			case eTransformMode::TM_YZ_ROTATE:	R3 = -1.0;	break;
			default:							R3 = +1.0;	break;
		};

		// compute the intrinsic voxel origin
		// (no need to half -- lod0 shift takes care of that)
		static const double vfIntrinsicOriginX = (1 << VoxelFarm::CELL_BITS_X) * VoxelFarm::CELL_SIZE;
		static const double vfIntrinsicOriginY = (1 << VoxelFarm::CELL_BITS_Y) * VoxelFarm::CELL_SIZE;
		static const double vfIntrinsicOriginZ = (1 << VoxelFarm::CELL_BITS_Z) * VoxelFarm::CELL_SIZE;

		// set the T contributions
		// vf
		TVF[0] = vfIntrinsicOriginX;
		TVF[1] = vfIntrinsicOriginY;
		TVF[2] = vfIntrinsicOriginZ;
		// world
		TWC[0] = WC_CENT.X;
		TWC[1] = WC_CENT.Y;
		TWC[2] = WC_CENT.Z;
	};

	~CAffineTransform() {};

	inline sAffineVector VF_TO_WC(const sAffineVector& VFC) const
	{
		return sAffineVector
		{
			((VFC.X - TVF[0]) * S + TWC[0]),
			((VFC.Z - TVF[2]) * R3 * S + TWC[1]),
			((VFC.Y - TVF[1]) * S + TWC[2])
		};
	};

	inline sAffineVector WC_TO_VF(const sAffineVector& WCC) const
	{
		return sAffineVector
		{
			((WCC.X - TWC[0]) * iS + TVF[0]),
			((WCC.Z - TWC[2]) * iS + TVF[1]),
			((WCC.Y - TWC[1]) * R3 * iS + TVF[2])
		};
	};

private:
	double S;
	double iS;
	double R3;
	double TVF[3];
	double TWC[3];
};