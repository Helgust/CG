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

Vec3f crossProduct(const Vec3f &a, const Vec3f &b) 
    { return Vec3f(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y *  b.x); }

float deg2rad(const float &deg) 
{ return deg * M_PI / 180; }


Vec3f reflect(const Vec3f &I, const Vec3f &N) 
{ 
    return I -  ((N* dotProduct(N,I) * 2)); 
} 

float norma(const Vec3f &v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

float clamp(const float &lo, const float &hi, const float &v) 
{ return std::max(lo, std::min(hi, v)); } 

void fresnel(const Vec3f &I, const Vec3f &N, const float &ior, float &kr) 
{ 
    float cosi = clamp(-1, 1, dotProduct(I, N)); 
    float etai = 1, etat = ior; 
    if (cosi > 0) {  std::swap(etai, etat); } 
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi)); 
    // Total internal reflection
    if (sint >= 1) { 
        kr = 1; 
    } 
    else { 
        float cost = sqrtf(std::max(0.f, 1 - sint * sint)); 
        cosi = fabsf(cosi); 
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        kr = (Rs * Rs + Rp * Rp) / 2; 
    } 
}  

Vec3f refract(const Vec3f &I, const Vec3f &N, const float &refractive_index) 
{ // Snell's law
    float cosi = - std::max(-1.f, std::min(1.f, dotProduct(I,N)));
    float etai = 1, etat = refractive_index;
    Vec3f n = N;
    if (cosi < 0) { // if the ray is inside the object, swap the indices and invert the normal to get the correct result
    cosi = -cosi;
        std::swap(etai, etat); n = -N;
    }
    float eta = etai / etat;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k < 0 ? Vec3f(0,0,0) : I*eta + n*(eta * cosi - sqrtf(k));
}


bool intersectPlane(const Vec3f &n, const Vec3f &p0, const Vec3f &l0, const Vec3f &l, float &t) 
{ 
    // assuming vectors are all normalized
    float denom = dotProduct(n, l); 
    if (denom > 1e-6) { 
        Vec3f p0l0 = p0 - l0; 
        t = dotProduct(p0l0, n) / denom; 
        return (t >= 0); 
    } 
 
    return false; 
}

bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0)
        return false;
    else if (discr == 0)
        x0 = x1 = -0.5 * b / a;
    else
    {
        float q = (b > 0) ? -0.5 * (b + sqrt(discr)) : -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1)
        std::swap(x0, x1);
    return true;
}







#endif