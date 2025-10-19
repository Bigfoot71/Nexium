/* NX_Math.h -- API declaration for Nexium's math module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_MATH_H
#define NX_MATH_H

#include "./NX_BitUtils.h"
#include "./NX_Platform.h"
#include "./NX_Macros.h"

#include <math.h>

/* === Constants === */

#define NX_PI   3.1415926535897931f
#define NX_TAU  6.2831853071795862f

#define NX_DEG2RAD (NX_PI / 180.0)
#define NX_RAD2DEG (180.0 / NX_PI)

#define NX_IVEC2_ZERO       NX_IVEC2( 0,  0)
#define NX_IVEC2_ONE        NX_IVEC2( 1,  1)
#define NX_IVEC3_ZERO       NX_IVEC3( 0,  0,  0)
#define NX_IVEC3_ONE        NX_IVEC3( 1,  1,  1)
#define NX_IVEC4_ZERO       NX_IVEC4( 0, 0, 0, 0)
#define NX_IVEC4_ONE        NX_IVEC4( 1, 1, 1, 1)

#define NX_VEC2_ZERO        NX_VEC2( 0,  0)
#define NX_VEC2_ONE         NX_VEC2( 1,  1)
#define NX_VEC2_UP          NX_VEC2( 0,  1)
#define NX_VEC2_DOWN        NX_VEC2( 0, -1)
#define NX_VEC2_LEFT        NX_VEC2(-1,  0)
#define NX_VEC2_RIGHT       NX_VEC2( 1,  0)

#define NX_VEC3_ZERO        NX_VEC3( 0,  0,  0)
#define NX_VEC3_ONE         NX_VEC3( 1,  1,  1)
#define NX_VEC3_UP          NX_VEC3( 0,  1,  0)
#define NX_VEC3_DOWN        NX_VEC3( 0, -1,  0)
#define NX_VEC3_LEFT        NX_VEC3(-1,  0,  0)
#define NX_VEC3_RIGHT       NX_VEC3( 1,  0,  0)
#define NX_VEC3_FORWARD     NX_VEC3( 0,  0, -1)
#define NX_VEC3_BACK        NX_VEC3( 0,  0,  1)

#define NX_VEC4_IDENTITY    NX_VEC4( 0, 0, 0, 1)
#define NX_VEC4_ZERO        NX_VEC4( 0, 0, 0, 0)
#define NX_VEC4_ONE         NX_VEC4( 1, 1, 1, 1)

#define NX_BLANK            NX_COLOR(0.00f, 0.00f, 0.00f, 0.00f)
#define NX_WHITE            NX_COLOR(1.00f, 1.00f, 1.00f, 1.00f)
#define NX_BLACK            NX_COLOR(0.00f, 0.00f, 0.00f, 1.00f)
#define NX_GRAY             NX_COLOR(0.50f, 0.50f, 0.50f, 1.00f)
#define NX_LIGHT_GRAY       NX_COLOR(0.75f, 0.75f, 0.75f, 1.00f)
#define NX_DARK_GRAY        NX_COLOR(0.25f, 0.25f, 0.25f, 1.00f)
#define NX_RED              NX_COLOR(1.00f, 0.00f, 0.00f, 1.00f)
#define NX_GREEN            NX_COLOR(0.00f, 1.00f, 0.00f, 1.00f)
#define NX_BLUE             NX_COLOR(0.00f, 0.00f, 1.00f, 1.00f)
#define NX_YELLOW           NX_COLOR(1.00f, 1.00f, 0.00f, 1.00f)
#define NX_CYAN             NX_COLOR(0.00f, 1.00f, 1.00f, 1.00f)
#define NX_MAGENTA          NX_COLOR(1.00f, 0.00f, 1.00f, 1.00f)
#define NX_ORANGE           NX_COLOR(1.00f, 0.65f, 0.00f, 1.00f)
#define NX_BROWN            NX_COLOR(0.65f, 0.16f, 0.16f, 1.00f)
#define NX_PURPLE           NX_COLOR(0.50f, 0.00f, 0.50f, 1.00f)
#define NX_PINK             NX_COLOR(1.00f, 0.75f, 0.80f, 1.00f)
#define NX_GOLD             NX_COLOR(0.83f, 0.69f, 0.22f, 1.00f)
#define NX_SILVER           NX_COLOR(0.77f, 0.77f, 0.77f, 1.00f)
#define NX_COPPER           NX_COLOR(0.78f, 0.51f, 0.27f, 1.00f)

#define NX_QUAT_IDENTITY    NX_QUAT( 1, 0, 0, 0)

#define NX_MAT3_IDENTITY        \
    NX_MAT3_T {                 \
        1.0f, 0.0f, 0.0f,       \
        0.0f, 1.0f, 0.0f,       \
        0.0f, 0.0f, 1.0f,       \
    }

#define NX_MAT4_IDENTITY        \
    NX_MAT4_T {                 \
        1.0f, 0.0f, 0.0f, 0.0f, \
        0.0f, 1.0f, 0.0f, 0.0f, \
        0.0f, 0.0f, 1.0f, 0.0f, \
        0.0f, 0.0f, 0.0f, 1.0f  \
    }

#define NX_TRANSFORM_IDENTITY   \
    NX_TRANSFORM_T {            \
        NX_VEC3_ZERO,           \
        NX_QUAT_IDENTITY,       \
        NX_VEC3_ONE             \
    }

/* === Constructors Helpers === */

/**
 * @biref Create Integer 2D vector from single value
 */
#define NX_IVEC2_1(x)           \
    NX_STRUCT_LITERAL(NX_IVec2, \
        NX_CAST(int, (x)),      \
        NX_CAST(int, (x))       \
    )

/**
 * @biref Create Integer 2D vector from x, y values
 */
#define NX_IVEC2(x, y)          \
    NX_STRUCT_LITERAL(NX_IVec2, \
        NX_CAST(int, (x)),      \
        NX_CAST(int, (y))       \
    )

/**
 * @biref Create Integer 3D vector from single value
 */
#define NX_IVEC3_1(x)           \
    NX_STRUCT_LITERAL(NX_IVec3, \
        NX_CAST(int, (x)),      \
        NX_CAST(int, (x)),      \
        NX_CAST(int, (x))       \
    )

/**
 * @biref Create Integer 3D vector from x, y, z values
 */
#define NX_IVEC3(x, y, z)       \
    NX_STRUCT_LITERAL(NX_IVec3, \
        NX_CAST(int, (x)),      \
        NX_CAST(int, (y)),      \
        NX_CAST(int, (z))       \
    )

/**
 * @biref Create Integer 4D vector from single value
 */
#define NX_IVEC4_1(x)           \
    NX_STRUCT_LITERAL(NX_IVec4, \
        NX_CAST(int, (x)),      \
        NX_CAST(int, (x)),      \
        NX_CAST(int, (x)),      \
        NX_CAST(int, (x))       \
    )

/**
 * @biref Create Integer 4D vector from x, y, z, w values
 */
#define NX_IVEC4(x, y, z, w)    \
    NX_STRUCT_LITERAL(NX_IVec4, \
        NX_CAST(int, (x)),      \
        NX_CAST(int, (y)),      \
        NX_CAST(int, (z)),      \
        NX_CAST(int, (w))       \
    )

/**
 * @biref Create Float 2D vector from single value
 */
#define NX_VEC2_1(x)            \
    NX_STRUCT_LITERAL(NX_Vec2,  \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (x))     \
    )

/**
 * @biref Create Float 2D vector from x, y values
 */
#define NX_VEC2(x, y)           \
    NX_STRUCT_LITERAL(NX_Vec2,  \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (y))     \
    )

/**
 * @biref Create Float 3D vector from single value
 */
#define NX_VEC3_1(x)            \
    NX_STRUCT_LITERAL(NX_Vec3,  \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (x))     \
    )

/**
 * @biref Create Float 3D vector from x, y, z values
 */
#define NX_VEC3(x, y, z)        \
    NX_STRUCT_LITERAL(NX_Vec3,  \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (y)),    \
        NX_CAST(float, (z))     \
    )

/**
 * @biref Create Float 4D vector from single value
 */
#define NX_VEC4_1(x)            \
    NX_STRUCT_LITERAL(NX_Vec4,  \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (x))     \
    )

/**
 * @biref Create Float 4D vector from x, y, z, w values
 */
#define NX_VEC4(x, y, z, w)     \
    NX_STRUCT_LITERAL(NX_Vec4,  \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (y)),    \
        NX_CAST(float, (z)),    \
        NX_CAST(float, (w))     \
    )

/**
 * @biref Create Color from single grayscale value (alpha = 1)
 */
#define NX_COLOR_1(x)           \
    NX_STRUCT_LITERAL(NX_Color, \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (1))     \
    )

/**
 * @biref Create Color from grayscale and alpha values
 */
#define NX_COLOR_RGB(r, g, b)   \
    NX_STRUCT_LITERAL(NX_Color, \
        NX_CAST(float, (r)),    \
        NX_CAST(float, (g)),    \
        NX_CAST(float, (b)),    \
        NX_CAST(float, (1))     \
    )

/**
 * @biref Create Color from existing color with new alpha value
 */
#define NX_COLOR_ALPHA(c, a)    \
    NX_STRUCT_LITERAL(NX_Color, \
        NX_CAST(float, (c).r),  \
        NX_CAST(float, (c).g),  \
        NX_CAST(float, (c).b),  \
        NX_CAST(float, (a))     \
    )

/**
 * @biref Create Color from r, g, b, a values
 */
#define NX_COLOR(r, g, b, a)    \
    NX_STRUCT_LITERAL(NX_Color, \
        NX_CAST(float, (r)),    \
        NX_CAST(float, (g)),    \
        NX_CAST(float, (b)),    \
        NX_CAST(float, (a))     \
    )

/**
 * @biref Create Quaternion from w, x, y, z values
 */
#define NX_QUAT(w, x, y, z)     \
    NX_STRUCT_LITERAL(NX_Quat,  \
        NX_CAST(float, (w)),    \
        NX_CAST(float, (x)),    \
        NX_CAST(float, (y)),    \
        NX_CAST(float, (z))     \
    )

/**
 * @biref Compound literal helper for 3x3 matrix (row-major)
 */
#define NX_MAT3_T NX_LITERAL(NX_Mat3)

/**
 * @biref Compound literal helper for 4x4 matrix (row-major)
 */
#define NX_MAT4_T NX_LITERAL(NX_Mat4)

/**
 * @biref Compound literal helper for Transform (translation, rotation, scale)
 */
#define NX_TRANSFORM_T NX_LITERAL(NX_Transform)

/* === Macros === */

/**
 * @brief Returns the minimum of two values
 */
#define NX_MIN(a, b) \
    ((a) < (b) ? (a) : (b))

/**
 * @brief Returns the maximum of two values
 */
#define NX_MAX(a, b) \
    ((a) > (b) ? (a) : (b))

/**
 * @brief Returns the minimum of three values
 */
#define NX_MIN3(a, b, c) \
    NX_MIN(a, NX_MIN(b, c))

/**
 * @brief Returns the maximum of three values
 */
#define NX_MAX3(a, b, c) \
    NX_MAX(a, NX_MAX(b, c))

/**
 * @brief Clamps a value between minimum and maximum bounds
 */
#define NX_CLAMP(v, min, max) \
    NX_MIN(NX_MAX((v), (min)), (max))

/**
 * @brief Returns the absolute value of a number
 */
#define NX_ABS(x) \
    ((x) < 0 ? -(x) : (x))

/**
 * @brief Returns the sign of a number (-1, 0, or 1)
 */
#define NX_SIGN(x) \
    (((x) > 0) - ((x) < 0))

/**
 * @brief Returns the square of a number
 */
#define NX_POW2(x) \
    ((x) * (x))

/**
 * @brief Returns the cube of a number
 */
#define NX_POW3(x) \
    ((x) * (x) * (x))

/**
 * @brief The multiple of B after A
 */
#define NX_NEXT_MULTIPLE(a, b) \
    ((b) * ((int)ceilf((float)(a) / (b))))

/**
 * @brief The multiple of B before A
 */
#define NX_PREV_MULTIPLE(a, b) \
    ((b) * ((int)floorf((float)(a) / (b))))

/**
 * @brief The closest multiple of B to A
 */
#define NX_NEAR_MULTIPLE(a, b) \
    ((b) * ((int)roundf((float)(a) / (b))))

/**
 * @brief Integer division with ceiling (round up)
 */
#define NX_DIV_CEIL(num, denom) \
    (((num) + (denom) - 1) / (denom))

/**
 * @brief Checks if value is within inclusive range
 */
#define NX_IN_RANGE(x, low, high) \
    ((x) >= (low) && (x) <= (high))

/**
 * @brief Rounds value up to the next alignment boundary
 */
#define NX_ALIGN_UP(value, alignment) \
    (((value) + (alignment) - 1) & ~((alignment) - 1))

/**
 * @brief Rounds value down to the previous alignment boundary
 */
#define NX_ALIGN_DOWN(value, alignment) \
    ((value) & ~((alignment) - 1))

/**
 * @brief Checks if addition would overflow
 */
#define NX_WOULD_OVERFLOW_ADD(a, b, max) \
    ((a) > (max) - (b))

/**
 * @brief Checks if multiplication would overflow
 */
#define NX_WOULD_OVERFLOW_MUL(a, b, max) \
    ((a) != 0 && (b) > (max) / (a))

/* === Structures === */

/**
 * @biref Integer 2D vector (x, y)
 */
typedef struct NX_IVec2 {
    union {
        struct { int x, y; };
        int v[2];
    };
} NX_IVec2;

/**
 * @biref Integer 3D vector (x, y, z)
 */
typedef struct NX_IVec3 {
    union {
        struct { int x, y, z; };
        int v[3];
    };
} NX_IVec3;

/**
 * @biref Integer 4D vector (x, y, z, w)
 */
typedef struct NX_IVec4 {
    union {
        struct { int x, y, z, w; };
        int v[4];
    };
} NX_IVec4;

/**
 * @biref Float 2D vector (x, y)
 */
typedef struct NX_Vec2 {
    union {
        struct { float x, y; };
        float v[2];
    };
} NX_Vec2;

/**
 * @biref Float 3D vector (x, y, z)
 */
typedef struct NX_Vec3 {
    union {
        struct { float x, y, z; };
        float v[3];
    };
} NX_Vec3;

/**
 * @biref Float 4D vector (x, y, z, w)
 */
typedef struct NX_Vec4 {
    union {
        struct { float x, y, z, w; };
        float v[4];
    };
} NX_Vec4;

/**
 * @biref Floating-point RGBA color (r, g, b, a)
 */
typedef struct NX_Color {
    float r, g, b, a;
} NX_Color;

/**
 * @biref Quaternion (w, x, y, z)
 */
typedef struct NX_Quat {
    union {
        struct { float w, x, y, z; };
        float v[4];
    };
} NX_Quat;

/**
 * @biref 3x3 Matrix (row-major)
 */
typedef struct NX_Mat3 {
    union {
        struct {
            float m00, m01, m02;
            float m10, m11, m12;
            float m20, m21, m22;
        };
        float v[3][3];
        float a[9];
    };
} NX_Mat3;

/**
 * @biref 4x4 Matrix (row-major)
 */
typedef struct NX_Mat4 {
    union {
        struct {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
        float v[4][4];
        float a[16];
    };
} NX_Mat4;

/**
 * @biref Transform (translation, rotation, scale)
 */
typedef struct NX_Transform {
    NX_Vec3 translation;
    NX_Quat rotation;
    NX_Vec3 scale;
} NX_Transform;

#if defined(__cplusplus)
extern "C" {
#endif

/* === General Math Functions === */

/** @defgroup Math General Math Functions
 *  Inline utility functions for general math operations.
 *  @{
 */

/**
 * @brief Says if a 32-bit integer is a power of two
 */
static inline bool NX_IsPowerOfTwo(uint64_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

/**
 * @brief Return the next highest power of two for a 64-bit integer
 */
static inline uint64_t NX_NextPowerOfTwo(uint64_t x)
{
    return (x <= 1) ? 1ULL : 1ULL << (64 - NX_Clz64(x - 1));
}

/**
 * @brief Return the previous lowest power of two for a 64-bit integer
 */
static inline uint64_t NX_PrevPowerOfTwo(uint64_t x)
{
    return (x == 0) ? 0ULL : 1ULL << (63 - NX_Clz64(x));
}

/**
 * @brief Return the nearest power of two for a 64-bit integer
 */
static inline uint64_t NX_NearPowerOfTwo(uint64_t x)
{
    if (x <= 1) return 1ULL;
    uint64_t next = NX_NextPowerOfTwo(x);
    uint64_t prev = NX_PrevPowerOfTwo(x);
    return (x - prev < next - x) ? prev : next;
}

/**
 * @brief Clamp float to [0.0, 1.0]
 */
static inline float NX_Saturate(float x)
{
    return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

/**
 * @brief Wrap integer to [min, max)
 */
static inline int NX_WrapInt(int value, int min, int max)
{
    int range = max - min;
    int offset = (value - min) % range;
    return min + offset + (offset < 0 ? range : 0);
}

/**
 * @brief Wrap float to [min, max)
 */
static inline float NX_Wrap(float value, float min, float max)
{
    float range = max - min;
    float offset = value - min;
    return min + offset - range * floorf(offset / range);
}

/**
 * @brief Wrap radians to [-Pi, Pi)
 */
static inline float NX_WrapRadians(float radians)
{
    return NX_Wrap(radians, -NX_PI, +NX_PI);
}

/**
 * @brief Normalize value from [start, end] to [0,1]
 */
static inline float NX_Normalize(float value, float start, float end)
{
    return (value - start) / (end - start);
}

/**
 * @brief Remap value from [inStart, inEnd] to [outStart, outEnd]
 */
static inline float NX_Remap(float value, float inStart, float inEnd, float outStart, float outEnd)
{
    return (value - inStart) / (inEnd - inStart) * (outEnd - outStart) + outStart;
}

/**
 * @brief Ping-pong a value between min and max.
 */
static inline float NX_PingPong(float value, float min, float max)
{
    float range = max - min;
    if (range == 0.0f) return min;

    float wrapped = fmodf(value - min, 2.0f * range);
    if (wrapped < 0.0f) wrapped += 2.0f * range;

    // Reflect around the max value
    if (wrapped < range) return min + wrapped;
    return max - (wrapped - range);
}

/**
 * @brief Return fractional part of float
 */
static inline float NX_Fract(float x)
{
    return x - floorf(x);
}

/**
 * @brief Step function: 0 if x < edge, else 1
 */
static inline float NX_Step(float edge, float x)
{
    return (x < edge) ? 0.0f : 1.0f;
}

/**
 * @brief Approximate equality for floats with epsilon
 */
static inline bool NX_Approx(float a, float b, float epsilon)
{
    return fabsf(a - b) < epsilon;
}

/**
 * @brief Linear interpolation
 */
static inline float NX_Lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

/**
 * @brief Linear interpolation for radians
 */
static inline float NX_LerpRadians(float a, float b, float t)
{
    return a + NX_WrapRadians(b - a) * t;
}

/**
 * @brief Inverse linear interpolation
 */
static inline float NX_LerpInverse(float a, float b, float value)
{
    return (value - a) / (b - a);
}

/**
 * @brief Smoothstep interpolation
 */
static inline float NX_Smoothstep(float edge0, float edge1, float x)
{
    float t = (x - edge0) / (edge1 - edge0);
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    return t * t * t * (t * (t * 6 - 15) + 10);
}

/**
 * @brief Exponential decay
 */
static inline float NX_ExpDecay(float initial, float decayRate, float time)
{
    return initial * expf(-decayRate * time);
}

/**
 * @brief Move current value toward target by maxDelta
 */
static inline float NX_MoveToward(float current, float target, float maxDelta)
{
    float delta = target - current;
    float distance = fabsf(delta);
    if (distance <= maxDelta) return target;
    else return current + (delta / distance) * maxDelta;
}

/**
 * @brief Convert an angle from degrees to radians
 */
static inline float NX_Radians(float degrees)
{
    return degrees * NX_DEG2RAD;
}

/**
 * @brief Convert an angle from radians to degrees
 */
static inline float NX_Degrees(float radians)
{
    return radians * NX_RAD2DEG;
}

/** @} */ // Math

/* === Ease Functions === */

/** @defgroup Easing Ease Functions
 *  Inline functions for easing/interpolation.
 *  t is expected to be in [0,1].
 *  @{
 */

/**
 * @brief Sine easing in
 * @see https://easings.net/#easeInSine
 */
static inline float NX_EaseSineIn(float t)
{
    return sinf(NX_PI / 2 * t);
}

/**
 * @brief Sine easing out
 * @see https://easings.net/#easeOutSine
 */
static inline float NX_EaseSineOut(float t)
{
    return 1 + sinf(NX_PI / 2 * (--t));
}

/**
 * @brief Sine easing in-out
 * @see https://easings.net/#easeInOutSine
 */
static inline float NX_EaseSineInOut(float t)
{
    return 0.5f * (1 + sinf(NX_PI * (t - 0.5f)));
}

/**
 * @brief Quadratic easing in
 * @see https://easings.net/#easeInQuad
 */
static inline float NX_EaseQuadIn(float t)
{
    return t * t;
}

/**
 * @brief Quadratic easing out
 * @see https://easings.net/#easeOutQuad
 */
static inline float NX_EaseQuadOut(float t)
{
    return t * (2 - t);
}

/**
 * @brief Quadratic easing in-out
 * @see https://easings.net/#easeInOutQuad
 */
static inline float NX_EaseQuadInOut(float t)
{
    return t < 0.5f ? 2 * t * t : t * (4 - 2 * t) - 1;
}

/**
 * @brief Cubic easing in
 * @see https://easings.net/#easeInCubic
 */
static inline float NX_EaseCubicIn(float t)
{
    return t * t * t;
}

/**
 * @brief Cubic easing out
 * @see https://easings.net/#easeOutCubic
 */
static inline float NX_EaseCubicOut(float t)
{
    --t; return 1 + t * t * t;
}

/**
 * @brief Cubic easing in-out
 * @see https://easings.net/#easeInOutCubic
 */
static inline float NX_EaseCubicInOut(float t)
{
    return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
}

/**
 * @brief Quartic easing in
 * @see https://easings.net/#easeInQuart
 */
static inline float NX_EaseQuartIn(float t)
{
    t *= t;
    return t * t;
}

/**
 * @brief Quartic easing out
 * @see https://easings.net/#easeOutQuart
 */
static inline float NX_EaseQuartOut(float t)
{
    --t; t = t * t;
    return 1 - t * t;
}

/**
 * @brief Quartic easing in-out
 * @see https://easings.net/#easeInOutQuart
 */
static inline float NX_EaseQuartInOut(float t)
{
    if (t < 0.5f) {
        t *= t;
        return 8 * t * t;
    }
    else {
        --t; t = t * t;
        return 1 - 8 * t * t;
    }
}

/**
 * @brief Quintic easing in
 * @see https://easings.net/#easeInQuint
 */
static inline float NX_EaseQuintIn(float t)
{
    float t2 = t * t;
    return t * t2 * t2;
}

/**
 * @brief Quintic easing out
 * @see https://easings.net/#easeInOutQuint
 */
static inline float NX_EaseQuintOut(float t)
{
    --t; float t2 = t * t;
    return 1 + t * t2 * t2;
}

/**
 * @brief Quintic easing in-out
 * @see https://easings.net/#easeInOutQuint
 */
static inline float NX_EaseQuintInOut(float t)
{
    float t2;
    if (t < 0.5f) {
        t2 = t * t;
        return 16 * t * t2 * t2;
    }
    --t; t2 = t * t;
    return 1 + 16 * t * t2 * t2;
}

/**
 * @brief Exponential easing in
 * @see https://easings.net/#easeInExpo
 */
static inline float NX_EaseExpoIn(float t)
{
    return (powf(2, 8 * t) - 1) / 255;
}

/**
 * @brief Exponential easing out
 * @see https://easings.net/#easeOutExpo
 */
static inline float NX_EaseExpoOut(float t)
{
    return 1 - powf(2, -8 * t);
}

/**
 * @brief Exponential easing in-out
 * @see https://easings.net/#easeInOutExpo
 */
static inline float NX_EaseExpoInOut(float t)
{
    if (t < 0.5f) {
        return (powf(2, 16 * t) - 1) / 510;
    }
    return 1 - 0.5f * powf(2, -16 * (t - 0.5f));
}

/**
 * @brief Circular easing in
 * @see https://easings.net/#easeInCirc
 */
static inline float NX_EaseCircIn(float t)
{
    return 1 - sqrtf(1 - t);
}

/**
 * @brief Circular easing out
 * @see https://easings.net/#easeOutCirc
 */
static inline float NX_EaseCircOut(float t)
{
    return sqrtf(t);
}

/**
 * @brief Circular easing in-out
 * @see https://easings.net/#easeInOutCirc
 */
static inline float NX_EaseCircInOut(float t)
{
    if (t < 0.5f) {
        return (1 - sqrtf(1 - 2 * t)) * 0.5f;
    }
    return (1 + sqrtf(2 * t - 1)) * 0.5f;
}

/**
 * @brief Back easing in
 * @see https://easings.net/#easeInBack
 */
static inline float NX_EaseBackIn(float t)
{
    return t * t * (2.70158f * t - 1.70158f);
}

/**
 * @brief Back easing out
 * @see https://easings.net/#easeOutBack
 */
static inline float NX_EaseBackOut(float t)
{
    --t; return 1 + t * t * (2.70158f * t + 1.70158f);
}

/**
 * @brief Back easing in-out
 * @see https://easings.net/#easeInOutBack
 */
static inline float NX_EaseBackInOut(float t)
{
    if (t < 0.5f) {
        return t * t * (7 * t - 2.5f) * 2;
    }
    --t; return 1 + t * t * 2 * (7 * t + 2.5f);
}

/**
 * @brief Elastic easing in
 * @see https://easings.net/#easeInElastic
 */
static inline float NX_EaseElasticIn(float t)
{
    float t2 = t * t;
    return t2 * t2 * sinf(t * NX_PI * 4.5f);
}

/**
 * @brief Elastic easing out
 * @see https://easings.net/#easeOutElastic
 */
static inline float NX_EaseElasticOut(float t)
{
    float t2 = (t - 1) * (t - 1);
    return 1 - t2 * t2 * cosf(t * NX_PI * 4.5f);
}

/**
 * @brief Elastic easing in-out
 * @see https://easings.net/#easeInOutElastic
 */
static inline float NX_EaseElasticInOut(float t)
{
    float t2;
    if (t < 0.45f) {
        t2 = t * t;
        return 8 * t2 * t2 * sinf(t * NX_PI * 9);
    }
    else if (t < 0.55f) {
        return 0.5f + 0.75f * sinf(t * NX_PI * 4);
    }
    t2 = (t - 1) * (t - 1);
    return 1 - 8 * t2 * t2 * sinf(t * NX_PI * 9);
}

/**
 * @brief Bounce easing in
 * @see https://easings.net/#easeInBounce
 */
static inline float NX_EaseBounceIn(float t)
{
    return powf(2, 6 * (t - 1)) * fabsf(sinf(t * NX_PI * 3.5f));
}

/**
 * @brief Bounce easing out
 * @see https://easings.net/#easeOutBounce
 */
static inline float NX_EaseBounceOut(float t)
{
    return 1 - powf(2, -6 * t) * fabsf(cosf(t * NX_PI * 3.5f));
}

/**
 * @brief Bounce easing in-out
 * @see https://easings.net/#easeInOutBounce
 */
static inline float NX_EaseBounceInOut(float t)
{
    if (t < 0.5f) {
        return 8 * powf(2, 8 * (t - 1)) * fabsf(sinf(t * NX_PI * 7));
    }
    return 1 - 8 * powf(2, -8 * t) * fabsf(sinf(t * NX_PI * 7));
}

/** @} */ // Easing

/* === 2D Integer Vector Functions === */

/** @defgroup IVec2 2D Integer Vector Functions
 *  Inline functions for 2D integer vector operations.
 *  @{
 */

/**
 * @brief Component-wise minimum of two vectors
 */
static inline NX_IVec2 NX_IVec2Min(NX_IVec2 v, NX_IVec2 min)
{
    return NX_IVEC2(NX_MIN(v.x, min.x), NX_MIN(v.y, min.y));
}

/**
 * @brief Component-wise maximum of two vectors
 */
static inline NX_IVec2 NX_IVec2Max(NX_IVec2 v, NX_IVec2 max)
{
    return NX_IVEC2(NX_MAX(v.x, max.x), NX_MAX(v.y, max.y));
}

/**
 * @brief Clamp vector components between min and max
 */
static inline NX_IVec2 NX_IVec2Clamp(NX_IVec2 v, NX_IVec2 min, NX_IVec2 max)
{
    return NX_IVEC2(NX_CLAMP(v.x, min.x, max.x), NX_CLAMP(v.y, min.y, max.y));
}

/**
 * @brief Absolute value of vector components
 */
static inline NX_IVec2 NX_IVec2Abs(NX_IVec2 v)
{
    return NX_IVEC2(abs(v.x), abs(v.y));
}

/**
 * @brief Negate vector components
 */
static inline NX_IVec2 NX_IVec2Neg(NX_IVec2 v)
{
    return NX_IVEC2(-v.x, -v.y);
}

/**
 * @brief Reciprocal of vector components
 */
static inline NX_Vec2 NX_IVec2Rcp(NX_IVec2 v)
{
    return NX_VEC2(1.0f / v.x, 1.0f / v.y);
}

/**
 * @brief True if all components are != 0
 */
static inline bool NX_IVec2Any(NX_IVec2 v0)
{
    return (v0.x || v0.y);
}

/**
 * @brief True if any component is != 0
 */
static inline bool NX_IVec2All(NX_IVec2 v0)
{
    return (v0.x && v0.y);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline NX_IVec2 NX_IVec2Equals(NX_IVec2 v0, NX_IVec2 v1)
{
    return NX_IVEC2(v0.x == v1.x, v0.y == v1.y);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline NX_IVec2 NX_IVec2GreaterThan(NX_IVec2 v0, NX_IVec2 v1)
{
    return NX_IVEC2(v0.x > v1.x, v0.y > v1.y);
}

/**
 * @brief Component-wise addition
 */
static inline NX_IVec2 NX_IVec2Add(NX_IVec2 v0, NX_IVec2 v1)
{
    return NX_IVEC2(v0.x + v1.x, v0.y + v1.y);
}

/**
 * @brief Component-wise subtraction
 */
static inline NX_IVec2 NX_IVec2Sub(NX_IVec2 v0, NX_IVec2 v1)
{
    return NX_IVEC2(v0.x - v1.x, v0.y - v1.y);
}

/**
 * @brief Component-wise multiplication
 */
static inline NX_IVec2 NX_IVec2Mul(NX_IVec2 v0, NX_IVec2 v1)
{
    return NX_IVEC2(v0.x * v1.x, v0.y * v1.y);
}

/**
 * @brief Component-wise division
 */
static inline NX_IVec2 NX_IVec2Div(NX_IVec2 v0, NX_IVec2 v1)
{
    return NX_IVEC2(v0.x / v1.x, v0.y / v1.y);
}

/**
 * @brief Offset vector by scalar
 */
static inline NX_IVec2 NX_IVec2Offset(NX_IVec2 v, int s)
{
    return NX_IVEC2(v.x + s, v.y + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline NX_IVec2 NX_IVec2Scale(NX_IVec2 v, int s)
{
    return NX_IVEC2(v.x * s, v.y * s);
}

/** @} */ // IVec2

/* === 3D Integer Vector Functions === */

/** @defgroup IVec3 3D Integer Vector Functions
 *  Inline functions for 3D integer vector operations.
 *  @{
 */

/**
 * @brief Component-wise minimum of two vectors
 */
static inline NX_IVec3 NX_IVec3Min(NX_IVec3 v, NX_IVec3 min)
{
    return NX_IVEC3(NX_MIN(v.x, min.x), NX_MIN(v.y, min.y), NX_MIN(v.z, min.z));
}

/**
 * @brief Component-wise maximum of two vectors
 */
static inline NX_IVec3 NX_IVec3Max(NX_IVec3 v, NX_IVec3 max)
{
    return NX_IVEC3(NX_MAX(v.x, max.x), NX_MAX(v.y, max.y), NX_MAX(v.z, max.z));
}

/**
 * @brief Clamp vector components between min and max
 */
static inline NX_IVec3 NX_IVec3Clamp(NX_IVec3 v, NX_IVec3 min, NX_IVec3 max)
{
    return NX_IVEC3(NX_CLAMP(v.x, min.x, max.x), NX_CLAMP(v.y, min.y, max.y), NX_CLAMP(v.z, min.z, max.z));
}

/**
 * @brief Absolute value of vector components
 */
static inline NX_IVec3 NX_IVec3Abs(NX_IVec3 v)
{
    return NX_IVEC3(abs(v.x), abs(v.y), abs(v.z));
}

/**
 * @brief Negate vector components
 */
static inline NX_IVec3 NX_IVec3Neg(NX_IVec3 v)
{
    return NX_IVEC3(-v.x, -v.y, -v.z);
}

/**
 * @brief Reciprocal of vector components
 */
static inline NX_Vec3 NX_IVec3Rcp(NX_IVec3 v)
{
    return NX_VEC3(1.0f / v.x, 1.0f / v.y, 1.0f / v.z);
}

/**
 * @brief True if all components are != 0
 */
static inline bool NX_IVec3Any(NX_IVec3 v0)
{
    return (v0.x || v0.y || v0.z);
}

/**
 * @brief True if any component is != 0
 */
static inline bool NX_IVec3All(NX_IVec3 v0)
{
    return (v0.x && v0.y && v0.z);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline NX_IVec3 NX_IVec3Equals(NX_IVec3 v0, NX_IVec3 v1)
{
    return NX_IVEC3(v0.x == v1.x, v0.y == v1.y, v0.z == v1.z);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline NX_IVec3 NX_IVec3GreaterThan(NX_IVec3 v0, NX_IVec3 v1)
{
    return NX_IVEC3(v0.x > v1.x, v0.y > v1.y, v0.z > v1.z);
}

/**
 * @brief Component-wise addition
 */
static inline NX_IVec3 NX_IVec3Add(NX_IVec3 v0, NX_IVec3 v1)
{
    return NX_IVEC3(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

/**
 * @brief Component-wise subtraction
 */
static inline NX_IVec3 NX_IVec3Sub(NX_IVec3 v0, NX_IVec3 v1)
{
    return NX_IVEC3(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

/**
 * @brief Component-wise multiplication
 */
static inline NX_IVec3 NX_IVec3Mul(NX_IVec3 v0, NX_IVec3 v1)
{
    return NX_IVEC3(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z);
}

/**
 * @brief Component-wise division
 */
static inline NX_IVec3 NX_IVec3Div(NX_IVec3 v0, NX_IVec3 v1)
{
    return NX_IVEC3(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z);
}

/**
 * @brief Offset vector by scalar
 */
static inline NX_IVec3 NX_IVec3Offset(NX_IVec3 v, int s)
{
    return NX_IVEC3(v.x + s, v.y + s, v.z + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline NX_IVec3 NX_IVec3Scale(NX_IVec3 v, int s)
{
    return NX_IVEC3(v.x * s, v.y * s, v.z * s);
}

/** @} */ // IVec3

/* === 4D Integer Vector Functions === */

/** @defgroup IVec4 4D Integer Vector Functions
 *  Inline functions for 4D integer vector operations.
 *  @{
 */

/**
 * @brief Component-wise minimum of two vectors
 */
static inline NX_IVec4 NX_IVec4Min(NX_IVec4 v, NX_IVec4 min)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = NX_MIN(v.v[i], min.v[i]);
    }
    return v;
}

/**
 * @brief Component-wise maximum of two vectors
 */
static inline NX_IVec4 NX_IVec4Max(NX_IVec4 v, NX_IVec4 max)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = NX_MAX(v.v[i], max.v[i]);
    }
    return v;
}

/**
 * @brief Clamp vector components between min and max
 */
static inline NX_IVec4 NX_IVec4Clamp(NX_IVec4 v, NX_IVec4 min, NX_IVec4 max)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = NX_CLAMP(v.v[i], min.v[i], max.v[i]);
    }
    return v;
}

/**
 * @brief Absolute value of vector components
 */
static inline NX_IVec4 NX_IVec4Abs(NX_IVec4 v)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = abs(v.v[i]);
    }
    return v;
}

/**
 * @brief Negate vector components
 */
static inline NX_IVec4 NX_IVec4Neg(NX_IVec4 v)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = -v.v[i];
    }
    return v;
}

/**
 * @brief Reciprocal of vector components
 */
static inline NX_Vec4 NX_IVec4Rcp(NX_IVec4 v)
{
    NX_Vec4 result;
    for (int i = 0; i < 4; i++) {
        result.v[i] = 1.0f / v.v[i];
    }
    return result;
}

/**
 * @brief True if all components are != 0
 */
static inline bool NX_IVec4Any(NX_IVec4 v0)
{
    return (v0.x || v0.y || v0.z || v0.w);
}

/**
 * @brief True if any component is != 0
 */
static inline bool NX_IVec4All(NX_IVec4 v0)
{
    return (v0.x && v0.y && v0.z && v0.w);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline NX_IVec4 NX_IVec4Equals(NX_IVec4 v0, NX_IVec4 v1)
{
    return NX_IVEC4(v0.x == v1.x, v0.y == v1.y, v0.z == v1.z, v0.w == v1.w);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline NX_IVec4 NX_IVec4GreaterThan(NX_IVec4 v0, NX_IVec4 v1)
{
    return NX_IVEC4(v0.x > v1.x, v0.y > v1.y, v0.z > v1.z, v0.w > v1.w);
}

/**
 * @brief Component-wise addition
 */
static inline NX_IVec4 NX_IVec4Add(NX_IVec4 v0, NX_IVec4 v1)
{
    for (int i = 0; i < 4; i++) {
        v0.v[i] += v1.v[i];
    }
    return v0;
}

/**
 * @brief Component-wise subtraction
 */
static inline NX_IVec4 NX_IVec4Sub(NX_IVec4 v0, NX_IVec4 v1)
{
    for (int i = 0; i < 4; i++) {
        v0.v[i] -= v1.v[i];
    }
    return v0;
}

/**
 * @brief Component-wise multiplication
 */
static inline NX_IVec4 NX_IVec4Mul(NX_IVec4 v0, NX_IVec4 v1)
{
    for (int i = 0; i < 4; i++) {
        v0.v[i] *= v1.v[i];
    }
    return v0;
}

/**
 * @brief Component-wise division
 */
static inline NX_IVec4 NX_IVec4Div(NX_IVec4 v0, NX_IVec4 v1)
{
    for (int i = 0; i < 4; i++) {
        v0.v[i] /= v1.v[i];
    }
    return v0;
}

/**
 * @brief Offset vector by scalar
 */
static inline NX_IVec4 NX_IVec4Offset(NX_IVec4 v, int s)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] += s;
    }
    return v;
}

/**
 * @brief Scale vector by scalar
 */
static inline NX_IVec4 NX_IVec4Scale(NX_IVec4 v, int s)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] *= s;
    }
    return v;
}

/** @} */ // IVec4

/* === 2D Vector Functions === */

/** @defgroup Vec2 2D Vector Functions
 *  Inline functions for 2D vector operations.
 *  @{
 */

/**
 * @brief Returns a vector containing the component-wise minimum of v0 and v1.
 */
static inline NX_Vec2 NX_Vec2Min(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_VEC2(fminf(v0.x, v1.x), fminf(v0.y, v1.y));
}

/**
 * @brief Returns a vector containing the component-wise maximum of v0 and v1.
 */
static inline NX_Vec2 NX_Vec2Max(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_VEC2(fmaxf(v0.x, v1.x), fmaxf(v0.y, v1.y));
}

/**
 * @brief Clamp each component of vector x to [min,max].
 */
static inline NX_Vec2 NX_Vec2Clamp(NX_Vec2 v, NX_Vec2 min, NX_Vec2 max)
{
    return NX_VEC2(
        fmaxf(fminf(v.x, max.x), min.x),
        fmaxf(fminf(v.y, max.y), min.y)
    );
}

/**
 * @brief Absolute value of vector components
 */
static inline NX_Vec2 NX_Vec2Abs(NX_Vec2 v)
{
    return NX_VEC2(fabsf(v.x), fabsf(v.y));
}

/**
 * @brief Negate vector components
 */
static inline NX_Vec2 NX_Vec2Neg(NX_Vec2 v)
{
    return NX_VEC2(-v.x, -v.y);
}

/**
 * @brief Reciprocal of vector components
 */
static inline NX_Vec2 NX_Vec2Rcp(NX_Vec2 v)
{
    return NX_VEC2(1.0f / v.x, 1.0f / v.y);
}

/**
 * @brief Check approximate equality of two vectors
 */
static inline int NX_Vec2Approx(NX_Vec2 v0, NX_Vec2 v1, float epsilon)
{
    return (fabsf(v0.x - v1.x) < epsilon) &&
           (fabsf(v0.y - v1.y) < epsilon);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline NX_IVec2 NX_Vec2Equals(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_IVEC2(v0.x == v1.x, v0.y == v1.y);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline NX_IVec2 NX_Vec2GreaterThan(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_IVEC2(v0.x > v1.x, v0.y > v1.y);
}

/**
 * @brief Component-wise addition
 */
static inline NX_Vec2 NX_Vec2Add(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_VEC2(v0.x + v1.x, v0.y + v1.y);
}

/**
 * @brief Component-wise subtraction
 */
static inline NX_Vec2 NX_Vec2Sub(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_VEC2(v0.x - v1.x, v0.y - v1.y);
}

/**
 * @brief Component-wise multiplication
 */
static inline NX_Vec2 NX_Vec2Mul(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_VEC2(v0.x * v1.x, v0.y * v1.y);
}

/**
 * @brief Component-wise division
 */
static inline NX_Vec2 NX_Vec2Div(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_VEC2(v0.x / v1.x, v0.y / v1.y);
}

/**
 * @brief Offset vector by scalar
 */
static inline NX_Vec2 NX_Vec2Offset(NX_Vec2 v, float s)
{
    return NX_VEC2(v.x + s, v.y + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline NX_Vec2 NX_Vec2Scale(NX_Vec2 v, float s)
{
    return NX_VEC2(v.x * s, v.y * s);
}

/**
 * @brief Scalar multiply and add
 */
static inline NX_Vec2 NX_Vec2MulAdd(NX_Vec2 a, float s, NX_Vec2 b)
{
    return NX_VEC2(
        fmaf(a.x, s, b.x),
        fmaf(a.y, s, b.y)
    );
}

/**
 * @brief Dot product
 */
static inline float NX_Vec2Dot(NX_Vec2 v0, NX_Vec2 v1)
{
    return v0.x * v1.x + v0.y * v1.y;
}

/**
 * @brief Vector length
 */
static inline float NX_Vec2Length(NX_Vec2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

/**
 * @brief Squared vector length
 */
static inline float NX_Vec2LengthSq(NX_Vec2 v)
{
    return v.x * v.x + v.y * v.y;
}

/**
 * @brief Normalize vector
 */
static inline NX_Vec2 NX_Vec2Normalize(NX_Vec2 v)
{
    float len = NX_Vec2Length(v);
    if (len > 0.0f) {
        return NX_Vec2Scale(v, 1.0f / len);
    }
    return NX_VEC2(0.0f, 0.0f);
}

/**
 * @brief Distance between two vectors
 */
static inline float NX_Vec2Distance(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_Vec2Length(NX_Vec2Sub(v1, v0));
}

/**
 * @brief Squared distance between two vectors
 */
static inline float NX_Vec2DistanceSq(NX_Vec2 v0, NX_Vec2 v1)
{
    NX_Vec2 diff = NX_Vec2Sub(v1, v0);
    return NX_Vec2LengthSq(diff);
}

/**
 * @brief CCW angle from X axis (radians)
 */
static inline float NX_Vec2AngleCCW(NX_Vec2 v)
{
    return atan2f(v.y, v.x);
}

/**
 * @brief CW angle from X axis (radians)
 */
static inline float NX_Vec2AngleCW(NX_Vec2 v)
{
    return -atan2f(v.y, v.x);
}

/**
 * @brief CCW angle between two vectors (radians)
 */
static inline float NX_Vec2LineAngleCCW(NX_Vec2 v0, NX_Vec2 v1)
{
    return atan2f(v1.y - v0.y, v1.x - v0.x);
}

/**
 * @brief CW angle between two vectors (radians)
 */
static inline float NX_Vec2LineAngleCW(NX_Vec2 v0, NX_Vec2 v1)
{
    return -atan2f(v1.y - v0.y, v1.x - v0.x);
}

/**
 * @brief Create unit vector from angle (radians)
 */
static inline NX_Vec2 NX_Vec2FromAngle(float angle)
{
    return NX_VEC2(cosf(angle), sinf(angle));
}

/**
 * @brief Rotate vector by angle (radians)
 */
static inline NX_Vec2 NX_Vec2Rotate(NX_Vec2 v, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return NX_VEC2(v.x * c - v.y * s, v.x * s + v.y * c);
}

/**
 * @brief Get direction from v0 to v1, normalized
 */
static inline NX_Vec2 NX_Vec2Direction(NX_Vec2 v0, NX_Vec2 v1)
{
    return NX_Vec2Normalize(NX_Vec2Sub(v1, v0));
}

/**
 * @brief Linear interpolation between vectors
 */
static inline NX_Vec2 NX_Vec2Lerp(NX_Vec2 v0, NX_Vec2 v1, float t)
{
    float w1 = 1.0f - t;
    float w2 = t;

    NX_Vec2 result;
    result.x = fmaf(w1, v0.x, w2 * v1.x);
    result.y = fmaf(w1, v0.y, w2 * v1.y);

    return result;
}

/**
 * @brief Move vector toward target without exceeding max_delta
 */
static inline NX_Vec2 NX_Vec2MoveToward(NX_Vec2 from, NX_Vec2 to, float max_delta)
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;

    float distSq = dx * dx + dy * dy;
    float maxDeltaSq = max_delta * max_delta;

    if (distSq <= maxDeltaSq) {
        return to;
    }

    float dist = sqrtf(distSq);
    float ratio = max_delta / dist;

    NX_Vec2 result;
    result.x = fmaf(dx, ratio, from.x);
    result.y = fmaf(dy, ratio, from.y);

    return result;
}

/**
 * @brief Reflect vector across normal
 */
static inline NX_Vec2 NX_Vec2Reflect(NX_Vec2 v, NX_Vec2 n)
{
    float dot = NX_Vec2Dot(v, n);
    return NX_Vec2Sub(v, NX_Vec2Scale(n, 2.0f * dot));
}

/**
 * @brief Perpendicular vector (rotated 90° CCW)
 */
static inline NX_Vec2 NX_Vec2Perp(NX_Vec2 v)
{
    return NX_VEC2(-v.y, v.x);
}

/**
 * @brief Transform a 2D vector by 3x3 matrix
 */
static inline NX_Vec2 NX_Vec2TransformByMat3(NX_Vec2 v, const NX_Mat3* mat)
{
    NX_Vec2 result;
    result.x = fmaf(mat->m00, v.x, fmaf(mat->m10, v.y, mat->m20));
    result.y = fmaf(mat->m01, v.x, fmaf(mat->m11, v.y, mat->m21));

    return result;
}

/**
 * @brief Transform a 2D vector by 4x4 matrix
 */
static inline NX_Vec2 NX_Vec2TransformByMat4(NX_Vec2 v, const NX_Mat4* mat)
{
    NX_Vec2 result;
    result.x = fmaf(mat->m00, v.x, fmaf(mat->m10, v.y, mat->m30));
    result.y = fmaf(mat->m01, v.x, fmaf(mat->m11, v.y, mat->m31));

    return result;
}

/** @} */ // Vec2

/* === 3D Vector Functions === */

/** @defgroup Vec3 3D Vector Functions
 *  Inline functions for 3D vector operations.
 *  @{
 */

/**
 * @brief Returns a vector containing the component-wise minimum of v0 and v1.
 */
static inline NX_Vec3 NX_Vec3Min(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_VEC3(
        fminf(v0.x, v1.x),
        fminf(v0.y, v1.y),
        fminf(v0.z, v1.z)
    );
}

/**
 * @brief Returns a vector containing the component-wise maximum of v0 and v1.
 */
static inline NX_Vec3 NX_Vec3Max(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_VEC3(
        fmaxf(v0.x, v1.x),
        fmaxf(v0.y, v1.y),
        fmaxf(v0.z, v1.z)
    );
}

/**
 * @brief Clamp each component of vector x to [min,max].
 */
static inline NX_Vec3 NX_Vec3Clamp(NX_Vec3 v, NX_Vec3 min, NX_Vec3 max)
{
    return NX_VEC3(
        fmaxf(fminf(v.x, max.x), min.x),
        fmaxf(fminf(v.y, max.y), min.y),
        fmaxf(fminf(v.z, max.z), min.z)
    );
}

/**
 * @brief Component-wise absolute value
 */
static inline NX_Vec3 NX_Vec3Abs(NX_Vec3 v)
{
    return NX_VEC3(fabsf(v.x), fabsf(v.y), fabsf(v.z));
}

/**
 * @brief Negate vector
 */
static inline NX_Vec3 NX_Vec3Neg(NX_Vec3 v)
{
    return NX_VEC3(-v.x, -v.y, -v.z);
}

/**
 * @brief Reciprocal of vector components
 */
static inline NX_Vec3 NX_Vec3Rcp(NX_Vec3 v)
{
    return NX_VEC3(1.0f / v.x, 1.0f / v.y, 1.0f / v.z);
}

/**
 * @brief Approximate equality within epsilon
 */
static inline int NX_Vec3Approx(NX_Vec3 v0, NX_Vec3 v1, float epsilon)
{
    return (fabsf(v0.x - v1.x) < epsilon) &&
           (fabsf(v0.y - v1.y) < epsilon) &&
           (fabsf(v0.z - v1.z) < epsilon);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline NX_IVec3 NX_Vec3Equals(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_IVEC3(v0.x == v1.x, v0.y == v1.y, v0.z == v1.z);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline NX_IVec3 NX_Vec3GreaterThan(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_IVEC3(v0.x > v1.x, v0.y > v1.y, v0.z > v1.z);
}

/**
 * @brief Vector addition
 */
static inline NX_Vec3 NX_Vec3Add(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_VEC3(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

/**
 * @brief Vector subtraction
 */
static inline NX_Vec3 NX_Vec3Sub(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_VEC3(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

/**
 * @brief Component-wise multiplication
 */
static inline NX_Vec3 NX_Vec3Mul(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_VEC3(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z);
}

/**
 * @brief Component-wise division
 */
static inline NX_Vec3 NX_Vec3Div(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_VEC3(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z);
}

/**
 * @brief Add scalar to each component
 */
static inline NX_Vec3 NX_Vec3Offset(NX_Vec3 v, float s)
{
    return NX_VEC3(v.x + s, v.y + s, v.z + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline NX_Vec3 NX_Vec3Scale(NX_Vec3 v, float s)
{
    return NX_VEC3(v.x * s, v.y * s, v.z * s);
}

/**
 * @brief Scalar multiply and add
 */
static inline NX_Vec3 NX_Vec3MulAdd(NX_Vec3 a, float s, NX_Vec3 b)
{
    return NX_VEC3(
        fmaf(a.x, s, b.x),
        fmaf(a.y, s, b.y),
        fmaf(a.z, s, b.z)
    );
}

/**
 * @brief Dot product
 */
static inline float NX_Vec3Dot(NX_Vec3 v0, NX_Vec3 v1)
{
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

/**
 * @brief Cross product
 */
static inline NX_Vec3 NX_Vec3Cross(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_VEC3(
        v0.y * v1.z - v0.z * v1.y,
        v0.z * v1.x - v0.x * v1.z,
        v0.x * v1.y - v0.y * v1.x
    );
}

/**
 * @brief Vector length (magnitude)
 */
static inline float NX_Vec3Length(NX_Vec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * @brief Squared length (avoids sqrt)
 */
static inline float NX_Vec3LengthSq(NX_Vec3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

/**
 * @brief Distance between two vectors
 */
static inline float NX_Vec3Distance(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_Vec3Length(NX_Vec3Sub(v1, v0));
}

/**
 * @brief Squared distance
 */
static inline float NX_Vec3DistanceSq(NX_Vec3 v0, NX_Vec3 v1)
{
    return NX_Vec3LengthSq(NX_Vec3Sub(v1, v0));
}

/**
 * @brief Normalize vector
 */
static inline NX_Vec3 NX_Vec3Normalize(NX_Vec3 v)
{
    float len = NX_Vec3Length(v);
    return (len > 0.0f) ? NX_Vec3Scale(v, 1.0f / len) : NX_VEC3(0.0f, 0.0f, 0.0f);
}

/**
 * @brief Rotate a vector by a quaternion
 */
static inline NX_Vec3 NX_Vec3Rotate(NX_Vec3 v, NX_Quat q)
{
    NX_Vec3 qv = { q.x, q.y, q.z };

    NX_Vec3 t;
    t.x = 2.0f * (qv.y * v.z - qv.z * v.y);
    t.y = 2.0f * (qv.z * v.x - qv.x * v.z);
    t.z = 2.0f * (qv.x * v.y - qv.y * v.x);

    NX_Vec3 res;
    res.x = v.x + q.w * t.x + (qv.y * t.z - qv.z * t.y);
    res.y = v.y + q.w * t.y + (qv.z * t.x - qv.x * t.z);
    res.z = v.z + q.w * t.z + (qv.x * t.y - qv.y * t.x);

    return res;
}

/**
 * @brief Rotate a vector by Euler angles (yaw, pitch, roll)
 */
static inline NX_Vec3 NX_Vec3RotateEuler(NX_Vec3 v, float yaw, float pitch, float roll)
{
    float cy = cosf(yaw * 0.5f), sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f), sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f), sr = sinf(roll * 0.5f);

    NX_Quat q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return NX_Vec3Rotate(v, q);
}

/**
 * @brief Rotate a vector around a given axis by a specified angle
 */
static inline NX_Vec3 NX_Vec3RotateAxisAngle(NX_Vec3 v, NX_Vec3 axis, float angle)
{
    axis = NX_Vec3Normalize(axis);
    float s = sinf(angle * 0.5f);
    NX_Quat q;
    q.w = cosf(angle * 0.5f);
    q.x = axis.x * s;
    q.y = axis.y * s;
    q.z = axis.z * s;

    return NX_Vec3Rotate(v, q);
}

/**
 * @brief Direction vector from 'from' to 'to'
 */
static inline NX_Vec3 NX_Vec3Direction(NX_Vec3 from, NX_Vec3 to)
{
    return NX_Vec3Normalize(NX_Vec3Sub(to, from));
}

/**
 * @brief Linear interpolation between two vectors
 */
static inline NX_Vec3 NX_Vec3Lerp(NX_Vec3 v0, NX_Vec3 v1, float t)
{
    float w1 = 1.0f - t;
    float w2 = t;

    NX_Vec3 result;
    result.x = fmaf(w1, v0.x, w2 * v1.x);
    result.y = fmaf(w1, v0.y, w2 * v1.y);
    result.z = fmaf(w1, v0.z, w2 * v1.z);

    return result;
}

/**
 * @brief Move vector toward target by max_delta
 */
static inline NX_Vec3 NX_Vec3MoveToward(NX_Vec3 from, NX_Vec3 to, float max_delta)
{
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float dz = to.z - from.z;

    float distSq = dx * dx + dy * dy + dz * dz;
    float maxDeltaSq = max_delta * max_delta;

    if (distSq <= maxDeltaSq) {
        return to;
    }

    float dist = sqrtf(distSq);
    float ratio = max_delta / dist;

    NX_Vec3 result;
    result.x = fmaf(dx, ratio, from.x);
    result.y = fmaf(dy, ratio, from.y);
    result.z = fmaf(dz, ratio, from.z);

    return result;
}

/**
 * @brief Reflect vector v around normal
 * Formula: v - 2 * dot(v, normal) * normal
 */
static inline NX_Vec3 NX_Vec3Reflect(NX_Vec3 v, NX_Vec3 normal)
{
    float dot = v.x * normal.x + v.y * normal.y + v.z * normal.z;
    float factor = 2.0f * dot;

    NX_Vec3 result;
    result.x = fmaf(-factor, normal.x, v.x);
    result.y = fmaf(-factor, normal.y, v.y);
    result.z = fmaf(-factor, normal.z, v.z);

    return result;
}

/**
 * @brief Project vector v onto another vector
 * Formula: onto * (dot(v, onto) / lengthSq(onto))
 */
static inline NX_Vec3 NX_Vec3Project(NX_Vec3 v, NX_Vec3 onto)
{
    float dot = v.x * onto.x + v.y * onto.y + v.z * onto.z;
    float lenSq = onto.x * onto.x + onto.y * onto.y + onto.z * onto.z;
    float factor = dot / lenSq;

    NX_Vec3 result;
    result.x = onto.x * factor;
    result.y = onto.y * factor;
    result.z = onto.z * factor;

    return result;
}

/**
 * @brief Reject vector v from another vector (component perpendicular)
 * Formula: v - project(v, onto)
 */
static inline NX_Vec3 NX_Vec3Reject(NX_Vec3 v, NX_Vec3 onto)
{
    float dot = v.x * onto.x + v.y * onto.y + v.z * onto.z;
    float lenSq = onto.x * onto.x + onto.y * onto.y + onto.z * onto.z;
    float factor = dot / lenSq;

    NX_Vec3 result;
    result.x = fmaf(-factor, onto.x, v.x);
    result.y = fmaf(-factor, onto.y, v.y);
    result.z = fmaf(-factor, onto.z, v.z);

    return result;
}

/**
 * @brief Transform a 3D vector by TRS transform
 */
static inline NX_Vec3 NX_Vec3Transform(NX_Vec3 vec, const NX_Transform* trs)
{
    vec = NX_Vec3Mul(vec, trs->scale);
    vec = NX_Vec3Rotate(vec, trs->rotation);
    vec = NX_Vec3Add(vec, trs->translation);

    return vec;
}

/**
 * @brief Transform a 3D vector by 3x3 matrix (rotation/scale only)
 */
static inline NX_Vec3 NX_Vec3TransformByMat3(NX_Vec3 vec, const NX_Mat3* mat)
{
    float vx = vec.x;
    float vy = vec.y;
    float vz = vec.z;

    NX_Vec3 result;
    result.x = fmaf(mat->m00, vx, fmaf(mat->m10, vy, mat->m20 * vz));
    result.y = fmaf(mat->m01, vx, fmaf(mat->m11, vy, mat->m21 * vz));
    result.z = fmaf(mat->m02, vx, fmaf(mat->m12, vy, mat->m22 * vz));

    return result;
}

/**
 * @brief Transform a 3D vector by 4x4 matrix
 */
static inline NX_Vec3 NX_Vec3TransformByMat4(NX_Vec3 v, const NX_Mat4* mat)
{
    float vx = v.x;
    float vy = v.y;
    float vz = v.z;

    NX_Vec3 result;
    result.x = fmaf(mat->m00, vx, fmaf(mat->m10, vy, fmaf(mat->m20, vz, mat->m30)));
    result.y = fmaf(mat->m01, vx, fmaf(mat->m11, vy, fmaf(mat->m21, vz, mat->m31)));
    result.z = fmaf(mat->m02, vx, fmaf(mat->m12, vy, fmaf(mat->m22, vz, mat->m32)));

    return result;
}

/** @} */ // Vec3

/* === 4D Vector Functions === */

/** @defgroup Vec4 4D Vector Functions
 *  Inline functions for 4D vector operations.
 *  @{
 */

/**
 * @brief Returns a vector containing the component-wise minimum of v0 and v1.
 */
static inline NX_Vec4 NX_Vec4Min(NX_Vec4 v0, NX_Vec4 v1)
{
    v0.x = fminf(v0.x, v1.x);
    v0.y = fminf(v0.y, v1.y);
    v0.z = fminf(v0.z, v1.z);
    v0.w = fminf(v0.w, v1.w);

    return v0;
}

/**
 * @brief Returns a vector containing the component-wise maximum of v0 and v1.
 */
static inline NX_Vec4 NX_Vec4Max(NX_Vec4 v0, NX_Vec4 v1)
{
    v0.x = fmaxf(v0.x, v1.x);
    v0.y = fmaxf(v0.y, v1.y);
    v0.z = fmaxf(v0.z, v1.z);
    v0.w = fmaxf(v0.w, v1.w);

    return v0;
}

/**
 * @brief Clamp each component of vector x to [min,max].
 */
static inline NX_Vec4 NX_Vec4Clamp(NX_Vec4 v, NX_Vec4 min, NX_Vec4 max)
{
    v.x = fmaxf(fminf(v.x, max.x), min.x);
    v.y = fmaxf(fminf(v.y, max.y), min.y);
    v.z = fmaxf(fminf(v.z, max.z), min.z);
    v.w = fmaxf(fminf(v.w, max.w), min.w);

    return v;
}

/**
 * @brief Absolute value of each component.
 */
static inline NX_Vec4 NX_Vec4Abs(NX_Vec4 v)
{
    v.x = fabsf(v.x);
    v.y = fabsf(v.y);
    v.z = fabsf(v.z);
    v.w = fabsf(v.w);

    return v;
}

/**
 * @brief Negate each component.
 */
static inline NX_Vec4 NX_Vec4Neg(NX_Vec4 v)
{
    v.x = -v.x;
    v.y = -v.y;
    v.z = -v.z;
    v.w = -v.w;

    return v;
}

/**
 * @brief Reciprocal of vector components
 */
static inline NX_Vec4 NX_Vec4Rcp(NX_Vec4 v)
{
    v.x = 1.0f / v.x;
    v.y = 1.0f / v.y;
    v.z = 1.0f / v.z;
    v.w = 1.0f / v.w;

    return v;
}

/**
 * @brief Test approximate equality between two vectors, within epsilon tolerance.
 */
static inline bool NX_Vec4Approx(NX_Vec4 v0, NX_Vec4 v1, float epsilon)
{
    return (fabsf(v0.x - v1.x) < epsilon) &&
           (fabsf(v0.y - v1.y) < epsilon) &&
           (fabsf(v0.z - v1.z) < epsilon) &&
           (fabsf(v0.w - v1.w) < epsilon);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline NX_IVec4 NX_Vec4Equals(NX_Vec4 v0, NX_Vec4 v1)
{
    return NX_IVEC4(v0.x == v1.x, v0.y == v1.y, v0.z == v1.z, v0.w == v1.w);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline NX_IVec4 NX_Vec4GreaterThan(NX_Vec4 v0, NX_Vec4 v1)
{
    NX_IVec4 result;
    result.x = (v0.x > v1.x);
    result.y = (v0.y > v1.y);
    result.z = (v0.z > v1.z);
    result.w = (v0.x > v1.w);

    return result;
}

/**
 * @brief Add two vectors (component-wise).
 */
static inline NX_Vec4 NX_Vec4Add(NX_Vec4 v0, NX_Vec4 v1)
{
    v0.x += v1.x;
    v0.y += v1.y;
    v0.z += v1.z;
    v0.w += v1.w;

    return v0;
}

/**
 * @brief Subtract two vectors (component-wise).
 */
static inline NX_Vec4 NX_Vec4Sub(NX_Vec4 v0, NX_Vec4 v1)
{
    v0.x -= v1.x;
    v0.y -= v1.y;
    v0.z -= v1.z;
    v0.w -= v1.w;

    return v0;
}

/**
 * @brief Multiply two vectors (component-wise).
 */
static inline NX_Vec4 NX_Vec4Mul(NX_Vec4 v0, NX_Vec4 v1)
{
    v0.x *= v1.x;
    v0.y *= v1.y;
    v0.z *= v1.z;
    v0.w *= v1.w;

    return v0;
}

/**
 * @brief Divide two vectors (component-wise).
 */
static inline NX_Vec4 NX_Vec4Div(NX_Vec4 v0, NX_Vec4 v1)
{
    v0.x /= v1.x;
    v0.y /= v1.y;
    v0.z /= v1.z;
    v0.w /= v1.w;

    return v0;
}

/**
 * @brief Add scalar to each component.
 */
static inline NX_Vec4 NX_Vec4Offset(NX_Vec4 v, float s)
{
    v.x += s;
    v.y += s;
    v.z += s;
    v.w += s;

    return v;
}

/**
 * @brief Multiply each component by scalar.
 */
static inline NX_Vec4 NX_Vec4Scale(NX_Vec4 v, float s)
{
    v.x *= s;
    v.y *= s;
    v.z *= s;
    v.w *= s;

    return v;
}

/**
 * @brief Scalar multiply and add
 */
static inline NX_Vec4 NX_Vec4MulAdd(NX_Vec4 a, float s, NX_Vec4 b)
{
    a.x = fmaf(a.x, s, b.x);
    a.y = fmaf(a.y, s, b.y);
    a.z = fmaf(a.z, s, b.z);
    a.w = fmaf(a.w, s, b.w);

    return a;
}

/**
 * @brief Normalize vector (length = 1). Returns NX_VEC4_ZERO if length is too small.
 */
static inline NX_Vec4 NX_Vec4Normalize(NX_Vec4 v)
{
    float lenSq = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    float mask = (lenSq > 1e-8f) ? 1.0f : 0.0f;

    float invLen = mask * (1.0f / sqrtf(lenSq + (1.0f - mask)));

    NX_Vec4 result;
    result.x = v.x * invLen;
    result.y = v.y * invLen;
    result.z = v.z * invLen;
    result.w = v.w * invLen;

    return result;
}

/**
 * @brief Compute vector length.
 */
static inline float NX_Vec4Length(NX_Vec4 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

/**
 * @brief Compute squared vector length.
 */
static inline float NX_Vec4LengthSq(NX_Vec4 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

/**
 * @brief Compute dot product between v1 and v2.
 */
static inline float NX_Vec4Dot(NX_Vec4 v1, NX_Vec4 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

/**
 * @brief Linear interpolation between v1 and v2.
 */
static inline NX_Vec4 NX_Vec4Lerp(NX_Vec4 v1, NX_Vec4 v2, float t)
{
    float w1 = 1.0f - t;
    float w2 = t;

    NX_Vec4 result;
    result.x = fmaf(w2, v2.x, w1 * v1.x);
    result.y = fmaf(w2, v2.y, w1 * v1.y);
    result.z = fmaf(w2, v2.z, w1 * v1.z);
    result.w = fmaf(w2, v2.w, w1 * v1.w);

    return result;
}

/**
 * @brief Move current vector toward target vector by at most maxDelta.
 */
static inline NX_Vec4 NX_Vec4MoveToward(NX_Vec4 current, NX_Vec4 target, float maxDelta)
{
    float dx = target.x - current.x;
    float dy = target.y - current.y;
    float dz = target.z - current.z;
    float dw = target.w - current.w;

    float distSq = dx * dx + dy * dy + dz * dz + dw * dw;
    float maxDeltaSq = maxDelta * maxDelta;

    if (distSq <= maxDeltaSq) {
        return target;
    }

    float dist = sqrtf(distSq);
    float ratio = maxDelta / dist;

    NX_Vec4 result;
    result.x = fmaf(dx, ratio, current.x);
    result.y = fmaf(dy, ratio, current.y);
    result.z = fmaf(dz, ratio, current.z);
    result.w = fmaf(dw, ratio, current.w);

    return result;
}

/**
 * @brief Transform vector by a 4x4 matrix.
 */
static inline NX_Vec4 NX_Vec4TransformByMat4(NX_Vec4 v, const NX_Mat4* mat)
{
    NX_Vec4 result;
    result.x = fmaf(mat->m00, v.x, fmaf(mat->m10, v.y, fmaf(mat->m20, v.z, mat->m30 * v.w)));
    result.y = fmaf(mat->m01, v.x, fmaf(mat->m11, v.y, fmaf(mat->m21, v.z, mat->m31 * v.w)));
    result.z = fmaf(mat->m02, v.x, fmaf(mat->m12, v.y, fmaf(mat->m22, v.z, mat->m32 * v.w)));
    result.w = fmaf(mat->m03, v.x, fmaf(mat->m13, v.y, fmaf(mat->m23, v.z, mat->m33 * v.w)));

    return result;
}

/** @} */ // Vec4

/* === Color Functions === */

/** @defgroup Color Color Functions
 *  Inline functions for color operations.
 *  @{
 */

/**
 * @brief Creates a color from 8-bit RGBA components.
 * @param r Red component (0-255).
 * @param g Green component (0-255).
 * @param b Blue component (0-255).
 * @param a Alpha component (0-255).
 * @return NX_Color with normalized float components (0.0-1.0).
 */
static inline NX_Color NX_Color8(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return NX_COLOR(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

/**
 * @brief Creates a color from a 32-bit hexadecimal value.
 * @param hex Hexadecimal color in 0xRRGGBBAA format.
 * @return NX_Color with normalized float components (0.0-1.0).
 */
static inline NX_Color NX_ColorFromHex(unsigned int hex)
{
    return NX_COLOR(
        ((hex >> 24) & 0xFF) / 255.0f,
        ((hex >> 16) & 0xFF) / 255.0f,
        ((hex >> 8) & 0xFF) / 255.0f,
        (hex & 0xFF) / 255.0f
    );
}

/**
 * @brief Converts an NX_Color to a 32-bit hexadecimal value.
 * @param color NX_Color to convert.
 * @return Hexadecimal color in 0xRRGGBBAA format.
 * @note Components are clamped to the [0.0, 1.0] range before conversion.
 */
static inline unsigned int NX_ColorToHex(NX_Color color)
{
    uint8_t r = (uint8_t)(NX_CLAMP(color.r, 0.0f, 1.0f) * 255.0f);
    uint8_t g = (uint8_t)(NX_CLAMP(color.g, 0.0f, 1.0f) * 255.0f);
    uint8_t b = (uint8_t)(NX_CLAMP(color.b, 0.0f, 1.0f) * 255.0f);
    uint8_t a = (uint8_t)(NX_CLAMP(color.a, 0.0f, 1.0f) * 255.0f);
    return (r << 24) | (g << 16) | (b << 8) | a;
}

/**
 * @brief Converts an NX_Color to a 3-component vector (RGB).
 * @param color NX_Color to convert.
 * @return NX_Vec3 containing the red, green, and blue components.
 */
static inline NX_Vec3 NX_ColorToVec3(NX_Color color)
{
    return NX_VEC3(color.r, color.g, color.b);
}

/**
 * @brief Converts an NX_Color to a 4-component vector (RGBA).
 * @param color NX_Color to convert.
 * @return NX_Vec4 containing the red, green, blue, and alpha components.
 */
static inline NX_Vec4 NX_ColorToVec4(NX_Color color)
{
    return NX_VEC4(color.r, color.g, color.b, color.a);
}

/**
 * @brief Checks if a color has components out of the [0.0, 1.0] range.
 * @param color NX_Color to check.
 * @return true if any component (r, g, b) is out of range, false otherwise.
 * @note Alpha component is ignored in this check.
 */
static inline bool NX_IsColorOutOfRange(NX_Color color)
{
    return (color.r < 0.0f || color.r > 1.0f) ||
           (color.g < 0.0f || color.g > 1.0f) ||
           (color.b < 0.0f || color.b > 1.0f);
}

/**
 * @brief Clamps all components of a color to the [0.0, 1.0] range.
 * @param color NX_Color to clamp.
 * @return NX_Color with all components clamped.
 */
static inline NX_Color NX_ColorClamp(NX_Color color)
{
    return NX_COLOR(
        NX_CLAMP(color.r, 0.0f, 1.0f),
        NX_CLAMP(color.g, 0.0f, 1.0f),
        NX_CLAMP(color.b, 0.0f, 1.0f),
        NX_CLAMP(color.a, 0.0f, 1.0f)
    );
}

/**
 * @brief Normalizes the RGB components of a color if any exceeds 1.0.
 * @param color NX_Color to normalize.
 * @return NX_Color with RGB components scaled to a maximum of 1.0 and alpha clamped.
 * @note If RGB components are all within [0.0, 1.0], this function behaves like NX_ColorClamp.
 */
static inline NX_Color NX_ColorNormalize(NX_Color color)
{
    float max = NX_MAX3(color.r, color.g, color.b);
    if (max > 1.0f) {
        float invMax = 1.0f / max;
        color.r *= invMax;
        color.g *= invMax;
        color.b *= invMax;
        color.a = NX_CLAMP(color.a, 0.0f, 1.0f);
        return color;
    }
    return NX_ColorClamp(color);
}

/**
 * @brief Adds the RGB components of two colors.
 * @param a First color.
 * @param b Second color.
 * @return NX_Color with summed RGB components; alpha from the first color.
 */
static inline NX_Color NX_ColorAdd(NX_Color a, NX_Color b)
{
    return NX_COLOR(a.r + b.r, a.g + b.g, a.b + b.b, a.a);
}

/**
 * @brief Subtracts the RGB components of one color from another.
 * @param a First color.
 * @param b Second color to subtract.
 * @return NX_Color with subtracted RGB components; alpha from the first color.
 */
static inline NX_Color NX_ColorSub(NX_Color a, NX_Color b)
{
    return NX_COLOR(a.r - b.r, a.g - b.g, a.b - b.b, a.a);
}

/**
 * @brief Multiplies the RGB components of two colors.
 * @param a First color.
 * @param b Second color.
 * @return NX_Color with multiplied RGB components; alpha from the first color.
 */
static inline NX_Color NX_ColorMul(NX_Color a, NX_Color b)
{
    return NX_COLOR(a.r * b.r, a.g * b.g, a.b * b.b, a.a);
}

/**
 * @brief Divides the RGB components of one color by another.
 * @param a Numerator color.
 * @param b Denominator color.
 * @return NX_Color with divided RGB components; alpha from the first color.
 * @note Division by zero is not handled.
 */
static inline NX_Color NX_ColorDiv(NX_Color a, NX_Color b)
{
    return NX_COLOR(a.r / b.r, a.g / b.g, a.b / b.b, a.a);
}

/**
 * @brief Adds an offset to the RGB components of a color.
 * @param color Color to modify.
 * @param offset Value to add to each RGB component.
 * @return NX_Color with offset RGB components; alpha unchanged.
 */
static inline NX_Color NX_ColorOffset(NX_Color color, float offset)
{
    return NX_COLOR(color.r + offset, color.g + offset, color.b + offset, color.a);
}

/**
 * @brief Scales the RGB components of a color by a factor.
 * @param color Color to scale.
 * @param factor Multiplicative factor.
 * @return NX_Color with scaled RGB components; alpha unchanged.
 */
static inline NX_Color NX_ColorScale(NX_Color color, float factor)
{
    return NX_COLOR(color.r * factor, color.g * factor, color.b * factor, color.a);
}

/**
 * @brief Checks if two colors are approximately equal within a tolerance.
 * @param c0 First color.
 * @param c1 Second color.
 * @param epsilon Maximum allowed difference per component.
 * @return true if all components differ by less than epsilon, false otherwise.
 */
static inline bool NX_ColorApprox(NX_Color c0, NX_Color c1, float epsilon)
{
    return (fabsf(c0.r - c1.r) < epsilon) &&
           (fabsf(c0.g - c1.g) < epsilon) &&
           (fabsf(c0.b - c1.b) < epsilon) &&
           (fabsf(c0.a - c1.a) < epsilon);
}

/**
 * @brief Linearly interpolates between two colors.
 * @param a Starting color.
 * @param b Ending color.
 * @param t Interpolation factor in [0.0, 1.0].
 * @return Interpolated color between a and b.
 */
static inline NX_Color NX_ColorLerp(NX_Color a, NX_Color b, float t)
{
    float invT = 1.0f - t;
    return NX_COLOR(
        a.r * invT + b.r * t,
        a.g * invT + b.g * t,
        a.b * invT + b.b * t,
        a.a * invT + b.a * t
    );
}

/**
 * @brief Converts an RGB color to HSV representation.
 * @param color Input RGB color (components in [0.0, 1.0]).
 * @return NX_Vec3 representing HSV (hue in degrees [0, 360), saturation [0.0-1.0], value [0.0-1.0]).
 */
static inline NX_Vec3 NX_ColorToHSV(NX_Color color)
{
    float maxVal = NX_MAX3(color.r, color.g, color.b);
    float minVal = NX_MIN3(color.r, color.g, color.b);
    float delta = maxVal - minVal;

    if (maxVal == 0.0f || delta == 0.0f) {
        return NX_VEC3(0, 0, maxVal);
    }

    float s = delta / maxVal;
    float h = 0.0f;

    if (maxVal == color.r) {
        h = 60.0f * ((color.g - color.b) / delta);
        if (h < 0.0f) h += 360.0f;
    }
    else if (maxVal == color.g) {
        h = 60.0f * ((color.b - color.r) / delta) + 120.0f;
    }
    else {
        h = 60.0f * ((color.r - color.g) / delta) + 240.0f;
    }

    return NX_VEC3(h, s, maxVal);
}

/**
 * @brief Creates an RGB color from HSV components.
 * @param h Hue in degrees [0, 360).
 * @param s Saturation [0.0-1.0].
 * @param v Value [0.0-1.0].
 * @param a Alpha [0.0-1.0].
 * @return NX_Color converted from HSV to RGB.
 */
static inline NX_Color NX_ColorFromHSV(float h, float s, float v, float a)
{
    if (s == 0.0f) {
        return NX_COLOR(v, v, v, a);
    }

    float h_sector = h / 60.0f;
    int sector = (int)h_sector;
    float f = h_sector - sector;

    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (sector % 6) {
        case 0: return NX_COLOR(v, t, p, a);
        case 1: return NX_COLOR(q, v, p, a);
        case 2: return NX_COLOR(p, v, t, a);
        case 3: return NX_COLOR(p, q, v, a);
        case 4: return NX_COLOR(t, p, v, a);
        case 5: return NX_COLOR(v, p, q, a);
        default: break;
    }

    return NX_COLOR(v, v, v, a);
}

/**
 * @brief Converts an RGB color to HSL representation.
 * @param color Input RGB color (components in [0.0, 1.0]).
 * @return NX_Vec3 representing HSL (hue in degrees [0, 360), saturation [0.0-1.0], lightness [0.0-1.0]).
 */
static inline NX_Vec3 NX_ColorToHSL(NX_Color color)
{
    float maxVal = NX_MAX3(color.r, color.g, color.b);
    float minVal = NX_MIN3(color.r, color.g, color.b);
    float delta = maxVal - minVal;

    float l = (maxVal + minVal) * 0.5f;

    if (delta == 0.0f) {
        return NX_VEC3(0, 0, l);
    }

    float s = (l > 0.5f) ? delta / (2.0f - maxVal - minVal) : delta / (maxVal + minVal);
    float h = 0.0f;

    if (maxVal == color.r) {
        h = 60.0f * ((color.g - color.b) / delta);
        if (h < 0.0f) h += 360.0f;
    }
    else if (maxVal == color.g) {
        h = 60.0f * ((color.b - color.r) / delta) + 120.0f;
    }
    else {
        h = 60.0f * ((color.r - color.g) / delta) + 240.0f;
    }

    return NX_VEC3(h, s, l);
}

/**
 * @brief Creates an RGB color from HSL components.
 * @param h Hue in degrees [0, 360).
 * @param s Saturation [0.0-1.0].
 * @param l Lightness [0.0-1.0].
 * @param a Alpha [0.0-1.0].
 * @return NX_Color converted from HSL to RGB.
 */
static inline NX_Color NX_ColorFromHSL(float h, float s, float l, float a)
{
    if (s == 0.0f) {
        return NX_COLOR(l, l, l, a);
    }

    float hNorm = h / 360.0f;
    float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
    float p = 2.0f * l - q;
    float diff = q - p;

    // cry with me...
    #define HSL_TO_RGB(p, q, diff, t) \
            (((t) < 0.0f ? (t) + 1.0f : ((t) > 1.0f ? (t) - 1.0f : (t))) < 1.0f/6.0f ? (p) + (diff) * 6.0f * ((t) < 0.0f ? (t) + 1.0f : ((t) > 1.0f ? (t) - 1.0f : (t))) : \
            (((t) < 0.0f ? (t) + 1.0f : ((t) > 1.0f ? (t) - 1.0f : (t))) < 1.0f/2.0f ? (q) : \
            (((t) < 0.0f ? (t) + 1.0f : ((t) > 1.0f ? (t) - 1.0f : (t))) < 2.0f/3.0f ? (p) + (diff) * (2.0f/3.0f - ((t) < 0.0f ? (t) + 1.0f : ((t) > 1.0f ? (t) - 1.0f : (t)))) * 6.0f : (p))))

    float r = HSL_TO_RGB(p, q, diff, hNorm + 1.0f / 3.0f);
    float g = HSL_TO_RGB(p, q, diff, hNorm);
    float b = HSL_TO_RGB(p, q, diff, hNorm - 1.0f / 3.0f);

    #undef HSL_TO_RGB

    return NX_COLOR(r, g, b, a);
}

/**
 * @brief Computes the relative luminance of a color.
 * @param color Input color.
 * @return Luminance value in [0.0, 1.0].
 */
static inline float NX_GetColorLuminance(NX_Color color)
{
    return 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
}

/**
 * @brief Computes the brightness of a color as the maximum RGB component.
 * @param color Input color.
 * @return Brightness value in [0.0, 1.0].
 */
static inline float NX_GetColorBrightness(NX_Color color)
{
    return NX_MAX3(color.r, color.g, color.b);
}

/**
 * @brief Converts a color to grayscale using luminance.
 * @param color Input color.
 * @return Grayscale color with preserved alpha.
 */
static inline NX_Color NX_GetColorGrayscale(NX_Color color)
{
    float gray = NX_GetColorLuminance(color);
    return NX_COLOR(gray, gray, gray, color.a);
}

/**
 * @brief Inverts the RGB components of a color.
 * @param color Input color.
 * @return Inverted color with preserved alpha.
 */
static inline NX_Color NX_ColorInvert(NX_Color color)
{
    return NX_COLOR(1.0f - color.r, 1.0f - color.g, 1.0f - color.b, color.a);
}

/** @} */ // Color

/* === Quaternion Functions === */

/** @defgroup Quat Quaternion Functions
 *  Functions for quaternion creation, manipulation and interpolation.
 *  @{
 */

/**
 * @brief Create a quaternion from an axis and an angle in radians.
 */
static inline NX_Quat NX_QuatFromAxisAngle(NX_Vec3 axis, float radians)
{
    NX_Quat result;

    float half = radians * 0.5f;
    float s = sinf(half);
    float c = cosf(half);

    result.w = c;
    result.x = axis.x * s;
    result.y = axis.y * s;
    result.z = axis.z * s;

    return result;
}

/**
 * @brief Create a quaternion from Euler angles (pitch, yaw, roll).
 */
NXAPI NX_Quat NX_QuatFromEuler(NX_Vec3 v);

/**
 * @brief Convert a quaternion to Euler angles (pitch, yaw, roll).
 */
NXAPI NX_Vec3 NX_QuatToEuler(NX_Quat q);

/**
 * @brief Get the pitch (X-axis rotation) from a quaternion.
 */
static inline float NX_QuatPitch(NX_Quat q)
{
    float sinp = 2.0f * (q.w * q.x - q.y * q.z);
    if (fabsf(sinp) >= 1.0f) {
        return copysignf(NX_PI * 0.5f, sinp);
    }
    return asinf(sinp);
}

/**
 * @brief Get the yaw (Y-axis rotation) from a quaternion.
 */
static inline float NX_QuatYaw(NX_Quat q)
{
    float qxx = q.x * q.x;
    float qyy = q.y * q.y;
    
    float sinYcosP = 2.0f * (q.w * q.y + q.x * q.z);
    float cosYcosP = 1.0f - 2.0f * (qxx + qyy);
    return atan2f(sinYcosP, cosYcosP);
}

/**
 * @brief Get the roll (Z-axis rotation) from a quaternion.
 */
static inline float NX_QuatRoll(NX_Quat q)
{
    float qxx = q.x * q.x;
    float qzz = q.z * q.z;
    
    float sinRcosP = 2.0f * (q.w * q.z + q.x * q.y);
    float cosRcosP = 1.0f - 2.0f * (qxx + qzz);
    return atan2f(sinRcosP, cosRcosP);
}

/**
 * @brief Create a quaternion from a 4x4 rotation matrix.
 */
NXAPI NX_Quat NX_QuatFromMat4(const NX_Mat4* m);

/**
 * @brief Convert a quaternion to a 4x4 rotation matrix.
 */
NXAPI NX_Mat4 NX_QuatToMat4(NX_Quat q);

/**
 * @brief Compute a quaternion that rotates the forward vector (-Z) to the given direction.
 */
NXAPI NX_Quat NX_QuatLookTo(NX_Vec3 direction, NX_Vec3 up);

/**
 * @brief Compute a quaternion that rotates from the world forward (-Z) to point from 'from' to 'to'.
 */
NXAPI NX_Quat NX_QuatLookAt(NX_Vec3 eye, NX_Vec3 target, NX_Vec3 up);

/**
 * @brief Returns the forward direction (-Z) of the quaternion
 */
static inline NX_Vec3 NX_QuatForward(NX_Quat q)
{
    return NX_Vec3Rotate(NX_VEC3(0, 0, -1), q);
}

/**
 * @brief Returns the right direction (+X) of the quaternion
 */
static inline NX_Vec3 NX_QuatRight(NX_Quat q)
{
    return NX_Vec3Rotate(NX_VEC3(1, 0, 0), q);
}

/**
 * @brief Returns the up direction (+Y) of the quaternion
 */
static inline NX_Vec3 NX_QuatUp(NX_Quat q)
{
    return NX_Vec3Rotate(NX_VEC3(0, 1, 0), q);
}

/**
 * @brief Compute the length (magnitude) of a quaternion.
 */
static inline float NX_QuatLength(NX_Quat q)
{
    return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

/**
 * @brief Normalize a quaternion to unit length.
 */
static inline NX_Quat NX_QuatNormalize(NX_Quat q)
{
    float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (lenSq < 1e-4f) return NX_QUAT_IDENTITY;

    float inv_len = 1.0f / sqrtf(lenSq);
    for (int i = 0; i < 4; ++i) {
        q.v[i] *= inv_len;
    }

    return q;
}

/**
 * @brief Conjugate of a quaternion.
 * Equivalent to negating the vector part (x,y,z).
 */
static inline NX_Quat NX_QuatConjugate(NX_Quat q)
{
    return NX_QUAT(q.w, -q.x, -q.y, -q.z);
}

/**
 * @brief Inverse of a quaternion.
 */
static inline NX_Quat NX_QuatInverse(NX_Quat q)
{
    float lenSq = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;
    if (lenSq < 1e-4f) {
        return q;
    }

    float invLenSq = 1.0f / lenSq;

    q.w = q.w * +invLenSq;
    q.x = q.x * -invLenSq;
    q.y = q.y * -invLenSq;
    q.z = q.z * -invLenSq;

    return q;
}

/**
 * @brief Multiply two quaternions (Hamilton product).
 */
static inline NX_Quat NX_QuatMul(NX_Quat a, NX_Quat b)
{
    NX_Quat result;
    result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;

    return result;
}

/**
 * @brief Linear interpolation between two quaternions.
 */
NXAPI NX_Quat NX_QuatLerp(NX_Quat a, NX_Quat b, float t);

/**
 * @brief Spherical linear interpolation (slerp) between two quaternions.
 */
NXAPI NX_Quat NX_QuatSLerp(NX_Quat a, NX_Quat b, float t);

/** @} */ // Quat

/* === Matrix 3x3 Functions === */

/** @defgroup Mat3 Matrix 3x3 Functions
 *  Functions for creating and manipulating 3x3 matrices (NX_Mat3).
 *  Primarily used for normal transformations and 2D operations.
 *  @{
 */

/**
 * @brief Checks if the matrix is ​​an identity
 */
NXAPI bool NX_IsMat3Identity(const NX_Mat3* mat);

/**
 * @brief Extract upper-left 3x3 from a 4x4 matrix
 */
static inline NX_Mat3 NX_Mat3FromMat4(const NX_Mat4* mat4)
{
    NX_Mat3 result;
    result.m00 = mat4->m00; result.m01 = mat4->m01; result.m02 = mat4->m02;
    result.m10 = mat4->m10; result.m11 = mat4->m11; result.m12 = mat4->m12;
    result.m20 = mat4->m20; result.m21 = mat4->m21; result.m22 = mat4->m22;
    return result;
}

/**
 * @brief Convert a 3x3 matrix to a 4x4 matrix
 */
static inline NX_Mat4 NX_Mat3ToMat4(const NX_Mat3* mat3)
{
    NX_Mat4 result;
    result.m00 = mat3->m00; result.m01 = mat3->m01; result.m02 = mat3->m02; result.m03 = 0.0f;
    result.m10 = mat3->m10; result.m11 = mat3->m11; result.m12 = mat3->m12; result.m13 = 0.0f;
    result.m20 = mat3->m20; result.m21 = mat3->m21; result.m22 = mat3->m22; result.m23 = 0.0f;
    result.m30 = 0.0f;      result.m31 = 0.0f;      result.m32 = 0.0f;      result.m33 = 1.0f;
    return result;
}

/**
 * @brief Create 2D transformation matrix (TRS)
 */
NXAPI NX_Mat3 NX_Mat3Transform2D(NX_Vec2 translation, float rotation, NX_Vec2 scale);

/**
 * @brief Create 2D translation matrix
 */
NXAPI NX_Mat3 NX_Mat3Translate2D(NX_Vec2 translation);

/**
 * @brief Create 2D rotation matrix (around Z axis)
 */
NXAPI NX_Mat3 NX_Mat3Rotate2D(float radians);

/**
 * @brief Create 2D scaling matrix
 */
NXAPI NX_Mat3 NX_Mat3Scale2D(NX_Vec2 scale);

/**
 * @brief Create 3D rotation matrix around X axis
 */
NXAPI NX_Mat3 NX_Mat3RotateX(float radians);

/**
 * @brief Create 3D rotation matrix around Y axis
 */
NXAPI NX_Mat3 NX_Mat3RotateY(float radians);

/**
 * @brief Create 3D rotation matrix around Z axis
 */
NXAPI NX_Mat3 NX_Mat3RotateZ(float radians);

/**
 * @brief Create 3D rotation matrix around arbitrary axis
 */
NXAPI NX_Mat3 NX_Mat3Rotate(NX_Vec3 axis, float radians);

/**
 * @brief Create rotation matrix from Euler angles (XYZ order)
 */
NXAPI NX_Mat3 NX_Mat3RotateXYZ(NX_Vec3 radians);

/**
 * @brief Transpose a 3x3 matrix
 */
NXAPI NX_Mat3 NX_Mat3Transpose(const NX_Mat3* mat);

/**
 * @brief Compute determinant of 3x3 matrix
 */
NXAPI float NX_Mat3Determinant(const NX_Mat3* mat);

/**
 * @brief Compute trace of 3x3 matrix
 */
static inline float NX_Mat3Trace(const NX_Mat3* mat)
{
    return mat->m00 + mat->m11 + mat->m22;
}

/**
 * @brief Invert a 3x3 matrix
 */
NXAPI NX_Mat3 NX_Mat3Inverse(const NX_Mat3* mat);

/**
 * @brief Create normal matrix from 4x4 matrix
 */
NXAPI NX_Mat3 NX_Mat3Normal(const NX_Mat4* mat);

/**
 * @brief Add two 3x3 matrices
 */
NXAPI NX_Mat3 NX_Mat3Add(const NX_Mat3* left, const NX_Mat3* right);

/**
 * @brief Subtract two 3x3 matrices
 */
NXAPI NX_Mat3 NX_Mat3Sub(const NX_Mat3* left, const NX_Mat3* right);

/**
 * @brief Multiply two 3x3 matrices
 */
NXAPI NX_Mat3 NX_Mat3Mul(const NX_Mat3* NX_RESTRICT left, const NX_Mat3* NX_RESTRICT right);

/** @} */ // Mat3

/* === Matrix 4x4 Functions === */

/** @defgroup Mat4 Matrix 4x4 Functions
 *  Functions for creating and manipulating 4x4 matrices (NX_Mat4).
 *  @{
 */

/**
 * @brief Checks if the matrix is ​​an identity
 */
NXAPI bool NX_IsMat4Identity(const NX_Mat4* mat);

/**
 * @brief Create a translation matrix
 */
NXAPI NX_Mat4 NX_Mat4Translate(NX_Vec3 v);

/**
 * @brief Create a rotation matrix around an arbitrary axis
 */
NXAPI NX_Mat4 NX_Mat4Rotate(NX_Vec3 axis, float radians);

/**
 * @brief Create a rotation matrix around the X axis
 */
NXAPI NX_Mat4 NX_Mat4RotateX(float radians);

/**
 * @brief Create a rotation matrix around the Y axis
 */
NXAPI NX_Mat4 NX_Mat4RotateY(float radians);

/**
 * @brief Create a rotation matrix around the Z axis
 */
NXAPI NX_Mat4 NX_Mat4RotateZ(float radians);

/**
 * @brief Create a rotation matrix from Euler angles (XYZ order)
 */
NXAPI NX_Mat4 NX_Mat4RotateXYZ(NX_Vec3 radians);

/**
 * @brief Create a rotation matrix from Euler angles (ZYX order)
 */
NXAPI NX_Mat4 NX_Mat4RotateZYX(NX_Vec3 radians);

/**
 * @brief Create a scaling matrix
 */
NXAPI NX_Mat4 NX_Mat4Scale(NX_Vec3 scale);

/**
 * @brief Decompose a matrix into TRS
 */
NXAPI NX_Transform NX_Mat4Decompose(const NX_Mat4* mat);

/**
 * @brief Create a perspective frustum projection matrix
 */
NXAPI NX_Mat4 NX_Mat4Frustum(float left, float right, float bottom, float top, float znear, float zfar);

/**
 * @brief Create a perspective projection matrix
 */
NXAPI NX_Mat4 NX_Mat4Perspective(float fovy, float aspect, float znear, float zfar);

/**
 * @brief Create an orthographic projection matrix
 */
NXAPI NX_Mat4 NX_Mat4Ortho(float left, float right, float bottom, float top, float znear, float zfar);

/**
 * @brief Create a look-to view matrix
 */
NXAPI NX_Mat4 NX_Mat4LookTo(NX_Vec3 eye, NX_Vec3 direction, NX_Vec3 up);

/**
 * @brief Create a look-at view matrix
 */
NXAPI NX_Mat4 NX_Mat4LookAt(NX_Vec3 eye, NX_Vec3 target, NX_Vec3 up);

/**
 * @brief Compute the determinant of a matrix
 */
NXAPI float NX_Mat4Determinant(const NX_Mat4* mat);

/**
 * @brief Transpose a matrix
 */
NXAPI NX_Mat4 NX_Mat4Transpose(const NX_Mat4* mat);

/**
 * @brief Invert a matrix
 */
NXAPI NX_Mat4 NX_Mat4Inverse(const NX_Mat4* mat);

/**
 * @brief Compute the trace of a matrix
 */
NXAPI float NX_Mat4Trace(const NX_Mat4* mat);

/**
 * @brief Add two matrices
 */
NXAPI NX_Mat4 NX_Mat4Add(const NX_Mat4* left, const NX_Mat4* right);

/**
 * @brief Subtract two matrices
 */
NXAPI NX_Mat4 NX_Mat4Sub(const NX_Mat4* left, const NX_Mat4* right);

/**
 * @brief Multiply two matrices
 */
NXAPI NX_Mat4 NX_Mat4Mul(const NX_Mat4* NX_RESTRICT left, const NX_Mat4* NX_RESTRICT right);

/**
 * @brief Multiply two arrays of matrices
 */
NXAPI void NX_Mat4MulBatch(NX_Mat4* NX_RESTRICT results,
                           const NX_Mat4* NX_RESTRICT left,
                           const NX_Mat4* NX_RESTRICT right,
                           size_t count);

/** @} */ // Mat4

/* === Transform Functions === */

/** @defgroup Transform 3D Transform Functions
 *  Functions for creating and manipulating 3D transformations (NX_Transform).
 *  @{
 */

/** Convert NX_Transform to a 4x4 matrix */
NXAPI NX_Mat4 NX_TransformToMat4(const NX_Transform* transform);

/** Calculation of the normal matrix of the transformation */
NXAPI NX_Mat3 NX_TransformToNormalMat3(const NX_Transform* transform);

/** Combine parent and child transform (parent first) */
NXAPI NX_Transform NX_TransformCombine(const NX_Transform* parent, const NX_Transform* child);

/** Linearly interpolate between two transforms (LERP for translation & scale, SLERP for rotation) */
NXAPI NX_Transform NX_TransformLerp(const NX_Transform* a, const NX_Transform* b, float t);

/** @} */ // Transform

#if defined(__cplusplus)
} // extern "C"
#endif

/* === C++ Operators === */

#if defined(__cplusplus)

#include <string.h> // memcmp

/* === Addition Operators === */

// Vector + Vector
inline NX_IVec2 operator+(const NX_IVec2& lhs, const NX_IVec2& rhs) { return NX_IVec2Add(lhs, rhs); }
inline NX_IVec3 operator+(const NX_IVec3& lhs, const NX_IVec3& rhs) { return NX_IVec3Add(lhs, rhs); }
inline NX_IVec4 operator+(const NX_IVec4& lhs, const NX_IVec4& rhs) { return NX_IVec4Add(lhs, rhs); }
inline NX_Vec2 operator+(const NX_Vec2& lhs, const NX_Vec2& rhs) { return NX_Vec2Add(lhs, rhs); }
inline NX_Vec3 operator+(const NX_Vec3& lhs, const NX_Vec3& rhs) { return NX_Vec3Add(lhs, rhs); }
inline NX_Vec4 operator+(const NX_Vec4& lhs, const NX_Vec4& rhs) { return NX_Vec4Add(lhs, rhs); }
inline NX_Color operator+(const NX_Color& lhs, const NX_Color& rhs) { return NX_ColorAdd(lhs, rhs); }
inline NX_Mat3 operator+(const NX_Mat3& lhs, const NX_Mat3& rhs) { return NX_Mat3Add(&lhs, &rhs); }
inline NX_Mat4 operator+(const NX_Mat4& lhs, const NX_Mat4& rhs) { return NX_Mat4Add(&lhs, &rhs); }

// Vector + Scalar
inline NX_IVec2 operator+(const NX_IVec2& lhs, int rhs) { return NX_IVec2Offset(lhs, rhs); }
inline NX_IVec3 operator+(const NX_IVec3& lhs, int rhs) { return NX_IVec3Offset(lhs, rhs); }
inline NX_IVec4 operator+(const NX_IVec4& lhs, int rhs) { return NX_IVec4Offset(lhs, rhs); }
inline NX_Vec2 operator+(const NX_Vec2& lhs, float rhs) { return NX_Vec2Offset(lhs, rhs); }
inline NX_Vec3 operator+(const NX_Vec3& lhs, float rhs) { return NX_Vec3Offset(lhs, rhs); }
inline NX_Vec4 operator+(const NX_Vec4& lhs, float rhs) { return NX_Vec4Offset(lhs, rhs); }
inline NX_Color operator+(const NX_Color& lhs, float rhs) { return NX_ColorOffset(lhs, rhs); }

// Scalar + Vector
inline NX_IVec2 operator+(int lhs, const NX_IVec2& rhs) { return NX_IVec2Add(NX_IVEC2_1(lhs), rhs); }
inline NX_IVec3 operator+(int lhs, const NX_IVec3& rhs) { return NX_IVec3Add(NX_IVEC3_1(lhs), rhs); }
inline NX_IVec4 operator+(int lhs, const NX_IVec4& rhs) { return NX_IVec4Add(NX_IVEC4_1(lhs), rhs); }
inline NX_Vec2 operator+(float lhs, const NX_Vec2& rhs) { return NX_Vec2Add(NX_VEC2_1(lhs), rhs); }
inline NX_Vec3 operator+(float lhs, const NX_Vec3& rhs) { return NX_Vec3Add(NX_VEC3_1(lhs), rhs); }
inline NX_Vec4 operator+(float lhs, const NX_Vec4& rhs) { return NX_Vec4Add(NX_VEC4_1(lhs), rhs); }
inline NX_Color operator+(float lhs, const NX_Color& rhs) { return NX_ColorAdd(NX_COLOR_1(lhs), rhs); }

// Addition assignment operators
inline const NX_IVec2& operator+=(NX_IVec2& lhs, const NX_IVec2& rhs) { lhs = NX_IVec2Add(lhs, rhs); return lhs; }
inline const NX_IVec3& operator+=(NX_IVec3& lhs, const NX_IVec3& rhs) { lhs = NX_IVec3Add(lhs, rhs); return lhs; }
inline const NX_IVec4& operator+=(NX_IVec4& lhs, const NX_IVec4& rhs) { lhs = NX_IVec4Add(lhs, rhs); return lhs; }
inline const NX_Vec2& operator+=(NX_Vec2& lhs, const NX_Vec2& rhs) { lhs = NX_Vec2Add(lhs, rhs); return lhs; }
inline const NX_Vec3& operator+=(NX_Vec3& lhs, const NX_Vec3& rhs) { lhs = NX_Vec3Add(lhs, rhs); return lhs; }
inline const NX_Vec4& operator+=(NX_Vec4& lhs, const NX_Vec4& rhs) { lhs = NX_Vec4Add(lhs, rhs); return lhs; }
inline const NX_Color& operator+=(NX_Color& lhs, const NX_Color& rhs) { lhs = NX_ColorAdd(lhs, rhs); return lhs; }
inline const NX_Mat3& operator+=(NX_Mat3& lhs, const NX_Mat3& rhs) { lhs = NX_Mat3Add(&lhs, &rhs); return lhs; }
inline const NX_Mat4& operator+=(NX_Mat4& lhs, const NX_Mat4& rhs) { lhs = NX_Mat4Add(&lhs, &rhs); return lhs; }

inline const NX_IVec2& operator+=(NX_IVec2& lhs, int rhs) { lhs = NX_IVec2Offset(lhs, rhs); return lhs; }
inline const NX_IVec3& operator+=(NX_IVec3& lhs, int rhs) { lhs = NX_IVec3Offset(lhs, rhs); return lhs; }
inline const NX_IVec4& operator+=(NX_IVec4& lhs, int rhs) { lhs = NX_IVec4Offset(lhs, rhs); return lhs; }
inline const NX_Vec2& operator+=(NX_Vec2& lhs, float rhs) { lhs = NX_Vec2Offset(lhs, rhs); return lhs; }
inline const NX_Vec3& operator+=(NX_Vec3& lhs, float rhs) { lhs = NX_Vec3Offset(lhs, rhs); return lhs; }
inline const NX_Vec4& operator+=(NX_Vec4& lhs, float rhs) { lhs = NX_Vec4Offset(lhs, rhs); return lhs; }
inline const NX_Color& operator+=(NX_Color& lhs, float rhs) { lhs = NX_ColorOffset(lhs, rhs); return lhs; }

/* === Subtraction Operators === */

// Vector - Vector
inline NX_IVec2 operator-(const NX_IVec2& lhs, const NX_IVec2& rhs) { return NX_IVec2Sub(lhs, rhs); }
inline NX_IVec3 operator-(const NX_IVec3& lhs, const NX_IVec3& rhs) { return NX_IVec3Sub(lhs, rhs); }
inline NX_IVec4 operator-(const NX_IVec4& lhs, const NX_IVec4& rhs) { return NX_IVec4Sub(lhs, rhs); }
inline NX_Vec2 operator-(const NX_Vec2& lhs, const NX_Vec2& rhs) { return NX_Vec2Sub(lhs, rhs); }
inline NX_Vec3 operator-(const NX_Vec3& lhs, const NX_Vec3& rhs) { return NX_Vec3Sub(lhs, rhs); }
inline NX_Vec4 operator-(const NX_Vec4& lhs, const NX_Vec4& rhs) { return NX_Vec4Sub(lhs, rhs); }
inline NX_Color operator-(const NX_Color& lhs, const NX_Color& rhs) { return NX_ColorSub(lhs, rhs); }
inline NX_Mat3 operator-(const NX_Mat3& lhs, const NX_Mat3& rhs) { return NX_Mat3Sub(&lhs, &rhs); }
inline NX_Mat4 operator-(const NX_Mat4& lhs, const NX_Mat4& rhs) { return NX_Mat4Sub(&lhs, &rhs); }

// Vector - Scalar
inline NX_IVec2 operator-(const NX_IVec2& lhs, int rhs) { return NX_IVec2Offset(lhs, -rhs); }
inline NX_IVec3 operator-(const NX_IVec3& lhs, int rhs) { return NX_IVec3Offset(lhs, -rhs); }
inline NX_IVec4 operator-(const NX_IVec4& lhs, int rhs) { return NX_IVec4Offset(lhs, -rhs); }
inline NX_Vec2 operator-(const NX_Vec2& lhs, float rhs) { return NX_Vec2Offset(lhs, -rhs); }
inline NX_Vec3 operator-(const NX_Vec3& lhs, float rhs) { return NX_Vec3Offset(lhs, -rhs); }
inline NX_Vec4 operator-(const NX_Vec4& lhs, float rhs) { return NX_Vec4Offset(lhs, -rhs); }
inline NX_Color operator-(const NX_Color& lhs, float rhs) { return NX_ColorOffset(lhs, -rhs); }

// Scalar - Vector
inline NX_IVec2 operator-(int lhs, const NX_IVec2& rhs) { return NX_IVec2Sub(NX_IVEC2_1(lhs), rhs); }
inline NX_IVec3 operator-(int lhs, const NX_IVec3& rhs) { return NX_IVec3Sub(NX_IVEC3_1(lhs), rhs); }
inline NX_IVec4 operator-(int lhs, const NX_IVec4& rhs) { return NX_IVec4Sub(NX_IVEC4_1(lhs), rhs); }
inline NX_Vec2 operator-(float lhs, const NX_Vec2& rhs) { return NX_Vec2Sub(NX_VEC2_1(lhs), rhs); }
inline NX_Vec3 operator-(float lhs, const NX_Vec3& rhs) { return NX_Vec3Sub(NX_VEC3_1(lhs), rhs); }
inline NX_Vec4 operator-(float lhs, const NX_Vec4& rhs) { return NX_Vec4Sub(NX_VEC4_1(lhs), rhs); }
inline NX_Color operator-(float lhs, const NX_Color& rhs) { return NX_ColorSub(NX_COLOR_1(lhs), rhs); }

// Unary minus (negation)
inline NX_IVec2 operator-(const NX_IVec2& vec) { return NX_IVec2Neg(vec); }
inline NX_IVec3 operator-(const NX_IVec3& vec) { return NX_IVec3Neg(vec); }
inline NX_IVec4 operator-(const NX_IVec4& vec) { return NX_IVec4Neg(vec); }
inline NX_Vec2 operator-(const NX_Vec2& vec) { return NX_Vec2Neg(vec); }
inline NX_Vec3 operator-(const NX_Vec3& vec) { return NX_Vec3Neg(vec); }
inline NX_Vec4 operator-(const NX_Vec4& vec) { return NX_Vec4Neg(vec); }

// Subtraction assignment operators
inline const NX_IVec2& operator-=(NX_IVec2& lhs, const NX_IVec2& rhs) { lhs = NX_IVec2Sub(lhs, rhs); return lhs; }
inline const NX_IVec3& operator-=(NX_IVec3& lhs, const NX_IVec3& rhs) { lhs = NX_IVec3Sub(lhs, rhs); return lhs; }
inline const NX_IVec4& operator-=(NX_IVec4& lhs, const NX_IVec4& rhs) { lhs = NX_IVec4Sub(lhs, rhs); return lhs; }
inline const NX_Vec2& operator-=(NX_Vec2& lhs, const NX_Vec2& rhs) { lhs = NX_Vec2Sub(lhs, rhs); return lhs; }
inline const NX_Vec3& operator-=(NX_Vec3& lhs, const NX_Vec3& rhs) { lhs = NX_Vec3Sub(lhs, rhs); return lhs; }
inline const NX_Vec4& operator-=(NX_Vec4& lhs, const NX_Vec4& rhs) { lhs = NX_Vec4Sub(lhs, rhs); return lhs; }
inline const NX_Color& operator-=(NX_Color& lhs, const NX_Color& rhs) { lhs = NX_ColorSub(lhs, rhs); return lhs; }
inline const NX_Mat3& operator-=(NX_Mat3& lhs, const NX_Mat3& rhs) { lhs = NX_Mat3Sub(&lhs, &rhs); return lhs; }
inline const NX_Mat4& operator-=(NX_Mat4& lhs, const NX_Mat4& rhs) { lhs = NX_Mat4Sub(&lhs, &rhs); return lhs; }

inline const NX_IVec2& operator-=(NX_IVec2& lhs, int rhs) { lhs = NX_IVec2Offset(lhs, -rhs); return lhs; }
inline const NX_IVec3& operator-=(NX_IVec3& lhs, int rhs) { lhs = NX_IVec3Offset(lhs, -rhs); return lhs; }
inline const NX_IVec4& operator-=(NX_IVec4& lhs, int rhs) { lhs = NX_IVec4Offset(lhs, -rhs); return lhs; }
inline const NX_Vec2& operator-=(NX_Vec2& lhs, float rhs) { lhs = NX_Vec2Offset(lhs, -rhs); return lhs; }
inline const NX_Vec3& operator-=(NX_Vec3& lhs, float rhs) { lhs = NX_Vec3Offset(lhs, -rhs); return lhs; }
inline const NX_Vec4& operator-=(NX_Vec4& lhs, float rhs) { lhs = NX_Vec4Offset(lhs, -rhs); return lhs; }
inline const NX_Color& operator-=(NX_Color& lhs, float rhs) { lhs = NX_ColorOffset(lhs, -rhs); return lhs; }

/* === Multiplication Operators === */

// Vector * Scalar
inline NX_IVec2 operator*(const NX_IVec2& lhs, int rhs) { return NX_IVec2Scale(lhs, rhs); }
inline NX_IVec3 operator*(const NX_IVec3& lhs, int rhs) { return NX_IVec3Scale(lhs, rhs); }
inline NX_IVec4 operator*(const NX_IVec4& lhs, int rhs) { return NX_IVec4Scale(lhs, rhs); }
inline NX_Vec2 operator*(const NX_Vec2& lhs, float rhs) { return NX_Vec2Scale(lhs, rhs); }
inline NX_Vec3 operator*(const NX_Vec3& lhs, float rhs) { return NX_Vec3Scale(lhs, rhs); }
inline NX_Vec4 operator*(const NX_Vec4& lhs, float rhs) { return NX_Vec4Scale(lhs, rhs); }
inline NX_Color operator*(const NX_Color& lhs, float rhs) { return NX_ColorScale(lhs, rhs); }

// Scalar * Vector
inline NX_IVec2 operator*(int lhs, const NX_IVec2& rhs) { return NX_IVec2Scale(rhs, lhs); }
inline NX_IVec3 operator*(int lhs, const NX_IVec3& rhs) { return NX_IVec3Scale(rhs, lhs); }
inline NX_IVec4 operator*(int lhs, const NX_IVec4& rhs) { return NX_IVec4Scale(rhs, lhs); }
inline NX_Vec2 operator*(float lhs, const NX_Vec2& rhs) { return NX_Vec2Scale(rhs, lhs); }
inline NX_Vec3 operator*(float lhs, const NX_Vec3& rhs) { return NX_Vec3Scale(rhs, lhs); }
inline NX_Vec4 operator*(float lhs, const NX_Vec4& rhs) { return NX_Vec4Scale(rhs, lhs); }
inline NX_Color operator*(float lhs, const NX_Color& rhs) { return NX_ColorScale(rhs, lhs); }

// Vector * Vector (component-wise)
inline NX_IVec2 operator*(const NX_IVec2& lhs, const NX_IVec2& rhs) { return NX_IVec2Mul(lhs, rhs); }
inline NX_IVec3 operator*(const NX_IVec3& lhs, const NX_IVec3& rhs) { return NX_IVec3Mul(lhs, rhs); }
inline NX_IVec4 operator*(const NX_IVec4& lhs, const NX_IVec4& rhs) { return NX_IVec4Mul(lhs, rhs); }
inline NX_Vec2 operator*(const NX_Vec2& lhs, const NX_Vec2& rhs) { return NX_Vec2Mul(lhs, rhs); }
inline NX_Vec3 operator*(const NX_Vec3& lhs, const NX_Vec3& rhs) { return NX_Vec3Mul(lhs, rhs); }
inline NX_Vec4 operator*(const NX_Vec4& lhs, const NX_Vec4& rhs) { return NX_Vec4Mul(lhs, rhs); }
inline NX_Color operator*(const NX_Color& lhs, const NX_Color& rhs) { return NX_ColorMul(lhs, rhs); }

// Vector * Transform (transformation)
inline NX_Vec3 operator*(const NX_Vec3& lhs, const NX_Transform& rhs) { return NX_Vec3Transform(lhs, &rhs); }

// Vector * Quaternion (transformation)
inline NX_Vec3 operator*(const NX_Vec3& lhs, const NX_Quat& rhs) { return NX_Vec3Rotate(lhs, rhs); }

// Vector * Matrix 3x3 (transformation)
inline NX_Vec2 operator*(const NX_Vec2& lhs, const NX_Mat3& rhs) { return NX_Vec2TransformByMat3(lhs, &rhs); }
inline NX_Vec3 operator*(const NX_Vec3& lhs, const NX_Mat3& rhs) { return NX_Vec3TransformByMat3(lhs, &rhs); }

// Vector * Matrix 4x4 (transformation)
inline NX_Vec2 operator*(const NX_Vec2& lhs, const NX_Mat4& rhs) { return NX_Vec2TransformByMat4(lhs, &rhs); }
inline NX_Vec3 operator*(const NX_Vec3& lhs, const NX_Mat4& rhs) { return NX_Vec3TransformByMat4(lhs, &rhs); }
inline NX_Vec4 operator*(const NX_Vec4& lhs, const NX_Mat4& rhs) { return NX_Vec4TransformByMat4(lhs, &rhs); }

// Matrix * Matrix
inline NX_Mat3 operator*(const NX_Mat3& lhs, const NX_Mat3& rhs) { return NX_Mat3Mul(&lhs, &rhs); }
inline NX_Mat4 operator*(const NX_Mat4& lhs, const NX_Mat4& rhs) { return NX_Mat4Mul(&lhs, &rhs); }

// Quaternion multiplication
inline NX_Quat operator*(const NX_Quat& lhs, const NX_Quat& rhs) { return NX_QuatMul(lhs, rhs); }

// Vector *= Scalar
inline const NX_IVec2& operator*=(NX_IVec2& lhs, int rhs) { lhs = NX_IVec2Scale(lhs, rhs); return lhs; }
inline const NX_IVec3& operator*=(NX_IVec3& lhs, int rhs) { lhs = NX_IVec3Scale(lhs, rhs); return lhs; }
inline const NX_IVec4& operator*=(NX_IVec4& lhs, int rhs) { lhs = NX_IVec4Scale(lhs, rhs); return lhs; }
inline const NX_Vec2& operator*=(NX_Vec2& lhs, float rhs) { lhs = NX_Vec2Scale(lhs, rhs); return lhs; }
inline const NX_Vec3& operator*=(NX_Vec3& lhs, float rhs) { lhs = NX_Vec3Scale(lhs, rhs); return lhs; }
inline const NX_Vec4& operator*=(NX_Vec4& lhs, float rhs) { lhs = NX_Vec4Scale(lhs, rhs); return lhs; }
inline const NX_Color& operator*=(NX_Color& lhs, float rhs) { lhs = NX_ColorScale(lhs, rhs); return lhs; }

// Vector *= Vector (component-wise)
inline const NX_IVec2& operator*=(NX_IVec2& lhs, const NX_IVec2& rhs) { lhs = NX_IVec2Mul(lhs, rhs); return lhs; }
inline const NX_IVec3& operator*=(NX_IVec3& lhs, const NX_IVec3& rhs) { lhs = NX_IVec3Mul(lhs, rhs); return lhs; }
inline const NX_IVec4& operator*=(NX_IVec4& lhs, const NX_IVec4& rhs) { lhs = NX_IVec4Mul(lhs, rhs); return lhs; }
inline const NX_Vec2& operator*=(NX_Vec2& lhs, const NX_Vec2& rhs) { lhs = NX_Vec2Mul(lhs, rhs); return lhs; }
inline const NX_Vec3& operator*=(NX_Vec3& lhs, const NX_Vec3& rhs) { lhs = NX_Vec3Mul(lhs, rhs); return lhs; }
inline const NX_Vec4& operator*=(NX_Vec4& lhs, const NX_Vec4& rhs) { lhs = NX_Vec4Mul(lhs, rhs); return lhs; }
inline const NX_Color& operator*=(NX_Color& lhs, const NX_Color& rhs) { lhs = NX_ColorMul(lhs, rhs); return lhs; }

// Vector *= Transform (transformation)
inline const NX_Vec3& operator*=(NX_Vec3& lhs, const NX_Transform& rhs) { lhs = NX_Vec3Transform(lhs, &rhs); return lhs; }

// Vector *= Quaternion (transformation)
inline const NX_Vec3& operator*=(NX_Vec3& lhs, const NX_Quat& rhs) { lhs = NX_Vec3Rotate(lhs, rhs); return lhs; }

// Vector *= Matrix 3x3 (transformation)
inline const NX_Vec2& operator*=(NX_Vec2& lhs, const NX_Mat3& rhs) { lhs = NX_Vec2TransformByMat3(lhs, &rhs); return lhs; }
inline const NX_Vec3& operator*=(NX_Vec3& lhs, const NX_Mat3& rhs) { lhs = NX_Vec3TransformByMat3(lhs, &rhs); return lhs; }

// Vector *= Matrix 4x4 (transformation)
inline const NX_Vec2& operator*=(NX_Vec2& lhs, const NX_Mat4& rhs) { lhs = NX_Vec2TransformByMat4(lhs, &rhs); return lhs; }
inline const NX_Vec3& operator*=(NX_Vec3& lhs, const NX_Mat4& rhs) { lhs = NX_Vec3TransformByMat4(lhs, &rhs); return lhs; }
inline const NX_Vec4& operator*=(NX_Vec4& lhs, const NX_Mat4& rhs) { lhs = NX_Vec4TransformByMat4(lhs, &rhs); return lhs; }

// Matrix *= Matrix
inline const NX_Mat3& operator*=(NX_Mat3& lhs, const NX_Mat3& rhs) { lhs = NX_Mat3Mul(&lhs, &rhs); return lhs; }
inline const NX_Mat4& operator*=(NX_Mat4& lhs, const NX_Mat4& rhs) { lhs = NX_Mat4Mul(&lhs, &rhs); return lhs; }
inline const NX_Quat& operator*=(NX_Quat& lhs, const NX_Quat& rhs) { lhs = NX_QuatMul(lhs, rhs); return lhs; }

/* === Division Operators === */

// Vector / Scalar
inline NX_IVec2 operator/(const NX_IVec2& lhs, int rhs) { return NX_IVec2Div(lhs, NX_IVEC2_1(rhs)); }
inline NX_IVec3 operator/(const NX_IVec3& lhs, int rhs) { return NX_IVec3Div(lhs, NX_IVEC3_1(rhs)); }
inline NX_IVec4 operator/(const NX_IVec4& lhs, int rhs) { return NX_IVec4Div(lhs, NX_IVEC4_1(rhs)); }
inline NX_Vec2 operator/(const NX_Vec2& lhs, float rhs) { return NX_Vec2Scale(lhs, 1.0f / rhs); }
inline NX_Vec3 operator/(const NX_Vec3& lhs, float rhs) { return NX_Vec3Scale(lhs, 1.0f / rhs); }
inline NX_Vec4 operator/(const NX_Vec4& lhs, float rhs) { return NX_Vec4Scale(lhs, 1.0f / rhs); }
inline NX_Color operator/(const NX_Color& lhs, float rhs) { return NX_ColorScale(lhs, 1.0f / rhs); }

// Scalar / Vector
inline NX_Vec2 operator/(float rhs, const NX_Vec2& lhs) { return NX_Vec2Div(NX_VEC2_1(rhs), lhs); }
inline NX_Vec3 operator/(float rhs, const NX_Vec3& lhs) { return NX_Vec3Div(NX_VEC3_1(rhs), lhs); }
inline NX_Vec4 operator/(float rhs, const NX_Vec4& lhs) { return NX_Vec4Div(NX_VEC4_1(rhs), lhs); }
inline NX_Color operator/(float rhs, const NX_Color& lhs) { return NX_ColorDiv(NX_COLOR_1(rhs), lhs); }

// Vector / Vector (component-wise)
inline NX_IVec2 operator/(const NX_IVec2& lhs, const NX_IVec2& rhs) { return NX_IVec2Div(lhs, rhs); }
inline NX_IVec3 operator/(const NX_IVec3& lhs, const NX_IVec3& rhs) { return NX_IVec3Div(lhs, rhs); }
inline NX_IVec4 operator/(const NX_IVec4& lhs, const NX_IVec4& rhs) { return NX_IVec4Div(lhs, rhs); }
inline NX_Vec2 operator/(const NX_Vec2& lhs, const NX_Vec2& rhs) { return NX_Vec2Div(lhs, rhs); }
inline NX_Vec3 operator/(const NX_Vec3& lhs, const NX_Vec3& rhs) { return NX_Vec3Div(lhs, rhs); }
inline NX_Vec4 operator/(const NX_Vec4& lhs, const NX_Vec4& rhs) { return NX_Vec4Div(lhs, rhs); }
inline NX_Color operator/(const NX_Color& lhs, const NX_Color& rhs) { return NX_ColorDiv(lhs, rhs); }

// Division assignment operators
inline const NX_IVec2& operator/=(NX_IVec2& lhs, int rhs) { lhs = NX_IVec2Div(lhs, NX_IVEC2_1(rhs)); return lhs; }
inline const NX_IVec3& operator/=(NX_IVec3& lhs, int rhs) { lhs = NX_IVec3Div(lhs, NX_IVEC3_1(rhs)); return lhs; }
inline const NX_IVec4& operator/=(NX_IVec4& lhs, int rhs) { lhs = NX_IVec4Div(lhs, NX_IVEC4_1(rhs)); return lhs; }
inline const NX_Vec2& operator/=(NX_Vec2& lhs, float rhs) { lhs = NX_Vec2Scale(lhs, 1.0f / rhs); return lhs; }
inline const NX_Vec3& operator/=(NX_Vec3& lhs, float rhs) { lhs = NX_Vec3Scale(lhs, 1.0f / rhs); return lhs; }
inline const NX_Vec4& operator/=(NX_Vec4& lhs, float rhs) { lhs = NX_Vec4Scale(lhs, 1.0f / rhs); return lhs; }
inline const NX_Color& operator/=(NX_Color& lhs, float rhs) { lhs = NX_ColorScale(lhs, 1.0f / rhs); return lhs; }

inline const NX_IVec2& operator/=(NX_IVec2& lhs, const NX_IVec2& rhs) { lhs = NX_IVec2Div(lhs, rhs); return lhs; }
inline const NX_IVec3& operator/=(NX_IVec3& lhs, const NX_IVec3& rhs) { lhs = NX_IVec3Div(lhs, rhs); return lhs; }
inline const NX_IVec4& operator/=(NX_IVec4& lhs, const NX_IVec4& rhs) { lhs = NX_IVec4Div(lhs, rhs); return lhs; }
inline const NX_Vec2& operator/=(NX_Vec2& lhs, const NX_Vec2& rhs) { lhs = NX_Vec2Div(lhs, rhs); return lhs; }
inline const NX_Vec3& operator/=(NX_Vec3& lhs, const NX_Vec3& rhs) { lhs = NX_Vec3Div(lhs, rhs); return lhs; }
inline const NX_Vec4& operator/=(NX_Vec4& lhs, const NX_Vec4& rhs) { lhs = NX_Vec4Div(lhs, rhs); return lhs; }
inline const NX_Color& operator/=(NX_Color& lhs, const NX_Color& rhs) { lhs = NX_ColorDiv(lhs, rhs); return lhs; }

/* === Comparison Operators === */

// Equality operators
inline bool operator==(const NX_IVec2& lhs, const NX_IVec2& rhs) { return NX_IVec2All(NX_IVec2Equals(lhs, rhs)); }
inline bool operator==(const NX_IVec3& lhs, const NX_IVec3& rhs) { return NX_IVec3All(NX_IVec3Equals(lhs, rhs)); }
inline bool operator==(const NX_IVec4& lhs, const NX_IVec4& rhs) { return NX_IVec4All(NX_IVec4Equals(lhs, rhs)); }
inline bool operator==(const NX_Vec2& lhs, const NX_Vec2& rhs) { return NX_IVec2All(NX_Vec2Equals(lhs, rhs)); }
inline bool operator==(const NX_Vec3& lhs, const NX_Vec3& rhs) { return NX_IVec3All(NX_Vec3Equals(lhs, rhs)); }
inline bool operator==(const NX_Vec4& lhs, const NX_Vec4& rhs) { return NX_IVec4All(NX_Vec4Equals(lhs, rhs)); }
inline bool operator==(const NX_Color& lhs, const NX_Color& rhs) { return NX_ColorApprox(lhs, rhs, 1e-6f); }
inline bool operator==(const NX_Quat& lhs, const NX_Quat& rhs) { return memcmp(&lhs, &rhs, sizeof(NX_Quat)) == 0; }
inline bool operator==(const NX_Mat3& lhs, const NX_Mat3& rhs) { return memcmp(&lhs, &rhs, sizeof(NX_Mat3)) == 0; }
inline bool operator==(const NX_Mat4& lhs, const NX_Mat4& rhs) { return memcmp(&lhs, &rhs, sizeof(NX_Mat4)) == 0; }

// Inequality operators
inline bool operator!=(const NX_IVec2& lhs, const NX_IVec2& rhs) { return !NX_IVec2All(NX_IVec2Equals(lhs, rhs)); }
inline bool operator!=(const NX_IVec3& lhs, const NX_IVec3& rhs) { return !NX_IVec3All(NX_IVec3Equals(lhs, rhs)); }
inline bool operator!=(const NX_IVec4& lhs, const NX_IVec4& rhs) { return !NX_IVec4All(NX_IVec4Equals(lhs, rhs)); }
inline bool operator!=(const NX_Vec2& lhs, const NX_Vec2& rhs) { return !NX_IVec2All(NX_Vec2Equals(lhs, rhs)); }
inline bool operator!=(const NX_Vec3& lhs, const NX_Vec3& rhs) { return !NX_IVec3All(NX_Vec3Equals(lhs, rhs)); }
inline bool operator!=(const NX_Vec4& lhs, const NX_Vec4& rhs) { return !NX_IVec4All(NX_Vec4Equals(lhs, rhs)); }
inline bool operator!=(const NX_Color& lhs, const NX_Color& rhs) { return !NX_ColorApprox(lhs, rhs, 1e-6f); }
inline bool operator!=(const NX_Quat& lhs, const NX_Quat& rhs) { return memcmp(&lhs, &rhs, sizeof(NX_Quat)) != 0; }
inline bool operator!=(const NX_Mat3& lhs, const NX_Mat3& rhs) { return memcmp(&lhs, &rhs, sizeof(NX_Mat3)) != 0; }
inline bool operator!=(const NX_Mat4& lhs, const NX_Mat4& rhs) { return memcmp(&lhs, &rhs, sizeof(NX_Mat4)) != 0; }

// Greater than operators
inline bool operator>(const NX_IVec2& lhs, const NX_IVec2& rhs) { return NX_IVec2All(NX_IVec2GreaterThan(lhs, rhs)); }
inline bool operator>(const NX_IVec3& lhs, const NX_IVec3& rhs) { return NX_IVec3All(NX_IVec3GreaterThan(lhs, rhs)); }
inline bool operator>(const NX_IVec4& lhs, const NX_IVec4& rhs) { return NX_IVec4All(NX_IVec4GreaterThan(lhs, rhs)); }
inline bool operator>(const NX_Vec2& lhs, const NX_Vec2& rhs) { return NX_IVec2All(NX_Vec2GreaterThan(lhs, rhs)); }
inline bool operator>(const NX_Vec3& lhs, const NX_Vec3& rhs) { return NX_IVec3All(NX_Vec3GreaterThan(lhs, rhs)); }
inline bool operator>(const NX_Vec4& lhs, const NX_Vec4& rhs) { return NX_IVec4All(NX_Vec4GreaterThan(lhs, rhs)); }

// Less than operators
inline bool operator<(const NX_IVec2& lhs, const NX_IVec2& rhs) { return NX_IVec2All(NX_IVec2GreaterThan(rhs, lhs)); }
inline bool operator<(const NX_IVec3& lhs, const NX_IVec3& rhs) { return NX_IVec3All(NX_IVec3GreaterThan(rhs, lhs)); }
inline bool operator<(const NX_IVec4& lhs, const NX_IVec4& rhs) { return NX_IVec4All(NX_IVec4GreaterThan(rhs, lhs)); }
inline bool operator<(const NX_Vec2& lhs, const NX_Vec2& rhs) { return NX_IVec2All(NX_Vec2GreaterThan(rhs, lhs)); }
inline bool operator<(const NX_Vec3& lhs, const NX_Vec3& rhs) { return NX_IVec3All(NX_Vec3GreaterThan(rhs, lhs)); }
inline bool operator<(const NX_Vec4& lhs, const NX_Vec4& rhs) { return NX_IVec4All(NX_Vec4GreaterThan(rhs, lhs)); }

#endif // __cplusplus

#endif // NX_MATH_H
