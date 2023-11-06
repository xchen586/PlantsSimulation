
#define VOXELFARM_CROSSPLATFORM
#include "VoxelFarmPlatform.h"

namespace VoxelFarm
{
    const float64_t CELL_SIZE = 30.0;
    const int32_t LOD_0 = 2;
    const int32_t LEVELS = 14;
    const int32_t BLOCK_DIMENSION = 40;
    const int32_t BLOCK_DIMENSION_3D = BLOCK_DIMENSION * BLOCK_DIMENSION * BLOCK_DIMENSION;
    const int32_t BLOCK_MARGIN = 2;
    //const int32_t CELL_BITS_X = 24;
    //const int32_t CELL_BITS_Y = 12;
    //const int32_t CELL_BITS_Z = 24;

    const uint8_t MSG_REQ_SCENE = 110;
    const uint8_t MSG_REQ_ORG = 115;
    const uint8_t MSG_REQ_CONVERT_W2VF = 116;
    const uint8_t MSG_WORK_COMPLETE = 150;
    const uint8_t MSG_SEND_MESH_STD = 120;
    const uint8_t MSG_SEND_MESH_PT = 122;
    const uint8_t MSG_SEND_VOXELS = 123;
    const uint8_t MSG_SEND_TEXTURE = 131;
    const uint8_t MSG_SEND_ORG = 102;
    const uint8_t MSG_SEND_CONVERT_W2VF = 103;
    const uint8_t MSG_INVALID_RECV_BUFFER = 222;

    const uint8_t MSG_ECHO = 250;

    const uint8_t SCENE_FORMAT_TRIMESH = 0;
    const uint8_t SCENE_FORMAT_POINTCLOUD = 1;

    const uint8_t MSG_OFFS_MSGID = 0;
    const uint8_t MSG_OFFS_VIEWID = 1;
    const uint8_t MSG_OFFS_REQID = 3;
    const uint8_t MSG_OFFS_CLUSTERID = 5;
    const uint8_t MSG_OFFS_CELLID = 7;
    const uint8_t MSG_OFFS_LAYER = 15;
    const uint8_t MSG_OFFS_SUBMESH = 16;
    const uint8_t MSG_OFFS_PAYLOAD = 17;
    const uint8_t MSG_OFFS_PAYLOAD_PT = 16;
    const uint8_t MSG_OFFS_PAYLOAD_VOXELS = 16;

    const uint8_t MSG_OFFS_ORG_FOUND = 1;
    const uint8_t MSG_OFFS_ORG_X = 2;
    const uint8_t MSG_OFFS_ORG_Y = (uint8_t)(MSG_OFFS_ORG_X + 8);
    const uint8_t MSG_OFFS_ORG_Z = (uint8_t)(MSG_OFFS_ORG_Y + 8);

    const uint8_t MSG_OFFS_CONV_REQID = 1;
    const uint8_t MSG_OFFS_CONV_POINTCOUNT = 3;

    const uint8_t MSG_OFFS_TEXTURE_TYPE = 16;
    const uint8_t MSG_OFFS_TEXTURE_FORMAT = 17;
    const uint8_t MSG_OFFS_TEXTURE_SIZE = 18;
    const uint8_t MSG_OFFS_TEXTURE_PAYLOAD = 20;

    const uint8_t MAX_THREADS = 4;
    const uint8_t THREAD_MESSAGE_START = 1;
    const uint8_t THREAD_MESSAGE_STOP = 2;
    const uint8_t THREAD_MESSAGE_COMPUTESCENE = 3;
    const uint8_t THREAD_MESSAGE_NEWSCENE = 4;
    const uint8_t THREAD_MESSAGE_DECOMPRESS = 5;
    const uint8_t THREAD_MESSAGE_DECOMPRESS_RESULT = 6;
    const uint8_t THREAD_MESSAGE_DECOMPRESS_TEXTURE_ZLIB = 7;
    const uint8_t THREAD_MESSAGE_DECOMPRESS_TEXTURE_ZLIB_RESULT = 8;

    const uint32_t TIMEOUT_INFINITE = 0xFFFFFFFF;
}

// a performance-optimized (~30% improvement) of the pack/unpack cellid system
//#define USE_OPTIMIZED_CELLID

namespace VoxelFarm
{
	/// Bits to encode octree level in Cell Id
	static const int CELL_BITS_LEVEL	= 4;
	/// Bits to encode octree X coordinate in Cell Id
	static const int CELL_BITS_X		= 24;
	/// Bits to encode octree Y coordinate in Cell Id
	static const int CELL_BITS_Y		= 12;
	/// Bits to encode octree Z coordinate in Cell Id
	static const int CELL_BITS_Z		= 24;
	/// Maximum Values for the above
	static const int CELL_MAX_LEVEL		= (1 << CELL_BITS_LEVEL);
	static const int CELL_MAX_X			= (1 << CELL_BITS_X);
	static const int CELL_MAX_Y			= (1 << CELL_BITS_Y);
	static const int CELL_MAX_Z			= (1 << CELL_BITS_Z);

#ifdef USE_OPTIMIZED_CELLID
	/// A 64bit integer that identifies a single world octree Cell
	struct CellId
	{
		unsigned __int64 z		: 24;
		unsigned __int64 y		: 12;
		unsigned __int64 x		: 24;
		unsigned __int64 level	: 4;

		CellId() {};
		CellId(const unsigned __int64 c) { *this = c; };
		~CellId() {};

		template <typename T>
		inline bool operator<(const T& c) const
		{
			return (unsigned __int64)(*this) < static_cast<unsigned __int64>(c);
		}
		template <typename T>
		inline bool operator==(const T& c) const
		{
			return (unsigned __int64)(*this) == static_cast<unsigned __int64>(c);
		}
		inline void operator=(const CellId& c)
		{
			memcpy(this, &c, sizeof(CellId));
		}
		inline void operator=(const unsigned __int64 c)
		{
			memcpy(this, &c, sizeof(CellId));
		}
		inline operator unsigned __int64() const
		{
			return *(unsigned __int64*)this;
		}
	};
};

/// Override std::hash for this struct (to allow std::unordered_map, std::pair, etc.)
namespace std
{
	template <> struct hash<VoxelFarm::CellId>
	{
		size_t operator()(const VoxelFarm::CellId& c) const
		{
			return std::hash<unsigned __int64>()((unsigned __int64)c);
		};
	};
};

namespace VoxelFarm
{
/// Compose a CellId from the location of a Cell in the octree
inline CellId packCellId(int level, int x, int y, int z)
{
    CellId id;
    id.level = (unsigned __int64)level;
    id.x = (unsigned __int64)x;
    id.y = (unsigned __int64)y;
    id.z = (unsigned __int64)z;
    return id;
}

/// Obtain location of a Cell in the octree from its CellId identifier
inline void unpackCellId(CellId id, int &level, int &x, int &y, int &z)
{
    level = (int)id.level;
    x = (int)id.x;
    y = (int)id.y;
    z = (int)id.z;
}
#else
/// A 64bit integer that identifies a single world octree Cell
typedef unsigned long long CellId;

/// Compose a CellId from the location of a Cell in the octree
inline CellId packCellId(int level, int x, int y, int z)
{
    CellId id = 0;
    id |= (CellId)level << (CELL_BITS_X + CELL_BITS_Y + CELL_BITS_Z);
    id |= (CellId)x << (CELL_BITS_Y + CELL_BITS_Z);
    id |= (CellId)y << (CELL_BITS_Z);
    id |= z;
    return id;
}

/// Obtain location of a Cell in the octree from its CellId identifier
inline void unpackCellId(CellId id, int &level, int &x, int &y, int &z)
{
    level = (int)(id >> (CELL_BITS_X + CELL_BITS_Y + CELL_BITS_Z));
    id &= ~((CellId)level << (CELL_BITS_X + CELL_BITS_Y + CELL_BITS_Z));
    
    x = (int)(id >> (CELL_BITS_Y + CELL_BITS_Z));
    id &= ~((CellId)x << (CELL_BITS_Y + CELL_BITS_Z));
    
    y = (int)(id >> CELL_BITS_Z);
    id &= ~((CellId)y << CELL_BITS_Z);
    
    z = (int)id;
}
#endif
}

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

#ifdef __APPLE__
        sAffineVector(double x, double y, double z)
        : X(x)
        , Y(y)
        , Z(z)
        {
            
        }
#endif
	};

public:
	CAffineTransform(
		const sAffineVector& WC_CENT,
		const double VF_SCALE,
		const eTransformMode MODE
	)
	{
		// compute the scale factor
		static const double vfIntrinsicVoxelSize = ((1 << VoxelFarm::LOD_0)*VoxelFarm::CELL_SIZE) / VoxelFarm::BLOCK_DIMENSION;
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
		static const double vfIntrinsicOriginX = (1 << VoxelFarm::CELL_BITS_X)*VoxelFarm::CELL_SIZE;
		static const double vfIntrinsicOriginY = (1 << VoxelFarm::CELL_BITS_Y)*VoxelFarm::CELL_SIZE;
		static const double vfIntrinsicOriginZ = (1 << VoxelFarm::CELL_BITS_Z)*VoxelFarm::CELL_SIZE;

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
			((VFC.X - TVF[0])*S + TWC[0]),
			((VFC.Z - TVF[2])*R3*S + TWC[1]),
			((VFC.Y - TVF[1])*S + TWC[2])
		};
	};

	inline sAffineVector WC_TO_VF(const sAffineVector& WCC) const
	{
		return sAffineVector
		{
			((WCC.X - TWC[0])*iS + TVF[0]),
			((WCC.Z - TWC[2])*iS + TVF[1]),
			((WCC.Y - TWC[1])*R3*iS + TVF[2])
		};
	};

private:
	double S;
	double iS;
	double R3;
	double TVF[3];
	double TWC[3];
};
