/* HP_Math.h -- API declaration for Hyperion's math module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef HP_MATH_H
#define HP_MATH_H

#include "./HP_BitUtils.h"
#include "./HP_Platform.h"
#include "./HP_Macros.h"

#include <string.h>
#include <math.h>

/* === Constants === */

#define HP_PI   3.1415926535897931f
#define HP_TAU  6.2831853071795862f

#define HP_DEG2RAD (HP_PI / 180.0)
#define HP_RAD2DEG (180.0 / HP_PI)

#define HP_IVEC2_ZERO       HP_IVEC2( 0,  0)
#define HP_IVEC2_ONE        HP_IVEC2( 1,  1)
#define HP_IVEC3_ZERO       HP_IVEC3( 0,  0,  0)
#define HP_IVEC3_ONE        HP_IVEC3( 1,  1,  1)
#define HP_IVEC4_ZERO       HP_IVEC4( 0, 0, 0, 0)
#define HP_IVEC4_ONE        HP_IVEC4( 1, 1, 1, 1)

#define HP_VEC2_ZERO        HP_VEC2( 0,  0)
#define HP_VEC2_ONE         HP_VEC2( 1,  1)
#define HP_VEC2_UP          HP_VEC2( 0,  1)
#define HP_VEC2_DOWN        HP_VEC2( 0, -1)
#define HP_VEC2_LEFT        HP_VEC2(-1,  0)
#define HP_VEC2_RIGHT       HP_VEC2( 1,  0)

#define HP_VEC3_ZERO        HP_VEC3( 0,  0,  0)
#define HP_VEC3_ONE         HP_VEC3( 1,  1,  1)
#define HP_VEC3_UP          HP_VEC3( 0,  1,  0)
#define HP_VEC3_DOWN        HP_VEC3( 0, -1,  0)
#define HP_VEC3_LEFT        HP_VEC3(-1,  0,  0)
#define HP_VEC3_RIGHT       HP_VEC3( 1,  0,  0)
#define HP_VEC3_FORWARD     HP_VEC3( 0,  0, -1)
#define HP_VEC3_BACK        HP_VEC3( 0,  0,  1)

#define HP_VEC4_ZERO        HP_VEC4( 0, 0, 0, 0)
#define HP_VEC4_ONE         HP_VEC4( 1, 1, 1, 1)

#define HP_BLANK            HP_COLOR(0.00f, 0.00f, 0.00f, 0.00f)
#define HP_WHITE            HP_COLOR(1.00f, 1.00f, 1.00f, 1.00f)
#define HP_BLACK            HP_COLOR(0.00f, 0.00f, 0.00f, 1.00f)
#define HP_GRAY             HP_COLOR(0.50f, 0.50f, 0.50f, 1.00f)
#define HP_LIGHT_GRAY       HP_COLOR(0.75f, 0.75f, 0.75f, 1.00f)
#define HP_DARK_GRAY        HP_COLOR(0.25f, 0.25f, 0.25f, 1.00f)
#define HP_RED              HP_COLOR(1.00f, 0.00f, 0.00f, 1.00f)
#define HP_GREEN            HP_COLOR(0.00f, 1.00f, 0.00f, 1.00f)
#define HP_BLUE             HP_COLOR(0.00f, 0.00f, 1.00f, 1.00f)
#define HP_YELLOW           HP_COLOR(1.00f, 1.00f, 0.00f, 1.00f)
#define HP_CYAN             HP_COLOR(0.00f, 1.00f, 1.00f, 1.00f)
#define HP_MAGENTA          HP_COLOR(1.00f, 0.00f, 1.00f, 1.00f)
#define HP_ORANGE           HP_COLOR(1.00f, 0.65f, 0.00f, 1.00f)
#define HP_BROWN            HP_COLOR(0.65f, 0.16f, 0.16f, 1.00f)
#define HP_PURPLE           HP_COLOR(0.50f, 0.00f, 0.50f, 1.00f)
#define HP_PINK             HP_COLOR(1.00f, 0.75f, 0.80f, 1.00f)
#define HP_GOLD             HP_COLOR(0.83f, 0.69f, 0.22f, 1.00f)
#define HP_SILVER           HP_COLOR(0.77f, 0.77f, 0.77f, 1.00f)
#define HP_COPPER           HP_COLOR(0.78f, 0.51f, 0.27f, 1.00f)

#define HP_QUAT_IDENTITY    HP_QUAT( 1, 0, 0, 0)

#define HP_MAT4_IDENTITY        \
    HP_MAT4_T {                 \
        1.0f, 0.0f, 0.0f, 0.0f, \
        0.0f, 1.0f, 0.0f, 0.0f, \
        0.0f, 0.0f, 1.0f, 0.0f, \
        0.0f, 0.0f, 0.0f, 1.0f  \
    }

#define HP_TRANSFORM_IDENTITY   \
    HP_TRANSFORM_T {            \
        HP_VEC3_ZERO,           \
        HP_QUAT_IDENTITY,       \
        HP_VEC3_ONE             \
    }

/* === Constructors Helpers === */

/**
 * @biref Create Integer 2D vector from single value
 */
#define HP_IVEC2_1(x)           \
    HP_STRUCT_LITERAL(HP_IVec2, \
        HP_CAST(int, (x)),      \
        HP_CAST(int, (x))       \
    )

/**
 * @biref Create Integer 2D vector from x, y values
 */
#define HP_IVEC2(x, y)          \
    HP_STRUCT_LITERAL(HP_IVec2, \
        HP_CAST(int, (x)),      \
        HP_CAST(int, (y))       \
    )

/**
 * @biref Create Integer 3D vector from single value
 */
#define HP_IVEC3_1(x)           \
    HP_STRUCT_LITERAL(HP_IVec3, \
        HP_CAST(int, (x)),      \
        HP_CAST(int, (x)),      \
        HP_CAST(int, (x))       \
    )

/**
 * @biref Create Integer 3D vector from x, y, z values
 */
#define HP_IVEC3(x, y, z)       \
    HP_STRUCT_LITERAL(HP_IVec3, \
        HP_CAST(int, (x)),      \
        HP_CAST(int, (y)),      \
        HP_CAST(int, (z))       \
    )

/**
 * @biref Create Integer 4D vector from single value
 */
#define HP_IVEC4_1(x)           \
    HP_STRUCT_LITERAL(HP_IVec4, \
        HP_CAST(int, (x)),      \
        HP_CAST(int, (x)),      \
        HP_CAST(int, (x)),      \
        HP_CAST(int, (x))       \
    )

/**
 * @biref Create Integer 4D vector from x, y, z, w values
 */
#define HP_IVEC4(x, y, z, w)    \
    HP_STRUCT_LITERAL(HP_IVec4, \
        HP_CAST(int, (x)),      \
        HP_CAST(int, (y)),      \
        HP_CAST(int, (z)),      \
        HP_CAST(int, (w))       \
    )

/**
 * @biref Create Float 2D vector from single value
 */
#define HP_VEC2_1(x)            \
    HP_STRUCT_LITERAL(HP_Vec2,  \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (x))     \
    )

/**
 * @biref Create Float 2D vector from x, y values
 */
#define HP_VEC2(x, y)           \
    HP_STRUCT_LITERAL(HP_Vec2,  \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (y))     \
    )

/**
 * @biref Create Float 3D vector from single value
 */
#define HP_VEC3_1(x)            \
    HP_STRUCT_LITERAL(HP_Vec3,  \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (x))     \
    )

/**
 * @biref Create Float 3D vector from x, y, z values
 */
#define HP_VEC3(x, y, z)        \
    HP_STRUCT_LITERAL(HP_Vec3,  \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (y)),    \
        HP_CAST(float, (z))     \
    )

/**
 * @biref Create Float 4D vector from single value
 */
#define HP_VEC4_1(x)            \
    HP_STRUCT_LITERAL(HP_Vec4,  \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (x))     \
    )

/**
 * @biref Create Float 4D vector from x, y, z, w values
 */
#define HP_VEC4(x, y, z, w)     \
    HP_STRUCT_LITERAL(HP_Vec4,  \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (y)),    \
        HP_CAST(float, (z)),    \
        HP_CAST(float, (w))     \
    )

/**
 * @biref Create Color from single grayscale value (alpha = 1)
 */
#define HP_COLOR_1(x)           \
    HP_STRUCT_LITERAL(HP_Color, \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (1))     \
    )

/**
 * @biref Create Color from grayscale and alpha values
 */
#define HP_COLOR_RGB(r, g, b)   \
    HP_STRUCT_LITERAL(HP_Color, \
        HP_CAST(float, (r)),    \
        HP_CAST(float, (g)),    \
        HP_CAST(float, (b)),    \
        HP_CAST(float, (1))     \
    )

/**
 * @biref Create Color from existing color with new alpha value
 */
#define HP_COLOR_ALPHA(c, a)    \
    HP_STRUCT_LITERAL(HP_Color, \
        HP_CAST(float, (c).r),  \
        HP_CAST(float, (c).g),  \
        HP_CAST(float, (c).b),  \
        HP_CAST(float, (a))     \
    )

/**
 * @biref Create Color from r, g, b, a values
 */
#define HP_COLOR(r, g, b, a)    \
    HP_STRUCT_LITERAL(HP_Color, \
        HP_CAST(float, (r)),    \
        HP_CAST(float, (g)),    \
        HP_CAST(float, (b)),    \
        HP_CAST(float, (a))     \
    )

/**
 * @biref Create Quaternion from w, x, y, z values
 */
#define HP_QUAT(w, x, y, z)     \
    HP_STRUCT_LITERAL(HP_Quat,  \
        HP_CAST(float, (w)),    \
        HP_CAST(float, (x)),    \
        HP_CAST(float, (y)),    \
        HP_CAST(float, (z))     \
    )

/**
 * @biref Compound literal helper for 3x3 matrix (row-major)
 */
#define HP_MAT3_T HP_LITERAL(HP_Mat3)

/**
 * @biref Compound literal helper for 4x4 matrix (row-major)
 */
#define HP_MAT4_T HP_LITERAL(HP_Mat4)

/**
 * @biref Compound literal helper for Transform (translation, rotation, scale)
 */
#define HP_TRANSFORM_T HP_LITERAL(HP_Transform)

/* === Macros === */

/**
 * @brief Returns the minimum of two values
 */
#define HP_MIN(a, b) \
    ((a) < (b) ? (a) : (b))

/**
 * @brief Returns the maximum of two values
 */
#define HP_MAX(a, b) \
    ((a) > (b) ? (a) : (b))

/**
 * @brief Returns the minimum of three values
 */
#define HP_MIN3(a, b, c) \
    HP_MIN(a, HP_MIN(b, c))

/**
 * @brief Returns the maximum of three values
 */
#define HP_MAX3(a, b, c) \
    HP_MAX(a, HP_MAX(b, c))

/**
 * @brief Clamps a value between minimum and maximum bounds
 */
#define HP_CLAMP(v, min, max) \
    HP_MIN(HP_MAX((v), (min)), (max))

/**
 * @brief Returns the absolute value of a number
 */
#define HP_ABS(x) \
    ((x) < 0 ? -(x) : (x))

/**
 * @brief Returns the sign of a number (-1, 0, or 1)
 */
#define HP_SIGN(x) \
    (((x) > 0) - ((x) < 0))

/**
 * @brief Returns the square of a number
 */
#define HP_POW2(x) \
    ((x) * (x))

/**
 * @brief Returns the cube of a number
 */
#define HP_POW3(x) \
    ((x) * (x) * (x))

/**
 * @brief The multiple of B after A
 */
#define HP_NEXT_MULTIPLE(a, b) \
    ((b) * ((int)ceilf((float)(a) / (b))))

/**
 * @brief The multiple of B before A
 */
#define HP_PREV_MULTIPLE(a, b) \
    ((b) * ((int)floorf((float)(a) / (b))))

/**
 * @brief The closest multiple of B to A
 */
#define HP_NEAR_MULTIPLE(a, b) \
    ((b) * ((int)roundf((float)(a) / (b))))

/**
 * @brief Integer division with ceiling (round up)
 */
#define HP_DIV_CEIL(num, denom) \
    (((num) + (denom) - 1) / (denom))

/**
 * @brief Checks if value is within inclusive range
 */
#define HP_IN_RANGE(x, low, high) \
    ((x) >= (low) && (x) <= (high))

/**
 * @brief Rounds value up to the next alignment boundary
 */
#define HP_ALIGN_UP(value, alignment) \
    (((value) + (alignment) - 1) & ~((alignment) - 1))

/**
 * @brief Rounds value down to the previous alignment boundary
 */
#define HP_ALIGN_DOWN(value, alignment) \
    ((value) & ~((alignment) - 1))

/**
 * @brief Checks if addition would overflow
 */
#define HP_WOULD_OVERFLOW_ADD(a, b, max) \
    ((a) > (max) - (b))

/**
 * @brief Checks if multiplication would overflow
 */
#define HP_WOULD_OVERFLOW_MUL(a, b, max) \
    ((a) != 0 && (b) > (max) / (a))

/* === Structures === */

/**
 * @biref Integer 2D vector (x, y)
 */
typedef struct HP_IVec2 {
    union {
        struct { int x, y; };
        int v[2];
    };
} HP_IVec2;

/**
 * @biref Integer 3D vector (x, y, z)
 */
typedef struct HP_IVec3 {
    union {
        struct { int x, y, z; };
        int v[3];
    };
} HP_IVec3;

/**
 * @biref Integer 4D vector (x, y, z, w)
 */
typedef struct HP_IVec4 {
    union {
        struct { int x, y, z, w; };
        int v[4];
    };
} HP_IVec4;

/**
 * @biref Float 2D vector (x, y)
 */
typedef struct HP_Vec2 {
    union {
        struct { float x, y; };
        float v[2];
    };
} HP_Vec2;

/**
 * @biref Float 3D vector (x, y, z)
 */
typedef struct HP_Vec3 {
    union {
        struct { float x, y, z; };
        float v[3];
    };
} HP_Vec3;

/**
 * @biref Float 4D vector (x, y, z, w)
 */
typedef struct HP_Vec4 {
    union {
        struct { float x, y, z, w; };
        float v[4];
    };
} HP_Vec4;

/**
 * @biref Floating-point RGBA color (r, g, b, a)
 */
typedef struct HP_Color {
    float r, g, b, a;
} HP_Color;

/**
 * @biref Quaternion (w, x, y, z)
 */
typedef struct HP_Quat {
    union {
        struct { float w, x, y, z; };
        float v[4];
    };
} HP_Quat;

/**
 * @biref 3x3 Matrix (row-major)
 */
typedef struct HP_Mat3 {
    union {
        struct {
            float m00, m01, m02;
            float m10, m11, m12;
            float m20, m21, m22;
        };
        float v[3][3];
        float a[9];
    };
} HP_Mat3;

/**
 * @biref 4x4 Matrix (row-major)
 */
typedef struct HP_Mat4 {
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
} HP_Mat4;

/**
 * @biref Transform (translation, rotation, scale)
 */
typedef struct HP_Transform {
    HP_Vec3 translation;
    HP_Quat rotation;
    HP_Vec3 scale;
} HP_Transform;

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
static inline bool HP_IsPowerOfTwo(uint64_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

/**
 * @brief Return the next highest power of two for a 64-bit integer
 */
static inline uint64_t HP_NextPowerOfTwo(uint64_t x)
{
    return (x <= 1) ? 1ULL : 1ULL << (64 - HP_Clz64(x - 1));
}

/**
 * @brief Return the previous lowest power of two for a 64-bit integer
 */
static inline uint64_t HP_PrevPowerOfTwo(uint64_t x)
{
    return (x == 0) ? 0ULL : 1ULL << (63 - HP_Clz64(x));
}

/**
 * @brief Return the nearest power of two for a 64-bit integer
 */
static inline uint64_t HP_NearPowerOfTwo(uint64_t x)
{
    if (x <= 1) return 1ULL;
    uint64_t next = HP_NextPowerOfTwo(x);
    uint64_t prev = HP_PrevPowerOfTwo(x);
    return (x - prev < next - x) ? prev : next;
}

/**
 * @brief Clamp float to [0.0, 1.0]
 */
static inline float HP_Saturate(float x)
{
    return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

/**
 * @brief Wrap integer to [min, max)
 */
static inline int HP_WrapInt(int value, int min, int max)
{
    int range = max - min;
    return min + (value - min) % range;
}

/**
 * @brief Wrap float to [min, max)
 */
static inline float HP_Wrap(float value, float min, float max)
{
    float range = max - min;
    return min + fmodf(value - min, range);
}

/**
 * @brief Wrap radians to [-Pi, Pi]
 */
static inline float HP_WrapRadians(float radians)
{
    float wrapped = fmodf(radians + HP_PI, HP_TAU);
    if (wrapped < 0.0f) wrapped += HP_TAU;
    return wrapped - HP_PI;
}

/**
 * @brief Normalize value from [start, end] to [0,1]
 */
static inline float HP_Normalize(float value, float start, float end)
{
    return (value - start) / (end - start);
}

/**
 * @brief Remap value from [inStart, inEnd] to [outStart, outEnd]
 */
static inline float HP_Remap(float value, float inStart, float inEnd, float outStart, float outEnd)
{
    return (value - inStart) / (inEnd - inStart) * (outEnd - outStart) + outStart;
}

/**
 * @brief Ping-pong a value between min and max.
 */
static inline float HP_PingPong(float value, float min, float max)
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
static inline float HP_Fract(float x)
{
    return x - floorf(x);
}

/**
 * @brief Step function: 0 if x < edge, else 1
 */
static inline float HP_Step(float edge, float x)
{
    return (x < edge) ? 0.0f : 1.0f;
}

/**
 * @brief Sign of integer: -1, 0, 1
 */
static inline int HP_Sign(int x)
{
    return (x > 0) - (x < 0);
}

/**
 * @brief Approximate equality for floats with epsilon
 */
static inline int HP_Approx(float a, float b, float epsilon)
{
    return fabsf(a - b) < epsilon;
}

/**
 * @brief Linear interpolation
 */
static inline float HP_Lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

/**
 * @brief Linear interpolation for radians
 */
static inline float HP_LerpRadians(float a, float b, float t)
{
    return a + HP_WrapRadians(b - a) * t;
}

/**
 * @brief Inverse linear interpolation
 */
static inline float HP_LerpInverse(float a, float b, float value)
{
    return (value - a) / (b - a);
}

/**
 * @brief Smoothstep interpolation
 */
static inline float HP_Smoothstep(float edge0, float edge1, float x)
{
    float t = (x - edge0) / (edge1 - edge0);
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    return t * t * t * (t * (t * 6 - 15) + 10);
}

/**
 * @brief Exponential decay
 */
static inline float HP_ExpDecay(float initial, float decayRate, float time)
{
    return initial * expf(-decayRate * time);
}

/**
 * @brief Move current value toward target by maxDelta
 */
static inline float HP_MoveToward(float current, float target, float maxDelta)
{
    float delta = target - current;
    float distance = fabsf(delta);
    if (distance <= maxDelta) return target;
    else return current + (delta / distance) * maxDelta;
}

/**
 * @brief Convert an angle from degrees to radians
 */
static inline float HP_Radians(float degrees)
{
    return degrees * HP_DEG2RAD;
}

/**
 * @brief Convert an angle from radians to degrees
 */
static inline float HP_Degrees(float radians)
{
    return radians * HP_RAD2DEG;
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
static inline float HP_EaseSineIn(float t)
{
    return sinf(HP_PI / 2 * t);
}

/**
 * @brief Sine easing out
 * @see https://easings.net/#easeOutSine
 */
static inline float HP_EaseSineOut(float t)
{
    return 1 + sinf(HP_PI / 2 * (--t));
}

/**
 * @brief Sine easing in-out
 * @see https://easings.net/#easeInOutSine
 */
static inline float HP_EaseSineInOut(float t)
{
    return 0.5f * (1 + sinf(HP_PI * (t - 0.5f)));
}

/**
 * @brief Quadratic easing in
 * @see https://easings.net/#easeInQuad
 */
static inline float HP_EaseQuadIn(float t)
{
    return t * t;
}

/**
 * @brief Quadratic easing out
 * @see https://easings.net/#easeOutQuad
 */
static inline float HP_EaseQuadOut(float t)
{
    return t * (2 - t);
}

/**
 * @brief Quadratic easing in-out
 * @see https://easings.net/#easeInOutQuad
 */
static inline float HP_EaseQuadInOut(float t)
{
    return t < 0.5f ? 2 * t * t : t * (4 - 2 * t) - 1;
}

/**
 * @brief Cubic easing in
 * @see https://easings.net/#easeInCubic
 */
static inline float HP_EaseCubicIn(float t)
{
    return t * t * t;
}

/**
 * @brief Cubic easing out
 * @see https://easings.net/#easeOutCubic
 */
static inline float HP_EaseCubicOut(float t)
{
    --t; return 1 + t * t * t;
}

/**
 * @brief Cubic easing in-out
 * @see https://easings.net/#easeInOutCubic
 */
static inline float HP_EaseCubicInOut(float t)
{
    return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
}

/**
 * @brief Quartic easing in
 * @see https://easings.net/#easeInQuart
 */
static inline float HP_EaseQuartIn(float t)
{
    t *= t;
    return t * t;
}

/**
 * @brief Quartic easing out
 * @see https://easings.net/#easeOutQuart
 */
static inline float HP_EaseQuartOut(float t)
{
    --t; t = t * t;
    return 1 - t * t;
}

/**
 * @brief Quartic easing in-out
 * @see https://easings.net/#easeInOutQuart
 */
static inline float HP_EaseQuartInOut(float t)
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
static inline float HP_EaseQuintIn(float t)
{
    float t2 = t * t;
    return t * t2 * t2;
}

/**
 * @brief Quintic easing out
 * @see https://easings.net/#easeInOutQuint
 */
static inline float HP_EaseQuintOut(float t)
{
    --t; float t2 = t * t;
    return 1 + t * t2 * t2;
}

/**
 * @brief Quintic easing in-out
 * @see https://easings.net/#easeInOutQuint
 */
static inline float HP_EaseQuintInOut(float t)
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
static inline float HP_EaseExpoIn(float t)
{
    return (powf(2, 8 * t) - 1) / 255;
}

/**
 * @brief Exponential easing out
 * @see https://easings.net/#easeOutExpo
 */
static inline float HP_EaseExpoOut(float t)
{
    return 1 - powf(2, -8 * t);
}

/**
 * @brief Exponential easing in-out
 * @see https://easings.net/#easeInOutExpo
 */
static inline float HP_EaseExpoInOut(float t)
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
static inline float HP_EaseCircIn(float t)
{
    return 1 - sqrtf(1 - t);
}

/**
 * @brief Circular easing out
 * @see https://easings.net/#easeOutCirc
 */
static inline float HP_EaseCircOut(float t)
{
    return sqrtf(t);
}

/**
 * @brief Circular easing in-out
 * @see https://easings.net/#easeInOutCirc
 */
static inline float HP_EaseCircInOut(float t)
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
static inline float HP_EaseBackIn(float t)
{
    return t * t * (2.70158f * t - 1.70158f);
}

/**
 * @brief Back easing out
 * @see https://easings.net/#easeOutBack
 */
static inline float HP_EaseBackOut(float t)
{
    --t; return 1 + t * t * (2.70158f * t + 1.70158f);
}

/**
 * @brief Back easing in-out
 * @see https://easings.net/#easeInOutBack
 */
static inline float HP_EaseBackInOut(float t)
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
static inline float HP_EaseElasticIn(float t)
{
    float t2 = t * t;
    return t2 * t2 * sinf(t * HP_PI * 4.5f);
}

/**
 * @brief Elastic easing out
 * @see https://easings.net/#easeOutElastic
 */
static inline float HP_EaseElasticOut(float t)
{
    float t2 = (t - 1) * (t - 1);
    return 1 - t2 * t2 * cosf(t * HP_PI * 4.5f);
}

/**
 * @brief Elastic easing in-out
 * @see https://easings.net/#easeInOutElastic
 */
static inline float HP_EaseElasticInOut(float t)
{
    float t2;
    if (t < 0.45f) {
        t2 = t * t;
        return 8 * t2 * t2 * sinf(t * HP_PI * 9);
    }
    else if (t < 0.55f) {
        return 0.5f + 0.75f * sinf(t * HP_PI * 4);
    }
    t2 = (t - 1) * (t - 1);
    return 1 - 8 * t2 * t2 * sinf(t * HP_PI * 9);
}

/**
 * @brief Bounce easing in
 * @see https://easings.net/#easeInBounce
 */
static inline float HP_EaseBounceIn(float t)
{
    return powf(2, 6 * (t - 1)) * fabsf(sinf(t * HP_PI * 3.5f));
}

/**
 * @brief Bounce easing out
 * @see https://easings.net/#easeOutBounce
 */
static inline float HP_EaseBounceOut(float t)
{
    return 1 - powf(2, -6 * t) * fabsf(cosf(t * HP_PI * 3.5f));
}

/**
 * @brief Bounce easing in-out
 * @see https://easings.net/#easeInOutBounce
 */
static inline float HP_EaseBounceInOut(float t)
{
    if (t < 0.5f) {
        return 8 * powf(2, 8 * (t - 1)) * fabsf(sinf(t * HP_PI * 7));
    }
    return 1 - 8 * powf(2, -8 * t) * fabsf(sinf(t * HP_PI * 7));
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
static inline HP_IVec2 HP_IVec2Min(HP_IVec2 v, HP_IVec2 min)
{
    return HP_IVEC2(HP_MIN(v.x, min.x), HP_MIN(v.y, min.y));
}

/**
 * @brief Component-wise maximum of two vectors
 */
static inline HP_IVec2 HP_IVec2Max(HP_IVec2 v, HP_IVec2 max)
{
    return HP_IVEC2(HP_MAX(v.x, max.x), HP_MAX(v.y, max.y));
}

/**
 * @brief Clamp vector components between min and max
 */
static inline HP_IVec2 HP_IVec2Clamp(HP_IVec2 v, HP_IVec2 min, HP_IVec2 max)
{
    return HP_IVEC2(HP_CLAMP(v.x, min.x, max.x), HP_CLAMP(v.y, min.y, max.y));
}

/**
 * @brief Absolute value of vector components
 */
static inline HP_IVec2 HP_IVec2Abs(HP_IVec2 v)
{
    return HP_IVEC2(abs(v.x), abs(v.y));
}

/**
 * @brief Negate vector components
 */
static inline HP_IVec2 HP_IVec2Neg(HP_IVec2 v)
{
    return HP_IVEC2(-v.x, -v.y);
}

/**
 * @brief Reciprocal of vector components
 */
static inline HP_Vec2 HP_IVec2Rcp(HP_IVec2 v)
{
    return HP_VEC2(1.0f / v.x, 1.0f / v.y);
}

/**
 * @brief True if all components are != 0
 */
static inline bool HP_IVec2Any(HP_IVec2 v0)
{
    return (v0.x || v0.y);
}

/**
 * @brief True if any component is != 0
 */
static inline bool HP_IVec2All(HP_IVec2 v0)
{
    return (v0.x && v0.y);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline HP_IVec2 HP_IVec2Equals(HP_IVec2 v0, HP_IVec2 v1)
{
    return HP_IVEC2(v0.x == v1.x, v0.y == v1.y);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline HP_IVec2 HP_IVec2GreaterThan(HP_IVec2 v0, HP_IVec2 v1)
{
    return HP_IVEC2(v0.x > v1.x, v0.y > v1.y);
}

/**
 * @brief Component-wise addition
 */
static inline HP_IVec2 HP_IVec2Add(HP_IVec2 v0, HP_IVec2 v1)
{
    return HP_IVEC2(v0.x + v1.x, v0.y + v1.y);
}

/**
 * @brief Component-wise subtraction
 */
static inline HP_IVec2 HP_IVec2Sub(HP_IVec2 v0, HP_IVec2 v1)
{
    return HP_IVEC2(v0.x - v1.x, v0.y - v1.y);
}

/**
 * @brief Component-wise multiplication
 */
static inline HP_IVec2 HP_IVec2Mul(HP_IVec2 v0, HP_IVec2 v1)
{
    return HP_IVEC2(v0.x * v1.x, v0.y * v1.y);
}

/**
 * @brief Component-wise division
 */
static inline HP_IVec2 HP_IVec2Div(HP_IVec2 v0, HP_IVec2 v1)
{
    return HP_IVEC2(v0.x / v1.x, v0.y / v1.y);
}

/**
 * @brief Offset vector by scalar
 */
static inline HP_IVec2 HP_IVec2Offset(HP_IVec2 v, int s)
{
    return HP_IVEC2(v.x + s, v.y + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline HP_IVec2 HP_IVec2Scale(HP_IVec2 v, int s)
{
    return HP_IVEC2(v.x * s, v.y * s);
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
static inline HP_IVec3 HP_IVec3Min(HP_IVec3 v, HP_IVec3 min)
{
    return HP_IVEC3(HP_MIN(v.x, min.x), HP_MIN(v.y, min.y), HP_MIN(v.z, min.z));
}

/**
 * @brief Component-wise maximum of two vectors
 */
static inline HP_IVec3 HP_IVec3Max(HP_IVec3 v, HP_IVec3 max)
{
    return HP_IVEC3(HP_MAX(v.x, max.x), HP_MAX(v.y, max.y), HP_MAX(v.z, max.z));
}

/**
 * @brief Clamp vector components between min and max
 */
static inline HP_IVec3 HP_IVec3Clamp(HP_IVec3 v, HP_IVec3 min, HP_IVec3 max)
{
    return HP_IVEC3(HP_CLAMP(v.x, min.x, max.x), HP_CLAMP(v.y, min.y, max.y), HP_CLAMP(v.z, min.z, max.z));
}

/**
 * @brief Absolute value of vector components
 */
static inline HP_IVec3 HP_IVec3Abs(HP_IVec3 v)
{
    return HP_IVEC3(abs(v.x), abs(v.y), abs(v.z));
}

/**
 * @brief Negate vector components
 */
static inline HP_IVec3 HP_IVec3Neg(HP_IVec3 v)
{
    return HP_IVEC3(-v.x, -v.y, -v.z);
}

/**
 * @brief Reciprocal of vector components
 */
static inline HP_Vec3 HP_IVec3Rcp(HP_IVec3 v)
{
    return HP_VEC3(1.0f / v.x, 1.0f / v.y, 1.0f / v.z);
}

/**
 * @brief True if all components are != 0
 */
static inline bool HP_IVec3Any(HP_IVec3 v0)
{
    return (v0.x || v0.y || v0.z);
}

/**
 * @brief True if any component is != 0
 */
static inline bool HP_IVec3All(HP_IVec3 v0)
{
    return (v0.x && v0.y && v0.z);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline HP_IVec3 HP_IVec3Equals(HP_IVec3 v0, HP_IVec3 v1)
{
    return HP_IVEC3(v0.x == v1.x, v0.y == v1.y, v0.z == v1.z);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline HP_IVec3 HP_IVec3GreaterThan(HP_IVec3 v0, HP_IVec3 v1)
{
    return HP_IVEC3(v0.x > v1.x, v0.y > v1.y, v0.z > v1.z);
}

/**
 * @brief Component-wise addition
 */
static inline HP_IVec3 HP_IVec3Add(HP_IVec3 v0, HP_IVec3 v1)
{
    return HP_IVEC3(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

/**
 * @brief Component-wise subtraction
 */
static inline HP_IVec3 HP_IVec3Sub(HP_IVec3 v0, HP_IVec3 v1)
{
    return HP_IVEC3(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

/**
 * @brief Component-wise multiplication
 */
static inline HP_IVec3 HP_IVec3Mul(HP_IVec3 v0, HP_IVec3 v1)
{
    return HP_IVEC3(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z);
}

/**
 * @brief Component-wise division
 */
static inline HP_IVec3 HP_IVec3Div(HP_IVec3 v0, HP_IVec3 v1)
{
    return HP_IVEC3(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z);
}

/**
 * @brief Offset vector by scalar
 */
static inline HP_IVec3 HP_IVec3Offset(HP_IVec3 v, int s)
{
    return HP_IVEC3(v.x + s, v.y + s, v.z + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline HP_IVec3 HP_IVec3Scale(HP_IVec3 v, int s)
{
    return HP_IVEC3(v.x * s, v.y * s, v.z * s);
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
static inline HP_IVec4 HP_IVec4Min(HP_IVec4 v, HP_IVec4 min)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = HP_MIN(v.v[i], min.v[i]);
    }
    return v;
}

/**
 * @brief Component-wise maximum of two vectors
 */
static inline HP_IVec4 HP_IVec4Max(HP_IVec4 v, HP_IVec4 max)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = HP_MAX(v.v[i], max.v[i]);
    }
    return v;
}

/**
 * @brief Clamp vector components between min and max
 */
static inline HP_IVec4 HP_IVec4Clamp(HP_IVec4 v, HP_IVec4 min, HP_IVec4 max)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = HP_CLAMP(v.v[i], min.v[i], max.v[i]);
    }
    return v;
}

/**
 * @brief Absolute value of vector components
 */
static inline HP_IVec4 HP_IVec4Abs(HP_IVec4 v)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = abs(v.v[i]);
    }
    return v;
}

/**
 * @brief Negate vector components
 */
static inline HP_IVec4 HP_IVec4Neg(HP_IVec4 v)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] = -v.v[i];
    }
    return v;
}

/**
 * @brief Reciprocal of vector components
 */
static inline HP_Vec4 HP_IVec4Rcp(HP_IVec4 v)
{
    HP_Vec4 result;
    for (int i = 0; i < 4; i++) {
        result.v[i] = 1.0f / v.v[i];
    }
    return result;
}

/**
 * @brief True if all components are != 0
 */
static inline bool HP_IVec4Any(HP_IVec4 v0)
{
    return (v0.x || v0.y || v0.z || v0.w);
}

/**
 * @brief True if any component is != 0
 */
static inline bool HP_IVec4All(HP_IVec4 v0)
{
    return (v0.x && v0.y && v0.z && v0.w);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline HP_IVec4 HP_IVec4Equals(HP_IVec4 v0, HP_IVec4 v1)
{
    return HP_IVEC4(v0.x == v1.x, v0.y == v1.y, v0.z == v1.z, v0.w == v1.w);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline HP_IVec4 HP_IVec4GreaterThan(HP_IVec4 v0, HP_IVec4 v1)
{
    return HP_IVEC4(v0.x > v1.x, v0.y > v1.y, v0.z > v1.z, v0.w > v1.w);
}

/**
 * @brief Component-wise addition
 */
static inline HP_IVec4 HP_IVec4Add(HP_IVec4 v0, HP_IVec4 v1)
{
    for (int i = 0; i < 4; i++) {
        v0.v[i] += v1.v[i];
    }
    return v0;
}

/**
 * @brief Component-wise subtraction
 */
static inline HP_IVec4 HP_IVec4Sub(HP_IVec4 v0, HP_IVec4 v1)
{
    for (int i = 0; i < 4; i++) {
        v0.v[i] -= v1.v[i];
    }
    return v0;
}

/**
 * @brief Component-wise multiplication
 */
static inline HP_IVec4 HP_IVec4Mul(HP_IVec4 v0, HP_IVec4 v1)
{
    for (int i = 0; i < 4; i++) {
        v0.v[i] *= v1.v[i];
    }
    return v0;
}

/**
 * @brief Component-wise division
 */
static inline HP_IVec4 HP_IVec4Div(HP_IVec4 v0, HP_IVec4 v1)
{
    for (int i = 0; i < 4; i++) {
        v0.v[i] /= v1.v[i];
    }
    return v0;
}

/**
 * @brief Offset vector by scalar
 */
static inline HP_IVec4 HP_IVec4Offset(HP_IVec4 v, int s)
{
    for (int i = 0; i < 4; i++) {
        v.v[i] += s;
    }
    return v;
}

/**
 * @brief Scale vector by scalar
 */
static inline HP_IVec4 HP_IVec4Scale(HP_IVec4 v, int s)
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
 * @brief Component-wise minimum of two vectors
 */
static inline HP_Vec2 HP_Vec2Min(HP_Vec2 v, HP_Vec2 min)
{
    return HP_VEC2(HP_MIN(v.x, min.x), HP_MIN(v.y, min.y));
}

/**
 * @brief Component-wise maximum of two vectors
 */
static inline HP_Vec2 HP_Vec2Max(HP_Vec2 v, HP_Vec2 max)
{
    return HP_VEC2(HP_MAX(v.x, max.x), HP_MAX(v.y, max.y));
}

/**
 * @brief Clamp vector components between min and max
 */
static inline HP_Vec2 HP_Vec2Clamp(HP_Vec2 v, HP_Vec2 min, HP_Vec2 max)
{
    return HP_VEC2(HP_CLAMP(v.x, min.x, max.x), HP_CLAMP(v.y, min.y, max.y));
}

/**
 * @brief Absolute value of vector components
 */
static inline HP_Vec2 HP_Vec2Abs(HP_Vec2 v)
{
    return HP_VEC2(fabsf(v.x), fabsf(v.y));
}

/**
 * @brief Negate vector components
 */
static inline HP_Vec2 HP_Vec2Neg(HP_Vec2 v)
{
    return HP_VEC2(-v.x, -v.y);
}

/**
 * @brief Reciprocal of vector components
 */
static inline HP_Vec2 HP_Vec2Rcp(HP_Vec2 v)
{
    return HP_VEC2(1.0f / v.x, 1.0f / v.y);
}

/**
 * @brief Check approximate equality of two vectors
 */
static inline int HP_Vec2Approx(HP_Vec2 v0, HP_Vec2 v1, float epsilon)
{
    return (fabsf(v0.x - v1.x) < epsilon) &&
           (fabsf(v0.y - v1.y) < epsilon);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline HP_IVec2 HP_Vec2Equals(HP_Vec2 v0, HP_Vec2 v1)
{
    return HP_IVEC2(v0.x == v1.x, v0.y == v1.y);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline HP_IVec2 HP_Vec2GreaterThan(HP_Vec2 v0, HP_Vec2 v1)
{
    return HP_IVEC2(v0.x > v1.x, v0.y > v1.y);
}

/**
 * @brief Component-wise addition
 */
static inline HP_Vec2 HP_Vec2Add(HP_Vec2 v0, HP_Vec2 v1)
{
    return HP_VEC2(v0.x + v1.x, v0.y + v1.y);
}

/**
 * @brief Component-wise subtraction
 */
static inline HP_Vec2 HP_Vec2Sub(HP_Vec2 v0, HP_Vec2 v1)
{
    return HP_VEC2(v0.x - v1.x, v0.y - v1.y);
}

/**
 * @brief Component-wise multiplication
 */
static inline HP_Vec2 HP_Vec2Mul(HP_Vec2 v0, HP_Vec2 v1)
{
    return HP_VEC2(v0.x * v1.x, v0.y * v1.y);
}

/**
 * @brief Component-wise division
 */
static inline HP_Vec2 HP_Vec2Div(HP_Vec2 v0, HP_Vec2 v1)
{
    return HP_VEC2(v0.x / v1.x, v0.y / v1.y);
}

/**
 * @brief Offset vector by scalar
 */
static inline HP_Vec2 HP_Vec2Offset(HP_Vec2 v, float s)
{
    return HP_VEC2(v.x + s, v.y + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline HP_Vec2 HP_Vec2Scale(HP_Vec2 v, float s)
{
    return HP_VEC2(v.x * s, v.y * s);
}

/**
 * @brief Scalar multiply and add
 */
static inline HP_Vec2 HP_Vec2MulAdd(HP_Vec2 a, float s, HP_Vec2 b)
{
    return HP_VEC2(a.x * s + b.x, a.y * s + b.y);
}

/**
 * @brief Dot product
 */
static inline float HP_Vec2Dot(HP_Vec2 v0, HP_Vec2 v1)
{
    return v0.x * v1.x + v0.y * v1.y;
}

/**
 * @brief Vector length
 */
static inline float HP_Vec2Length(HP_Vec2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

/**
 * @brief Squared vector length
 */
static inline float HP_Vec2LengthSq(HP_Vec2 v)
{
    return v.x * v.x + v.y * v.y;
}

/**
 * @brief Normalize vector
 */
static inline HP_Vec2 HP_Vec2Normalize(HP_Vec2 v)
{
    float len = HP_Vec2Length(v);
    if (len > 0.0f) {
        return HP_Vec2Scale(v, 1.0f / len);
    }
    return HP_VEC2(0.0f, 0.0f);
}

/**
 * @brief Distance between two vectors
 */
static inline float HP_Vec2Distance(HP_Vec2 v0, HP_Vec2 v1)
{
    return HP_Vec2Length(HP_Vec2Sub(v1, v0));
}

/**
 * @brief Squared distance between two vectors
 */
static inline float HP_Vec2DistanceSq(HP_Vec2 v0, HP_Vec2 v1)
{
    HP_Vec2 diff = HP_Vec2Sub(v1, v0);
    return HP_Vec2LengthSq(diff);
}

/**
 * @brief CCW angle from X axis (radians)
 */
static inline float HP_Vec2AngleCCW(HP_Vec2 v)
{
    return atan2f(v.y, v.x);
}

/**
 * @brief CW angle from X axis (radians)
 */
static inline float HP_Vec2AngleCW(HP_Vec2 v)
{
    return -atan2f(v.y, v.x);
}

/**
 * @brief CCW angle between two vectors (radians)
 */
static inline float HP_Vec2LineAngleCCW(HP_Vec2 v0, HP_Vec2 v1)
{
    return atan2f(v1.y - v0.y, v1.x - v0.x);
}

/**
 * @brief CW angle between two vectors (radians)
 */
static inline float HP_Vec2LineAngleCW(HP_Vec2 v0, HP_Vec2 v1)
{
    return -atan2f(v1.y - v0.y, v1.x - v0.x);
}

/**
 * @brief Create unit vector from angle (radians)
 */
static inline HP_Vec2 HP_Vec2FromAngle(float angle)
{
    return HP_VEC2(cosf(angle), sinf(angle));
}

/**
 * @brief Rotate vector by angle (radians)
 */
static inline HP_Vec2 HP_Vec2Rotate(HP_Vec2 v, float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);
    return HP_VEC2(v.x * c - v.y * s, v.x * s + v.y * c);
}

/**
 * @brief Get direction from v0 to v1, normalized
 */
static inline HP_Vec2 HP_Vec2Direction(HP_Vec2 v0, HP_Vec2 v1)
{
    return HP_Vec2Normalize(HP_Vec2Sub(v1, v0));
}

/**
 * @brief Linear interpolation between vectors
 */
static inline HP_Vec2 HP_Vec2Lerp(HP_Vec2 v0, HP_Vec2 v1, float t)
{
    return HP_VEC2(
        v0.x + (v1.x - v0.x) * t,
        v0.y + (v1.y - v0.y) * t
    );
}

/**
 * @brief Move vector toward target without exceeding max_delta
 */
static inline HP_Vec2 HP_Vec2MoveToward(HP_Vec2 from, HP_Vec2 to, float max_delta)
{
    HP_Vec2 delta = HP_VEC2(to.x - from.x, to.y - from.y);
    float dist = sqrtf(delta.x * delta.x + delta.y * delta.y);

    if (dist <= max_delta || dist < 1e-6f) {
        return to;
    }

    float scale = max_delta / dist;
    return HP_VEC2(from.x + delta.x * scale, from.y + delta.y * scale);
}

/**
 * @brief Reflect vector across normal
 */
static inline HP_Vec2 HP_Vec2Reflect(HP_Vec2 v, HP_Vec2 n)
{
    float dot = HP_Vec2Dot(v, n);
    return HP_Vec2Sub(v, HP_Vec2Scale(n, 2.0f * dot));
}

/**
 * @brief Perpendicular vector (rotated 90° CCW)
 */
static inline HP_Vec2 HP_Vec2Perp(HP_Vec2 v)
{
    return HP_VEC2(-v.y, v.x);
}

/**
 * @brief Transform a 3D vector by 3x3 matrix
 */
static inline HP_Vec2 HP_Vec2TransformByMat3(HP_Vec2 v, const HP_Mat3* mat)
{
    HP_Vec2 result;
    result.x = mat->m00 * v.x + mat->m10 * v.y + mat->m20;
    result.y = mat->m01 * v.x + mat->m11 * v.y + mat->m21;
    return result;
}

/**
 * @brief Transform a 2D vector by 4x4 matrix
 */
static inline HP_Vec2 HP_Vec2TransformByMat4(HP_Vec2 v, const HP_Mat4* mat)
{
    HP_Vec2 result;
    result.x = mat->m00 * v.x + mat->m10 * v.y + mat->m30;
    result.y = mat->m01 * v.x + mat->m11 * v.y + mat->m31;

    return result;
}

/** @} */ // Vec2

/* === 3D Vector Functions === */

/** @defgroup Vec3 3D Vector Functions
 *  Inline functions for 3D vector operations.
 *  @{
 */

/**
 * @brief Component-wise minimum
 */
static inline HP_Vec3 HP_Vec3Min(HP_Vec3 v, HP_Vec3 min)
{
    return HP_VEC3(HP_MIN(v.x, min.x), HP_MIN(v.y, min.y), HP_MIN(v.z, min.z));
}

/**
 * @brief Component-wise maximum
 */
static inline HP_Vec3 HP_Vec3Max(HP_Vec3 v, HP_Vec3 max)
{
    return HP_VEC3(HP_MAX(v.x, max.x), HP_MAX(v.y, max.y), HP_MAX(v.z, max.z));
}

/**
 * @brief Clamp each component between min and max
 */
static inline HP_Vec3 HP_Vec3Clamp(HP_Vec3 v, HP_Vec3 min, HP_Vec3 max)
{
    return HP_VEC3(HP_CLAMP(v.x, min.x, max.x), HP_CLAMP(v.y, min.y, max.y), HP_CLAMP(v.z, min.z, max.z));
}

/**
 * @brief Component-wise absolute value
 */
static inline HP_Vec3 HP_Vec3Abs(HP_Vec3 v)
{
    return HP_VEC3(fabsf(v.x), fabsf(v.y), fabsf(v.z));
}

/**
 * @brief Negate vector
 */
static inline HP_Vec3 HP_Vec3Neg(HP_Vec3 v)
{
    return HP_VEC3(-v.x, -v.y, -v.z);
}

/**
 * @brief Reciprocal of vector components
 */
static inline HP_Vec3 HP_Vec3Rcp(HP_Vec3 v)
{
    return HP_VEC3(1.0f / v.x, 1.0f / v.y, 1.0f / v.z);
}

/**
 * @brief Approximate equality within epsilon
 */
static inline int HP_Vec3Approx(HP_Vec3 v0, HP_Vec3 v1, float epsilon)
{
    return (fabsf(v0.x - v1.x) < epsilon) &&
           (fabsf(v0.y - v1.y) < epsilon) &&
           (fabsf(v0.z - v1.z) < epsilon);
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline HP_IVec3 HP_Vec3Equals(HP_Vec3 v0, HP_Vec3 v1)
{
    return HP_IVEC3(v0.x == v1.x, v0.y == v1.y, v0.z == v1.z);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline HP_IVec3 HP_Vec3GreaterThan(HP_Vec3 v0, HP_Vec3 v1)
{
    return HP_IVEC3(v0.x > v1.x, v0.y > v1.y, v0.z > v1.z);
}

/**
 * @brief Vector addition
 */
static inline HP_Vec3 HP_Vec3Add(HP_Vec3 v0, HP_Vec3 v1)
{
    return HP_VEC3(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

/**
 * @brief Vector subtraction
 */
static inline HP_Vec3 HP_Vec3Sub(HP_Vec3 v0, HP_Vec3 v1)
{
    return HP_VEC3(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

/**
 * @brief Component-wise multiplication
 */
static inline HP_Vec3 HP_Vec3Mul(HP_Vec3 v0, HP_Vec3 v1)
{
    return HP_VEC3(v0.x * v1.x, v0.y * v1.y, v0.z * v1.z);
}

/**
 * @brief Component-wise division
 */
static inline HP_Vec3 HP_Vec3Div(HP_Vec3 v0, HP_Vec3 v1)
{
    return HP_VEC3(v0.x / v1.x, v0.y / v1.y, v0.z / v1.z);
}

/**
 * @brief Add scalar to each component
 */
static inline HP_Vec3 HP_Vec3Offset(HP_Vec3 v, float s)
{
    return HP_VEC3(v.x + s, v.y + s, v.z + s);
}

/**
 * @brief Scale vector by scalar
 */
static inline HP_Vec3 HP_Vec3Scale(HP_Vec3 v, float s)
{
    return HP_VEC3(v.x * s, v.y * s, v.z * s);
}

/**
 * @brief Scalar multiply and add
 */
static inline HP_Vec3 HP_Vec3MulAdd(HP_Vec3 a, float s, HP_Vec3 b)
{
    return HP_VEC3(a.x * s + b.x, a.y * s + b.y, a.z * s + b.z);
}

/**
 * @brief Dot product
 */
static inline float HP_Vec3Dot(HP_Vec3 v0, HP_Vec3 v1)
{
    return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

/**
 * @brief Cross product
 */
static inline HP_Vec3 HP_Vec3Cross(HP_Vec3 v0, HP_Vec3 v1)
{
    return HP_VEC3(
        v0.y * v1.z - v0.z * v1.y,
        v0.z * v1.x - v0.x * v1.z,
        v0.x * v1.y - v0.y * v1.x
    );
}

/**
 * @brief Vector length (magnitude)
 */
static inline float HP_Vec3Length(HP_Vec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

/**
 * @brief Squared length (avoids sqrt)
 */
static inline float HP_Vec3LengthSq(HP_Vec3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

/**
 * @brief Distance between two vectors
 */
static inline float HP_Vec3Distance(HP_Vec3 v0, HP_Vec3 v1)
{
    return HP_Vec3Length(HP_Vec3Sub(v1, v0));
}

/**
 * @brief Squared distance
 */
static inline float HP_Vec3DistanceSq(HP_Vec3 v0, HP_Vec3 v1)
{
    return HP_Vec3LengthSq(HP_Vec3Sub(v1, v0));
}

/**
 * @brief Normalize vector
 */
static inline HP_Vec3 HP_Vec3Normalize(HP_Vec3 v)
{
    float len = HP_Vec3Length(v);
    return (len > 0.0f) ? HP_Vec3Scale(v, 1.0f / len) : HP_VEC3(0.0f, 0.0f, 0.0f);
}

/**
 * @brief Rotate a vector by a quaternion
 */
static inline HP_Vec3 HP_Vec3Rotate(HP_Vec3 v, HP_Quat q)
{
    float w = q.w, x = q.x, y = q.y, z = q.z;
    float vx = v.x, vy = v.y, vz = v.z;

    float tx = 2.0f * (y * vz - z * vy);
    float ty = 2.0f * (z * vx - x * vz);
    float tz = 2.0f * (x * vy - y * vx);

    HP_Vec3 res;
    res.x = vx + w * tx + (y * tz - z * ty);
    res.y = vy + w * ty + (z * tx - x * tz);
    res.z = vz + w * tz + (x * ty - y * tx);

    return res;
}

/**
 * @brief Rotate a vector by Euler angles (yaw, pitch, roll)
 */
static inline HP_Vec3 HP_Vec3RotateEuler(HP_Vec3 v, float yaw, float pitch, float roll)
{
    float cy = cosf(yaw * 0.5f), sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f), sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f), sr = sinf(roll * 0.5f);

    HP_Quat q;
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return HP_Vec3Rotate(v, q);
}

/**
 * @brief Rotate a vector around a given axis by a specified angle
 */
static inline HP_Vec3 HP_Vec3RotateAxisAngle(HP_Vec3 v, HP_Vec3 axis, float angle)
{
    axis = HP_Vec3Normalize(axis);
    float s = sinf(angle * 0.5f);
    HP_Quat q;
    q.w = cosf(angle * 0.5f);
    q.x = axis.x * s;
    q.y = axis.y * s;
    q.z = axis.z * s;

    return HP_Vec3Rotate(v, q);
}

/**
 * @brief Direction vector from 'from' to 'to'
 */
static inline HP_Vec3 HP_Vec3Direction(HP_Vec3 from, HP_Vec3 to)
{
    return HP_Vec3Normalize(HP_Vec3Sub(to, from));
}

/**
 * @brief Linear interpolation between two vectors
 */
static inline HP_Vec3 HP_Vec3Lerp(HP_Vec3 v0, HP_Vec3 v1, float t)
{
    return HP_VEC3(
        v0.x + (v1.x - v0.x) * t,
        v0.y + (v1.y - v0.y) * t,
        v0.z + (v1.z - v0.z) * t
    );
}

/**
 * @brief Move vector toward target by max_delta
 */
static inline HP_Vec3 HP_Vec3MoveToward(HP_Vec3 from, HP_Vec3 to, float max_delta)
{
    HP_Vec3 delta = HP_VEC3(to.x - from.x, to.y - from.y, to.z - from.z);
    float dist = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);

    if (dist <= max_delta || dist < 1e-6f)
        return to;

    float scale = max_delta / dist;
    return HP_VEC3(from.x + delta.x * scale, from.y + delta.y * scale, from.z + delta.z * scale);
}

/**
 * @brief Reflect vector v around normal
 */
static inline HP_Vec3 HP_Vec3Reflect(HP_Vec3 v, HP_Vec3 normal)
{
    return HP_Vec3Sub(v, HP_Vec3Scale(normal, 2.0f * HP_Vec3Dot(v, normal)));
}

/**
 * @brief Project vector v onto another vector
 */
static inline HP_Vec3 HP_Vec3Project(HP_Vec3 v, HP_Vec3 onto)
{
    return HP_Vec3Scale(onto, HP_Vec3Dot(v, onto) / HP_Vec3LengthSq(onto));
}

/**
 * @brief Reject vector v from another vector (component perpendicular)
 */
static inline HP_Vec3 HP_Vec3Reject(HP_Vec3 v, HP_Vec3 onto)
{
    return HP_Vec3Sub(v, HP_Vec3Project(v, onto));
}

/**
 * @brief Transform a 3D vector by 3x3 matrix
 */
static inline HP_Vec3 HP_Vec3TransformByMat3(HP_Vec3 vec, const HP_Mat3* mat)
{
    HP_Vec3 result;
    result.x = mat->m00 * vec.x + mat->m10 * vec.y + mat->m20 * vec.z;
    result.y = mat->m01 * vec.x + mat->m11 * vec.y + mat->m21 * vec.z;
    result.z = mat->m02 * vec.x + mat->m12 * vec.y + mat->m22 * vec.z;
    return result;
}

/**
 * @brief Transform a 3D vector by 4x4 matrix
 */
static inline HP_Vec3 HP_Vec3TransformByMat4(HP_Vec3 v, const HP_Mat4* mat)
{
    HP_Vec3 result;
    result.x = mat->m00 * v.x + mat->m10 * v.y + mat->m20 * v.z + mat->m30;
    result.y = mat->m01 * v.x + mat->m11 * v.y + mat->m21 * v.z + mat->m31;
    result.z = mat->m02 * v.x + mat->m12 * v.y + mat->m22 * v.z + mat->m32;

    return result;
}

/** @} */ // Vec3

/* === 4D Vector Functions === */

/** @defgroup Vec4 4D Vector Functions
 *  Inline functions for 4D vector operations.
 *  @{
 */

/**
 * @brief Clamp each component of vector x to be >= min.
 */
static inline HP_Vec4 HP_Vec4Min(HP_Vec4 x, HP_Vec4 min)
{
    for (int i = 0; i < 4; ++i) {
        if (x.v[i] < min.v[i]) x.v[i] = min.v[i];
    }
    return x;
}

/**
 * @brief Clamp each component of vector x to be <= max.
 */
static inline HP_Vec4 HP_Vec4Max(HP_Vec4 x, HP_Vec4 max)
{
    for (int i = 0; i < 4; ++i) {
        if (x.v[i] < max.v[i]) x.v[i] = max.v[i];
    }
    return x;
}

/**
 * @brief Clamp each component of vector x to [min,max].
 */
static inline HP_Vec4 HP_Vec4Clamp(HP_Vec4 x, HP_Vec4 min, HP_Vec4 max)
{
    for (int i = 0; i < 4; ++i) {
        if (x.v[i] < min.v[i]) x.v[i] = min.v[i];
        else
        if (x.v[i] > max.v[i]) x.v[i] = max.v[i];
    }
    return x;
}

/**
 * @brief Absolute value of each component.
 */
static inline HP_Vec4 HP_Vec4Abs(HP_Vec4 v)
{
    for (int i = 0; i < 4; ++i) {
        v.v[i] = fabsf(v.v[i]);
    }
    return v;
}

/**
 * @brief Negate each component.
 */
static inline HP_Vec4 HP_Vec4Neg(HP_Vec4 v)
{
    for (int i = 0; i < 4; ++i) {
        v.v[i] = -v.v[i];
    }
    return v;
}

/**
 * @brief Reciprocal of vector components
 */
static inline HP_Vec4 HP_Vec4Rcp(HP_Vec4 v)
{
    for (int i = 0; i < 4; ++i) {
        v.v[i] = 1.0f / v.v[i];
    }
    return v;
}

/**
 * @brief Test approximate equality between two vectors, within epsilon tolerance.
 */
static inline int HP_Vec4Approx(HP_Vec4 v0, HP_Vec4 v1, float epsilon)
{
    for (int i = 0; i < 4; ++i) {
        if (!(fabsf(v0.x - v1.x) < epsilon)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Component-wise equality, returns int vector
 */
static inline HP_IVec4 HP_Vec4Equals(HP_Vec4 v0, HP_Vec4 v1)
{
    return HP_IVEC4(v0.x == v1.x, v0.y == v1.y, v0.z == v1.z, v0.w == v1.w);
}

/**
 * @brief Component-wise greater than, returns int vector
 */
static inline HP_IVec4 HP_Vec4GreaterThan(HP_Vec4 v0, HP_Vec4 v1)
{
    HP_IVec4 result;
    for (int i = 0; i < 4; ++i) {
        result.v[i] = (v0.v[i] > v1.v[i]);
    }
    return result;
}

/**
 * @brief Add two vectors (component-wise).
 */
static inline HP_Vec4 HP_Vec4Add(HP_Vec4 v1, HP_Vec4 v2)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] += v2.v[i];
    }
    return v1;
}

/**
 * @brief Subtract two vectors (component-wise).
 */
static inline HP_Vec4 HP_Vec4Sub(HP_Vec4 v1, HP_Vec4 v2)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] -= v2.v[i];
    }
    return v1;
}

/**
 * @brief Multiply two vectors (component-wise).
 */
static inline HP_Vec4 HP_Vec4Mul(HP_Vec4 v1, HP_Vec4 v2)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] *= v2.v[i];
    }
    return v1;
}

/**
 * @brief Divide two vectors (component-wise).
 */
static inline HP_Vec4 HP_Vec4Div(HP_Vec4 v1, HP_Vec4 v2)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] /= v2.v[i];
    }
    return v1;
}

/**
 * @brief Add scalar to each component.
 */
static inline HP_Vec4 HP_Vec4Offset(HP_Vec4 v, float s)
{
    for (int i = 0; i < 4; ++i) {
        v.v[i] += s;
    }
    return v;
}

/**
 * @brief Multiply each component by scalar.
 */
static inline HP_Vec4 HP_Vec4Scale(HP_Vec4 v, float s)
{
    for (int i = 0; i < 4; ++i) {
        v.v[i] *= s;
    }
    return v;
}

/**
 * @brief Scalar multiply and add
 */
static inline HP_Vec4 HP_Vec4MulAdd(HP_Vec4 a, float s, HP_Vec4 b)
{
    for (int i = 0; i < 4; ++i) {
        a.v[i] = a.v[i] * s + b.v[i];
    }
    return a;
}

/**
 * @brief Normalize vector (length = 1). Returns HP_VEC4_ZERO if length is too small.
 */
static inline HP_Vec4 HP_Vec4Normalize(HP_Vec4 v)
{
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    if (len < 1e-4f) return HP_VEC4_ZERO;

    float inv_len = 1.0f / len;
    for (int i = 0; i < 4; ++i) {
        v.v[i] *= inv_len;
    }

    return v;
}

/**
 * @brief Compute vector length.
 */
static inline float HP_Vec4Length(HP_Vec4 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

/**
 * @brief Compute squared vector length.
 */
static inline float HP_Vec4LengthSq(HP_Vec4 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

/**
 * @brief Compute dot product between v1 and v2.
 */
static inline float HP_Vec4Dot(HP_Vec4 v1, HP_Vec4 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

/**
 * @brief Move current vector toward target vector by at most maxDelta.
 */
static inline HP_Vec4 HP_Vec4MoveToward(HP_Vec4 current, HP_Vec4 target, float maxDelta)
{
    HP_Vec4 delta;
    for (int i = 0; i < 4; ++i) {
        delta.v[i] = target.v[i] - current.v[i];
    }

    float dist = sqrtf(delta.x * delta.x + delta.y * delta.y);
    if (dist <= maxDelta) {
        return target;
    }

    float ratio = maxDelta / dist;
    for (int i = 0; i < 4; ++i) {
        current.v[i] = delta.v[i] * ratio;
    }

    return current;
}

/**
 * @brief Linear interpolation between v1 and v2.
 */
static inline HP_Vec4 HP_Vec4Lerp(HP_Vec4 v1, HP_Vec4 v2, float t)
{
    for (int i = 0; i < 4; ++i) {
        v1.v[i] += t * (v2.v[i] - v1.v[i]);
    }
    return v1;
}

/**
 * @brief Transform vector by a 4x4 matrix.
 */
static inline HP_Vec4 HP_Vec4TransformByMat4(HP_Vec4 v, const HP_Mat4* mat)
{
    HP_Vec4 result;
    result.x = mat->m00 * v.x + mat->m10 * v.y + mat->m20 * v.z + mat->m30 * v.w;
    result.y = mat->m01 * v.x + mat->m11 * v.y + mat->m21 * v.z + mat->m31 * v.w;
    result.z = mat->m02 * v.x + mat->m12 * v.y + mat->m22 * v.z + mat->m32 * v.w;
    result.w = mat->m03 * v.x + mat->m13 * v.y + mat->m23 * v.z + mat->m33 * v.w;

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
 * @return HP_Color with normalized float components (0.0-1.0).
 */
static inline HP_Color HP_Color8(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return HP_COLOR(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

/**
 * @brief Creates a color from a 32-bit hexadecimal value.
 * @param hex Hexadecimal color in 0xRRGGBBAA format.
 * @return HP_Color with normalized float components (0.0-1.0).
 */
static inline HP_Color HP_ColorFromHex(unsigned int hex)
{
    return HP_COLOR(
        ((hex >> 24) & 0xFF) / 255.0f,
        ((hex >> 16) & 0xFF) / 255.0f,
        ((hex >> 8) & 0xFF) / 255.0f,
        (hex & 0xFF) / 255.0f
    );
}

/**
 * @brief Converts an HP_Color to a 32-bit hexadecimal value.
 * @param color HP_Color to convert.
 * @return Hexadecimal color in 0xRRGGBBAA format.
 * @note Components are clamped to the [0.0, 1.0] range before conversion.
 */
static inline unsigned int HP_ColorToHex(HP_Color color)
{
    uint8_t r = (uint8_t)(HP_CLAMP(color.r, 0.0f, 1.0f) * 255.0f);
    uint8_t g = (uint8_t)(HP_CLAMP(color.g, 0.0f, 1.0f) * 255.0f);
    uint8_t b = (uint8_t)(HP_CLAMP(color.b, 0.0f, 1.0f) * 255.0f);
    uint8_t a = (uint8_t)(HP_CLAMP(color.a, 0.0f, 1.0f) * 255.0f);
    return (r << 24) | (g << 16) | (b << 8) | a;
}

/**
 * @brief Converts an HP_Color to a 3-component vector (RGB).
 * @param color HP_Color to convert.
 * @return HP_Vec3 containing the red, green, and blue components.
 */
static inline HP_Vec3 HP_ColorToVec3(HP_Color color)
{
    return HP_VEC3(color.r, color.g, color.b);
}

/**
 * @brief Converts an HP_Color to a 4-component vector (RGBA).
 * @param color HP_Color to convert.
 * @return HP_Vec4 containing the red, green, blue, and alpha components.
 */
static inline HP_Vec4 HP_ColorToVec4(HP_Color color)
{
    return HP_VEC4(color.r, color.g, color.b, color.a);
}

/**
 * @brief Checks if a color has components out of the [0.0, 1.0] range.
 * @param color HP_Color to check.
 * @return true if any component (r, g, b) is out of range, false otherwise.
 * @note Alpha component is ignored in this check.
 */
static inline bool HP_IsColorOutOfRange(HP_Color color)
{
    return (color.r < 0.0f || color.r > 1.0f) ||
           (color.g < 0.0f || color.g > 1.0f) ||
           (color.b < 0.0f || color.b > 1.0f);
}

/**
 * @brief Clamps all components of a color to the [0.0, 1.0] range.
 * @param color HP_Color to clamp.
 * @return HP_Color with all components clamped.
 */
static inline HP_Color HP_ColorClamp(HP_Color color)
{
    return HP_COLOR(
        HP_CLAMP(color.r, 0.0f, 1.0f),
        HP_CLAMP(color.g, 0.0f, 1.0f),
        HP_CLAMP(color.b, 0.0f, 1.0f),
        HP_CLAMP(color.a, 0.0f, 1.0f)
    );
}

/**
 * @brief Normalizes the RGB components of a color if any exceeds 1.0.
 * @param color HP_Color to normalize.
 * @return HP_Color with RGB components scaled to a maximum of 1.0 and alpha clamped.
 * @note If RGB components are all within [0.0, 1.0], this function behaves like HP_ColorClamp.
 */
static inline HP_Color HP_ColorNormalize(HP_Color color)
{
    float max = HP_MAX3(color.r, color.g, color.b);
    if (max > 1.0f) {
        float invMax = 1.0f / max;
        color.r *= invMax;
        color.g *= invMax;
        color.b *= invMax;
        color.a = HP_CLAMP(color.a, 0.0f, 1.0f);
        return color;
    }
    return HP_ColorClamp(color);
}

/**
 * @brief Adds the RGB components of two colors.
 * @param a First color.
 * @param b Second color.
 * @return HP_Color with summed RGB components; alpha from the first color.
 */
static inline HP_Color HP_ColorAdd(HP_Color a, HP_Color b)
{
    return HP_COLOR(a.r + b.r, a.g + b.g, a.b + b.b, a.a);
}

/**
 * @brief Subtracts the RGB components of one color from another.
 * @param a First color.
 * @param b Second color to subtract.
 * @return HP_Color with subtracted RGB components; alpha from the first color.
 */
static inline HP_Color HP_ColorSub(HP_Color a, HP_Color b)
{
    return HP_COLOR(a.r - b.r, a.g - b.g, a.b - b.b, a.a);
}

/**
 * @brief Multiplies the RGB components of two colors.
 * @param a First color.
 * @param b Second color.
 * @return HP_Color with multiplied RGB components; alpha from the first color.
 */
static inline HP_Color HP_ColorMul(HP_Color a, HP_Color b)
{
    return HP_COLOR(a.r * b.r, a.g * b.g, a.b * b.b, a.a);
}

/**
 * @brief Divides the RGB components of one color by another.
 * @param a Numerator color.
 * @param b Denominator color.
 * @return HP_Color with divided RGB components; alpha from the first color.
 * @note Division by zero is not handled.
 */
static inline HP_Color HP_ColorDiv(HP_Color a, HP_Color b)
{
    return HP_COLOR(a.r / b.r, a.g / b.g, a.b / b.b, a.a);
}

/**
 * @brief Adds an offset to the RGB components of a color.
 * @param color Color to modify.
 * @param offset Value to add to each RGB component.
 * @return HP_Color with offset RGB components; alpha unchanged.
 */
static inline HP_Color HP_ColorOffset(HP_Color color, float offset)
{
    return HP_COLOR(color.r + offset, color.g + offset, color.b + offset, color.a);
}

/**
 * @brief Scales the RGB components of a color by a factor.
 * @param color Color to scale.
 * @param factor Multiplicative factor.
 * @return HP_Color with scaled RGB components; alpha unchanged.
 */
static inline HP_Color HP_ColorScale(HP_Color color, float factor)
{
    return HP_COLOR(color.r * factor, color.g * factor, color.b * factor, color.a);
}

/**
 * @brief Checks if two colors are approximately equal within a tolerance.
 * @param c0 First color.
 * @param c1 Second color.
 * @param epsilon Maximum allowed difference per component.
 * @return true if all components differ by less than epsilon, false otherwise.
 */
static inline bool HP_ColorApprox(HP_Color c0, HP_Color c1, float epsilon)
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
static inline HP_Color HP_ColorLerp(HP_Color a, HP_Color b, float t)
{
    float invT = 1.0f - t;
    return HP_COLOR(
        a.r * invT + b.r * t,
        a.g * invT + b.g * t,
        a.b * invT + b.b * t,
        a.a * invT + b.a * t
    );
}

/**
 * @brief Converts an RGB color to HSV representation.
 * @param color Input RGB color (components in [0.0, 1.0]).
 * @return HP_Vec3 representing HSV (hue in degrees [0, 360), saturation [0.0-1.0], value [0.0-1.0]).
 */
static inline HP_Vec3 HP_ColorToHSV(HP_Color color)
{
    float maxVal = HP_MAX3(color.r, color.g, color.b);
    float minVal = HP_MIN3(color.r, color.g, color.b);
    float delta = maxVal - minVal;

    if (maxVal == 0.0f || delta == 0.0f) {
        return HP_VEC3(0, 0, maxVal);
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

    return HP_VEC3(h, s, maxVal);
}

/**
 * @brief Creates an RGB color from HSV components.
 * @param h Hue in degrees [0, 360).
 * @param s Saturation [0.0-1.0].
 * @param v Value [0.0-1.0].
 * @param a Alpha [0.0-1.0].
 * @return HP_Color converted from HSV to RGB.
 */
static inline HP_Color HP_ColorFromHSV(float h, float s, float v, float a)
{
    if (s == 0.0f) {
        return HP_COLOR(v, v, v, a);
    }

    float h_sector = h / 60.0f;
    int sector = (int)h_sector;
    float f = h_sector - sector;

    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (sector % 6) {
        case 0: return HP_COLOR(v, t, p, a);
        case 1: return HP_COLOR(q, v, p, a);
        case 2: return HP_COLOR(p, v, t, a);
        case 3: return HP_COLOR(p, q, v, a);
        case 4: return HP_COLOR(t, p, v, a);
        case 5: return HP_COLOR(v, p, q, a);
        default: break;
    }

    return HP_COLOR(v, v, v, a);
}

/**
 * @brief Converts an RGB color to HSL representation.
 * @param color Input RGB color (components in [0.0, 1.0]).
 * @return HP_Vec3 representing HSL (hue in degrees [0, 360), saturation [0.0-1.0], lightness [0.0-1.0]).
 */
static inline HP_Vec3 HP_ColorToHSL(HP_Color color)
{
    float maxVal = HP_MAX3(color.r, color.g, color.b);
    float minVal = HP_MIN3(color.r, color.g, color.b);
    float delta = maxVal - minVal;

    float l = (maxVal + minVal) * 0.5f;

    if (delta == 0.0f) {
        return HP_VEC3(0, 0, l);
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

    return HP_VEC3(h, s, l);
}

/**
 * @brief Creates an RGB color from HSL components.
 * @param h Hue in degrees [0, 360).
 * @param s Saturation [0.0-1.0].
 * @param l Lightness [0.0-1.0].
 * @param a Alpha [0.0-1.0].
 * @return HP_Color converted from HSL to RGB.
 */
static inline HP_Color HP_ColorFromHSL(float h, float s, float l, float a)
{
    if (s == 0.0f) {
        return HP_COLOR(l, l, l, a);
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

    return HP_COLOR(r, g, b, a);
}

/**
 * @brief Computes the relative luminance of a color.
 * @param color Input color.
 * @return Luminance value in [0.0, 1.0].
 */
static inline float HP_GetColorLuminance(HP_Color color)
{
    return 0.299f * color.r + 0.587f * color.g + 0.114f * color.b;
}

/**
 * @brief Computes the brightness of a color as the maximum RGB component.
 * @param color Input color.
 * @return Brightness value in [0.0, 1.0].
 */
static inline float HP_GetColorBrightness(HP_Color color)
{
    return HP_MAX3(color.r, color.g, color.b);
}

/**
 * @brief Converts a color to grayscale using luminance.
 * @param color Input color.
 * @return Grayscale color with preserved alpha.
 */
static inline HP_Color HP_GetColorGrayscale(HP_Color color)
{
    float gray = HP_GetColorLuminance(color);
    return HP_COLOR(gray, gray, gray, color.a);
}

/**
 * @brief Inverts the RGB components of a color.
 * @param color Input color.
 * @return Inverted color with preserved alpha.
 */
static inline HP_Color HP_ColorInvert(HP_Color color)
{
    return HP_COLOR(1.0f - color.r, 1.0f - color.g, 1.0f - color.b, color.a);
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
static inline HP_Quat HP_QuatFromAxisAngle(HP_Vec3 axis, float radians)
{
    HP_Quat result;

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
HPAPI HP_Quat HP_QuatFromEuler(HP_Vec3 v);

/**
 * @brief Convert a quaternion to Euler angles (pitch, yaw, roll).
 */
HPAPI HP_Vec3 HP_QuatToEuler(HP_Quat q);

/**
 * @brief Create a quaternion from a 4x4 rotation matrix.
 */
HPAPI HP_Quat HP_QuatFromMat4(const HP_Mat4* m);

/**
 * @brief Convert a quaternion to a 4x4 rotation matrix.
 */
HPAPI HP_Mat4 HP_QuatToMat4(HP_Quat q);

/**
 * @brief Compute a quaternion that rotates from the world forward (-Z) to point from 'from' to 'to'.
 */
HPAPI HP_Quat HP_QuatLookAt(HP_Vec3 from, HP_Vec3 to, HP_Vec3 up);

/**
 * @brief Returns the forward direction (-Z) of the quaternion
 */
static inline HP_Vec3 HP_QuatForward(HP_Quat q)
{
    return HP_Vec3Rotate(HP_VEC3(0, 0, -1), q);
}

/**
 * @brief Returns the right direction (+X) of the quaternion
 */
static inline HP_Vec3 HP_QuatRight(HP_Quat q)
{
    return HP_Vec3Rotate(HP_VEC3(1, 0, 0), q);
}

/**
 * @brief Returns the up direction (+Y) of the quaternion
 */
static inline HP_Vec3 HP_QuatUp(HP_Quat q)
{
    return HP_Vec3Rotate(HP_VEC3(0, 1, 0), q);
}

/**
 * @brief Compute the length (magnitude) of a quaternion.
 */
static inline float HP_QuatLength(HP_Quat q)
{
    return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
}

/**
 * @brief Normalize a quaternion to unit length.
 */
static inline HP_Quat HP_QuatNormalize(HP_Quat q)
{
    float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (lenSq < 1e-4f) return HP_QUAT_IDENTITY;

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
static inline HP_Quat HP_QuatConjugate(HP_Quat q)
{
    return HP_QUAT(q.w, -q.x, -q.y, -q.z);
}

/**
 * @brief Inverse of a quaternion.
 */
static inline HP_Quat HP_QuatInverse(HP_Quat q)
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
static inline HP_Quat HP_QuatMul(HP_Quat a, HP_Quat b)
{
    HP_Quat result;
    result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;

    return result;
}

/**
 * @brief Linear interpolation between two quaternions.
 */
HPAPI HP_Quat HP_QuatLerp(HP_Quat a, HP_Quat b, float t);

/**
 * @brief Spherical linear interpolation (slerp) between two quaternions.
 */
HPAPI HP_Quat HP_QuatSLerp(HP_Quat a, HP_Quat b, float t);

/** @} */ // Quat

/* === Matrix 3x3 Functions === */

/** @defgroup Mat3 Matrix 3x3 Functions
 *  Functions for creating and manipulating 3x3 matrices (HP_Mat3).
 *  Primarily used for normal transformations and 2D operations.
 *  @{
 */

/**
 * @brief Extract upper-left 3x3 from a 4x4 matrix
 */
HPAPI HP_Mat3 HP_Mat3FromMat4(const HP_Mat4* mat4);

/**
 * @brief Create 2D transformation matrix (TRS)
 */
HPAPI HP_Mat3 HP_Mat3Transform2D(HP_Vec2 translation, float rotation, HP_Vec2 scale);

/**
 * @brief Create 2D translation matrix
 */
HPAPI HP_Mat3 HP_Mat3Translate2D(HP_Vec2 translation);

/**
 * @brief Create 2D rotation matrix (around Z axis)
 */
HPAPI HP_Mat3 HP_Mat3Rotate2D(float radians);

/**
 * @brief Create 2D scaling matrix
 */
HPAPI HP_Mat3 HP_Mat3Scale2D(HP_Vec2 scale);

/**
 * @brief Create 3D rotation matrix around X axis
 */
HPAPI HP_Mat3 HP_Mat3RotateX(float radians);

/**
 * @brief Create 3D rotation matrix around Y axis
 */
HPAPI HP_Mat3 HP_Mat3RotateY(float radians);

/**
 * @brief Create 3D rotation matrix around Z axis
 */
HPAPI HP_Mat3 HP_Mat3RotateZ(float radians);

/**
 * @brief Create 3D rotation matrix around arbitrary axis
 */
HPAPI HP_Mat3 HP_Mat3Rotate(HP_Vec3 axis, float radians);

/**
 * @brief Create rotation matrix from Euler angles (XYZ order)
 */
HPAPI HP_Mat3 HP_Mat3RotateXYZ(HP_Vec3 radians);

/**
 * @brief Transpose a 3x3 matrix
 */
HPAPI HP_Mat3 HP_Mat3Transpose(const HP_Mat3* mat);

/**
 * @brief Compute determinant of 3x3 matrix
 */
HPAPI float HP_Mat3Determinant(const HP_Mat3* mat);

/**
 * @brief Compute trace of 3x3 matrix
 */
static inline float HP_Mat3Trace(const HP_Mat3* mat)
{
    return mat->m00 + mat->m11 + mat->m22;
}

/**
 * @brief Invert a 3x3 matrix
 */
HPAPI HP_Mat3 HP_Mat3Inverse(const HP_Mat3* mat);

/**
 * @brief Create normal matrix from 4x4 matrix
 */
HPAPI HP_Mat3 HP_Mat3Normal(const HP_Mat4* mat);

/**
 * @brief Add two 3x3 matrices
 */
HPAPI HP_Mat3 HP_Mat3Add(const HP_Mat3* left, const HP_Mat3* right);

/**
 * @brief Subtract two 3x3 matrices
 */
HPAPI HP_Mat3 HP_Mat3Sub(const HP_Mat3* left, const HP_Mat3* right);

/**
 * @brief Multiply two 3x3 matrices
 */
HPAPI HP_Mat3 HP_Mat3Mul(const HP_Mat3* left, const HP_Mat3* right);

/** @} */ // Mat3

/* === Matrix 4x4 Functions === */

/** @defgroup Mat4 Matrix 4x4 Functions
 *  Functions for creating and manipulating 4x4 matrices (HP_Mat4).
 *  @{
 */

/**
 * @brief Create a translation matrix
 */
HPAPI HP_Mat4 HP_Mat4Translate(HP_Vec3 v);

/**
 * @brief Create a rotation matrix around an arbitrary axis
 */
HPAPI HP_Mat4 HP_Mat4Rotate(HP_Vec3 axis, float radians);

/**
 * @brief Create a rotation matrix around the X axis
 */
HPAPI HP_Mat4 HP_Mat4RotateX(float radians);

/**
 * @brief Create a rotation matrix around the Y axis
 */
HPAPI HP_Mat4 HP_Mat4RotateY(float radians);

/**
 * @brief Create a rotation matrix around the Z axis
 */
HPAPI HP_Mat4 HP_Mat4RotateZ(float radians);

/**
 * @brief Create a rotation matrix from Euler angles (XYZ order)
 */
HPAPI HP_Mat4 HP_Mat4RotateXYZ(HP_Vec3 radians);

/**
 * @brief Create a rotation matrix from Euler angles (ZYX order)
 */
HPAPI HP_Mat4 HP_Mat4RotateZYX(HP_Vec3 radians);

/**
 * @brief Create a scaling matrix
 */
HPAPI HP_Mat4 HP_Mat4Scale(HP_Vec3 scale);

/**
 * @brief Decompose a matrix into TRS
 */
HPAPI HP_Transform HP_Mat4Decompose(const HP_Mat4* mat);

/**
 * @brief Create a perspective frustum projection matrix
 */
HPAPI HP_Mat4 HP_Mat4Frustum(float left, float right, float bottom, float top, float znear, float zfar);

/**
 * @brief Create a perspective projection matrix
 */
HPAPI HP_Mat4 HP_Mat4Perspective(float fovy, float aspect, float znear, float zfar);

/**
 * @brief Create an orthographic projection matrix
 */
HPAPI HP_Mat4 HP_Mat4Ortho(float left, float right, float bottom, float top, float znear, float zfar);

/**
 * @brief Create a look-at view matrix
 */
HPAPI HP_Mat4 HP_Mat4LookAt(HP_Vec3 eye, HP_Vec3 target, HP_Vec3 up);

/**
 * @brief Compute the determinant of a matrix
 */
HPAPI float HP_Mat4Determinant(const HP_Mat4* mat);

/**
 * @brief Transpose a matrix
 */
HPAPI HP_Mat4 HP_Mat4Transpose(const HP_Mat4* mat);

/**
 * @brief Invert a matrix
 */
HPAPI HP_Mat4 HP_Mat4Inverse(const HP_Mat4* mat);

/**
 * @brief Compute the trace of a matrix
 */
HPAPI float HP_Mat4Trace(const HP_Mat4* mat);

/**
 * @brief Add two matrices
 */
HPAPI HP_Mat4 HP_Mat4Add(const HP_Mat4* left, const HP_Mat4* right);

/**
 * @brief Subtract two matrices
 */
HPAPI HP_Mat4 HP_Mat4Sub(const HP_Mat4* left, const HP_Mat4* right);

/**
 * @brief Multiply two matrices
 */
HPAPI HP_Mat4 HP_Mat4Mul(const HP_Mat4* HP_RESTRICT left, const HP_Mat4* HP_RESTRICT right);

/**
 * @brief Multiply two arrays of matrices
 */
HPAPI void HP_Mat4MulBatch(HP_Mat4* HP_RESTRICT results,
                           const HP_Mat4* HP_RESTRICT leftMatrices,
                           const HP_Mat4* HP_RESTRICT rightMatrices,
                           size_t count);

/** @} */ // Mat4

/* === Transform Functions === */

/** @defgroup Transform 3D Transform Functions
 *  Functions for creating and manipulating 3D transformations (HP_Transform).
 *  @{
 */

/** Convert HP_Transform to a 4x4 matrix */
HPAPI HP_Mat4 HP_TransformToMat4(const HP_Transform* transform);

/** Combine parent and child transform (parent first) */
HPAPI HP_Transform HP_TransformCombine(const HP_Transform* parent, const HP_Transform* child);

/** Linearly interpolate between two transforms (LERP for translation & scale, SLERP for rotation) */
HPAPI HP_Transform HP_TransformLerp(const HP_Transform* a, const HP_Transform* b, float t);

/** @} */ // Transform

#if defined(__cplusplus)
} // extern "C"
#endif

/* === C++ Operators === */

#if defined(__cplusplus)

/* === Addition Operators === */

// Vector + Vector
inline HP_IVec2 operator+(const HP_IVec2& lhs, const HP_IVec2& rhs) { return HP_IVec2Add(lhs, rhs); }
inline HP_IVec3 operator+(const HP_IVec3& lhs, const HP_IVec3& rhs) { return HP_IVec3Add(lhs, rhs); }
inline HP_IVec4 operator+(const HP_IVec4& lhs, const HP_IVec4& rhs) { return HP_IVec4Add(lhs, rhs); }
inline HP_Vec2 operator+(const HP_Vec2& lhs, const HP_Vec2& rhs) { return HP_Vec2Add(lhs, rhs); }
inline HP_Vec3 operator+(const HP_Vec3& lhs, const HP_Vec3& rhs) { return HP_Vec3Add(lhs, rhs); }
inline HP_Vec4 operator+(const HP_Vec4& lhs, const HP_Vec4& rhs) { return HP_Vec4Add(lhs, rhs); }
inline HP_Color operator+(const HP_Color& lhs, const HP_Color& rhs) { return HP_ColorAdd(lhs, rhs); }
inline HP_Mat3 operator+(const HP_Mat3& lhs, const HP_Mat3& rhs) { return HP_Mat3Add(&lhs, &rhs); }
inline HP_Mat4 operator+(const HP_Mat4& lhs, const HP_Mat4& rhs) { return HP_Mat4Add(&lhs, &rhs); }

// Vector + Scalar
inline HP_IVec2 operator+(const HP_IVec2& lhs, int rhs) { return HP_IVec2Offset(lhs, rhs); }
inline HP_IVec3 operator+(const HP_IVec3& lhs, int rhs) { return HP_IVec3Offset(lhs, rhs); }
inline HP_IVec4 operator+(const HP_IVec4& lhs, int rhs) { return HP_IVec4Offset(lhs, rhs); }
inline HP_Vec2 operator+(const HP_Vec2& lhs, float rhs) { return HP_Vec2Offset(lhs, rhs); }
inline HP_Vec3 operator+(const HP_Vec3& lhs, float rhs) { return HP_Vec3Offset(lhs, rhs); }
inline HP_Vec4 operator+(const HP_Vec4& lhs, float rhs) { return HP_Vec4Offset(lhs, rhs); }
inline HP_Color operator+(const HP_Color& lhs, float rhs) { return HP_ColorOffset(lhs, rhs); }

// Scalar + Vector
inline HP_IVec2 operator+(int lhs, const HP_IVec2& rhs) { return HP_IVec2Add(HP_IVEC2_1(lhs), rhs); }
inline HP_IVec3 operator+(int lhs, const HP_IVec3& rhs) { return HP_IVec3Add(HP_IVEC3_1(lhs), rhs); }
inline HP_IVec4 operator+(int lhs, const HP_IVec4& rhs) { return HP_IVec4Add(HP_IVEC4_1(lhs), rhs); }
inline HP_Vec2 operator+(float lhs, const HP_Vec2& rhs) { return HP_Vec2Add(HP_VEC2_1(lhs), rhs); }
inline HP_Vec3 operator+(float lhs, const HP_Vec3& rhs) { return HP_Vec3Add(HP_VEC3_1(lhs), rhs); }
inline HP_Vec4 operator+(float lhs, const HP_Vec4& rhs) { return HP_Vec4Add(HP_VEC4_1(lhs), rhs); }
inline HP_Color operator+(float lhs, const HP_Color& rhs) { return HP_ColorAdd(HP_COLOR_1(lhs), rhs); }

// Addition assignment operators
inline const HP_IVec2& operator+=(HP_IVec2& lhs, const HP_IVec2& rhs) { lhs = HP_IVec2Add(lhs, rhs); return lhs; }
inline const HP_IVec3& operator+=(HP_IVec3& lhs, const HP_IVec3& rhs) { lhs = HP_IVec3Add(lhs, rhs); return lhs; }
inline const HP_IVec4& operator+=(HP_IVec4& lhs, const HP_IVec4& rhs) { lhs = HP_IVec4Add(lhs, rhs); return lhs; }
inline const HP_Vec2& operator+=(HP_Vec2& lhs, const HP_Vec2& rhs) { lhs = HP_Vec2Add(lhs, rhs); return lhs; }
inline const HP_Vec3& operator+=(HP_Vec3& lhs, const HP_Vec3& rhs) { lhs = HP_Vec3Add(lhs, rhs); return lhs; }
inline const HP_Vec4& operator+=(HP_Vec4& lhs, const HP_Vec4& rhs) { lhs = HP_Vec4Add(lhs, rhs); return lhs; }
inline const HP_Color& operator+=(HP_Color& lhs, const HP_Color& rhs) { lhs = HP_ColorAdd(lhs, rhs); return lhs; }
inline const HP_Mat3& operator+=(HP_Mat3& lhs, const HP_Mat3& rhs) { lhs = HP_Mat3Add(&lhs, &rhs); return lhs; }
inline const HP_Mat4& operator+=(HP_Mat4& lhs, const HP_Mat4& rhs) { lhs = HP_Mat4Add(&lhs, &rhs); return lhs; }

inline const HP_IVec2& operator+=(HP_IVec2& lhs, int rhs) { lhs = HP_IVec2Offset(lhs, rhs); return lhs; }
inline const HP_IVec3& operator+=(HP_IVec3& lhs, int rhs) { lhs = HP_IVec3Offset(lhs, rhs); return lhs; }
inline const HP_IVec4& operator+=(HP_IVec4& lhs, int rhs) { lhs = HP_IVec4Offset(lhs, rhs); return lhs; }
inline const HP_Vec2& operator+=(HP_Vec2& lhs, float rhs) { lhs = HP_Vec2Offset(lhs, rhs); return lhs; }
inline const HP_Vec3& operator+=(HP_Vec3& lhs, float rhs) { lhs = HP_Vec3Offset(lhs, rhs); return lhs; }
inline const HP_Vec4& operator+=(HP_Vec4& lhs, float rhs) { lhs = HP_Vec4Offset(lhs, rhs); return lhs; }
inline const HP_Color& operator+=(HP_Color& lhs, float rhs) { lhs = HP_ColorOffset(lhs, rhs); return lhs; }

/* === Subtraction Operators === */

// Vector - Vector
inline HP_IVec2 operator-(const HP_IVec2& lhs, const HP_IVec2& rhs) { return HP_IVec2Sub(lhs, rhs); }
inline HP_IVec3 operator-(const HP_IVec3& lhs, const HP_IVec3& rhs) { return HP_IVec3Sub(lhs, rhs); }
inline HP_IVec4 operator-(const HP_IVec4& lhs, const HP_IVec4& rhs) { return HP_IVec4Sub(lhs, rhs); }
inline HP_Vec2 operator-(const HP_Vec2& lhs, const HP_Vec2& rhs) { return HP_Vec2Sub(lhs, rhs); }
inline HP_Vec3 operator-(const HP_Vec3& lhs, const HP_Vec3& rhs) { return HP_Vec3Sub(lhs, rhs); }
inline HP_Vec4 operator-(const HP_Vec4& lhs, const HP_Vec4& rhs) { return HP_Vec4Sub(lhs, rhs); }
inline HP_Color operator-(const HP_Color& lhs, const HP_Color& rhs) { return HP_ColorSub(lhs, rhs); }
inline HP_Mat3 operator-(const HP_Mat3& lhs, const HP_Mat3& rhs) { return HP_Mat3Sub(&lhs, &rhs); }
inline HP_Mat4 operator-(const HP_Mat4& lhs, const HP_Mat4& rhs) { return HP_Mat4Sub(&lhs, &rhs); }

// Vector - Scalar
inline HP_IVec2 operator-(const HP_IVec2& lhs, int rhs) { return HP_IVec2Offset(lhs, -rhs); }
inline HP_IVec3 operator-(const HP_IVec3& lhs, int rhs) { return HP_IVec3Offset(lhs, -rhs); }
inline HP_IVec4 operator-(const HP_IVec4& lhs, int rhs) { return HP_IVec4Offset(lhs, -rhs); }
inline HP_Vec2 operator-(const HP_Vec2& lhs, float rhs) { return HP_Vec2Offset(lhs, -rhs); }
inline HP_Vec3 operator-(const HP_Vec3& lhs, float rhs) { return HP_Vec3Offset(lhs, -rhs); }
inline HP_Vec4 operator-(const HP_Vec4& lhs, float rhs) { return HP_Vec4Offset(lhs, -rhs); }
inline HP_Color operator-(const HP_Color& lhs, float rhs) { return HP_ColorOffset(lhs, -rhs); }

// Scalar - Vector
inline HP_IVec2 operator-(int lhs, const HP_IVec2& rhs) { return HP_IVec2Sub(HP_IVEC2_1(lhs), rhs); }
inline HP_IVec3 operator-(int lhs, const HP_IVec3& rhs) { return HP_IVec3Sub(HP_IVEC3_1(lhs), rhs); }
inline HP_IVec4 operator-(int lhs, const HP_IVec4& rhs) { return HP_IVec4Sub(HP_IVEC4_1(lhs), rhs); }
inline HP_Vec2 operator-(float lhs, const HP_Vec2& rhs) { return HP_Vec2Sub(HP_VEC2_1(lhs), rhs); }
inline HP_Vec3 operator-(float lhs, const HP_Vec3& rhs) { return HP_Vec3Sub(HP_VEC3_1(lhs), rhs); }
inline HP_Vec4 operator-(float lhs, const HP_Vec4& rhs) { return HP_Vec4Sub(HP_VEC4_1(lhs), rhs); }
inline HP_Color operator-(float lhs, const HP_Color& rhs) { return HP_ColorSub(HP_COLOR_1(lhs), rhs); }

// Unary minus (negation)
inline HP_IVec2 operator-(const HP_IVec2& vec) { return HP_IVec2Neg(vec); }
inline HP_IVec3 operator-(const HP_IVec3& vec) { return HP_IVec3Neg(vec); }
inline HP_IVec4 operator-(const HP_IVec4& vec) { return HP_IVec4Neg(vec); }
inline HP_Vec2 operator-(const HP_Vec2& vec) { return HP_Vec2Neg(vec); }
inline HP_Vec3 operator-(const HP_Vec3& vec) { return HP_Vec3Neg(vec); }
inline HP_Vec4 operator-(const HP_Vec4& vec) { return HP_Vec4Neg(vec); }

// Subtraction assignment operators
inline const HP_IVec2& operator-=(HP_IVec2& lhs, const HP_IVec2& rhs) { lhs = HP_IVec2Sub(lhs, rhs); return lhs; }
inline const HP_IVec3& operator-=(HP_IVec3& lhs, const HP_IVec3& rhs) { lhs = HP_IVec3Sub(lhs, rhs); return lhs; }
inline const HP_IVec4& operator-=(HP_IVec4& lhs, const HP_IVec4& rhs) { lhs = HP_IVec4Sub(lhs, rhs); return lhs; }
inline const HP_Vec2& operator-=(HP_Vec2& lhs, const HP_Vec2& rhs) { lhs = HP_Vec2Sub(lhs, rhs); return lhs; }
inline const HP_Vec3& operator-=(HP_Vec3& lhs, const HP_Vec3& rhs) { lhs = HP_Vec3Sub(lhs, rhs); return lhs; }
inline const HP_Vec4& operator-=(HP_Vec4& lhs, const HP_Vec4& rhs) { lhs = HP_Vec4Sub(lhs, rhs); return lhs; }
inline const HP_Color& operator-=(HP_Color& lhs, const HP_Color& rhs) { lhs = HP_ColorSub(lhs, rhs); return lhs; }
inline const HP_Mat3& operator-=(HP_Mat3& lhs, const HP_Mat3& rhs) { lhs = HP_Mat3Sub(&lhs, &rhs); return lhs; }
inline const HP_Mat4& operator-=(HP_Mat4& lhs, const HP_Mat4& rhs) { lhs = HP_Mat4Sub(&lhs, &rhs); return lhs; }

inline const HP_IVec2& operator-=(HP_IVec2& lhs, int rhs) { lhs = HP_IVec2Offset(lhs, -rhs); return lhs; }
inline const HP_IVec3& operator-=(HP_IVec3& lhs, int rhs) { lhs = HP_IVec3Offset(lhs, -rhs); return lhs; }
inline const HP_IVec4& operator-=(HP_IVec4& lhs, int rhs) { lhs = HP_IVec4Offset(lhs, -rhs); return lhs; }
inline const HP_Vec2& operator-=(HP_Vec2& lhs, float rhs) { lhs = HP_Vec2Offset(lhs, -rhs); return lhs; }
inline const HP_Vec3& operator-=(HP_Vec3& lhs, float rhs) { lhs = HP_Vec3Offset(lhs, -rhs); return lhs; }
inline const HP_Vec4& operator-=(HP_Vec4& lhs, float rhs) { lhs = HP_Vec4Offset(lhs, -rhs); return lhs; }
inline const HP_Color& operator-=(HP_Color& lhs, float rhs) { lhs = HP_ColorOffset(lhs, -rhs); return lhs; }

/* === Multiplication Operators === */

// Vector * Scalar
inline HP_IVec2 operator*(const HP_IVec2& lhs, int rhs) { return HP_IVec2Scale(lhs, rhs); }
inline HP_IVec3 operator*(const HP_IVec3& lhs, int rhs) { return HP_IVec3Scale(lhs, rhs); }
inline HP_IVec4 operator*(const HP_IVec4& lhs, int rhs) { return HP_IVec4Scale(lhs, rhs); }
inline HP_Vec2 operator*(const HP_Vec2& lhs, float rhs) { return HP_Vec2Scale(lhs, rhs); }
inline HP_Vec3 operator*(const HP_Vec3& lhs, float rhs) { return HP_Vec3Scale(lhs, rhs); }
inline HP_Vec4 operator*(const HP_Vec4& lhs, float rhs) { return HP_Vec4Scale(lhs, rhs); }
inline HP_Color operator*(const HP_Color& lhs, float rhs) { return HP_ColorScale(lhs, rhs); }

// Scalar * Vector
inline HP_IVec2 operator*(int lhs, const HP_IVec2& rhs) { return HP_IVec2Scale(rhs, lhs); }
inline HP_IVec3 operator*(int lhs, const HP_IVec3& rhs) { return HP_IVec3Scale(rhs, lhs); }
inline HP_IVec4 operator*(int lhs, const HP_IVec4& rhs) { return HP_IVec4Scale(rhs, lhs); }
inline HP_Vec2 operator*(float lhs, const HP_Vec2& rhs) { return HP_Vec2Scale(rhs, lhs); }
inline HP_Vec3 operator*(float lhs, const HP_Vec3& rhs) { return HP_Vec3Scale(rhs, lhs); }
inline HP_Vec4 operator*(float lhs, const HP_Vec4& rhs) { return HP_Vec4Scale(rhs, lhs); }
inline HP_Color operator*(float lhs, const HP_Color& rhs) { return HP_ColorScale(rhs, lhs); }

// Vector * Vector (component-wise)
inline HP_IVec2 operator*(const HP_IVec2& lhs, const HP_IVec2& rhs) { return HP_IVec2Mul(lhs, rhs); }
inline HP_IVec3 operator*(const HP_IVec3& lhs, const HP_IVec3& rhs) { return HP_IVec3Mul(lhs, rhs); }
inline HP_IVec4 operator*(const HP_IVec4& lhs, const HP_IVec4& rhs) { return HP_IVec4Mul(lhs, rhs); }
inline HP_Vec2 operator*(const HP_Vec2& lhs, const HP_Vec2& rhs) { return HP_Vec2Mul(lhs, rhs); }
inline HP_Vec3 operator*(const HP_Vec3& lhs, const HP_Vec3& rhs) { return HP_Vec3Mul(lhs, rhs); }
inline HP_Vec4 operator*(const HP_Vec4& lhs, const HP_Vec4& rhs) { return HP_Vec4Mul(lhs, rhs); }
inline HP_Color operator*(const HP_Color& lhs, const HP_Color& rhs) { return HP_ColorMul(lhs, rhs); }

// Vector * Matrix 3x3 (transformation)
inline HP_Vec2 operator*(const HP_Vec2& lhs, const HP_Mat3& rhs) { return HP_Vec2TransformByMat3(lhs, &rhs); }
inline HP_Vec3 operator*(const HP_Vec3& lhs, const HP_Mat3& rhs) { return HP_Vec3TransformByMat3(lhs, &rhs); }

// Vector * Matrix 4x4 (transformation)
inline HP_Vec2 operator*(const HP_Vec2& lhs, const HP_Mat4& rhs) { return HP_Vec2TransformByMat4(lhs, &rhs); }
inline HP_Vec3 operator*(const HP_Vec3& lhs, const HP_Mat4& rhs) { return HP_Vec3TransformByMat4(lhs, &rhs); }
inline HP_Vec4 operator*(const HP_Vec4& lhs, const HP_Mat4& rhs) { return HP_Vec4TransformByMat4(lhs, &rhs); }

// Matrix * Matrix
inline HP_Mat3 operator*(const HP_Mat3& lhs, const HP_Mat3& rhs) { return HP_Mat3Mul(&lhs, &rhs); }
inline HP_Mat4 operator*(const HP_Mat4& lhs, const HP_Mat4& rhs) { return HP_Mat4Mul(&lhs, &rhs); }

// Quaternion multiplication
inline HP_Quat operator*(const HP_Quat& lhs, const HP_Quat& rhs) { return HP_QuatMul(lhs, rhs); }

// Multiplication assignment operators
inline const HP_IVec2& operator*=(HP_IVec2& lhs, int rhs) { lhs = HP_IVec2Scale(lhs, rhs); return lhs; }
inline const HP_IVec3& operator*=(HP_IVec3& lhs, int rhs) { lhs = HP_IVec3Scale(lhs, rhs); return lhs; }
inline const HP_IVec4& operator*=(HP_IVec4& lhs, int rhs) { lhs = HP_IVec4Scale(lhs, rhs); return lhs; }
inline const HP_Vec2& operator*=(HP_Vec2& lhs, float rhs) { lhs = HP_Vec2Scale(lhs, rhs); return lhs; }
inline const HP_Vec3& operator*=(HP_Vec3& lhs, float rhs) { lhs = HP_Vec3Scale(lhs, rhs); return lhs; }
inline const HP_Vec4& operator*=(HP_Vec4& lhs, float rhs) { lhs = HP_Vec4Scale(lhs, rhs); return lhs; }
inline const HP_Color& operator*=(HP_Color& lhs, float rhs) { lhs = HP_ColorScale(lhs, rhs); return lhs; }

inline const HP_IVec2& operator*=(HP_IVec2& lhs, const HP_IVec2& rhs) { lhs = HP_IVec2Mul(lhs, rhs); return lhs; }
inline const HP_IVec3& operator*=(HP_IVec3& lhs, const HP_IVec3& rhs) { lhs = HP_IVec3Mul(lhs, rhs); return lhs; }
inline const HP_IVec4& operator*=(HP_IVec4& lhs, const HP_IVec4& rhs) { lhs = HP_IVec4Mul(lhs, rhs); return lhs; }
inline const HP_Vec2& operator*=(HP_Vec2& lhs, const HP_Vec2& rhs) { lhs = HP_Vec2Mul(lhs, rhs); return lhs; }
inline const HP_Vec3& operator*=(HP_Vec3& lhs, const HP_Vec3& rhs) { lhs = HP_Vec3Mul(lhs, rhs); return lhs; }
inline const HP_Vec4& operator*=(HP_Vec4& lhs, const HP_Vec4& rhs) { lhs = HP_Vec4Mul(lhs, rhs); return lhs; }
inline const HP_Color& operator*=(HP_Color& lhs, const HP_Color& rhs) { lhs = HP_ColorMul(lhs, rhs); return lhs; }

inline const HP_Vec2& operator*=(HP_Vec2& lhs, const HP_Mat3& rhs) { lhs = HP_Vec2TransformByMat3(lhs, &rhs); return lhs; }
inline const HP_Vec3& operator*=(HP_Vec3& lhs, const HP_Mat3& rhs) { lhs = HP_Vec3TransformByMat3(lhs, &rhs); return lhs; }

inline const HP_Vec2& operator*=(HP_Vec2& lhs, const HP_Mat4& rhs) { lhs = HP_Vec2TransformByMat4(lhs, &rhs); return lhs; }
inline const HP_Vec3& operator*=(HP_Vec3& lhs, const HP_Mat4& rhs) { lhs = HP_Vec3TransformByMat4(lhs, &rhs); return lhs; }
inline const HP_Vec4& operator*=(HP_Vec4& lhs, const HP_Mat4& rhs) { lhs = HP_Vec4TransformByMat4(lhs, &rhs); return lhs; }

inline const HP_Mat3& operator*=(HP_Mat3& lhs, const HP_Mat3& rhs) { lhs = HP_Mat3Mul(&lhs, &rhs); return lhs; }
inline const HP_Mat4& operator*=(HP_Mat4& lhs, const HP_Mat4& rhs) { lhs = HP_Mat4Mul(&lhs, &rhs); return lhs; }
inline const HP_Quat& operator*=(HP_Quat& lhs, const HP_Quat& rhs) { lhs = HP_QuatMul(lhs, rhs); return lhs; }

/* === Division Operators === */

// Vector / Scalar
inline HP_IVec2 operator/(const HP_IVec2& lhs, int rhs) { return HP_IVec2Div(lhs, HP_IVEC2_1(rhs)); }
inline HP_IVec3 operator/(const HP_IVec3& lhs, int rhs) { return HP_IVec3Div(lhs, HP_IVEC3_1(rhs)); }
inline HP_IVec4 operator/(const HP_IVec4& lhs, int rhs) { return HP_IVec4Div(lhs, HP_IVEC4_1(rhs)); }
inline HP_Vec2 operator/(const HP_Vec2& lhs, float rhs) { return HP_Vec2Scale(lhs, 1.0f / rhs); }
inline HP_Vec3 operator/(const HP_Vec3& lhs, float rhs) { return HP_Vec3Scale(lhs, 1.0f / rhs); }
inline HP_Vec4 operator/(const HP_Vec4& lhs, float rhs) { return HP_Vec4Scale(lhs, 1.0f / rhs); }
inline HP_Color operator/(const HP_Color& lhs, float rhs) { return HP_ColorScale(lhs, 1.0f / rhs); }

// Scalar / Vector
inline HP_Vec2 operator/(float rhs, const HP_Vec2& lhs) { return HP_Vec2Div(HP_VEC2_1(rhs), lhs); }
inline HP_Vec3 operator/(float rhs, const HP_Vec3& lhs) { return HP_Vec3Div(HP_VEC3_1(rhs), lhs); }
inline HP_Vec4 operator/(float rhs, const HP_Vec4& lhs) { return HP_Vec4Div(HP_VEC4_1(rhs), lhs); }
inline HP_Color operator/(float rhs, const HP_Color& lhs) { return HP_ColorDiv(HP_COLOR_1(rhs), lhs); }

// Vector / Vector (component-wise)
inline HP_IVec2 operator/(const HP_IVec2& lhs, const HP_IVec2& rhs) { return HP_IVec2Div(lhs, rhs); }
inline HP_IVec3 operator/(const HP_IVec3& lhs, const HP_IVec3& rhs) { return HP_IVec3Div(lhs, rhs); }
inline HP_IVec4 operator/(const HP_IVec4& lhs, const HP_IVec4& rhs) { return HP_IVec4Div(lhs, rhs); }
inline HP_Vec2 operator/(const HP_Vec2& lhs, const HP_Vec2& rhs) { return HP_Vec2Div(lhs, rhs); }
inline HP_Vec3 operator/(const HP_Vec3& lhs, const HP_Vec3& rhs) { return HP_Vec3Div(lhs, rhs); }
inline HP_Vec4 operator/(const HP_Vec4& lhs, const HP_Vec4& rhs) { return HP_Vec4Div(lhs, rhs); }
inline HP_Color operator/(const HP_Color& lhs, const HP_Color& rhs) { return HP_ColorDiv(lhs, rhs); }

// Division assignment operators
inline const HP_IVec2& operator/=(HP_IVec2& lhs, int rhs) { lhs = HP_IVec2Div(lhs, HP_IVEC2_1(rhs)); return lhs; }
inline const HP_IVec3& operator/=(HP_IVec3& lhs, int rhs) { lhs = HP_IVec3Div(lhs, HP_IVEC3_1(rhs)); return lhs; }
inline const HP_IVec4& operator/=(HP_IVec4& lhs, int rhs) { lhs = HP_IVec4Div(lhs, HP_IVEC4_1(rhs)); return lhs; }
inline const HP_Vec2& operator/=(HP_Vec2& lhs, float rhs) { lhs = HP_Vec2Scale(lhs, 1.0f / rhs); return lhs; }
inline const HP_Vec3& operator/=(HP_Vec3& lhs, float rhs) { lhs = HP_Vec3Scale(lhs, 1.0f / rhs); return lhs; }
inline const HP_Vec4& operator/=(HP_Vec4& lhs, float rhs) { lhs = HP_Vec4Scale(lhs, 1.0f / rhs); return lhs; }
inline const HP_Color& operator/=(HP_Color& lhs, float rhs) { lhs = HP_ColorScale(lhs, 1.0f / rhs); return lhs; }

inline const HP_IVec2& operator/=(HP_IVec2& lhs, const HP_IVec2& rhs) { lhs = HP_IVec2Div(lhs, rhs); return lhs; }
inline const HP_IVec3& operator/=(HP_IVec3& lhs, const HP_IVec3& rhs) { lhs = HP_IVec3Div(lhs, rhs); return lhs; }
inline const HP_IVec4& operator/=(HP_IVec4& lhs, const HP_IVec4& rhs) { lhs = HP_IVec4Div(lhs, rhs); return lhs; }
inline const HP_Vec2& operator/=(HP_Vec2& lhs, const HP_Vec2& rhs) { lhs = HP_Vec2Div(lhs, rhs); return lhs; }
inline const HP_Vec3& operator/=(HP_Vec3& lhs, const HP_Vec3& rhs) { lhs = HP_Vec3Div(lhs, rhs); return lhs; }
inline const HP_Vec4& operator/=(HP_Vec4& lhs, const HP_Vec4& rhs) { lhs = HP_Vec4Div(lhs, rhs); return lhs; }
inline const HP_Color& operator/=(HP_Color& lhs, const HP_Color& rhs) { lhs = HP_ColorDiv(lhs, rhs); return lhs; }

/* === Comparison Operators === */

// Equality operators
inline bool operator==(const HP_IVec2& lhs, const HP_IVec2& rhs) { return HP_IVec2All(HP_IVec2Equals(lhs, rhs)); }
inline bool operator==(const HP_IVec3& lhs, const HP_IVec3& rhs) { return HP_IVec3All(HP_IVec3Equals(lhs, rhs)); }
inline bool operator==(const HP_IVec4& lhs, const HP_IVec4& rhs) { return HP_IVec4All(HP_IVec4Equals(lhs, rhs)); }
inline bool operator==(const HP_Vec2& lhs, const HP_Vec2& rhs) { return HP_IVec2All(HP_Vec2Equals(lhs, rhs)); }
inline bool operator==(const HP_Vec3& lhs, const HP_Vec3& rhs) { return HP_IVec3All(HP_Vec3Equals(lhs, rhs)); }
inline bool operator==(const HP_Vec4& lhs, const HP_Vec4& rhs) { return HP_IVec4All(HP_Vec4Equals(lhs, rhs)); }
inline bool operator==(const HP_Color& lhs, const HP_Color& rhs) { return HP_ColorApprox(lhs, rhs, 1e-6f); }
inline bool operator==(const HP_Quat& lhs, const HP_Quat& rhs) { return memcmp(&lhs, &rhs, sizeof(HP_Quat)) == 0; }
inline bool operator==(const HP_Mat3& lhs, const HP_Mat3& rhs) { return memcmp(&lhs, &rhs, sizeof(HP_Mat3)) == 0; }
inline bool operator==(const HP_Mat4& lhs, const HP_Mat4& rhs) { return memcmp(&lhs, &rhs, sizeof(HP_Mat4)) == 0; }

// Inequality operators
inline bool operator!=(const HP_IVec2& lhs, const HP_IVec2& rhs) { return !HP_IVec2All(HP_IVec2Equals(lhs, rhs)); }
inline bool operator!=(const HP_IVec3& lhs, const HP_IVec3& rhs) { return !HP_IVec3All(HP_IVec3Equals(lhs, rhs)); }
inline bool operator!=(const HP_IVec4& lhs, const HP_IVec4& rhs) { return !HP_IVec4All(HP_IVec4Equals(lhs, rhs)); }
inline bool operator!=(const HP_Vec2& lhs, const HP_Vec2& rhs) { return !HP_IVec2All(HP_Vec2Equals(lhs, rhs)); }
inline bool operator!=(const HP_Vec3& lhs, const HP_Vec3& rhs) { return !HP_IVec3All(HP_Vec3Equals(lhs, rhs)); }
inline bool operator!=(const HP_Vec4& lhs, const HP_Vec4& rhs) { return !HP_IVec4All(HP_Vec4Equals(lhs, rhs)); }
inline bool operator!=(const HP_Color& lhs, const HP_Color& rhs) { return !HP_ColorApprox(lhs, rhs, 1e-6f); }
inline bool operator!=(const HP_Quat& lhs, const HP_Quat& rhs) { return memcmp(&lhs, &rhs, sizeof(HP_Quat)) != 0; }
inline bool operator!=(const HP_Mat3& lhs, const HP_Mat3& rhs) { return memcmp(&lhs, &rhs, sizeof(HP_Mat3)) != 0; }
inline bool operator!=(const HP_Mat4& lhs, const HP_Mat4& rhs) { return memcmp(&lhs, &rhs, sizeof(HP_Mat4)) != 0; }

// Greater than operators
inline bool operator>(const HP_IVec2& lhs, const HP_IVec2& rhs) { return HP_IVec2All(HP_IVec2GreaterThan(lhs, rhs)); }
inline bool operator>(const HP_IVec3& lhs, const HP_IVec3& rhs) { return HP_IVec3All(HP_IVec3GreaterThan(lhs, rhs)); }
inline bool operator>(const HP_IVec4& lhs, const HP_IVec4& rhs) { return HP_IVec4All(HP_IVec4GreaterThan(lhs, rhs)); }
inline bool operator>(const HP_Vec2& lhs, const HP_Vec2& rhs) { return HP_IVec2All(HP_Vec2GreaterThan(lhs, rhs)); }
inline bool operator>(const HP_Vec3& lhs, const HP_Vec3& rhs) { return HP_IVec3All(HP_Vec3GreaterThan(lhs, rhs)); }
inline bool operator>(const HP_Vec4& lhs, const HP_Vec4& rhs) { return HP_IVec4All(HP_Vec4GreaterThan(lhs, rhs)); }

// Less than operators
inline bool operator<(const HP_IVec2& lhs, const HP_IVec2& rhs) { return HP_IVec2All(HP_IVec2GreaterThan(rhs, lhs)); }
inline bool operator<(const HP_IVec3& lhs, const HP_IVec3& rhs) { return HP_IVec3All(HP_IVec3GreaterThan(rhs, lhs)); }
inline bool operator<(const HP_IVec4& lhs, const HP_IVec4& rhs) { return HP_IVec4All(HP_IVec4GreaterThan(rhs, lhs)); }
inline bool operator<(const HP_Vec2& lhs, const HP_Vec2& rhs) { return HP_IVec2All(HP_Vec2GreaterThan(rhs, lhs)); }
inline bool operator<(const HP_Vec3& lhs, const HP_Vec3& rhs) { return HP_IVec3All(HP_Vec3GreaterThan(rhs, lhs)); }
inline bool operator<(const HP_Vec4& lhs, const HP_Vec4& rhs) { return HP_IVec4All(HP_Vec4GreaterThan(rhs, lhs)); }

#endif // __cplusplus

#endif // HP_MATH_H
