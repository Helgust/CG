#ifndef Function_h
#define Function_h

#include "vectors.h"

float dotProduct(const Vec3f &a, const Vec3f &b) 
{ return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3f normalize(const Vec3f &v) 
{ 
    float mag2 = v.x * v.x + v.y * v.y + v.z * v.z; 
    if (mag2 > 0) { 
        float invMag = 1 / sqrtf(mag2); 
        return Vec3f(v.x * invMag, v.y * invMag, v.z * invMag); 
    } 
 
    return v; 
} 

float deg2rad(const float &deg) 
{ return deg * M_PI / 180; }


Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N*2.f*(I*N);
}

float norma(const Vec3f &v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
 

#endif