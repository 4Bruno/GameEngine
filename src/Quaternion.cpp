// Copyright (C) 2019 Martin Weigel <mail@MartinWeigel.com>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

/**
 * @file    Quaternion.c
 * @brief   A basic quaternion library written in C
 * @date    2019-11-28
 */
#include "Quaternion.h"
#include <stdlib.h>

void Quaternion_set(r32 w, r32 v1, r32 v2, r32 v3, Quaternion* output)
{
    Assert(output != NULL);
    output->w = w;
    output->v[0] = v1;
    output->v[1] = v2;
    output->v[2] = v3;
}

void Quaternion_setIdentity(Quaternion* q)
{
    Assert(q != NULL);
    Quaternion_set(1, 0, 0, 0, q);
}

void Quaternion_copy(Quaternion* q, Quaternion* output)
{
    Quaternion_set(q->w, q->v[0], q->v[1], q->v[2], output);
}

bool Quaternion_equal(Quaternion* q1, Quaternion* q2)
{
    bool equalW  = fabs(q1->w - q2->w) <= QUATERNION_EPS;
    bool equalV0 = fabs(q1->v[0] - q2->v[0]) <= QUATERNION_EPS;
    bool equalV1 = fabs(q1->v[1] - q2->v[1]) <= QUATERNION_EPS;
    bool equalV2 = fabs(q1->v[2] - q2->v[2]) <= QUATERNION_EPS;
    return equalW && equalV0 && equalV1 && equalV2;
}

void Quaternion_fprint(FILE* file, Quaternion* q)
{
    fprintf(file, "(%.3f, %.3f, %.3f, %.3f)",
        q->w, q->v[0], q->v[1], q->v[2]);
}


void Quaternion_fromAxisAngle(r32 axis[3], r32 angle, Quaternion* output)
{
    Assert(output != NULL);
    // Formula from http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToQuaternion/
    output->w = cosf(angle / 2.0f);
    r32 c = sinf(angle / 2.0f);
    output->v[0] = c * axis[0];
    output->v[1] = c * axis[1];
    output->v[2] = c * axis[2];
}

r32 Quaternion_toAxisAngle(Quaternion* q, r32 output[3])
{
    Assert(output != NULL);
    // Formula from http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToAngle/
    r32 angle = 2.0f * acosf(q->w);
    r32 divider = sqrtf(1.0f - q->w * q->w);

    if(divider != 0.0f) {
        // Calculate the axis
        output[0] = q->v[0] / divider;
        output[1] = q->v[1] / divider;
        output[2] = q->v[2] / divider;
    } else {
        // Arbitrary normalized axis
        output[0] = 1;
        output[1] = 0;
        output[2] = 0;
    }
    return angle;
}

void Quaternion_fromXRotation(r32 angle, Quaternion* output)
{
    Assert(output != NULL);
    r32 axis[3] = {1.0f, 0, 0};
    Quaternion_fromAxisAngle(axis, angle, output);
}

void Quaternion_fromYRotation(r32 angle, Quaternion* output)
{
    Assert(output != NULL);
    r32 axis[3] = {0, 1.0f, 0};
    Quaternion_fromAxisAngle(axis, angle, output);
}

void Quaternion_fromZRotation(r32 angle, Quaternion* output)
{
    Assert(output != NULL);
    r32 axis[3] = {0, 0, 1.0f};
    Quaternion_fromAxisAngle(axis, angle, output);
}

void Quaternion_fromEulerZYX(v3 * eulerZYX, Quaternion* output)
{
    Assert(output != NULL);
    // Based on https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    r32 cy = cosf(eulerZYX->z * 0.5f);
    r32 sy = sinf(eulerZYX->z * 0.5f);
    r32 cr = cosf(eulerZYX->x * 0.5f);
    r32 sr = sinf(eulerZYX->x * 0.5f);
    r32 cp = cosf(eulerZYX->y * 0.5f);
    r32 sp = sinf(eulerZYX->y * 0.5f);

    output->w = cy * cr * cp + sy * sr * sp;
    output->v[0] = cy * sr * cp - sy * cr * sp;
    output->v[1] = cy * cr * sp + sy * sr * cp;
    output->v[2] = sy * cr * cp - cy * sr * sp;
}

void Quaternion_toEulerZYX(Quaternion* q, r32 output[3])
{
    Assert(output != NULL);

    // Roll (x-axis rotation)
    r32 sinr_cosp = +2.0f * (q->w * q->v[0] + q->v[1] * q->v[2]);
    r32 cosr_cosp = +1.0f - 2.0f * (q->v[0] * q->v[0] + q->v[1] * q->v[1]);
    output[0] = atan2f(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    r32 sinp = +2.0f * (q->w * q->v[1] - q->v[2] * q->v[0]);
    if (fabs(sinp) >= 1)
        output[1] = copysignf((r32)PI / 2, sinp); // use 90 degrees if out of range
    else
        output[1] = asinf(sinp);

    // Yaw (z-axis rotation)
    r32 siny_cosp = +2.0f * (q->w * q->v[2] + q->v[0] * q->v[1]);
    r32 cosy_cosp = +1.0f - 2.0f * (q->v[1] * q->v[1] + q->v[2] * q->v[2]);
    output[2] = atan2f(siny_cosp, cosy_cosp);
}

void Quaternion_conjugate(Quaternion* q, Quaternion* output)
{
    Assert(output != NULL);
    output->w = q->w;
    output->v[0] = -q->v[0];
    output->v[1] = -q->v[1];
    output->v[2] = -q->v[2];
}

r32 Quaternion_norm(Quaternion* q)
{
    Assert(q != NULL);
    return sqrtf(q->w*q->w + q->v[0]*q->v[0] + q->v[1]*q->v[1] + q->v[2]*q->v[2]);
}

void Quaternion_normalize(Quaternion* q, Quaternion* output)
{
    Assert(output != NULL);
    r32 len = Quaternion_norm(q);
    Quaternion_set(
        q->w / len,
        q->v[0] / len,
        q->v[1] / len,
        q->v[2] / len,
        output);
}

void Quaternion_multiply(Quaternion* q1, Quaternion* q2, Quaternion* output)
{
    Assert(output != NULL);
    Quaternion result;

    /*
    Formula from http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/arithmetic/index.htm
             a*e - b*f - c*g - d*h
        + i (b*e + a*f + c*h- d*g)
        + j (a*g - b*h + c*e + d*f)
        + k (a*h + b*g - c*f + d*e)
    */
    result.w =    q1->w   *q2->w    - q1->v[0]*q2->v[0] - q1->v[1]*q2->v[1] - q1->v[2]*q2->v[2];
    result.v[0] = q1->v[0]*q2->w    + q1->w   *q2->v[0] + q1->v[1]*q2->v[2] - q1->v[2]*q2->v[1];
    result.v[1] = q1->w   *q2->v[1] - q1->v[0]*q2->v[2] + q1->v[1]*q2->w    + q1->v[2]*q2->v[0];
    result.v[2] = q1->w   *q2->v[2] + q1->v[0]*q2->v[1] - q1->v[1]*q2->v[0] + q1->v[2]*q2->w   ;

    *output = result;
}

void Quaternion_rotate(Quaternion * q, v3 * v, v3 * output)
{
    Assert(output != NULL);
    v3 result;

    r32 ww = q->w * q->w;
    r32 xx = q->v[0] * q->v[0];
    r32 yy = q->v[1] * q->v[1];
    r32 zz = q->v[2] * q->v[2];
    r32 wx = q->w * q->v[0];
    r32 wy = q->w * q->v[1];
    r32 wz = q->w * q->v[2];
    r32 xy = q->v[0] * q->v[1];
    r32 xz = q->v[0] * q->v[2];
    r32 yz = q->v[1] * q->v[2];

    // Formula from http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/transforms/index.htm
    // p2.x = w*w*p1.x + 2*y*w*p1.z - 2*z*w*p1.y + x*x*p1.x + 2*y*x*p1.y + 2*z*x*p1.z - z*z*p1.x - y*y*p1.x;
    // p2.y = 2*x*y*p1.x + y*y*p1.y + 2*z*y*p1.z + 2*w*z*p1.x - z*z*p1.y + w*w*p1.y - 2*x*w*p1.z - x*x*p1.y;
    // p2.z = 2*x*z*p1.x + 2*y*z*p1.y + z*z*p1.z - 2*w*y*p1.x - y*y*p1.z + 2*w*x*p1.y - x*x*p1.z + w*w*p1.z;

    result.x = ww*v->x + 2*wy*v->z - 2*wz*v->y +
                xx*v->x + 2*xy*v->y + 2*xz*v->z -
                zz*v->x - yy*v->x;
    result.y = 2*xy*v->x + yy*v->y + 2*yz*v->z +
                2*wz*v->x - zz*v->y + ww*v->y -
                2*wx*v->z - xx*v->y;
    result.z = 2*xz*v->x + 2*yz*v->y + zz*v->z -
                2*wy*v->x - yy*v->z + 2*wx*v->y -
                xx*v->z + ww*v->z;

    // Copy result to output
    *output = result;
}

void Quaternion_slerp(Quaternion* q1, Quaternion* q2, r32 t, Quaternion* output)
{
    Quaternion result;

    // Based on http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
    r32 cosHalfTheta = q1->w*q2->w + q1->v[0]*q2->v[0] + q1->v[1]*q2->v[1] + q1->v[2]*q2->v[2];

    // if q1=q2 or qa=-q2 then theta = 0 and we can return qa
    if (fabs(cosHalfTheta) >= 1.0f) {
        Quaternion_copy(q1, output);
        return;
    }

    r32 halfTheta = acosf(cosHalfTheta);
    r32 sinHalfTheta = sqrtf(1.0f - cosHalfTheta*cosHalfTheta);
    // If theta = 180 degrees then result is not fully defined
    // We could rotate around any axis normal to q1 or q2
    if (fabs(sinHalfTheta) < QUATERNION_EPS) {
        result.w = (q1->w * 0.5f + q2->w * 0.5f);
        result.v[0] = (q1->v[0] * 0.5f + q2->v[0] * 0.5f);
        result.v[1] = (q1->v[1] * 0.5f + q2->v[1] * 0.5f);
        result.v[2] = (q1->v[2] * 0.5f + q2->v[2] * 0.5f);
    }

    // Calculate Quaternion
    r32 ratioA = sinf((1 - t) * halfTheta) / sinHalfTheta;
    r32 ratioB = sinf(t * halfTheta) / sinHalfTheta;
    result.w = (q1->w * ratioA + q2->w * ratioB);
    result.v[0] = (q1->v[0] * ratioA + q2->v[0] * ratioB);
    result.v[1] = (q1->v[1] * ratioA + q2->v[1] * ratioB);
    result.v[2] = (q1->v[2] * ratioA + q2->v[2] * ratioB);

    *output = result;
}

m4 
Quaternion_toMatrix(const Quaternion& quat) 
{ 
    m4 m1{ 
            quat.w, quat.z, -quat.y, quat.x, 
            -quat.z, quat.w, quat.x, quat.y, 
            quat.y, -quat.x, quat.w, quat.z, 
            -quat.x, -quat.y, -quat.z, quat.w }; 

    m4 m2{ 
            quat.w, quat.z, -quat.y, -quat.x, 
            -quat.z, quat.w, quat.x, -quat.y, 
            quat.y, -quat.x, quat.w, -quat.z, 
            quat.x, quat.y, quat.z, quat.w }; 

    return m1 * m2; 
} 
