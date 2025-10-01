/* HP_Math.c -- API definition for Hyperion's math module
 *
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * For conditions of distribution and use, see accompanying LICENSE file.
 */

#include <Hyperion/HP_Math.h>

/* === Quaternion Functions === */

HP_Quat HP_QuatFromEuler(HP_Vec3 v)
{
    v.x *= 0.5f;
    v.y *= 0.5f;
    v.z *= 0.5f;

    float cp = cosf(v.x); // Pitch (X)
    float sp = sinf(v.x);
    float cy = cosf(v.y); // Yaw (Y)
    float sy = sinf(v.y);
    float cr = cosf(v.z); // Roll (Z)
    float sr = sinf(v.z);

    HP_Quat q;
    q.w = cy * cp * cr + sy * sp * sr;
    q.x = cy * sp * cr + sy * cp * sr;
    q.y = sy * cp * cr - cy * sp * sr;
    q.z = cy * cp * sr - sy * sp * cr;

    float lenSq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (lenSq < 1e-6f) {
        HP_Quat identity = { 1.0f, 0.0f, 0.0f, 0.0f };
        return identity;
    }

    float invLen = 1.0f / sqrtf(lenSq);
    q.w *= invLen;
    q.x *= invLen;
    q.y *= invLen;
    q.z *= invLen;

    return q;
}

HP_Vec3 HP_QuatToEuler(HP_Quat q)
{
    HP_Vec3 angles;

    // Pitch (X axis)
    float sinp = 2.0f * (q.w * q.x - q.y * q.z);
    if (fabsf(sinp) >= 1.0f) {
        angles.x = copysignf(HP_PI * 0.5f, sinp);
    }
    else {
        angles.x = asinf(sinp);
    }

    // Yaw (Y axis)
    float sinYcosP = 2.0f * (q.w * q.y + q.x * q.z);
    float cosYcosP = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    angles.y = atan2f(sinYcosP, cosYcosP);

    // Roll (Z axis)
    float sinRcosP = 2.0f * (q.w * q.z + q.x * q.y);
    float cosRcosP = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
    angles.z = atan2f(sinRcosP, cosRcosP);

    return angles;
}

HP_Quat HP_QuatFromMat4(const HP_Mat4* m)
{
    HP_Quat q;
    float trace = m->m00 + m->m11 + m->m22;

    if (trace > 0.0f) {
        float s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (m->m21 - m->m12) * s;
        q.y = (m->m02 - m->m20) * s;
        q.z = (m->m10 - m->m01) * s;
    }
    else {
        if (m->m00 > m->m11 && m->m00 > m->m22) {
            float s = 2.0f * sqrtf(1.0f + m->m00 - m->m11 - m->m22);
            q.w = (m->m21 - m->m12) / s;
            q.x = 0.25f * s;
            q.y = (m->m01 + m->m10) / s;
            q.z = (m->m02 + m->m20) / s;
        }
        else if (m->m11 > m->m22) {
            float s = 2.0f * sqrtf(1.0f + m->m11 - m->m00 - m->m22);
            q.w = (m->m02 - m->m20) / s;
            q.x = (m->m01 + m->m10) / s;
            q.y = 0.25f * s;
            q.z = (m->m12 + m->m21) / s;
        }
        else {
            float s = 2.0f * sqrtf(1.0f + m->m22 - m->m00 - m->m11);
            q.w = (m->m10 - m->m01) / s;
            q.x = (m->m02 + m->m20) / s;
            q.y = (m->m12 + m->m21) / s;
            q.z = 0.25f * s;
        }
    }

    return q;
}

HP_Mat4 HP_QuatToMat4(HP_Quat q)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

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

HP_Quat HP_QuatLookAt(HP_Vec3 from, HP_Vec3 to, HP_Vec3 up)
{
    HP_Mat4 M = HP_Mat4LookAt(from, to, up);
    return HP_QuatFromMat4(&M);
}

HP_Quat HP_QuatLerp(HP_Quat a, HP_Quat b, float t)
{
    float dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;

    if (dot < 0.0f) {
        b.w = -b.w;
        b.x = -b.x;
        b.y = -b.y;
        b.z = -b.z;
    }

    for (int i = 0; i < 4; ++i) {
        a.v[i] += t * (b.v[i] - a.v[i]);
    }

    return HP_QuatNormalize(a);
}

HP_Quat HP_QuatSLerp(HP_Quat a, HP_Quat b, float t)
{
    float dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;

    if (dot < 0.0f) {
        b.w = -b.w;
        b.x = -b.x;
        b.y = -b.y;
        b.z = -b.z;
        dot = -dot;
    }

    if (dot > 0.9995f) {
        for (int i = 0; i < 4; ++i) {
            a.v[i] += t * (b.v[i] - a.v[i]);
        }
        return HP_QuatNormalize(a);
    }

    float th0 = acosf(dot);
    float th = th0 * t;
    float sin_th = sinf(th0);

    float w1 = cosf(th) - dot * sinf(th) / sin_th;
    float w2 = sinf(th) / sin_th;

    for (int i = 0; i < 4; ++i) {
        a.v[i] = w1 * a.v[i] + w2 * b.v[i];
    }

    return a;
}

/* === Matrix 3x3 Functions === */

HP_Mat3 HP_Mat3Transform2D(HP_Vec2 translation, float rotation, HP_Vec2 scale)
{
    float c = cosf(rotation);
    float s = sinf(rotation);

    HP_Mat3 result;
    result.m00 = c * scale.x;  result.m01 = -s * scale.x; result.m02 = translation.x;
    result.m10 = s * scale.y;  result.m11 =  c * scale.y; result.m12 = translation.y;
    result.m20 = 0.0f;         result.m21 =  0.0f;        result.m22 = 1.0f;
    return result;
}

HP_Mat3 HP_Mat3Translate2D(HP_Vec2 translation)
{
    HP_Mat3 result;
    result.m00 = 1.0f; result.m01 = 0.0f; result.m02 = translation.x;
    result.m10 = 0.0f; result.m11 = 1.0f; result.m12 = translation.y;
    result.m20 = 0.0f; result.m21 = 0.0f; result.m22 = 1.0f;
    return result;
}

HP_Mat3 HP_Mat3Rotate2D(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    HP_Mat3 result;
    result.m00 = c;    result.m01 = -s;   result.m02 = 0.0f;
    result.m10 = s;    result.m11 =  c;   result.m12 = 0.0f;
    result.m20 = 0.0f; result.m21 = 0.0f; result.m22 = 1.0f;
    return result;
}

HP_Mat3 HP_Mat3Scale2D(HP_Vec2 scale)
{
    HP_Mat3 result;
    result.m00 = scale.x; result.m01 = 0.0f;    result.m02 = 0.0f;
    result.m10 = 0.0f;    result.m11 = scale.y; result.m12 = 0.0f;
    result.m20 = 0.0f;    result.m21 = 0.0f;    result.m22 = 1.0f;
    return result;
}

HP_Mat3 HP_Mat3RotateX(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    HP_Mat3 result;
    result.m00 = 1.0f; result.m01 = 0.0f; result.m02 = 0.0f;
    result.m10 = 0.0f; result.m11 = c;    result.m12 = -s;
    result.m20 = 0.0f; result.m21 = s;    result.m22 = c;
    return result;
}

HP_Mat3 HP_Mat3RotateY(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    HP_Mat3 result;
    result.m00 = c;    result.m01 = 0.0f; result.m02 = s;
    result.m10 = 0.0f; result.m11 = 1.0f; result.m12 = 0.0f;
    result.m20 = -s;   result.m21 = 0.0f; result.m22 = c;
    return result;
}

HP_Mat3 HP_Mat3RotateZ(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    HP_Mat3 result;
    result.m00 = c;    result.m01 = -s;   result.m02 = 0.0f;
    result.m10 = s;    result.m11 = c;    result.m12 = 0.0f;
    result.m20 = 0.0f; result.m21 = 0.0f; result.m22 = 1.0f;
    return result;
}

HP_Mat3 HP_Mat3Rotate(HP_Vec3 axis, float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);
    float oneMinusC = 1.0f - c;

    float len = sqrtf(axis.x * axis.x + axis.y * axis.y + axis.z * axis.z);
    if (len < 1e-6f) {
        HP_Mat3 result;
        result.m00 = 1.0f; result.m01 = 0.0f; result.m02 = 0.0f;
        result.m10 = 0.0f; result.m11 = 1.0f; result.m12 = 0.0f;
        result.m20 = 0.0f; result.m21 = 0.0f; result.m22 = 1.0f;
        return result;
    }

    float invLen = 1.0f / len;
    axis.x *= invLen;
    axis.y *= invLen;
    axis.z *= invLen;

    float x = axis.x, y = axis.y, z = axis.z;
    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, xz = x * z, yz = y * z;

    HP_Mat3 result;
    result.m00 = xx * oneMinusC + c;
    result.m01 = xy * oneMinusC - z * s;
    result.m02 = xz * oneMinusC + y * s;

    result.m10 = xy * oneMinusC + z * s;
    result.m11 = yy * oneMinusC + c;
    result.m12 = yz * oneMinusC - x * s;

    result.m20 = xz * oneMinusC - y * s;
    result.m21 = yz * oneMinusC + x * s;
    result.m22 = zz * oneMinusC + c;

    return result;
}

HP_Mat3 HP_Mat3RotateXYZ(HP_Vec3 radians)
{
    float cx = cosf(radians.x), sx = sinf(radians.x);
    float cy = cosf(radians.y), sy = sinf(radians.y);
    float cz = cosf(radians.z), sz = sinf(radians.z);

    HP_Mat3 result;
    result.m00 = cy * cz;
    result.m01 = -cy * sz;
    result.m02 = sy;

    result.m10 = sx * sy * cz + cx * sz;
    result.m11 = -sx * sy * sz + cx * cz;
    result.m12 = -sx * cy;

    result.m20 = -cx * sy * cz + sx * sz;
    result.m21 = cx * sy * sz + sx * cz;
    result.m22 = cx * cy;

    return result;
}

HP_Mat3 HP_Mat3Transpose(const HP_Mat3* mat)
{
    HP_Mat3 result;
    result.m00 = mat->m00; result.m01 = mat->m10; result.m02 = mat->m20;
    result.m10 = mat->m01; result.m11 = mat->m11; result.m12 = mat->m21;
    result.m20 = mat->m02; result.m21 = mat->m12; result.m22 = mat->m22;
    return result;
}

float HP_Mat3Determinant(const HP_Mat3* mat)
{
    return mat->m00 * (mat->m11 * mat->m22 - mat->m12 * mat->m21) -
           mat->m01 * (mat->m10 * mat->m22 - mat->m12 * mat->m20) +
           mat->m02 * (mat->m10 * mat->m21 - mat->m11 * mat->m20);
}

HP_Mat3 HP_Mat3Inverse(const HP_Mat3* mat)
{
    float det = HP_Mat3Determinant(mat);

    if (fabsf(det) < 1e-6f) {
        HP_Mat3 result;
        result.m00 = 1.0f; result.m01 = 0.0f; result.m02 = 0.0f;
        result.m10 = 0.0f; result.m11 = 1.0f; result.m12 = 0.0f;
        result.m20 = 0.0f; result.m21 = 0.0f; result.m22 = 1.0f;
        return result;
    }

    float invDet = 1.0f / det;

    HP_Mat3 result;

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

HP_Mat3 HP_Mat3Normal(const HP_Mat4* mat)
{
    float det = mat->m00 * (mat->m11 * mat->m22 - mat->m12 * mat->m21) -
                mat->m01 * (mat->m10 * mat->m22 - mat->m12 * mat->m20) +
                mat->m02 * (mat->m10 * mat->m21 - mat->m11 * mat->m20);

    if (fabsf(det) < 1e-6f) {
        HP_Mat3 result;
        result.m00 = 1.0f; result.m01 = 0.0f; result.m02 = 0.0f;
        result.m10 = 0.0f; result.m11 = 1.0f; result.m12 = 0.0f;
        result.m20 = 0.0f; result.m21 = 0.0f; result.m22 = 1.0f;
        return result;
    }

    float invDet = 1.0f / det;

    HP_Mat3 result;

    result.m00 = (mat->m11 * mat->m22 - mat->m12 * mat->m21) * invDet;
    result.m01 = (mat->m12 * mat->m20 - mat->m10 * mat->m22) * invDet;
    result.m02 = (mat->m10 * mat->m21 - mat->m11 * mat->m20) * invDet;

    result.m10 = (mat->m02 * mat->m21 - mat->m01 * mat->m22) * invDet;
    result.m11 = (mat->m00 * mat->m22 - mat->m02 * mat->m20) * invDet;
    result.m12 = (mat->m01 * mat->m20 - mat->m00 * mat->m21) * invDet;

    result.m20 = (mat->m01 * mat->m12 - mat->m02 * mat->m11) * invDet;
    result.m21 = (mat->m02 * mat->m10 - mat->m00 * mat->m12) * invDet;
    result.m22 = (mat->m00 * mat->m11 - mat->m01 * mat->m10) * invDet;

    return result;
}

HP_Mat3 HP_Mat3Add(const HP_Mat3* left, const HP_Mat3* right)
{
    HP_Mat3 result;
    for (int i = 0; i < 9; ++i) {
        result.a[i] = left->a[i] + right->a[i];
    }
    return result;
}

HP_Mat3 HP_Mat3Sub(const HP_Mat3* left, const HP_Mat3* right)
{
    HP_Mat3 result;
    for (int i = 0; i < 9; ++i) {
        result.a[i] = left->a[i] - right->a[i];
    }
    return result;
}

HP_Mat3 HP_Mat3Mul(const HP_Mat3* left, const HP_Mat3* right)
{
    HP_Mat3 result;

    result.m00 = left->m00 * right->m00 + left->m01 * right->m10 + left->m02 * right->m20;
    result.m01 = left->m00 * right->m01 + left->m01 * right->m11 + left->m02 * right->m21;
    result.m02 = left->m00 * right->m02 + left->m01 * right->m12 + left->m02 * right->m22;

    result.m10 = left->m10 * right->m00 + left->m11 * right->m10 + left->m12 * right->m20;
    result.m11 = left->m10 * right->m01 + left->m11 * right->m11 + left->m12 * right->m21;
    result.m12 = left->m10 * right->m02 + left->m11 * right->m12 + left->m12 * right->m22;

    result.m20 = left->m20 * right->m00 + left->m21 * right->m10 + left->m22 * right->m20;
    result.m21 = left->m20 * right->m01 + left->m21 * right->m11 + left->m22 * right->m21;
    result.m22 = left->m20 * right->m02 + left->m21 * right->m12 + left->m22 * right->m22;

    return result;
}

/* === Matrix 4x4 Functions === */

HP_Mat4 HP_Mat4Translate(HP_Vec3 v)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    result.m30 = v.x;
    result.m31 = v.y;
    result.m32 = v.z;

    return result;
}

HP_Mat4 HP_Mat4Rotate(HP_Vec3 axis, float radians)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    float x = axis.x, y = axis.y, z = axis.z;
    float lenSq = x * x + y * y + z * z;

    if (lenSq != 1.0f && lenSq != 0.0f) {
        float invLen = 1.0f / sqrtf(lenSq);
        x *= invLen;
        y *= invLen;
        z *= invLen;
    }

    float sinres = sinf(radians);
    float cosres = cosf(radians);
    float t = 1.0f - cosres;

    result.m00 = x * x * t + cosres;
    result.m01 = y * x * t + z * sinres;
    result.m02 = z * x * t - y * sinres;

    result.m10 = x * y * t - z * sinres;
    result.m11 = y * y * t + cosres;
    result.m12 = z * y * t + x * sinres;

    result.m20 = x * z * t + y * sinres;
    result.m21 = y * z * t - x * sinres;
    result.m22 = z * z * t + cosres;

    return result;
}

HP_Mat4 HP_Mat4RotateX(float radians)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    float c = cosf(radians);
    float s = sinf(radians);

    result.m11 = c;
    result.m12 = s;
    result.m21 = -s;
    result.m22 = c;

    return result;
}

HP_Mat4 HP_Mat4RotateY(float radians)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    float c = cosf(radians);
    float s = sinf(radians);

    result.m00 = c;
    result.m02 = -s;
    result.m20 = s;
    result.m22 = c;

    return result;
}

HP_Mat4 HP_Mat4RotateZ(float radians)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    float c = cosf(radians);
    float s = sinf(radians);

    result.m00 = c;
    result.m01 = s;
    result.m10 = -s;
    result.m11 = c;

    return result;
}

HP_Mat4 HP_Mat4RotateXYZ(HP_Vec3 radians)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    float cz = cosf(-radians.z);
    float sz = sinf(-radians.z);
    float cy = cosf(-radians.y);
    float sy = sinf(-radians.y);
    float cx = cosf(-radians.x);
    float sx = sinf(-radians.x);

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

HP_Mat4 HP_Mat4RotateZYX(HP_Vec3 radians)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    float cz = cosf(radians.z);
    float sz = sinf(radians.z);
    float cy = cosf(radians.y);
    float sy = sinf(radians.y);
    float cx = cosf(radians.x);
    float sx = sinf(radians.x);

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

HP_Mat4 HP_Mat4Scale(HP_Vec3 scale)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    result.m00 = scale.x;
    result.m11 = scale.y;
    result.m22 = scale.z;

    return result;
}

HP_Transform HP_Mat4Decompose(const HP_Mat4* mat)
{
    HP_Transform t;

    /* --- Translation --- */

    t.translation.x = mat->m30;
    t.translation.y = mat->m31;
    t.translation.z = mat->m32;

    /* --- Scale --- */

    t.scale.x = sqrtf(mat->m00*mat->m00 + mat->m01*mat->m01 + mat->m02*mat->m02);
    t.scale.y = sqrtf(mat->m10*mat->m10 + mat->m11*mat->m11 + mat->m12*mat->m12);
    t.scale.z = sqrtf(mat->m20*mat->m20 + mat->m21*mat->m21 + mat->m22*mat->m22);

    /* --- Rotation --- */

    float m00 = mat->m00 / t.scale.x;
    float m01 = mat->m01 / t.scale.x;
    float m02 = mat->m02 / t.scale.x;

    float m10 = mat->m10 / t.scale.y;
    float m11 = mat->m11 / t.scale.y;
    float m12 = mat->m12 / t.scale.y;

    float m20 = mat->m20 / t.scale.z;
    float m21 = mat->m21 / t.scale.z;
    float m22 = mat->m22 / t.scale.z;

    float trace = m00 + m11 + m22;

    if (trace > 0.0f) {
        float s = sqrtf(trace + 1.0f) * 2.0f;
        t.rotation.w = 0.25f * s;
        t.rotation.x = (m21 - m12) / s;
        t.rotation.y = (m02 - m20) / s;
        t.rotation.z = (m10 - m01) / s;
    }
    else if ((m00 > m11) && (m00 > m22)) {
        float s = sqrtf(1.0f + m00 - m11 - m22) * 2.0f;
        t.rotation.w = (m21 - m12) / s;
        t.rotation.x = 0.25f * s;
        t.rotation.y = (m01 + m10) / s;
        t.rotation.z = (m02 + m20) / s;
    }
    else if (m11 > m22) {
        float s = sqrtf(1.0f + m11 - m00 - m22) * 2.0f;
        t.rotation.w = (m02 - m20) / s;
        t.rotation.x = (m01 + m10) / s;
        t.rotation.y = 0.25f * s;
        t.rotation.z = (m12 + m21) / s;
    }
    else {
        float s = sqrtf(1.0f + m22 - m00 - m11) * 2.0f;
        t.rotation.w = (m10 - m01) / s;
        t.rotation.x = (m02 + m20) / s;
        t.rotation.y = (m12 + m21) / s;
        t.rotation.z = 0.25f * s;
    }

    return t;
}


HP_Mat4 HP_Mat4Frustum(float left, float right, float bottom, float top, float znear, float zfar)
{
    HP_Mat4 result = { 0 };

    float rl = right - left;
    float tb = top - bottom;
    float fn = zfar - znear;

    result.m00 = (znear * 2.0f) / rl;
    result.m11 = (znear * 2.0f) / tb;

    result.m20 = (right + left) / rl;
    result.m21 = (top + bottom) / tb;
    result.m22 = -(zfar + znear) / fn;
    result.m23 = -1.0f;

    result.m32 = -(zfar * znear * 2.0f) / fn;

    return result;
}

HP_Mat4 HP_Mat4Perspective(float fovy, float aspect, float znear, float zfar)
{
    HP_Mat4 result = { 0 };

    float top = znear * tanf(fovy * 0.5f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    float rl = right - left;
    float tb = top - bottom;
    float fn = zfar - znear;

    result.m00 = (znear * 2.0f) / rl;
    result.m11 = (znear * 2.0f) / tb;

    result.m20 = (right + left) / rl;
    result.m21 = (top + bottom) / tb;
    result.m22 = -(zfar + znear) / fn;
    result.m23 = -1.0f;

    result.m32 = -(zfar * znear * 2.0f) / fn;

    return result;
}

HP_Mat4 HP_Mat4Ortho(float left, float right, float bottom, float top, float znear, float zfar)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    float rl = (right - left);
    float tb = (top - bottom);
    float fn = (zfar - znear);

    result.m00 = 2.0f / rl;
    result.m11 = 2.0f / tb;
    result.m22 = -2.0f / fn;

    result.m23 = 0.0f;
    result.m30 = -(left + right) / rl;
    result.m31 = -(top + bottom) / tb;

    result.m32 = -(zfar + znear) / fn;

    return result;
}

HP_Mat4 HP_Mat4LookAt(HP_Vec3 eye, HP_Vec3 target, HP_Vec3 up)
{
    HP_Mat4 result = HP_MAT4_IDENTITY;

    float length = 0.0f;
    float invLen = 0.0f;

    HP_Vec3 vz = {
        eye.x - target.x,
        eye.y - target.y,
        eye.z - target.z
    };

    length = sqrtf(vz.x * vz.x + vz.y * vz.y + vz.z * vz.z);
    if (length < 1e-6f) length = 1.0f;
    invLen = 1.0f / length;
    vz.x *= invLen;
    vz.y *= invLen;
    vz.z *= invLen;

    HP_Vec3 vx = {
        up.y * vz.z - up.z * vz.y,
        up.z * vz.x - up.x * vz.z,
        up.x * vz.y - up.y * vz.x
    };

    length = sqrtf(vx.x * vx.x + vx.y * vx.y + vx.z * vx.z);
    if (length < 1e-6f) length = 1.0f;
    invLen = 1.0f / length;
    vx.x *= invLen;
    vx.y *= invLen;
    vx.z *= invLen;

    HP_Vec3 vy = {
        vz.y * vx.z - vz.z * vx.y,
        vz.z * vx.x - vz.x * vx.z,
        vz.x * vx.y - vz.y * vx.x
    };

    result.m00 = vx.x;
    result.m01 = vy.x;
    result.m02 = vz.x;

    result.m10 = vx.y;
    result.m11 = vy.y;
    result.m12 = vz.y;

    result.m20 = vx.z;
    result.m21 = vy.z;
    result.m22 = vz.z;

    result.m30 = -(vx.x * eye.x + vx.y * eye.y + vx.z * eye.z);
    result.m31 = -(vy.x * eye.x + vy.y * eye.y + vy.z * eye.z);
    result.m32 = -(vz.x * eye.x + vz.y * eye.y + vz.z * eye.z);

    return result;
}

float HP_Mat4Determinant(const HP_Mat4* mat)
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

HP_Mat4 HP_Mat4Transpose(const HP_Mat4* mat)
{
    HP_Mat4 result;
    float* HP_RESTRICT R = result.a;
    const float* HP_RESTRICT M = mat->a;

#if defined(HP_HAS_SSE)

    __m128 row0 = _mm_loadu_ps(&M[0]);
    __m128 row1 = _mm_loadu_ps(&M[4]);
    __m128 row2 = _mm_loadu_ps(&M[8]);
    __m128 row3 = _mm_loadu_ps(&M[12]);

    _MM_TRANSPOSE4_PS(row0, row1, row2, row3);

    _mm_storeu_ps(&R[0],  row0);
    _mm_storeu_ps(&R[4],  row1);
    _mm_storeu_ps(&R[8],  row2);
    _mm_storeu_ps(&R[12], row3);

#elif defined(HP_HAS_NEON)

    float32x4x2_t t0 = vtrnq_f32(vld1q_f32(&M[0]), vld1q_f32(&M[4]));
    float32x4x2_t t1 = vtrnq_f32(vld1q_f32(&M[8]), vld1q_f32(&M[12]));

    vst1q_f32(&R[0],  vcombine_f32(vget_low_f32(t0.val[0]), vget_low_f32(t1.val[0])));
    vst1q_f32(&R[4],  vcombine_f32(vget_low_f32(t0.val[1]), vget_low_f32(t1.val[1])));
    vst1q_f32(&R[8],  vcombine_f32(vget_high_f32(t0.val[0]), vget_high_f32(t1.val[0])));
    vst1q_f32(&R[12], vcombine_f32(vget_high_f32(t0.val[1]), vget_high_f32(t1.val[1])));

#else
    R[0]  = M[0];   R[1]  = M[4];   R[2]  = M[8];   R[3]  = M[12];
    R[4]  = M[1];   R[5]  = M[5];   R[6]  = M[9];   R[7]  = M[13];
    R[8]  = M[2];   R[9]  = M[6];   R[10] = M[10];  R[11] = M[14];
    R[12] = M[3];   R[13] = M[7];   R[14] = M[11];  R[15] = M[15];
#endif

    return result;
}

HP_Mat4 HP_Mat4Inverse(const HP_Mat4* mat)
{
    HP_Mat4 result = { 0 };

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

float HP_Mat4Trace(const HP_Mat4* mat)
{
    return mat->m00 + mat->m11 + mat->m22 + mat->m33;
}

HP_Mat4 HP_Mat4Add(const HP_Mat4* left, const HP_Mat4* right)
{
    HP_Mat4 result;
    for (int_fast8_t i = 0; i < 16; ++i) {
        result.a[i] = left->a[i] + right->a[i];
    }
    return result;
}

HP_Mat4 HP_Mat4Sub(const HP_Mat4* left, const HP_Mat4* right)
{
    HP_Mat4 result;
    for (int_fast8_t i = 0; i < 16; ++i) {
        result.a[i] = left->a[i] - right->a[i];
    }
    return result;
}

HP_Mat4 HP_Mat4Mul(const HP_Mat4* HP_RESTRICT left, const HP_Mat4* HP_RESTRICT right)
{
    HP_Mat4 result;
    float* HP_RESTRICT R = result.a;
    const float* HP_RESTRICT A = left->a;
    const float* HP_RESTRICT B = right->a;

#if defined(HP_HAS_FMA_AVX)

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

#elif defined(HP_HAS_AVX)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai0 = _mm_broadcast_ss(&A[i * 4 + 0]);
        __m128 ai1 = _mm_broadcast_ss(&A[i * 4 + 1]);
        __m128 ai2 = _mm_broadcast_ss(&A[i * 4 + 2]);
        __m128 ai3 = _mm_broadcast_ss(&A[i * 4 + 3]);

        __m128 row = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(ai0, col0), _mm_mul_ps(ai1, col1)),
            _mm_add_ps(_mm_mul_ps(ai2, col2), _mm_mul_ps(ai3, col3))
        );

        _mm_storeu_ps(&R[i * 4], row);
    }

#elif defined(HP_HAS_SSE42)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai0 = _mm_set1_ps(A[i * 4 + 0]);
        __m128 ai1 = _mm_set1_ps(A[i * 4 + 1]);
        __m128 ai2 = _mm_set1_ps(A[i * 4 + 2]);
        __m128 ai3 = _mm_set1_ps(A[i * 4 + 3]);

        __m128 row = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(ai0, col0), _mm_mul_ps(ai1, col1)),
            _mm_add_ps(_mm_mul_ps(ai2, col2), _mm_mul_ps(ai3, col3))
        );

        _mm_storeu_ps(&R[i * 4], row);
    }

#elif defined(HP_HAS_SSE41)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai = _mm_loadu_ps(&A[i * 4]);

        R[i * 4 + 0] = _mm_cvtss_f32(_mm_dp_ps(ai, col0, 0xF1));
        R[i * 4 + 1] = _mm_cvtss_f32(_mm_dp_ps(ai, col1, 0xF1));
        R[i * 4 + 2] = _mm_cvtss_f32(_mm_dp_ps(ai, col2, 0xF1));
        R[i * 4 + 3] = _mm_cvtss_f32(_mm_dp_ps(ai, col3, 0xF1));
    }

#elif defined(HP_HAS_SSE)

    __m128 col0 = _mm_loadu_ps(&B[0]);
    __m128 col1 = _mm_loadu_ps(&B[4]);
    __m128 col2 = _mm_loadu_ps(&B[8]);
    __m128 col3 = _mm_loadu_ps(&B[12]);

    for (int i = 0; i < 4; i++) {
        __m128 ai0 = _mm_set1_ps(A[i * 4 + 0]);
        __m128 ai1 = _mm_set1_ps(A[i * 4 + 1]);
        __m128 ai2 = _mm_set1_ps(A[i * 4 + 2]);
        __m128 ai3 = _mm_set1_ps(A[i * 4 + 3]);

        __m128 row = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(ai0, col0), _mm_mul_ps(ai1, col1)),
            _mm_add_ps(_mm_mul_ps(ai2, col2), _mm_mul_ps(ai3, col3))
        );

        _mm_storeu_ps(&R[i * 4], row);
    }

#elif defined(HP_HAS_NEON_FMA)

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
        row = vfmaq_f32(row, ai1, col1);  // FMA: row += ai1 * col1
        row = vfmaq_f32(row, ai2, col2);  // FMA: row += ai2 * col2
        row = vfmaq_f32(row, ai3, col3);  // FMA: row += ai3 * col3

        vst1q_f32(&R[i * 4], row);
    }

#elif defined(HP_HAS_NEON)

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
        row = vmlaq_f32(row, ai1, col1);  // FMA: row += ai1 * col1
        row = vmlaq_f32(row, ai2, col2);  // FMA: row += ai2 * col2
        row = vmlaq_f32(row, ai3, col3);  // FMA: row += ai3 * col3

        vst1q_f32(&R[i * 4], row);
    }

#else

    for (int i = 0; i < 4; i++) {
        const float ai0 = A[i * 4 + 0];
        const float ai1 = A[i * 4 + 1];
        const float ai2 = A[i * 4 + 2];
        const float ai3 = A[i * 4 + 3];

        result.a[i * 4 + 0] = ai0 * B[0]  + ai1 * B[4]  + ai2 * B[8]  + ai3 * B[12];
        result.a[i * 4 + 1] = ai0 * B[1]  + ai1 * B[5]  + ai2 * B[9]  + ai3 * B[13];
        result.a[i * 4 + 2] = ai0 * B[2]  + ai1 * B[6]  + ai2 * B[10] + ai3 * B[14];
        result.a[i * 4 + 3] = ai0 * B[3]  + ai1 * B[7]  + ai2 * B[11] + ai3 * B[15];
    }

#endif

    return result;
}

void HP_Mat4MulBatch(HP_Mat4* HP_RESTRICT results,
                     const HP_Mat4* HP_RESTRICT leftMatrices,
                     const HP_Mat4* HP_RESTRICT rightMatrices,
                     size_t count)
{
#if defined(HP_HAS_FMA_AVX)

    size_t simdCount = count & ~1;

    for (size_t batch = 0; batch < simdCount; batch += 2) {
        __m128 b0_col0 = _mm_loadu_ps(&rightMatrices[batch].a[0]);
        __m128 b0_col1 = _mm_loadu_ps(&rightMatrices[batch].a[4]);
        __m128 b0_col2 = _mm_loadu_ps(&rightMatrices[batch].a[8]);
        __m128 b0_col3 = _mm_loadu_ps(&rightMatrices[batch].a[12]);

        __m128 b1_col0 = _mm_loadu_ps(&rightMatrices[batch + 1].a[0]);
        __m128 b1_col1 = _mm_loadu_ps(&rightMatrices[batch + 1].a[4]);
        __m128 b1_col2 = _mm_loadu_ps(&rightMatrices[batch + 1].a[8]);
        __m128 b1_col3 = _mm_loadu_ps(&rightMatrices[batch + 1].a[12]);

        for (int i = 0; i < 4; i++) {
            __m128 a0_i0 = _mm_broadcast_ss(&leftMatrices[batch].a[i * 4 + 0]);
            __m128 a0_i1 = _mm_broadcast_ss(&leftMatrices[batch].a[i * 4 + 1]);
            __m128 a0_i2 = _mm_broadcast_ss(&leftMatrices[batch].a[i * 4 + 2]);
            __m128 a0_i3 = _mm_broadcast_ss(&leftMatrices[batch].a[i * 4 + 3]);

            __m128 row0 = _mm_mul_ps(a0_i0, b0_col0);
            row0 = _mm_fmadd_ps(a0_i1, b0_col1, row0);
            row0 = _mm_fmadd_ps(a0_i2, b0_col2, row0);
            row0 = _mm_fmadd_ps(a0_i3, b0_col3, row0);
            _mm_storeu_ps(&results[batch].a[i * 4], row0);

            __m128 a1_i0 = _mm_broadcast_ss(&leftMatrices[batch + 1].a[i * 4 + 0]);
            __m128 a1_i1 = _mm_broadcast_ss(&leftMatrices[batch + 1].a[i * 4 + 1]);
            __m128 a1_i2 = _mm_broadcast_ss(&leftMatrices[batch + 1].a[i * 4 + 2]);
            __m128 a1_i3 = _mm_broadcast_ss(&leftMatrices[batch + 1].a[i * 4 + 3]);

            __m128 row1 = _mm_mul_ps(a1_i0, b1_col0);
            row1 = _mm_fmadd_ps(a1_i1, b1_col1, row1);
            row1 = _mm_fmadd_ps(a1_i2, b1_col2, row1);
            row1 = _mm_fmadd_ps(a1_i3, b1_col3, row1);
            _mm_storeu_ps(&results[batch + 1].a[i * 4], row1);
        }
    }

    for (size_t i = simdCount; i < count; i++) {
        results[i] = HP_Mat4Mul(&leftMatrices[i], &rightMatrices[i]);
    }

#elif defined(HP_HAS_AVX)

    for (size_t batch = 0; batch < count; batch++) {
        const float* A = leftMatrices[batch].a;
        const float* B = rightMatrices[batch].a;
        float* R = results[batch].a;

        __m128 col0 = _mm_loadu_ps(&B[0]);
        __m128 col1 = _mm_loadu_ps(&B[4]);
        __m128 col2 = _mm_loadu_ps(&B[8]);
        __m128 col3 = _mm_loadu_ps(&B[12]);

        for (int i = 0; i < 4; i++) {
            __m128 ai0 = _mm_broadcast_ss(&A[i * 4 + 0]);
            __m128 ai1 = _mm_broadcast_ss(&A[i * 4 + 1]);
            __m128 ai2 = _mm_broadcast_ss(&A[i * 4 + 2]);
            __m128 ai3 = _mm_broadcast_ss(&A[i * 4 + 3]);

            __m128 row = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(ai0, col0), _mm_mul_ps(ai1, col1)),
                _mm_add_ps(_mm_mul_ps(ai2, col2), _mm_mul_ps(ai3, col3))
            );
            _mm_storeu_ps(&R[i * 4], row);
        }
    }

#elif defined(HP_HAS_SSE)

    for (size_t batch = 0; batch < count; batch++) {
        const float* A = leftMatrices[batch].a;
        const float* B = rightMatrices[batch].a;
        float* R = results[batch].a;

        if (batch + 1 < count) {
            _mm_prefetch((const char*)&leftMatrices[batch + 1], _MM_HINT_T0);
            _mm_prefetch((const char*)&rightMatrices[batch + 1], _MM_HINT_T0);
        }

        __m128 col0 = _mm_loadu_ps(&B[0]);
        __m128 col1 = _mm_loadu_ps(&B[4]);
        __m128 col2 = _mm_loadu_ps(&B[8]);
        __m128 col3 = _mm_loadu_ps(&B[12]);

        for (int i = 0; i < 4; i++) {
            __m128 ai0 = _mm_set1_ps(A[i * 4 + 0]);
            __m128 ai1 = _mm_set1_ps(A[i * 4 + 1]);
            __m128 ai2 = _mm_set1_ps(A[i * 4 + 2]);
            __m128 ai3 = _mm_set1_ps(A[i * 4 + 3]);

            __m128 row = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(ai0, col0), _mm_mul_ps(ai1, col1)),
                _mm_add_ps(_mm_mul_ps(ai2, col2), _mm_mul_ps(ai3, col3))
            );
            _mm_storeu_ps(&R[i * 4], row);
        }
    }

#elif defined(HP_HAS_NEON_FMA)

    for (size_t batch = 0; batch < count; batch++) {
        const float* A = leftMatrices[batch].a;
        const float* B = rightMatrices[batch].a;
        float* R = results[batch].a;

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
    }

#elif defined(HP_HAS_NEON)

    for (size_t batch = 0; batch < count; batch++) {
        const float* A = leftMatrices[batch].a;
        const float* B = rightMatrices[batch].a;
        float* R = results[batch].a;

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
    }

#else

    for (size_t batch = 0; batch < count; batch++) {
        const float* A = leftMatrices[batch].a;
        const float* B = rightMatrices[batch].a;
        float* R = results[batch].a;

        const float a00 = A[0], a01 = A[1], a02 = A[2], a03 = A[3];
        R[0] = a00 * B[0] + a01 * B[4] + a02 * B[8] + a03 * B[12];
        R[1] = a00 * B[1] + a01 * B[5] + a02 * B[9] + a03 * B[13];
        R[2] = a00 * B[2] + a01 * B[6] + a02 * B[10] + a03 * B[14];
        R[3] = a00 * B[3] + a01 * B[7] + a02 * B[11] + a03 * B[15];

        const float a10 = A[4], a11 = A[5], a12 = A[6], a13 = A[7];
        R[4] = a10 * B[0] + a11 * B[4] + a12 * B[8] + a13 * B[12];
        R[5] = a10 * B[1] + a11 * B[5] + a12 * B[9] + a13 * B[13];
        R[6] = a10 * B[2] + a11 * B[6] + a12 * B[10] + a13 * B[14];
        R[7] = a10 * B[3] + a11 * B[7] + a12 * B[11] + a13 * B[15];

        const float a20 = A[8], a21 = A[9], a22 = A[10], a23 = A[11];
        R[8] = a20 * B[0] + a21 * B[4] + a22 * B[8] + a23 * B[12];
        R[9] = a20 * B[1] + a21 * B[5] + a22 * B[9] + a23 * B[13];
        R[10] = a20 * B[2] + a21 * B[6] + a22 * B[10] + a23 * B[14];
        R[11] = a20 * B[3] + a21 * B[7] + a22 * B[11] + a23 * B[15];

        const float a30 = A[12], a31 = A[13], a32 = A[14], a33 = A[15];
        R[12] = a30 * B[0] + a31 * B[4] + a32 * B[8] + a33 * B[12];
        R[13] = a30 * B[1] + a31 * B[5] + a32 * B[9] + a33 * B[13];
        R[14] = a30 * B[2] + a31 * B[6] + a32 * B[10] + a33 * B[14];
        R[15] = a30 * B[3] + a31 * B[7] + a32 * B[11] + a33 * B[15];
    }

#endif
}

/* === Transform Functions === */

HP_Mat4 HP_TransformToMat4(const HP_Transform* transform)
{
    HP_Mat4 m;

    const HP_Vec3* t = &transform->translation;
    const HP_Quat* q = &transform->rotation;
    const HP_Vec3* s = &transform->scale;

    float qlen = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
    if (qlen < 1e-6f) {
        m.m00 = s->x; m.m01 = 0.0f; m.m02 = 0.0f; m.m03 = 0.0f;
        m.m10 = 0.0f; m.m11 = s->y; m.m12 = 0.0f; m.m13 = 0.0f;
        m.m20 = 0.0f; m.m21 = 0.0f; m.m22 = s->z; m.m23 = 0.0f;
        m.m30 = t->x; m.m31 = t->y; m.m32 = t->z; m.m33 = 1.0f;
        return m;
    }

    float invLen = 1.0f / qlen;
    float qx = q->x * invLen;
    float qy = q->y * invLen;
    float qz = q->z * invLen;
    float qw = q->w * invLen;

    float qx2 = qx * qx, qy2 = qy * qy, qz2 = qz * qz;
    float qxqy = qx * qy, qxqz = qx * qz, qxqw = qx * qw;
    float qyqz = qy * qz, qyqw = qy * qw, qzqw = qz * qw;

    m.m00 = s->x * (1.0f - 2.0f * (qy2 + qz2));
    m.m01 = s->y * (2.0f * (qxqy + qzqw));
    m.m02 = s->z * (2.0f * (qxqz - qyqw));
    m.m03 = 0.0f;

    m.m10 = s->x * (2.0f * (qxqy - qzqw));
    m.m11 = s->y * (1.0f - 2.0f * (qx2 + qz2));
    m.m12 = s->z * (2.0f * (qyqz + qxqw));
    m.m13 = 0.0f;

    m.m20 = s->x * (2.0f * (qxqz + qyqw));
    m.m21 = s->y * (2.0f * (qyqz - qxqw));
    m.m22 = s->z * (1.0f - 2.0f * (qx2 + qy2));
    m.m23 = 0.0f;

    m.m30 = t->x;
    m.m31 = t->y;
    m.m32 = t->z;
    m.m33 = 1.0f;

    return m;
}

HP_Transform HP_TransformCombine(const HP_Transform* parent, const HP_Transform* child)
{
    HP_Transform result;

    result.rotation = HP_QuatMul(parent->rotation, child->rotation);

    result.scale.x = parent->scale.x * child->scale.x;
    result.scale.y = parent->scale.y * child->scale.y;
    result.scale.z = parent->scale.z * child->scale.z;

    HP_Vec3 scaledChildTranslation;
    scaledChildTranslation.x = child->translation.x * parent->scale.x;
    scaledChildTranslation.y = child->translation.y * parent->scale.y;
    scaledChildTranslation.z = child->translation.z * parent->scale.z;

    HP_Vec3 rotatedChildTranslation = HP_Vec3Rotate(scaledChildTranslation, parent->rotation);

    result.translation.x = parent->translation.x + rotatedChildTranslation.x;
    result.translation.y = parent->translation.y + rotatedChildTranslation.y;
    result.translation.z = parent->translation.z + rotatedChildTranslation.z;

    return result;
}

HP_Transform HP_TransformLerp(const HP_Transform* a, const HP_Transform* b, float t)
{
    HP_Transform result;
    result.translation = HP_Vec3Lerp(a->translation, b->translation, t);
    result.rotation = HP_QuatSLerp(a->rotation, b->rotation, t);
    result.scale = HP_Vec3Lerp(a->scale, b->scale, t);
    return result;
}
