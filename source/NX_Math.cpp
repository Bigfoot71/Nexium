/* NX_Math.cpp -- API definition for Nexium's math module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <SDL3/SDL_stdinc.h>
#include <NX/NX_Math.h>
#include <cmath>

/* === Quaternion Functions === */

NX_Quat NX_QuatFromEuler(NX_Vec3 v)
{
    float hx = v.x * 0.5f;
    float hy = v.y * 0.5f;
    float hz = v.z * 0.5f;

    float cx = std::cos(hx);
    float sx = std::sin(hx);
    float cy = std::cos(hy);
    float sy = std::sin(hy);
    float cz = std::cos(hz);
    float sz = std::sin(hz);

    float cycp = cy * cx;
    float sysp = sy * sx;
    float cysp = cy * sx;
    float sycp = sy * cx;

    NX_Quat q;
    q.w = cycp * cz + sysp * sz;
    q.x = cysp * cz + sycp * sz;
    q.y = sycp * cz - cysp * sz;
    q.z = cycp * sz - sysp * cz;

    float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (lenSq < 1e-6f) {
        NX_Quat identity = { 0.0f, 0.0f, 0.0f, 1.0f };
        return identity;
    }

    float invLen = 1.0f / std::sqrt(lenSq);
    q.w *= invLen;
    q.x *= invLen;
    q.y *= invLen;
    q.z *= invLen;

    return q;
}

NX_Vec3 NX_QuatToEuler(NX_Quat q)
{
    float qxx = q.x * q.x;
    float qyy = q.y * q.y;
    float qzz = q.z * q.z;

    float sinp = 2.0f * (q.w * q.x - q.y * q.z);
    float pitch;
    if (std::abs(sinp) >= 1.0f) {
        pitch = std::copysign(NX_PI * 0.5f, sinp);
    }
    else {
        pitch = std::asin(sinp);
    }

    float sinYcosP = 2.0f * (q.w * q.y + q.x * q.z);
    float cosYcosP = 1.0f - 2.0f * (qxx + qyy);
    float yaw = std::atan2(sinYcosP, cosYcosP);

    float sinRcosP = 2.0f * (q.w * q.z + q.x * q.y);
    float cosRcosP = 1.0f - 2.0f * (qxx + qzz);
    float roll = std::atan2(sinRcosP, cosRcosP);

    return NX_VEC3(pitch, yaw, roll);
}

NX_Quat NX_QuatFromMat4(const NX_Mat4* m)
{
    NX_Quat q;
    float trace = m->m00 + m->m11 + m->m22;

    if (trace > 0.0f) {
        float s = std::sqrt(trace + 1.0f);
        float invS = 0.5f / s;
        q.w = s * 0.5f;
        q.x = (m->m21 - m->m12) * invS;
        q.y = (m->m02 - m->m20) * invS;
        q.z = (m->m10 - m->m01) * invS;
    }
    else {
        if (m->m00 > m->m11 && m->m00 > m->m22) {
            float s = std::sqrt(1.0f + m->m00 - m->m11 - m->m22);
            float invS = 0.5f / s;
            q.w = (m->m21 - m->m12) * invS;
            q.x = s * 0.5f;
            q.y = (m->m01 + m->m10) * invS;
            q.z = (m->m02 + m->m20) * invS;
        }
        else if (m->m11 > m->m22) {
            float s = std::sqrt(1.0f + m->m11 - m->m00 - m->m22);
            float invS = 0.5f / s;
            q.w = (m->m02 - m->m20) * invS;
            q.x = (m->m01 + m->m10) * invS;
            q.y = s * 0.5f;
            q.z = (m->m12 + m->m21) * invS;
        }
        else {
            float s = std::sqrt(1.0f + m->m22 - m->m00 - m->m11);
            float invS = 0.5f / s;
            q.w = (m->m10 - m->m01) * invS;
            q.x = (m->m02 + m->m20) * invS;
            q.y = (m->m12 + m->m21) * invS;
            q.z = s * 0.5f;
        }
    }

    return q;
}

NX_Mat4 NX_QuatToMat4(NX_Quat q)
{
    NX_Mat4 result = NX_MAT4_IDENTITY;

    float a2 = q.x * q.x;
    float b2 = q.y * q.y;
    float c2 = q.z * q.z;
    float ac = q.x * q.z;
    float ab = q.x * q.y;
    float bc = q.y * q.z;
    float ad = q.w * q.x;
    float bd = q.w * q.y;
    float cd = q.w * q.z;

    result.m00 = 1 - 2 * (b2 + c2);
    result.m01 = 2 * (ab + cd);
    result.m02 = 2 * (ac - bd);

    result.m10 = 2 * (ab - cd);
    result.m11 = 1 - 2 * (a2 + c2);
    result.m12 = 2 * (bc + ad);

    result.m20 = 2 * (ac + bd);
    result.m21 = 2 * (bc - ad);
    result.m22 = 1 - 2 * (a2 + b2);

    return result;
}

NX_Quat NX_QuatLookTo(NX_Vec3 direction, NX_Vec3 up)
{
    float fx = direction.x, fy = direction.y, fz = direction.z;
    float flenSq = fx * fx + fy * fy + fz * fz;

    if (flenSq > 1e-6f) {
        float invFlen = 1.0f / std::sqrt(flenSq);
        fx *= invFlen;
        fy *= invFlen;
        fz *= invFlen;
    }

    float rx = fy * up.z - fz * up.y;
    float ry = fz * up.x - fx * up.z;
    float rz = fx * up.y - fy * up.x;

    float rlenSq = rx * rx + ry * ry + rz * rz;
    if (rlenSq > 1e-6f) {
        float invRlen = 1.0f / std::sqrt(rlenSq);
        rx *= invRlen;
        ry *= invRlen;
        rz *= invRlen;
    }
    else {
        rx = 1.0f; ry = 0.0f; rz = 0.0f;
    }

    float ux = fz * ry - fy * rz;
    float uy = fx * rz - fz * rx;
    float uz = fy * rx - fx * ry;

    fx = -fx; fy = -fy; fz = -fz;

    float trace = rx + uy + fz;
    NX_Quat q;

    if (trace > 0.0f) {
        float s = std::sqrt(trace + 1.0f);
        float invS = 0.5f / s;
        q.w = s * 0.5f;
        q.x = (uz - fy) * invS;
        q.y = (fx - rz) * invS;
        q.z = (ry - ux) * invS;
    }
    else if (rx > uy && rx > fz) {
        float s = std::sqrt(1.0f + rx - uy - fz);
        float invS = 0.5f / s;
        q.w = (uz - fy) * invS;
        q.x = s * 0.5f;
        q.y = (ux + ry) * invS;
        q.z = (fx + rz) * invS;
    }
    else if (uy > fz) {
        float s = std::sqrt(1.0f + uy - rx - fz);
        float invS = 0.5f / s;
        q.w = (fx - rz) * invS;
        q.x = (ux + ry) * invS;
        q.y = s * 0.5f;
        q.z = (fy + uz) * invS;
    }
    else {
        float s = std::sqrt(1.0f + fz - rx - uy);
        float invS = 0.5f / s;
        q.w = (ry - ux) * invS;
        q.x = (fx + rz) * invS;
        q.y = (fy + uz) * invS;
        q.z = s * 0.5f;
    }

    return q;
}

NX_Quat NX_QuatLookAt(NX_Vec3 eye, NX_Vec3 target, NX_Vec3 up)
{
    float dx = target.x - eye.x;
    float dy = target.y - eye.y;
    float dz = target.z - eye.z;

    return NX_QuatLookTo(NX_VEC3(dx, dy, dz), up);
}

NX_Quat NX_QuatLerp(NX_Quat a, NX_Quat b, float t)
{
    float dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
    float sign = (dot < 0.0f) ? -1.0f : 1.0f;

    float w1 = 1.0f - t;
    float w2 = t * sign;

    NX_Quat result;
    result.x = w1 * a.x + w2 * b.x;
    result.y = w1 * a.y + w2 * b.y;
    result.z = w1 * a.z + w2 * b.z;
    result.w = w1 * a.w + w2 * b.w;

    float lenSq = result.x * result.x + result.y * result.y + 
                  result.z * result.z + result.w * result.w;

    if (lenSq > 1e-6f) {
        float invLen = 1.0f / std::sqrt(lenSq);
        result.x *= invLen;
        result.y *= invLen;
        result.z *= invLen;
        result.w *= invLen;
    }

    return result;
}

NX_Quat NX_QuatSLerp(NX_Quat a, NX_Quat b, float t)
{
    float dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
    float sign = (dot < 0.0f) ? -1.0f : 1.0f;
    dot *= sign;

    float w1, w2;

    if (dot > 0.9995f) {
        w1 = 1.0f - t;
        w2 = t * sign;
    }
    else {
        float th0 = std::acos(dot);
        float th = th0 * t;
        float inv_sin_th0 = 1.0f / std::sin(th0);
        
        w1 = std::sin(th0 - th) * inv_sin_th0;
        w2 = std::sin(th) * inv_sin_th0 * sign;
    }

    NX_Quat result;
    result.x = w1 * a.x + w2 * b.x;
    result.y = w1 * a.y + w2 * b.y;
    result.z = w1 * a.z + w2 * b.z;
    result.w = w1 * a.w + w2 * b.w;

    if (dot > 0.9995f) {
        float lenSq = result.x * result.x + result.y * result.y + 
                      result.z * result.z + result.w * result.w;
        if (lenSq > 1e-6f) {
            float invLen = 1.0f / std::sqrt(lenSq);
            result.x *= invLen;
            result.y *= invLen;
            result.z *= invLen;
            result.w *= invLen;
        }
    }
    
    return result;
}

/* === Matrix 3x3 Functions === */

bool NX_IsMat3Identity(const NX_Mat3* mat)
{
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float expected = (i == j) ? 1.0f : 0.0f;
            if (std::fabs(mat->v[i][j] - expected) > 1e-6f) {
                return false;
            }
        }
    }
    return true;
}

NX_Mat3 NX_Mat3Transform2D(NX_Vec2 translation, float rotation, NX_Vec2 scale)
{
    float c = std::cos(rotation);
    float s = std::sin(rotation);

    NX_Mat3 result;

    result.m00 =  c * scale.x;
    result.m01 =  s * scale.x;
    result.m02 =  0.0f;

    result.m10 = -s * scale.y;
    result.m11 =  c * scale.y;
    result.m12 =  0.0f;

    result.m20 =  translation.x;
    result.m21 =  translation.y;
    result.m22 =  1.0f;

    return result;
}

NX_Mat3 NX_Mat3Translate2D(NX_Vec2 translation)
{
    NX_Mat3 result;

    result.m00 = 1.0f;
    result.m01 = 0.0f;
    result.m02 = 0.0f;

    result.m10 = 0.0f;
    result.m11 = 1.0f;
    result.m12 = 0.0f;

    result.m20 = translation.x;
    result.m21 = translation.y;
    result.m22 = 1.0f;

    return result;
}

NX_Mat3 NX_Mat3Rotate2D(float radians)
{
    float c = std::cos(radians);
    float s = std::sin(radians);

    NX_Mat3 result;

    result.m00 = c;
    result.m01 = s;
    result.m02 = 0.0f;

    result.m10 = -s;
    result.m11 = c;
    result.m12 = 0.0f;

    result.m20 = 0.0f;
    result.m21 = 0.0f;
    result.m22 = 1.0f;

    return result;
}

NX_Mat3 NX_Mat3Scale2D(NX_Vec2 scale)
{
    NX_Mat3 result;

    result.m00 = scale.x;
    result.m01 = 0.0f;
    result.m02 = 0.0f;

    result.m10 = 0.0f;
    result.m11 = scale.y;
    result.m12 = 0.0f;

    result.m20 = 0.0f;
    result.m21 = 0.0f;
    result.m22 = 1.0f;

    return result;
}

NX_Mat3 NX_Mat3RotateX(float radians)
{
    float c = std::cos(radians);
    float s = std::sin(radians);

    NX_Mat3 result;

    result.m00 = 1.0f;
    result.m01 = 0.0f;
    result.m02 = 0.0f;

    result.m10 = 0.0f;
    result.m11 = c;
    result.m12 = s;

    result.m20 = 0.0f;
    result.m21 = -s;
    result.m22 = c;

    return result;
}

NX_Mat3 NX_Mat3RotateY(float radians)
{
    float c = std::cos(radians);
    float s = std::sin(radians);

    NX_Mat3 result;

    result.m00 = c;
    result.m01 = 0.0f;
    result.m02 = -s;

    result.m10 = 0.0f;
    result.m11 = 1.0f;
    result.m12 = 0.0f;

    result.m20 = s;
    result.m21 = 0.0f;
    result.m22 = c;

    return result;
}

NX_Mat3 NX_Mat3RotateZ(float radians)
{
    float c = std::cos(radians);
    float s = std::sin(radians);

    NX_Mat3 result;

    result.m00 = c;
    result.m01 = s;
    result.m02 = 0.0f;

    result.m10 = -s;
    result.m11 = c;
    result.m12 = 0.0f;

    result.m20 = 0.0f;
    result.m21 = 0.0f;
    result.m22 = 1.0f;

    return result;
}

NX_Mat3 NX_Mat3Rotate(NX_Vec3 axis, float radians)
{
    float x = axis.x, y = axis.y, z = axis.z;
    float lenSq = x * x + y * y + z * z;

    if (std::abs(lenSq - 1.0f) > 1e-6f) {
        if (lenSq < 1e-6f) {
            return NX_MAT3_IDENTITY;
        }
        float invLen = 1.0f / std::sqrt(lenSq);
        x *= invLen;
        y *= invLen;
        z *= invLen;
    }

    float c = std::cos(radians);
    float s = std::sin(radians);
    float t = 1.0f - c;

    float tx = t * x, ty = t * y, tz = t * z;
    float txy = tx * y, txz = tx * z, tyz = ty * z;
    float sx = s * x, sy = s * y, sz = s * z;

    NX_Mat3 m;

    m.m00 = tx * x + c;
    m.m01 = txy + sz;
    m.m02 = txz - sy;

    m.m10 = txy - sz;
    m.m11 = ty * y + c;
    m.m12 = tyz + sx;

    m.m20 = txz + sy;
    m.m21 = tyz - sx;
    m.m22 = tz * z + c;

    return m;
}

NX_Mat3 NX_Mat3RotateXYZ(NX_Vec3 radians)
{
    float cz = std::cos(-radians.z);
    float sz = std::sin(-radians.z);
    float cy = std::cos(-radians.y);
    float sy = std::sin(-radians.y);
    float cx = std::cos(-radians.x);
    float sx = std::sin(-radians.x);

    NX_Mat3 result;

    result.m00 = cz * cy;
    result.m01 = (cz * sy * sx) - (sz * cx);
    result.m02 = (cz * sy * cx) + (sz * sx);

    result.m10 = sz * cy;
    result.m11 = (sz * sy * sx) + (cz * cx);
    result.m12 = (sz * sy * cx) - (cz * sx);

    result.m20 = -sy;
    result.m21 = cy * sx;
    result.m22 = cy * cx;

    return result;
}

NX_Mat3 NX_Mat3Transpose(const NX_Mat3* mat)
{
    NX_Mat3 result;
    result.m00 = mat->m00; result.m01 = mat->m10; result.m02 = mat->m20;
    result.m10 = mat->m01; result.m11 = mat->m11; result.m12 = mat->m21;
    result.m20 = mat->m02; result.m21 = mat->m12; result.m22 = mat->m22;
    return result;
}

float NX_Mat3Determinant(const NX_Mat3* mat)
{
    return mat->m00 * (mat->m11 * mat->m22 - mat->m12 * mat->m21) -
           mat->m01 * (mat->m10 * mat->m22 - mat->m12 * mat->m20) +
           mat->m02 * (mat->m10 * mat->m21 - mat->m11 * mat->m20);
}

NX_Mat3 NX_Mat3Inverse(const NX_Mat3* mat)
{
    float det = NX_Mat3Determinant(mat);

    if (std::abs(det) < 1e-6f) {
        return NX_MAT3_IDENTITY;
    }

    float invDet = 1.0f / det;

    NX_Mat3 result;

    result.m00 = (mat->m11 * mat->m22 - mat->m12 * mat->m21) * invDet;
    result.m01 = (mat->m02 * mat->m21 - mat->m01 * mat->m22) * invDet;
    result.m02 = (mat->m01 * mat->m12 - mat->m02 * mat->m11) * invDet;

    result.m10 = (mat->m12 * mat->m20 - mat->m10 * mat->m22) * invDet;
    result.m11 = (mat->m00 * mat->m22 - mat->m02 * mat->m20) * invDet;
    result.m12 = (mat->m02 * mat->m10 - mat->m00 * mat->m12) * invDet;

    result.m20 = (mat->m10 * mat->m21 - mat->m11 * mat->m20) * invDet;
    result.m21 = (mat->m01 * mat->m20 - mat->m00 * mat->m21) * invDet;
    result.m22 = (mat->m00 * mat->m11 - mat->m01 * mat->m10) * invDet;

    return result;
}

NX_Mat3 NX_Mat3Normal(const NX_Mat4* mat)
{
    float m00 = mat->m00, m01 = mat->m01, m02 = mat->m02;
    float m10 = mat->m10, m11 = mat->m11, m12 = mat->m12;
    float m20 = mat->m20, m21 = mat->m21, m22 = mat->m22;

    float c00 = m11 * m22 - m12 * m21;
    float c01 = m12 * m20 - m10 * m22;
    float c02 = m10 * m21 - m11 * m20;

    float det = m00 * c00 + m01 * c01 + m02 * c02;

    if (std::abs(det) < 1e-6f) {
        return NX_MAT3_IDENTITY;
    }

    float invDet = 1.0f / det;

    float c10 = m02 * m21 - m01 * m22;
    float c11 = m00 * m22 - m02 * m20;
    float c12 = m01 * m20 - m00 * m21;

    float c20 = m01 * m12 - m02 * m11;
    float c21 = m02 * m10 - m00 * m12;
    float c22 = m00 * m11 - m01 * m10;

    NX_Mat3 result;
    result.m00 = c00 * invDet;
    result.m01 = c01 * invDet;
    result.m02 = c02 * invDet;

    result.m10 = c10 * invDet;
    result.m11 = c11 * invDet;
    result.m12 = c12 * invDet;

    result.m20 = c20 * invDet;
    result.m21 = c21 * invDet;
    result.m22 = c22 * invDet;

    return result;
}

NX_Mat3 NX_Mat3Add(const NX_Mat3* left, const NX_Mat3* right)
{
    NX_Mat3 result;
    for (int i = 0; i < 9; ++i) {
        result.a[i] = left->a[i] + right->a[i];
    }
    return result;
}

NX_Mat3 NX_Mat3Sub(const NX_Mat3* left, const NX_Mat3* right)
{
    NX_Mat3 result;
    for (int i = 0; i < 9; ++i) {
        result.a[i] = left->a[i] - right->a[i];
    }
    return result;
}

NX_Mat3 NX_Mat3Mul(const NX_Mat3* NX_RESTRICT left, const NX_Mat3* NX_RESTRICT right)
{
    NX_Mat3 result;

    const float* NX_RESTRICT A = left->a;
    const float* NX_RESTRICT B = right->a;
    float* NX_RESTRICT R = result.a;

    R[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6];
    R[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7];
    R[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8];

    R[3] = A[3] * B[0] + A[4] * B[3] + A[5] * B[6];
    R[4] = A[3] * B[1] + A[4] * B[4] + A[5] * B[7];
    R[5] = A[3] * B[2] + A[4] * B[5] + A[5] * B[8];

    R[6] = A[6] * B[0] + A[7] * B[3] + A[8] * B[6];
    R[7] = A[6] * B[1] + A[7] * B[4] + A[8] * B[7];
    R[8] = A[6] * B[2] + A[7] * B[5] + A[8] * B[8];

    return result;
}

/* === Matrix 4x4 Functions === */

bool NX_IsMat4Identity(const NX_Mat4* mat)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float expected = (i == j) ? 1.0f : 0.0f;
            if (std::fabs(mat->v[i][j] - expected) > 1e-6f) {
                return false;
            }
        }
    }
    return true;
}

NX_Mat4 NX_Mat4Translate(NX_Vec3 v)
{
    NX_Mat4 result = NX_MAT4_IDENTITY;

    result.m30 = v.x;
    result.m31 = v.y;
    result.m32 = v.z;

    return result;
}

NX_Mat4 NX_Mat4Rotate(NX_Vec3 axis, float radians)
{
    float x = axis.x, y = axis.y, z = axis.z;
    float lenSq = x * x + y * y + z * z;

    if (std::abs(lenSq - 1.0f) > 1e-6f) {
        if (lenSq < 1e-6f) {
            return NX_MAT4_IDENTITY;
        }
        float invLen = 1.0f / std::sqrt(lenSq);
        x *= invLen;
        y *= invLen;
        z *= invLen;
    }

    float c = std::cos(radians);
    float s = std::sin(radians);
    float t = 1.0f - c;

    float tx = t * x, ty = t * y, tz = t * z;
    float txy = tx * y, txz = tx * z, tyz = ty * z;
    float sx = s * x, sy = s * y, sz = s * z;

    NX_Mat4 m;

    m.m00 = tx * x + c;
    m.m01 = txy + sz;
    m.m02 = txz - sy;
    m.m03 = 0.0f;

    m.m10 = txy - sz;
    m.m11 = ty * y + c;
    m.m12 = tyz + sx;
    m.m13 = 0.0f;

    m.m20 = txz + sy;
    m.m21 = tyz - sx;
    m.m22 = tz * z + c;
    m.m23 = 0.0f;

    m.m30 = 0.0f;
    m.m31 = 0.0f;
    m.m32 = 0.0f;
    m.m33 = 1.0f;

    return m;
}

NX_Mat4 NX_Mat4RotateX(float radians)
{
    NX_Mat4 result = NX_MAT4_IDENTITY;

    float c = std::cos(radians);
    float s = std::sin(radians);

    result.m11 = c;
    result.m12 = s;
    result.m21 = -s;
    result.m22 = c;

    return result;
}

NX_Mat4 NX_Mat4RotateY(float radians)
{
    NX_Mat4 result = NX_MAT4_IDENTITY;

    float c = std::cos(radians);
    float s = std::sin(radians);

    result.m00 = c;
    result.m02 = -s;
    result.m20 = s;
    result.m22 = c;

    return result;
}

NX_Mat4 NX_Mat4RotateZ(float radians)
{
    NX_Mat4 result = NX_MAT4_IDENTITY;

    float c = std::cos(radians);
    float s = std::sin(radians);

    result.m00 = c;
    result.m01 = s;
    result.m10 = -s;
    result.m11 = c;

    return result;
}

NX_Mat4 NX_Mat4RotateXYZ(NX_Vec3 radians)
{
    NX_Mat4 result = NX_MAT4_IDENTITY;

    float cz = std::cos(-radians.z);
    float sz = std::sin(-radians.z);
    float cy = std::cos(-radians.y);
    float sy = std::sin(-radians.y);
    float cx = std::cos(-radians.x);
    float sx = std::sin(-radians.x);

    result.m00 = cz * cy;
    result.m01 = (cz * sy * sx) - (sz * cx);
    result.m02 = (cz * sy * cx) + (sz * sx);

    result.m10 = sz * cy;
    result.m11 = (sz * sy * sx) + (cz * cx);
    result.m12 = (sz * sy * cx) - (cz * sx);

    result.m20 = -sy;
    result.m21 = cy * sx;
    result.m22 = cy * cx;

    return result;
}

NX_Mat4 NX_Mat4RotateZYX(NX_Vec3 radians)
{
    NX_Mat4 result = NX_MAT4_IDENTITY;

    float cz = std::cos(radians.z);
    float sz = std::sin(radians.z);
    float cy = std::cos(radians.y);
    float sy = std::sin(radians.y);
    float cx = std::cos(radians.x);
    float sx = std::sin(radians.x);

    result.m00 = cz * cy;
    result.m10 = cz * sy * sx - cx * sz;
    result.m20 = sz * sx + cz * cx * sy;

    result.m01 = cy * sz;
    result.m11 = cz * cx + sz * sy * sx;
    result.m21 = cx * sz * sy - cz * sx;

    result.m02 = -sy;
    result.m12 = cy * sx;
    result.m22 = cy * cx;

    return result;
}

NX_Mat4 NX_Mat4Scale(NX_Vec3 scale)
{
    NX_Mat4 result = NX_MAT4_IDENTITY;

    result.m00 = scale.x;
    result.m11 = scale.y;
    result.m22 = scale.z;

    return result;
}

NX_Transform NX_Mat4Decompose(const NX_Mat4* mat)
{
    NX_Transform t;

    t.translation.x = mat->m30;
    t.translation.y = mat->m31;
    t.translation.z = mat->m32;

    float sx = std::sqrt(mat->m00 * mat->m00 + mat->m01 * mat->m01 + mat->m02 * mat->m02);
    float sy = std::sqrt(mat->m10 * mat->m10 + mat->m11 * mat->m11 + mat->m12 * mat->m12);
    float sz = std::sqrt(mat->m20 * mat->m20 + mat->m21 * mat->m21 + mat->m22 * mat->m22);

    t.scale.x = sx;
    t.scale.y = sy;
    t.scale.z = sz;

    float invSx = 1.0f / sx;
    float invSy = 1.0f / sy;
    float invSz = 1.0f / sz;

    float m00 = mat->m00 * invSx;
    float m01 = mat->m01 * invSx;
    float m02 = mat->m02 * invSx;

    float m10 = mat->m10 * invSy;
    float m11 = mat->m11 * invSy;
    float m12 = mat->m12 * invSy;

    float m20 = mat->m20 * invSz;
    float m21 = mat->m21 * invSz;
    float m22 = mat->m22 * invSz;

    float trace = m00 + m11 + m22;

    if (trace > 0.0f) {
        float s = 0.5f / std::sqrt(trace + 1.0f);
        t.rotation.w = 0.25f / s;
        t.rotation.x = (m21 - m12) * s;
        t.rotation.y = (m02 - m20) * s;
        t.rotation.z = (m10 - m01) * s;
    }
    else if (m00 > m11 && m00 > m22) {
        float s = 0.5f / std::sqrt(1.0f + m00 - m11 - m22);
        t.rotation.w = (m21 - m12) * s;
        t.rotation.x = 0.25f / s;
        t.rotation.y = (m01 + m10) * s;
        t.rotation.z = (m02 + m20) * s;
    }
    else if (m11 > m22) {
        float s = 0.5f / std::sqrt(1.0f + m11 - m00 - m22);
        t.rotation.w = (m02 - m20) * s;
        t.rotation.x = (m01 + m10) * s;
        t.rotation.y = 0.25f / s;
        t.rotation.z = (m12 + m21) * s;
    }
    else {
        float s = 0.5f / std::sqrt(1.0f + m22 - m00 - m11);
        t.rotation.w = (m10 - m01) * s;
        t.rotation.x = (m02 + m20) * s;
        t.rotation.y = (m12 + m21) * s;
        t.rotation.z = 0.25f / s;
    }

    return t;
}

NX_Mat4 NX_Mat4Frustum(float left, float right, float bottom, float top, float znear, float zfar)
{
    float invRL = 1.0f / (right - left);
    float invTB = 1.0f / (top - bottom);
    float invFN = 1.0f / (znear - zfar);
    
    float znear2 = 2.0f * znear;
    
    NX_Mat4 result;
    result.m00 = znear2 * invRL;
    result.m01 = 0.0f;
    result.m02 = 0.0f;
    result.m03 = 0.0f;
    
    result.m10 = 0.0f;
    result.m11 = znear2 * invTB;
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    
    result.m20 = (right + left) * invRL;
    result.m21 = (top + bottom) * invTB;
    result.m22 = (zfar + znear) * invFN;
    result.m23 = -1.0f;
    
    result.m30 = 0.0f;
    result.m31 = 0.0f;
    result.m32 = (2.0f * zfar * znear) * invFN;
    result.m33 = 0.0f;
    
    return result;
}

NX_Mat4 NX_Mat4Perspective(float fovy, float aspect, float znear, float zfar)
{
    float tanHalfFovy = tanf(fovy * 0.5f);

    float invAspectTan = 1.0f / (aspect * tanHalfFovy);
    float invTan = 1.0f / tanHalfFovy;
    float invDepth = 1.0f / (znear - zfar);
    
    NX_Mat4 result;

    result.m00 = invAspectTan;
    result.m01 = 0.0f;
    result.m02 = 0.0f;
    result.m03 = 0.0f;

    result.m10 = 0.0f;
    result.m11 = invTan;
    result.m12 = 0.0f;
    result.m13 = 0.0f;

    result.m20 = 0.0f;
    result.m21 = 0.0f;
    result.m22 = (zfar + znear) * invDepth;
    result.m23 = -1.0f;

    result.m30 = 0.0f;
    result.m31 = 0.0f;
    result.m32 = (2.0f * zfar * znear) * invDepth;
    result.m33 = 0.0f;

    return result;
}

NX_Mat4 NX_Mat4Ortho(float left, float right, float bottom, float top, float znear, float zfar)
{
    float invRL = 1.0f / (right - left);
    float invTB = 1.0f / (top - bottom);
    float invFN = 1.0f / (znear - zfar);

    NX_Mat4 result;
    result.m00 = 2.0f * invRL;
    result.m01 = 0.0f;
    result.m02 = 0.0f;
    result.m03 = 0.0f;

    result.m10 = 0.0f;
    result.m11 = 2.0f * invTB;
    result.m12 = 0.0f;
    result.m13 = 0.0f;

    result.m20 = 0.0f;
    result.m21 = 0.0f;
    result.m22 = 2.0f * invFN;
    result.m23 = 0.0f;

    result.m30 = -(left + right) * invRL;
    result.m31 = -(top + bottom) * invTB;
    result.m32 = (zfar + znear) * invFN;
    result.m33 = 1.0f;

    return result;
}

NX_Mat4 NX_Mat4LookTo(NX_Vec3 eye, NX_Vec3 direction, NX_Vec3 up)
{
    float fx = -direction.x, fy = -direction.y, fz = -direction.z;
    float flenSq = fx * fx + fy * fy + fz * fz;

    if (flenSq > 1e-6f) {
        float invFlen = 1.0f / std::sqrt(flenSq);
        fx *= invFlen;
        fy *= invFlen;
        fz *= invFlen;
    }

    float rx = up.y * fz - up.z * fy;
    float ry = up.z * fx - up.x * fz;
    float rz = up.x * fy - up.y * fx;

    float rlenSq = rx * rx + ry * ry + rz * rz;
    if (rlenSq > 1e-6f) {
        float invRlen = 1.0f / std::sqrt(rlenSq);
        rx *= invRlen;
        ry *= invRlen;
        rz *= invRlen;
    }
    else {
        rx = 1.0f; ry = 0.0f; rz = 0.0f;
    }

    float ux = fy * rz - fz * ry;
    float uy = fz * rx - fx * rz;
    float uz = fx * ry - fy * rx;

    float tx = -(rx * eye.x + ry * eye.y + rz * eye.z);
    float ty = -(ux * eye.x + uy * eye.y + uz * eye.z);
    float tz = -(fx * eye.x + fy * eye.y + fz * eye.z);

    NX_Mat4 m;
    m.m00 = rx;  m.m01 = ux;  m.m02 = fx;  m.m03 = 0.0f;
    m.m10 = ry;  m.m11 = uy;  m.m12 = fy;  m.m13 = 0.0f;
    m.m20 = rz;  m.m21 = uz;  m.m22 = fz;  m.m23 = 0.0f;
    m.m30 = tx;  m.m31 = ty;  m.m32 = tz;  m.m33 = 1.0f;

    return m;
}

NX_Mat4 NX_Mat4LookAt(NX_Vec3 eye, NX_Vec3 target, NX_Vec3 up)
{
    float dx = target.x - eye.x;
    float dy = target.y - eye.y;
    float dz = target.z - eye.z;

    float fx = -dx, fy = -dy, fz = -dz;
    float flenSq = fx * fx + fy * fy + fz * fz;

    if (flenSq > 1e-6f) {
        float invFlen = 1.0f / std::sqrt(flenSq);
        fx *= invFlen;
        fy *= invFlen;
        fz *= invFlen;
    }

    float rx = up.y * fz - up.z * fy;
    float ry = up.z * fx - up.x * fz;
    float rz = up.x * fy - up.y * fx;

    float rlenSq = rx * rx + ry * ry + rz * rz;
    if (rlenSq > 1e-6f) {
        float invRlen = 1.0f / std::sqrt(rlenSq);
        rx *= invRlen;
        ry *= invRlen;
        rz *= invRlen;
    }
    else {
        rx = 1.0f; ry = 0.0f; rz = 0.0f;
    }

    float ux = fy * rz - fz * ry;
    float uy = fz * rx - fx * rz;
    float uz = fx * ry - fy * rx;

    float tx = -(rx * eye.x + ry * eye.y + rz * eye.z);
    float ty = -(ux * eye.x + uy * eye.y + uz * eye.z);
    float tz = -(fx * eye.x + fy * eye.y + fz * eye.z);

    NX_Mat4 m;
    m.m00 = rx;  m.m01 = ux;  m.m02 = fx;  m.m03 = 0.0f;
    m.m10 = ry;  m.m11 = uy;  m.m12 = fy;  m.m13 = 0.0f;
    m.m20 = rz;  m.m21 = uz;  m.m22 = fz;  m.m23 = 0.0f;
    m.m30 = tx;  m.m31 = ty;  m.m32 = tz;  m.m33 = 1.0f;

    return m;
}

float NX_Mat4Determinant(const NX_Mat4* mat)
{
    float result = 0.0f;

    float a00 = mat->m00, a01 = mat->m01, a02 = mat->m02, a03 = mat->m03;
    float a10 = mat->m10, a11 = mat->m11, a12 = mat->m12, a13 = mat->m13;
    float a20 = mat->m20, a21 = mat->m21, a22 = mat->m22, a23 = mat->m23;
    float a30 = mat->m30, a31 = mat->m31, a32 = mat->m32, a33 = mat->m33;

    result = a30 * a21 * a12 * a03 - a20 * a31 * a12 * a03 - a30 * a11 * a22 * a03 + a10 * a31 * a22 * a03 +
             a20 * a11 * a32 * a03 - a10 * a21 * a32 * a03 - a30 * a21 * a02 * a13 + a20 * a31 * a02 * a13 +
             a30 * a01 * a22 * a13 - a00 * a31 * a22 * a13 - a20 * a01 * a32 * a13 + a00 * a21 * a32 * a13 +
             a30 * a11 * a02 * a23 - a10 * a31 * a02 * a23 - a30 * a01 * a12 * a23 + a00 * a31 * a12 * a23 +
             a10 * a01 * a32 * a23 - a00 * a11 * a32 * a23 - a20 * a11 * a02 * a33 + a10 * a21 * a02 * a33 +
             a20 * a01 * a12 * a33 - a00 * a21 * a12 * a33 - a10 * a01 * a22 * a33 + a00 * a11 * a22 * a33;

    return result;
}

NX_Mat4 NX_Mat4Transpose(const NX_Mat4* mat)
{
    NX_Mat4 result;
    float* NX_RESTRICT R = result.a;
    const float* NX_RESTRICT M = mat->a;

#if defined(NX_HAS_SSE)

    __m128 row0 = _mm_loadu_ps(&M[0]);
    __m128 row1 = _mm_loadu_ps(&M[4]);
    __m128 row2 = _mm_loadu_ps(&M[8]);
    __m128 row3 = _mm_loadu_ps(&M[12]);

    _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

    _mm_storeu_ps(&R[0],  row0);
    _mm_storeu_ps(&R[4],  row1);
    _mm_storeu_ps(&R[8],  row2);
    _mm_storeu_ps(&R[12], row3);

#elif defined(NX_HAS_NEON)

    float32x4x2_t t0 = vtrnq_f32(vld1q_f32(&M[0]), vld1q_f32(&M[4]));
    float32x4x2_t t1 = vtrnq_f32(vld1q_f32(&M[8]), vld1q_f32(&M[12]));

    vst1q_f32(&R[0],  vcombine_f32(vget_low_f32(t0.val[0]), vget_low_f32(t1.val[0])));
    vst1q_f32(&R[4],  vcombine_f32(vget_low_f32(t0.val[1]), vget_low_f32(t1.val[1])));
    vst1q_f32(&R[8],  vcombine_f32(vget_high_f32(t0.val[0]), vget_high_f32(t1.val[0])));
    vst1q_f32(&R[12], vcombine_f32(vget_high_f32(t0.val[1]), vget_high_f32(t1.val[1])));

#else
    R[0]  = M[0]; R[1]  = M[4]; R[2]  = M[8];  R[3]  = M[12];
    R[4]  = M[1]; R[5]  = M[5]; R[6]  = M[9];  R[7]  = M[13];
    R[8]  = M[2]; R[9]  = M[6]; R[10] = M[10]; R[11] = M[14];
    R[12] = M[3]; R[13] = M[7]; R[14] = M[11]; R[15] = M[15];
#endif

    return result;
}

NX_Mat4 NX_Mat4Inverse(const NX_Mat4* mat)
{
    NX_Mat4 result = { 0 };

    float a00 = mat->m00, a01 = mat->m01, a02 = mat->m02, a03 = mat->m03;
    float a10 = mat->m10, a11 = mat->m11, a12 = mat->m12, a13 = mat->m13;
    float a20 = mat->m20, a21 = mat->m21, a22 = mat->m22, a23 = mat->m23;
    float a30 = mat->m30, a31 = mat->m31, a32 = mat->m32, a33 = mat->m33;

    float b00 = a00 * a11 - a01 * a10;
    float b01 = a00 * a12 - a02 * a10;
    float b02 = a00 * a13 - a03 * a10;
    float b03 = a01 * a12 - a02 * a11;
    float b04 = a01 * a13 - a03 * a11;
    float b05 = a02 * a13 - a03 * a12;
    float b06 = a20 * a31 - a21 * a30;
    float b07 = a20 * a32 - a22 * a30;
    float b08 = a20 * a33 - a23 * a30;
    float b09 = a21 * a32 - a22 * a31;
    float b10 = a21 * a33 - a23 * a31;
    float b11 = a22 * a33 - a23 * a32;

    float invDet = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

    result.m00 = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
    result.m01 = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
    result.m02 = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
    result.m03 = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
    result.m10 = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
    result.m11 = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
    result.m12 = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
    result.m13 = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
    result.m20 = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
    result.m21 = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
    result.m22 = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
    result.m23 = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
    result.m30 = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
    result.m31 = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
    result.m32 = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
    result.m33 = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;

    return result;
}

float NX_Mat4Trace(const NX_Mat4* mat)
{
    return mat->m00 + mat->m11 + mat->m22 + mat->m33;
}

NX_Mat4 NX_Mat4Add(const NX_Mat4* left, const NX_Mat4* right)
{
    NX_Mat4 result;
    for (int_fast8_t i = 0; i < 16; ++i) {
        result.a[i] = left->a[i] + right->a[i];
    }
    return result;
}

NX_Mat4 NX_Mat4Sub(const NX_Mat4* left, const NX_Mat4* right)
{
    NX_Mat4 result;
    for (int_fast8_t i = 0; i < 16; ++i) {
        result.a[i] = left->a[i] - right->a[i];
    }
    return result;
}

NX_Mat4 NX_Mat4Mul(const NX_Mat4* NX_RESTRICT left, const NX_Mat4* NX_RESTRICT right)
{
    NX_Mat4 result;
    float* NX_RESTRICT R = result.a;
    const float* NX_RESTRICT A = left->a;
    const float* NX_RESTRICT B = right->a;

#if defined(NX_HAS_FMA_AVX)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai0 = _mm_broadcast_ss(&A[i * 4 + 0]);
        __m128 ai1 = _mm_broadcast_ss(&A[i * 4 + 1]);
        __m128 ai2 = _mm_broadcast_ss(&A[i * 4 + 2]);
        __m128 ai3 = _mm_broadcast_ss(&A[i * 4 + 3]);

        __m128 row = _mm_mul_ps(ai0, col0);
        row = _mm_fmadd_ps(ai1, col1, row);
        row = _mm_fmadd_ps(ai2, col2, row);
        row = _mm_fmadd_ps(ai3, col3, row);

        _mm_storeu_ps(&R[i * 4], row);
    }

#elif defined(NX_HAS_SSE)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        #if defined(NX_HAS_AVX)
            __m128 ai0 = _mm_broadcast_ss(&A[i * 4 + 0]);
            __m128 ai1 = _mm_broadcast_ss(&A[i * 4 + 1]);
            __m128 ai2 = _mm_broadcast_ss(&A[i * 4 + 2]);
            __m128 ai3 = _mm_broadcast_ss(&A[i * 4 + 3]);
        #else
            __m128 ai0 = _mm_set1_ps(A[i * 4 + 0]);
            __m128 ai1 = _mm_set1_ps(A[i * 4 + 1]);
            __m128 ai2 = _mm_set1_ps(A[i * 4 + 2]);
            __m128 ai3 = _mm_set1_ps(A[i * 4 + 3]);
        #endif

        __m128 t0 = _mm_mul_ps(ai0, col0);
        __m128 t1 = _mm_mul_ps(ai1, col1);
        __m128 t2 = _mm_mul_ps(ai2, col2);
        __m128 t3 = _mm_mul_ps(ai3, col3);
        
        __m128 row = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));
        _mm_storeu_ps(&R[i * 4], row);
    }

#elif defined(NX_HAS_NEON_FMA)

    float32x4_t col0 = vld1q_f32(&B[0]);
    float32x4_t col1 = vld1q_f32(&B[4]);
    float32x4_t col2 = vld1q_f32(&B[8]);
    float32x4_t col3 = vld1q_f32(&B[12]);

    for (int i = 0; i < 4; i++) {
        float32x4_t ai0 = vdupq_n_f32(A[i * 4 + 0]);
        float32x4_t ai1 = vdupq_n_f32(A[i * 4 + 1]);
        float32x4_t ai2 = vdupq_n_f32(A[i * 4 + 2]);
        float32x4_t ai3 = vdupq_n_f32(A[i * 4 + 3]);

        float32x4_t row = vmulq_f32(ai0, col0);
        row = vfmaq_f32(row, ai1, col1);
        row = vfmaq_f32(row, ai2, col2);
        row = vfmaq_f32(row, ai3, col3);

        vst1q_f32(&R[i * 4], row);
    }

#elif defined(NX_HAS_NEON)

    float32x4_t col0 = vld1q_f32(&B[0]);
    float32x4_t col1 = vld1q_f32(&B[4]);
    float32x4_t col2 = vld1q_f32(&B[8]);
    float32x4_t col3 = vld1q_f32(&B[12]);

    for (int i = 0; i < 4; i++) {
        float32x4_t ai0 = vdupq_n_f32(A[i * 4 + 0]);
        float32x4_t ai1 = vdupq_n_f32(A[i * 4 + 1]);
        float32x4_t ai2 = vdupq_n_f32(A[i * 4 + 2]);
        float32x4_t ai3 = vdupq_n_f32(A[i * 4 + 3]);

        float32x4_t row = vmulq_f32(ai0, col0);
        row = vmlaq_f32(row, ai1, col1);
        row = vmlaq_f32(row, ai2, col2);
        row = vmlaq_f32(row, ai3, col3);

        vst1q_f32(&R[i * 4], row);
    }

#else

    R[0]  = A[0] * B[0]  + A[1] * B[4]  + A[2] * B[8]   + A[3] * B[12];
    R[1]  = A[0] * B[1]  + A[1] * B[5]  + A[2] * B[9]   + A[3] * B[13];
    R[2]  = A[0] * B[2]  + A[1] * B[6]  + A[2] * B[10]  + A[3] * B[14];
    R[3]  = A[0] * B[3]  + A[1] * B[7]  + A[2] * B[11]  + A[3] * B[15];
    
    R[4]  = A[4] * B[0]  + A[5] * B[4]  + A[6] * B[8]   + A[7] * B[12];
    R[5]  = A[4] * B[1]  + A[5] * B[5]  + A[6] * B[9]   + A[7] * B[13];
    R[6]  = A[4] * B[2]  + A[5] * B[6]  + A[6] * B[10]  + A[7] * B[14];
    R[7]  = A[4] * B[3]  + A[5] * B[7]  + A[6] * B[11]  + A[7] * B[15];
    
    R[8]  = A[8] * B[0]  + A[9] * B[4]  + A[10] * B[8]  + A[11] * B[12];
    R[9]  = A[8] * B[1]  + A[9] * B[5]  + A[10] * B[9]  + A[11] * B[13];
    R[10] = A[8] * B[2]  + A[9] * B[6]  + A[10] * B[10] + A[11] * B[14];
    R[11] = A[8] * B[3]  + A[9] * B[7]  + A[10] * B[11] + A[11] * B[15];
    
    R[12] = A[12] * B[0] + A[13] * B[4] + A[14] * B[8]  + A[15] * B[12];
    R[13] = A[12] * B[1] + A[13] * B[5] + A[14] * B[9]  + A[15] * B[13];
    R[14] = A[12] * B[2] + A[13] * B[6] + A[14] * B[10] + A[15] * B[14];
    R[15] = A[12] * B[3] + A[13] * B[7] + A[14] * B[11] + A[15] * B[15];

#endif

    return result;
}

void NX_Mat4MulBatch(NX_Mat4* NX_RESTRICT results,
                     const NX_Mat4* NX_RESTRICT left,
                     const NX_Mat4* NX_RESTRICT right,
                     size_t count)
{
    if (count == 0) return;

#if defined(NX_HAS_FMA_AVX)

    size_t i = 0;

    if (count > 0) {
        _mm_prefetch((char*)&left[0], _MM_HINT_T0);
        _mm_prefetch((char*)&right[0], _MM_HINT_T0);
    }

    for (; i < count; i++) {
        const float* NX_RESTRICT A = left[i].a;
        const float* NX_RESTRICT B = right[i].a;
        float* NX_RESTRICT R = results[i].a;

        if (i + 1 < count) {
            _mm_prefetch((char*)&left[i + 1], _MM_HINT_T0);
            _mm_prefetch((char*)&right[i + 1], _MM_HINT_T0);
        }

        __m128 col0 = _mm_loadu_ps(&B[0]);
        __m128 col1 = _mm_loadu_ps(&B[4]);
        __m128 col2 = _mm_loadu_ps(&B[8]);
        __m128 col3 = _mm_loadu_ps(&B[12]);

        for (int row = 0; row < 4; row++) {
            __m128 ai0 = _mm_broadcast_ss(&A[row * 4 + 0]);
            __m128 ai1 = _mm_broadcast_ss(&A[row * 4 + 1]);
            __m128 ai2 = _mm_broadcast_ss(&A[row * 4 + 2]);
            __m128 ai3 = _mm_broadcast_ss(&A[row * 4 + 3]);

            __m128 result = _mm_mul_ps(ai0, col0);
            result = _mm_fmadd_ps(ai1, col1, result);
            result = _mm_fmadd_ps(ai2, col2, result);
            result = _mm_fmadd_ps(ai3, col3, result);

            _mm_storeu_ps(&R[row * 4], result);
        }
    }

#elif defined(NX_HAS_SSE)

    for (size_t i = 0; i < count; i++) {
        const float* NX_RESTRICT A = left[i].a;
        const float* NX_RESTRICT B = right[i].a;
        float* NX_RESTRICT R = results[i].a;

        #if defined(NX_HAS_AVX)
        if (i + 1 < count) {
            _mm_prefetch((char*)&left[i + 1], _MM_HINT_T0);
            _mm_prefetch((char*)&right[i + 1], _MM_HINT_T0);
        }
        #endif

        __m128 col0 = _mm_loadu_ps(&B[0]);
        __m128 col1 = _mm_loadu_ps(&B[4]);
        __m128 col2 = _mm_loadu_ps(&B[8]);
        __m128 col3 = _mm_loadu_ps(&B[12]);

        for (int row = 0; row < 4; row++) {
            #if defined(NX_HAS_AVX)
                __m128 ai0 = _mm_broadcast_ss(&A[row * 4 + 0]);
                __m128 ai1 = _mm_broadcast_ss(&A[row * 4 + 1]);
                __m128 ai2 = _mm_broadcast_ss(&A[row * 4 + 2]);
                __m128 ai3 = _mm_broadcast_ss(&A[row * 4 + 3]);
            #else
                __m128 ai0 = _mm_set1_ps(A[row * 4 + 0]);
                __m128 ai1 = _mm_set1_ps(A[row * 4 + 1]);
                __m128 ai2 = _mm_set1_ps(A[row * 4 + 2]);
                __m128 ai3 = _mm_set1_ps(A[row * 4 + 3]);
            #endif

            __m128 t0 = _mm_mul_ps(ai0, col0);
            __m128 t1 = _mm_mul_ps(ai1, col1);
            __m128 t2 = _mm_mul_ps(ai2, col2);
            __m128 t3 = _mm_mul_ps(ai3, col3);

            __m128 result = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));
            _mm_storeu_ps(&R[row * 4], result);
        }
    }

#elif defined(NX_HAS_NEON_FMA)

    for (size_t i = 0; i < count; i++) {
        const float* NX_RESTRICT A = left[i].a;
        const float* NX_RESTRICT B = right[i].a;
        float* NX_RESTRICT R = results[i].a;

        if (i + 1 < count) {
            __builtin_prefetch(&left[i + 1], 0, 0);
            __builtin_prefetch(&right[i + 1], 0, 0);
        }

        float32x4_t col0 = vld1q_f32(&B[0]);
        float32x4_t col1 = vld1q_f32(&B[4]);
        float32x4_t col2 = vld1q_f32(&B[8]);
        float32x4_t col3 = vld1q_f32(&B[12]);

        for (int row = 0; row < 4; row++) {
            float32x4_t ai0 = vdupq_n_f32(A[row * 4 + 0]);
            float32x4_t ai1 = vdupq_n_f32(A[row * 4 + 1]);
            float32x4_t ai2 = vdupq_n_f32(A[row * 4 + 2]);
            float32x4_t ai3 = vdupq_n_f32(A[row * 4 + 3]);

            float32x4_t result = vmulq_f32(ai0, col0);
            result = vfmaq_f32(result, ai1, col1);
            result = vfmaq_f32(result, ai2, col2);
            result = vfmaq_f32(result, ai3, col3);

            vst1q_f32(&R[row * 4], result);
        }
    }

#elif defined(NX_HAS_NEON)

    for (size_t i = 0; i < count; i++) {
        const float* NX_RESTRICT A = left[i].a;
        const float* NX_RESTRICT B = right[i].a;
        float* NX_RESTRICT R = results[i].a;

        if (i + 1 < count) {
            __builtin_prefetch(&left[i + 1], 0, 0);
            __builtin_prefetch(&right[i + 1], 0, 0);
        }

        float32x4_t col0 = vld1q_f32(&B[0]);
        float32x4_t col1 = vld1q_f32(&B[4]);
        float32x4_t col2 = vld1q_f32(&B[8]);
        float32x4_t col3 = vld1q_f32(&B[12]);

        for (int row = 0; row < 4; row++) {
            float32x4_t ai0 = vdupq_n_f32(A[row * 4 + 0]);
            float32x4_t ai1 = vdupq_n_f32(A[row * 4 + 1]);
            float32x4_t ai2 = vdupq_n_f32(A[row * 4 + 2]);
            float32x4_t ai3 = vdupq_n_f32(A[row * 4 + 3]);

            float32x4_t result = vmulq_f32(ai0, col0);
            result = vmlaq_f32(result, ai1, col1);
            result = vmlaq_f32(result, ai2, col2);
            result = vmlaq_f32(result, ai3, col3);

            vst1q_f32(&R[row * 4], result);
        }
    }

#else

    for (size_t i = 0; i < count; i++) {
        const float* NX_RESTRICT A = left[i].a;
        const float* NX_RESTRICT B = right[i].a;
        float* NX_RESTRICT R = results[i].a;

        R[0]  = A[0] * B[0]  + A[1] * B[4]  + A[2] * B[8]   + A[3] * B[12];
        R[1]  = A[0] * B[1]  + A[1] * B[5]  + A[2] * B[9]   + A[3] * B[13];
        R[2]  = A[0] * B[2]  + A[1] * B[6]  + A[2] * B[10]  + A[3] * B[14];
        R[3]  = A[0] * B[3]  + A[1] * B[7]  + A[2] * B[11]  + A[3] * B[15];
        
        R[4]  = A[4] * B[0]  + A[5] * B[4]  + A[6] * B[8]   + A[7] * B[12];
        R[5]  = A[4] * B[1]  + A[5] * B[5]  + A[6] * B[9]   + A[7] * B[13];
        R[6]  = A[4] * B[2]  + A[5] * B[6]  + A[6] * B[10]  + A[7] * B[14];
        R[7]  = A[4] * B[3]  + A[5] * B[7]  + A[6] * B[11]  + A[7] * B[15];
        
        R[8]  = A[8] * B[0]  + A[9] * B[4]  + A[10] * B[8]  + A[11] * B[12];
        R[9]  = A[8] * B[1]  + A[9] * B[5]  + A[10] * B[9]  + A[11] * B[13];
        R[10] = A[8] * B[2]  + A[9] * B[6]  + A[10] * B[10] + A[11] * B[14];
        R[11] = A[8] * B[3]  + A[9] * B[7]  + A[10] * B[11] + A[11] * B[15];
        
        R[12] = A[12] * B[0] + A[13] * B[4] + A[14] * B[8]  + A[15] * B[12];
        R[13] = A[12] * B[1] + A[13] * B[5] + A[14] * B[9]  + A[15] * B[13];
        R[14] = A[12] * B[2] + A[13] * B[6] + A[14] * B[10] + A[15] * B[14];
        R[15] = A[12] * B[3] + A[13] * B[7] + A[14] * B[11] + A[15] * B[15];
    }

#endif
}

/* === Transform Functions === */

NX_Mat4 NX_TransformToMat4(const NX_Transform* transform)
{
    const NX_Vec3* t = &transform->translation;
    const NX_Quat* q = &transform->rotation;
    const NX_Vec3* s = &transform->scale;

    float qx = q->x, qy = q->y;
    float qz = q->z, qw = q->w;

    float qlen2 = qx * qx + qy * qy + qz * qz + qw * qw;
    if (std::abs(qlen2 - 1.0f) > 1e-4f) {
        if (qlen2 < 1e-6f) {
            NX_Mat4 m = {
                s->x, 0.0f, 0.0f, 0.0f,
                0.0f, s->y, 0.0f, 0.0f,
                0.0f, 0.0f, s->z, 0.0f,
                t->x, t->y, t->z, 1.0f
            };
            return m;
        }
        float invLen = 1.0f / std::sqrt(qlen2);
        qx *= invLen; qy *= invLen; qz *= invLen; qw *= invLen;
    }

    float x2 = 2.0f * qx, y2 = 2.0f * qy, z2 = 2.0f * qz;
    float xx = qx * x2, yy = qy * y2, zz = qz * z2;
    float xy = qx * y2, xz = qx * z2, yz = qy * z2;
    float wx = qw * x2, wy = qw * y2, wz = qw * z2;

    float sx = s->x;
    float sy = s->y;
    float sz = s->z;
    
    NX_Mat4 m;

    m.m00 = (1.0f - yy - zz) * sx;
    m.m01 = (xy + wz) * sy;
    m.m02 = (xz - wy) * sz;
    m.m03 = 0.0f;

    m.m10 = (xy - wz) * sx;
    m.m11 = (1.0f - xx - zz) * sy;
    m.m12 = (yz + wx) * sz;
    m.m13 = 0.0f;

    m.m20 = (xz + wy) * sx;
    m.m21 = (yz - wx) * sy;
    m.m22 = (1.0f - xx - yy) * sz;
    m.m23 = 0.0f;

    m.m30 = t->x;
    m.m31 = t->y;
    m.m32 = t->z;
    m.m33 = 1.0f;
    
    return m;
}

NX_Mat3 NX_TransformToNormalMat3(const NX_Transform* t)
{
    float qx = t->rotation.x, qy = t->rotation.y;
    float qz = t->rotation.z, qw = t->rotation.w;

    float x2 = 2.0f * qx, y2 = 2.0f * qy, z2 = 2.0f * qz;
    float xx = qx * x2, yy = qy * y2, zz = qz * z2;
    float xy = qx * y2, xz = qx * z2, yz = qy * z2;
    float wx = qw * x2, wy = qw * y2, wz = qw * z2;

    float r00 = 1.0f - yy - zz, r01 = xy - wz, r02 = xz + wy;
    float r10 = xy + wz, r11 = 1.0f - xx - zz, r12 = yz - wx;
    float r20 = xz - wy, r21 = yz + wx, r22 = 1.0f - xx - yy;

    float sx = t->scale.x, sy = t->scale.y, sz = t->scale.z;
    float m00 = r00 * sx, m01 = r01 * sx, m02 = r02 * sx;
    float m10 = r10 * sy, m11 = r11 * sy, m12 = r12 * sy;
    float m20 = r20 * sz, m21 = r21 * sz, m22 = r22 * sz;

    float det = m00 * (m11 * m22 - m12 * m21)
              - m01 * (m10 * m22 - m12 * m20)
              + m02 * (m10 * m21 - m11 * m20);

    float invDet = 1.0f / det;

    NX_Mat3 result;
    result.m00 = (m11 * m22 - m12 * m21) * invDet;
    result.m01 = (m02 * m21 - m01 * m22) * invDet;
    result.m02 = (m01 * m12 - m02 * m11) * invDet;
    result.m10 = (m12 * m20 - m10 * m22) * invDet;
    result.m11 = (m00 * m22 - m02 * m20) * invDet;
    result.m12 = (m02 * m10 - m00 * m12) * invDet;
    result.m20 = (m10 * m21 - m11 * m20) * invDet;
    result.m21 = (m01 * m20 - m00 * m21) * invDet;
    result.m22 = (m00 * m11 - m01 * m10) * invDet;

    return result;
}

NX_Transform NX_TransformCombine(const NX_Transform* parent, const NX_Transform* child)
{
    NX_Transform result;

    result.rotation = NX_QuatMul(parent->rotation, child->rotation);

    result.scale.x = parent->scale.x * child->scale.x;
    result.scale.y = parent->scale.y * child->scale.y;
    result.scale.z = parent->scale.z * child->scale.z;

    float sx = child->translation.x * parent->scale.x;
    float sy = child->translation.y * parent->scale.y;
    float sz = child->translation.z * parent->scale.z;

    NX_Quat q = parent->rotation;

    float qx2 = q.x + q.x;
    float qy2 = q.y + q.y;
    float qz2 = q.z + q.z;
    
    float qxx2 = q.x * qx2;
    float qyy2 = q.y * qy2;
    float qzz2 = q.z * qz2;
    float qxy2 = q.x * qy2;
    float qxz2 = q.x * qz2;
    float qyz2 = q.y * qz2;
    float qwx2 = q.w * qx2;
    float qwy2 = q.w * qy2;
    float qwz2 = q.w * qz2;

    result.translation.x = parent->translation.x + 
                          (1.0f - qyy2 - qzz2) * sx + 
                          (qxy2 - qwz2) * sy + 
                          (qxz2 + qwy2) * sz;

    result.translation.y = parent->translation.y + 
                          (qxy2 + qwz2) * sx + 
                          (1.0f - qxx2 - qzz2) * sy + 
                          (qyz2 - qwx2) * sz;

    result.translation.z = parent->translation.z + 
                          (qxz2 - qwy2) * sx + 
                          (qyz2 + qwx2) * sy + 
                          (1.0f - qxx2 - qyy2) * sz;

    return result;
}

NX_Transform NX_TransformLerp(const NX_Transform* a, const NX_Transform* b, float t)
{
    NX_Transform result;

    float w1 = 1.0f - t;
    float w2 = t;

    result.translation.x = w1 * a->translation.x + w2 * b->translation.x;
    result.translation.y = w1 * a->translation.y + w2 * b->translation.y;
    result.translation.z = w1 * a->translation.z + w2 * b->translation.z;

    result.scale.x = w1 * a->scale.x + w2 * b->scale.x;
    result.scale.y = w1 * a->scale.y + w2 * b->scale.y;
    result.scale.z = w1 * a->scale.z + w2 * b->scale.z;

    result.rotation = NX_QuatSLerp(a->rotation, b->rotation, t);

    return result;
}
