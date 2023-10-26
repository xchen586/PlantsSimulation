// Copyright 2019 VoxelFarm. All Rights Reserved.

// This library uses the default STD containers and strings
#pragma once

#ifdef __APPLE__
#define VOXELFARM_PLATFORM_APPLE
#elif __linux__
#define VOXELFARM_PLATFORM_LINUX
#elif _WIN32
#define VOXELFARM_PLATFORM_WINDOWS
#endif

#if __cplusplus == 201103L
#define STL_LANG_CPP11
#endif
#if __cplusplus == 201402L
#define STL_LANG_CPP14
#endif
#if __cplusplus == 201703L
#define STL_LANG_CPP17
#endif
#if __cplusplus == 202002L
#define STL_LANG_CPP20
#endif
#if __cplusplus <= 202002L
#define STL_LANG_CPP20_OR_OLDER
#endif
#if __cplusplus > 202002L
#define STL_LANG_CPP23_OR_NEWER
#endif

#if defined(VOXELFARM_CROSSPLATFORM)
    #include <map>
    #include <algorithm>
    #include <utility>
    #include <set>

    #include <set>
    #include <map>
    #include <unordered_map>
    #include <array>
    #include <vector>
    #include <list>
    #include <string>
    #include <sstream>
    #include <queue>
    #include <deque>

    #include <functional>
    #include <chrono>

    #include <exception>

    #include <cstdlib>
    #include <limits>

    #include <charconv>

    #define VF_NEW new
    #define VF_DELETE delete

    #define VOXELFARM_USE_DEFAULT_ALLOC    1
    #define VF_RAWALLOC(bytes) (VoxelFarm::MallocDefault(bytes))
    #define VF_RAWCALLOC(num, bytes) (VoxelFarm::CallocDefault(num, bytes))
    #define VF_RAWREALLOC(ptr, bytes) (VoxelFarm::ReallocDefault(ptr, bytes))
    #define VF_ALLOC(typ, num) ((typ *)VF_RAWALLOC((num) * sizeof(typ)))
    #define VF_FREE(ptr) (free((ptr)))

    #define VF_RAWALLOC_ALIGNED(bytes) (VoxelFarm::MallocAlignedDefault(bytes))
    #define VF_ALLOC_ALIGNED(typ, num) ((typ*)VF_RAWALLOC_ALIGNED((num)*sizeof(typ)))
    #define VF_FREE_ALIGNED(ptr) (VoxelFarm::FreeAlignedDefault(ptr))

    #define VF_UNUSED(var) (void)var;

    namespace VoxelFarm
    {
        typedef signed char int8_t;
        typedef unsigned char uint8_t;

        typedef signed short int16_t;
        typedef unsigned short uint16_t;

        typedef signed int int32_t;
        typedef unsigned int uint32_t;

        typedef signed long long int64_t;
        typedef unsigned long long uint64_t;

        typedef float float32_t;
        typedef double float64_t;
        typedef size_t size_t;
        
        template<typename T>
        constexpr auto MinValue = std::numeric_limits<T>::min();
    
        template<typename T>
        constexpr auto MaxValue = std::numeric_limits<T>::max();
    
        template<typename T, size_t N>
        using TArray = std::array<T, N>;
        
        template<typename T>
        using TVector = std::vector<T>;
       
        template<typename Ty1, typename Ty2>
        using TPair = std::pair<Ty1, Ty2>;

        template <typename K, typename T>
        using TMap = std::unordered_map<K, T>;
       
        using String = std::string;
    
        using StringArray = TVector<String>;
    
        using StringStream = std::stringstream;

        template<typename T>
        using TSet = std::set<T>;

        template<typename T>
        using TQueue = std::queue<T>;

        using TCallBack = std::function<void()>;

        using hi_res_clock = std::chrono::high_resolution_clock;
        using timestamp = hi_res_clock::time_point;
        using seconds = std::chrono::seconds;
        using microseconds = std::chrono::microseconds;
        using milliseconds = std::chrono::milliseconds;
        inline size_t delta_sec(const timestamp& t1, const timestamp& t2)
        {
            return std::chrono::duration_cast<seconds>(t2 - t1).count();
        }
        inline size_t delta_micro(const timestamp& t1, const timestamp& t2)
        {
            return std::chrono::duration_cast<microseconds>(t2 - t1).count();
        }
        inline size_t delta_ms(const timestamp& t1, const timestamp& t2)
        {
            return std::chrono::duration_cast<milliseconds>(t2 - t1).count();
        }
    
        using Exception = std::exception;
    
        template<typename ResultType>
        static bool TryParse(const String& str, ResultType& result)
        {
            //C++17 or C++20 for integral types
            //C++23 for floating-points
            //Visual Studio May Allow Some C++23 Features Even If Using ISO C++20
            //XCode 15 Supports Partially C++23
            
            auto [ptr, er] = std::from_chars(str.data(), str.data() + str.size(), result);
            if (er == std::errc())
            {
                return true;
            }
            else if (er == std::errc::invalid_argument)
            {
                return false;
            }
            else if (er == std::errc::result_out_of_range)
            {
                return false;
            }
            
            return false;
        }
    
        //Currently XCode 15 (Apple) Supports Partially C++23
        //It is missing from_chars floating-point support..
#if defined(STL_LANG_CPP20_OR_OLDER) || defined(VOXELFARM_PLATFORM_APPLE)
        template<>
        bool TryParse<float32_t>(const String& str, float32_t& result)
        {
            try
            {
                result = std::stof(str);
                return true;
            }
            catch (const std::invalid_argument& ia)
            {
                VF_UNUSED(ia)
                return false;
            }
            catch (const std::out_of_range& oor)
            {
                VF_UNUSED(oor)
                return false;
            }
            
            return false;
        }
    
        template<>
        bool TryParse<float64_t>(const String& str, float64_t& result)
        {
            try
            {
                result = std::stod(str);
                return true;
            }
            catch (const std::invalid_argument& ia)
            {
                VF_UNUSED(ia)
                return false;
            }
            catch (const std::out_of_range& oor)
            {
                VF_UNUSED(oor)
                return false;
            }
            
            return false;
        }
#endif
    
        static inline void* MemCCpy(void* /*restrict*/ dest, const void* /*restrict*/ src, int c, size_t count)
        {
#ifdef _MSC_VER
            //For MSVC
            void* result = _memccpy(dest, src, c, count);
#else
            //C++23
            void* result = memccpy(dest, src, c, count);
#endif
            return result;
        }
    
        static inline void* MallocDefault(size_t nBytes)
        {
            void* result = std::malloc(nBytes);
            return result;
        }

        static inline void* CallocDefault(size_t nObjects, size_t nSize)
        {
            void* result = std::calloc(nObjects, nSize);
            return result;
        }

        static inline void* ReallocDefault(void* ptr, size_t nBytes)
        {
            void* result = std::realloc(ptr, nBytes);
            return result;
        }

        static inline void* MallocAlignedDefault(size_t nBytes)
        {
            const size_t alignment = 16;
            
#ifdef _MSC_VER
            //For MSVC
            void* result = _aligned_malloc(nBytes, alignment);
#else
            //C++17
            void* result = std::aligned_alloc(alignment, nBytes);
#endif
            return result;
        }

        static inline void FreeAlignedDefault(void* ptr)
        {
#ifdef _MSC_VER
            //For MSVC
            _aligned_free(ptr);
#else
            std::free(ptr);
#endif
        }
    }

#elif defined(UNIGINE) || defined(UNIGINE_DOUBLE)
	#include "..\..\Platforms\Unigine\include\VoxelFarmUniginePlatform.h"
#else
	#include "..\..\Platforms\Win32\include\VoxelFarmWin32Platform.h"
#endif