#ifndef Objects_h
#define Objects_h

#include <cmath>
#include <vector>
#include <iostream>

#include "vectors.h"
#include "functions.h"

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

struct Sphere
{
    Vec3f center;
    float radius;
    Sphere(const Vec3f &c, const float &r) : center(c), radius(r){};

    bool intersection(const Vec3f &orig, const Vec3f &dir, float &tnear) const 
    { 
        // analytic solution
        Vec3f L = orig - center; 
        float a = dotProduct(dir, dir); 
        float b = 2 * dotProduct(dir, L); 
        float c = dotProduct(L, L) - (radius*radius); 
        float t0, t1; 
        if (!solveQuadratic(a, b, c, t0, t1)) return false; 
        if (t0 < 0) t0 = t1; 
        if (t0 < 0) return false; 
        tnear = t0; 
 
        return true; 
    } 
};

#endif
