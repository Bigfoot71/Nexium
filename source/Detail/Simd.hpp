/* Simd.hpp -- Contains a small SIMD (SSE/NEON) abstraction
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#ifndef NX_DETAIL_SIMD_HPP
#define NX_DETAIL_SIMD_HPP

#include <NX/NX_Platform.h>
#include <NX/NX_Math.h>
#include <array>

namespace simd {

/* === Aliases === */

#if defined(NX_HAS_SSE)
  using float32x4 = __m128;
#elif defined(NX_HAS_NEON)
  using float32x4 = float32x4_t;
#else
  using float32x4 = float[4];
#endif

/* === Base Vector === */

struct Float4 {
    float32x4 v;

    Float4();
    Float4(float v);
    Float4(float x, float y, float z, float w);

    static Float4 fromBits(int maskBits);

    void get(float out[4]) const;
};

inline Float4::Float4()
{
#if defined(NX_HAS_SSE)
    v = _mm_setzero_ps();
#elif defined(NX_HAS_NEON)
    v = vdupq_n_f32(0.0f);
#else
    v[0] = v[1] = v[2] = v[3] = 0.0f;
#endif
}

inline Float4::Float4(float s)
{
#if defined(NX_HAS_SSE)
    v = _mm_set1_ps(s);
#elif defined(NX_HAS_NEON)
    v = vdupq_n_f32(s);
#else
    v[0] = v[1] = v[2] = v[3] = s;
#endif
}

inline Float4::Float4(float x, float y, float z, float w)
{
#if defined(NX_HAS_SSE)
    v = _mm_set_ps(w, z, y, x);
#elif defined(NX_HAS_NEON)
    float tmp[4] = {x, y, z, w};
    v = vld1q_f32(tmp);
#else
    v[0] = x; v[1] = y; v[2] = z; v[3] = w;
#endif
}

inline Float4 Float4::fromBits(int maskBits)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_castsi128_ps(_mm_set1_epi32(maskBits));
#elif defined(NX_HAS_NEON)
    r.v = vreinterpretq_f32_u32(vdupq_n_u32(static_cast<uint32_t>(maskBits)));
#else
    uint32_t m = static_cast<uint32_t>(maskBits);
    for (int i = 0; i < 4; ++i) {
        reinterpret_cast<uint32_t&>(r.v[i]) = m;
    }
#endif
    return r;
}

inline void Float4::get(float out[4]) const
{
#if defined(NX_HAS_SSE)
    _mm_storeu_ps(out, v);
#elif defined(NX_HAS_NEON)
    vst1q_f32(out, v);
#else
    out[0] = v[0];
    out[1] = v[1];
    out[2] = v[2];
    out[3] = v[3];
#endif
}

inline Float4 operator==(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_cmpeq_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    uint32x4_t mask = vceqq_f32(a.v, b.v);
    r.v = vreinterpretq_f32_u32(mask);
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        ta[i] = (ta[i] == tb[i]) ? 0xFFFFFFFF : 0.0f;
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 operator!=(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_cmpneq_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    uint32x4_t mask = vmvnq_u32(vceqq_f32(a.v, b.v));
    r.v = vreinterpretq_f32_u32(mask);
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        ta[i] = (ta[i] != tb[i]) ? 0xFFFFFFFF : 0.0f;
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 operator<(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_cmplt_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    uint32x4_t mask = vcltq_f32(a.v, b.v);
    r.v = vreinterpretq_f32_u32(mask);
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        ta[i] = (ta[i] < tb[i]) ? 0xFFFFFFFF : 0.0f;
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 operator>(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_cmpgt_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    uint32x4_t mask = vcgtq_f32(a.v, b.v);
    r.v = vreinterpretq_f32_u32(mask);
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        ta[i] = (ta[i] > tb[i]) ? 0xFFFFFFFF : 0.0f;
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 operator<=(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_cmple_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    uint32x4_t mask = vcleq_f32(a.v, b.v);
    r.v = vreinterpretq_f32_u32(mask);
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        ta[i] = (ta[i] <= tb[i]) ? 0xFFFFFFFF : 0.0f;
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 operator>=(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_cmpge_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    uint32x4_t mask = vcgeq_f32(a.v, b.v);
    r.v = vreinterpretq_f32_u32(mask);
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        ta[i] = (ta[i] >= tb[i]) ? 0xFFFFFFFF : 0.0f;
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 operator&(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_and_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    r.v = vreinterpretq_f32_u32(vandq_u32(vreinterpretq_u32_f32(a.v), vreinterpretq_u32_f32(b.v)));
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        uint32_t ua = std::bit_cast<uint32_t>(ta[i]);
        uint32_t ub = std::bit_cast<uint32_t>(tb[i]);
        uint32_t ur = ua & ub;
        ta[i] = std::bit_cast<float>(ur);
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 operator|(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_or_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    r.v = vreinterpretq_f32_u32(vorrq_u32(vreinterpretq_u32_f32(a.v), vreinterpretq_u32_f32(b.v)));
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        uint32_t ua = std::bit_cast<uint32_t>(ta[i]);
        uint32_t ub = std::bit_cast<uint32_t>(tb[i]);
        uint32_t ur = ua | ub;
        ta[i] = std::bit_cast<float>(ur);
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 operator^(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_xor_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    r.v = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(a.v), vreinterpretq_u32_f32(b.v)));
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        uint32_t ua = std::bit_cast<uint32_t>(ta[i]);
        uint32_t ub = std::bit_cast<uint32_t>(tb[i]);
        uint32_t ur = ua ^ ub;
        ta[i] = std::bit_cast<float>(ur);
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 operator~(const Float4& a)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    __m128 allOnes = _mm_castsi128_ps(_mm_set1_epi32(-1));
    r.v = _mm_xor_ps(a.v, allOnes);
#elif defined(NX_HAS_NEON)
    r.v = vreinterpretq_f32_u32(vmvnq_u32(vreinterpretq_u32_f32(a.v)));
#else
    float ta[4];
    a.get(ta);
    for (int i = 0; i < 4; ++i) {
        uint32_t ua = std::bit_cast<uint32_t>(ta[i]);
        ua = ~ua;
        ta[i] = std::bit_cast<float>(ua);
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4& operator&=(Float4& a, const Float4& b)
{
    a = a & b;
    return a;
}

inline Float4& operator|=(Float4& a, const Float4& b)
{
    a = a | b;
    return a;
}

inline Float4& operator^=(Float4& a, const Float4& b)
{
    a = a ^ b;
    return a;
}

inline Float4 operator-(const Float4& a)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_sub_ps(_mm_setzero_ps(), a.v);
#elif defined(NX_HAS_NEON)
    r.v = vnegq_f32(a.v);
#else
    for (int i = 0; i < 4; ++i) {
        r.v[i] = -a.v[i];
    }
#endif
    return r;
}

inline Float4 operator+(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_add_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    r.v = vaddq_f32(a.v, b.v);
#else
    for (int i = 0; i < 4; ++i) {
        r.v[i] = a.v[i] + b.v[i];
    }
#endif
    return r;
}

inline Float4 operator-(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_sub_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    r.v = vsubq_f32(a.v, b.v);
#else
    for (int i = 0; i < 4; ++i) {
        r.v[i] = a.v[i] - b.v[i];
    }
#endif
    return r;
}

inline Float4 operator*(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_mul_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    r.v = vmulq_f32(a.v, b.v);
#else
    for (int i = 0; i < 4; ++i) {
        r.v[i] = a.v[i] * b.v[i];
    }
#endif
    return r;
}

inline Float4 operator/(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_div_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    // NEON does not have a direct division before ARMv8, we approximate
    float32x4_t recip = vrecpeq_f32(b.v);
    recip = vmulq_f32(vrecpsq_f32(b.v, recip), recip); // Newton-Raphson
    recip = vmulq_f32(vrecpsq_f32(b.v, recip), recip); // 2nd iteration
    r.v = vmulq_f32(a.v, recip);
#else
    for (int i = 0; i < 4; ++i) {
        r.v[i] = a.v[i] / b.v[i];
    }
#endif
    return r;
}

inline Float4& operator+=(Float4& a, const Float4& b)
{
    a = a + b;
    return a;
}

inline Float4& operator-=(Float4& a, const Float4& b)
{
    a = a - b;
    return a;
}

inline Float4& operator*=(Float4& a, const Float4& b)
{
    a = a * b;
    return a;
}

inline Float4& operator/=(Float4& a, const Float4& b)
{
    a = a / b;
    return a;
}

inline int movemask(const Float4& a)
{
#if defined(NX_HAS_SSE)
    return _mm_movemask_ps(a.v);
#elif defined(NX_HAS_NEON)
    uint32x4_t mask_u32 = vreinterpretq_u32_f32(a.v);
    uint32_t tmp[4];
    vst1q_u32(tmp, mask_u32);
    int mask = 0;
    for (int i = 0; i < 4; ++i) {
        mask |= ((tmp[i] >> 31) & 1) << i;
    }
#else
    float tmp[4];
    a.get(tmp);
    int mask = 0;
    for (int i = 0; i < 4; ++i) {
        mask |= (tmp[i] < 0.0f ? 1 : 0) << i;
    }
    return mask;
#endif
}

inline Float4 sqrt(const Float4& a)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_sqrt_ps(a.v);

#elif defined(NX_HAS_NEON)
    r.v = vsqrtq_f32(a.v);

#else
    float tmp[4];
    a.get(tmp);
    for (int i = 0; i < 4; ++i) {
        tmp[i] = std::sqrt(tmp[i]);
    }
    r = Float4(tmp[0], tmp[1], tmp[2], tmp[3]);
#endif
    return r;
}

inline Float4 rsqrt(const Float4& a)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_rsqrt_ps(a.v);
#elif defined(NX_HAS_NEON)
    r.v = vrsqrteq_f32(a.v);
#else
    float tmp[4];
    a.get(tmp);
    for (int i = 0; i < 4; ++i) {
        tmp[i] = 1.0f / std::sqrt(tmp[i]);
    }
    r = Float4(tmp[0], tmp[1], tmp[2], tmp[3]);
#endif
    return r;
}

inline Float4 rcp(const Float4& a)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_rcp_ps(a.v);
#elif defined(NX_HAS_NEON)
    r.v = vrecpeq_f32(a.v);
#else
    float tmp[4];
    a.get(tmp);
    for (int i = 0; i < 4; ++i) {
        tmp[i] = 1.0f / tmp[i];
    }
    r = Float4(tmp[0], tmp[1], tmp[2], tmp[3]);
#endif
    return r;
}

inline Float4 abs(const Float4& a)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7FFFFFFF));
    r.v = _mm_and_ps(a.v, mask);
#elif defined(NX_HAS_NEON)
    r.v = vabsq_f32(a.v);
#else
    float tmp[4];
    a.get(tmp);
    for (int i = 0; i < 4; ++i) {
        tmp[i] = std::fabs(tmp[i]);
    }
    r = Float4(tmp[0], tmp[1], tmp[2], tmp[3]);
#endif
    return r;
}

inline Float4 min(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_min_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    r.v = vminq_f32(a.v, b.v);
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        ta[i] = (ta[i] < tb[i]) ? ta[i] : tb[i];
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 max(const Float4& a, const Float4& b)
{
    Float4 r;
#if defined(NX_HAS_SSE)
    r.v = _mm_max_ps(a.v, b.v);
#elif defined(NX_HAS_NEON)
    r.v = vmaxq_f32(a.v, b.v);
#else
    float ta[4], tb[4];
    a.get(ta); b.get(tb);
    for (int i = 0; i < 4; ++i) {
        ta[i] = (ta[i] > tb[i]) ? ta[i] : tb[i];
    }
    r = Float4(ta[0], ta[1], ta[2], ta[3]);
#endif
    return r;
}

inline Float4 clamp(const Float4& v, const Float4& low, const Float4& high)
{
#if defined(NX_HAS_SSE) || defined(NX_HAS_NEON)
    return min(max(v, low), high);
#else
    float tv[4], tl[4], th[4];
    v.get(tv); low.get(tl); high.get(th);
    for (int i = 0; i < 4; ++i) {
        if (tv[i] < tl[i]) tv[i] = tl[i];
        if (tv[i] > th[i]) tv[i] = th[i];
    }
    return Float4(tv[0], tv[1], tv[2], tv[3]);
#endif
}

/* === Multi Vector === */

template <int N>
struct Vector {
    std::array<Float4, N> v{};

    Vector() = default;
    explicit Vector(float f);
};

template <int N>
inline Vector<N>::Vector(float f)
{
    v.fill(Float4(f));
}

template <int N>
inline Vector<N> operator==(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] == b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator!=(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] != b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator<(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] < b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator>(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] > b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator<=(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] <= b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator>=(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] >= b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator&(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] & b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator|(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] | b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator^(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] ^ b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator~(const Vector<N>& a)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = ~a.v[i];
    }
    return r;
}

template <int N>
inline Vector<N>& operator&=(Vector<N>& a, const Vector<N>& b)
{
    a = a & b;
    return a;
}

template <int N>
inline Vector<N>& operator|=(Vector<N>& a, const Vector<N>& b)
{
    a = a | b;
    return a;
}

template <int N>
inline Vector<N>& operator^=(Vector<N>& a, const Vector<N>& b)
{
    a = a ^ b;
    return a;
}

template <int N>
inline Vector<N> operator-(const Vector<N>& a)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = -a.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator+(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] + b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator-(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] - b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator*(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] * b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N> operator/(const Vector<N>& a, const Vector<N>& b)
{
    Vector<N> r;
    for (int i = 0; i < N; ++i) {
        r.v[i] = a.v[i] / b.v[i];
    }
    return r;
}

template <int N>
inline Vector<N>& operator+=(Vector<N>& a, const Vector<N>& b)
{
    a = a + b;
    return a;
}

template <int N>
inline Vector<N>& operator-=(Vector<N>& a, const Vector<N>& b)
{
    a = a - b;
    return a;
}

template <int N>
inline Vector<N>& operator*=(Vector<N>& a, const Vector<N>& b)
{
    a = a * b;
    return a;
}

template <int N>
inline Vector<N>& operator/=(Vector<N>& a, const Vector<N>& b)
{
    a = a / b;
    return a;
}

/* === 2D Vector === */

struct Vec2 : public Vector<2> {
    using Base = Vector<2>;
    using Base::Base;

    Vec2(const NX_Vec2& vec);
    Vec2(const NX_Vec2& v0, const NX_Vec2& v1, const NX_Vec2& v2, const NX_Vec2& v3);

    const Float4& x() const;
    const Float4& y() const;

    Float4& x();
    Float4& y();
};

inline Vec2::Vec2(const NX_Vec2& vec)
{
    for (int i = 0; i < 2; i++) {
        v[i] = Float4(vec.v[i]);
    }
}

inline Vec2::Vec2(const NX_Vec2& v0, const NX_Vec2& v1, const NX_Vec2& v2, const NX_Vec2& v3)
{
    for (int i = 0; i < 2; i++) {
        v[i] = Float4(v0.v[i], v1.v[i], v2.v[i], v3.v[i]);
    }
}

inline const Float4& Vec2::x() const
{
    return v[0];
}

inline const Float4& Vec2::y() const
{
    return v[1];
}

inline Float4& Vec2::x()
{
    return v[0];
}

inline Float4& Vec2::y()
{
    return v[1];
}

inline Float4 dot(const Vec2& a, const Vec2& b)
{
    return a.v[0] * b.v[0] + a.v[1] * b.v[1];
}

inline Float4 lengthSq(const Vec2& a)
{
    return dot(a, a);
}

inline Float4 length(const Vec2& a)
{
    return sqrt(lengthSq(a));
}

inline Vec2 normalize(const Vec2& a)
{
    Float4 invLen = rsqrt(lengthSq(a));

    Vec2 r;
    r.v[0] = a.v[0] * invLen;
    r.v[1] = a.v[1] * invLen;
    return r;
}

/* === 3D Vector === */

struct Vec3 : public Vector<3> {
    using Base = Vector<3>;
    using Base::Base;

    Vec3(const NX_Vec3& vec);
    Vec3(const NX_Vec3& v0, const NX_Vec3& v1, const NX_Vec3& v2, const NX_Vec3& v3);

    inline operator Vec2() const;

    const Float4& x() const;
    const Float4& y() const;
    const Float4& z() const;

    Float4& x();
    Float4& y();
    Float4& z();
};

inline Vec3::Vec3(const NX_Vec3& vec)
{
    for (int i = 0; i < 3; i++) {
        v[i] = Float4(vec.v[i]);
    }
}

inline Vec3::Vec3(const NX_Vec3& v0, const NX_Vec3& v1, const NX_Vec3& v2, const NX_Vec3& v3)
{
    for (int i = 0; i < 3; i++) {
        v[i] = Float4(v0.v[i], v1.v[i], v2.v[i], v3.v[i]);
    }
}

inline Vec3::operator Vec2() const
{
    Vec2 r;
    r.x() = x();
    r.y() = y();
    return r;
}

inline const Float4& Vec3::x() const
{
    return v[0];
}

inline const Float4& Vec3::y() const
{
    return v[1];
}

inline const Float4& Vec3::z() const
{
    return v[2];
}

inline Float4& Vec3::x()
{
    return v[0];
}

inline Float4& Vec3::y()
{
    return v[1];
}

inline Float4& Vec3::z()
{
    return v[2];
}

inline Float4 dot(const Vec3& a, const Vec3& b)
{
    return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2];
}

inline Float4 lengthSq(const Vec3& a)
{
    return dot(a, a);
}

inline Float4 length(const Vec3& a)
{
    return sqrt(lengthSq(a));
}

inline Vec3 normalize(const Vec3& a)
{
    Float4 invLen = rsqrt(lengthSq(a));

    Vec3 r;
    r.v[0] = a.v[0] * invLen;
    r.v[1] = a.v[1] * invLen;
    r.v[2] = a.v[2] * invLen;
    return r;
}

inline Float4 distanceSq(const Vec3& a, const Vec3& b)
{
    Vec3 d;
    d.v[0] = a.v[0] - b.v[0];
    d.v[1] = a.v[1] - b.v[1];
    d.v[2] = a.v[2] - b.v[2];
    return dot(d, d);
}

inline Float4 distance(const Vec3& a, const Vec3& b)
{
    return sqrt(distanceSq(a, b));
}

inline Vec3 cross(const Vec3& a, const Vec3& b)
{
    Vec3 r;
    r.v[0] = a.v[1] * b.v[2] - a.v[2] * b.v[1];
    r.v[1] = a.v[2] * b.v[0] - a.v[0] * b.v[2];
    r.v[2] = a.v[0] * b.v[1] - a.v[1] * b.v[0];
    return r;
}

/* === Vector 4D === */

struct Vec4 : public Vector<4> {
    using Base = Vector<4>;
    using Base::Base;

    Vec4(const NX_Vec4& vec);
    Vec4(const NX_Vec4& v0, const NX_Vec4& v1, const NX_Vec4& v2, const NX_Vec4& v3);

    inline operator Vec3() const;

    const Float4& x() const;
    const Float4& y() const;
    const Float4& z() const;
    const Float4& w() const;

    Float4& x();
    Float4& y();
    Float4& z();
    Float4& w();
};

inline Vec4::Vec4(const NX_Vec4& vec)
{
    for (int i = 0; i < 4; i++) {
        v[i] = Float4(vec.v[i]);
    }
}

inline Vec4::Vec4(const NX_Vec4& v0, const NX_Vec4& v1, const NX_Vec4& v2, const NX_Vec4& v3)
{
    for (int i = 0; i < 4; i++) {
        v[i] = Float4(v0.v[i], v1.v[i], v2.v[i], v3.v[i]);
    }
}

inline Vec4::operator Vec3() const
{
    Vec3 r;
    r.x() = x();
    r.y() = y();
    r.z() = z();
    return r;
}

inline const Float4& Vec4::x() const
{
    return v[0];
}

inline const Float4& Vec4::y() const
{
    return v[1];
}

inline const Float4& Vec4::z() const
{
    return v[2];
}

inline const Float4& Vec4::w() const
{
    return v[3];
}

inline Float4& Vec4::x()
{
    return v[0];
}

inline Float4& Vec4::y()
{
    return v[1];
}

inline Float4& Vec4::z()
{
    return v[2];
}

inline Float4& Vec4::w()
{
    return v[3];
}

inline Float4 dot(const Vec4& a, const Vec4& b)
{
    return a.v[0] * b.v[0] + a.v[1] * b.v[1] + a.v[2] * b.v[2] + a.v[3] * b.v[3];
}

inline Float4 lengthSq(const Vec4& a)
{
    return dot(a, a);
}

inline Float4 length(const Vec4& a)
{
    return sqrt(lengthSq(a));
}

inline Vec4 normalize(const Vec4& a)
{
    Float4 invLen = rsqrt(lengthSq(a));

    Vec4 r;
    r.v[0] = a.v[0] * invLen;
    r.v[1] = a.v[1] * invLen;
    r.v[2] = a.v[2] * invLen;
    r.v[3] = a.v[3] * invLen;
    return r;
}

inline Float4 distanceSq(const Vec4& a, const Vec4& b)
{
    Vec4 d;
    d.v[0] = a.v[0] - b.v[0];
    d.v[1] = a.v[1] - b.v[1];
    d.v[2] = a.v[2] - b.v[2];
    d.v[3] = a.v[3] - b.v[3];
    return dot(d, d);
}

inline Float4 distance(const Vec4& a, const Vec4& b)
{
    return sqrt(distanceSq(a, b));
}

} // namespace simd

#endif // NX_DETAIL_SIMD_HPP
