/**
 * @File Defines.hpp
 * @Author dfnzhc (https://github.com/dfnzhc)
 * @Date 2024/5/26
 * @Brief 
 */

#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#  define ELMA_IN_WINDOWS
#elif defined(__unix__) || defined(__unix) || defined(__linux__)
#  define ELMA_IN_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#  define ELMA_IN_MAC
#else
#  error Unsupport Platform
#endif

#if defined(_MSC_VER)
#  define ELMA_CPLUSPLUS _MSVC_LANG
#else
#  define ELMA_CPLUSPLUS __cplusplus
#endif

static_assert(ELMA_CPLUSPLUS >= 202'002L, "__cplusplus >= 202002L: C++20 at least");

// noinline
#ifdef _MSC_VER
#  define ELMA_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#  define ELMA_NOINLINE __attribute__((__noinline__))
#else
#  define ELMA_NOINLINE
#endif

// always inline
#ifdef _MSC_VER
#  define ELMA_ALWAYS_INLINE __forceinline
#elif defined(__GNUC__)
#  define ELMA_ALWAYS_INLINE inline __attribute__((__always_inline__))
#else
#  define ELMA_ALWAYS_INLINE inline
#endif

#define ELMA_NODISCARD       [[nodiscard]]
#define ELMA_DEPRECATED(...) [[deprecated(__VA_ARGS__)]]

// Debug & Release
namespace elma {
#ifdef NDEBUG
constexpr auto kIsDebug   = false;
constexpr auto kIsRelease = true;
#  define ELMA_RELEASE_BUILD
#else
constexpr auto kIsDebug   = true;
constexpr auto kIsRelease = false;
#  define ELMA_DEBUG_BUILD
#  define ELMA_ENABLE_ASSERTS
#endif
} // namespace elma

#include <cstdint>
#include <cstddef>

// -------------------------
// 关于 Cuda、Optix 的宏定义

#if defined(__CUDA_ARCH__) || defined(__CUDACC__)
#  ifndef ELMA_NOINLINE
#    define ELMA_NOINLINE __attribute__((noinline))
#  endif
#  ifndef ELMA_GPU_CODE
#    define ELMA_GPU_CODE
#  endif
#  define ELMA_GPU               __device__
#  define ELMA_CPU               __host__
#  define ELMA_INLINE            __forceinline__
#  define ELMA_CONST             __device__ const
#  define CONST_STATIC_INIT(...) /* ignore */
#else
#  ifndef ELMA_HOST_CODE
#    define ELMA_HOST_CODE
#  endif
#  define ELMA_GPU               /* ignore */
#  define ELMA_CPU               /* ignore */
#  define ELMA_CONST             const
#  define ELMA_INLINE            inline
#  define CONST_STATIC_INIT(...) = __VA_ARGS__
#endif

#define ELMA_CPU_GPU ELMA_CPU ELMA_GPU
#define ELMA_FUNC    ELMA_INLINE
#define ELMA_FUNC_DECL

namespace elma {
// -------------------------
// 内置类型、常量别名

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8   = uint8_t;
using u16  = uint16_t;
using u32  = uint32_t;
using u64  = uint64_t;
using uint = unsigned int;

using f32 = float;
using f64 = double;

using size = std::size_t;

static constexpr i8 i8_min   = INT8_MIN;
static constexpr i16 i16_min = INT16_MIN;
static constexpr i32 i32_min = INT32_MIN;
static constexpr i64 i64_min = INT64_MIN;

static constexpr i8 i8_max   = INT8_MAX;
static constexpr i16 i16_max = INT16_MAX;
static constexpr i32 i32_max = INT32_MAX;
static constexpr i64 i64_max = INT64_MAX;

static constexpr u8 u8_max   = UINT8_MAX;
static constexpr u16 u16_max = UINT16_MAX;
static constexpr u32 u32_max = UINT32_MAX;
static constexpr u64 u64_max = UINT64_MAX;

#ifdef ELMA_FLOAT_AS_DOUBLE
using Float     = f64;
using FloatBits = u64;
#else
using Float     = f32;
using FloatBits = u32;
#endif

static_assert(sizeof(Float) == sizeof(FloatBits));

template<typename T, typename F>
    requires std::is_arithmetic_v<T> and std::is_arithmetic_v<F> and std::is_nothrow_convertible_v<F, T>
ELMA_FUNC constexpr T cast_to(F f) noexcept
{
    return static_cast<T>(f);
}

} // namespace elma

#define GLM_FORCE_RADIANS
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace elma {

using float1 = glm::vec<1, float>;
using float2 = glm::vec<2, float>;
using float3 = glm::vec<3, float>;
using float4 = glm::vec<4, float>;

using real1 = glm::vec<1, double>;
using real2 = glm::vec<2, double>;
using real3 = glm::vec<3, double>;
using real4 = glm::vec<4, double>;

using int1 = glm::vec<1, i32>;
using int2 = glm::vec<2, i32>;
using int3 = glm::vec<3, i32>;
using int4 = glm::vec<4, i32>;

using uint1 = glm::vec<1, uint>;
using uint2 = glm::vec<2, uint>;
using uint3 = glm::vec<3, uint>;
using uint4 = glm::vec<4, uint>;

} // namespace elma