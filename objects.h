#ifndef Objects_h
#define Objects_h

#include <cmath>
#include <vector>
#include <iostream>
#include <random> 

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

class Object 
{ 
 public: 
    Object()  {} 
    virtual ~Object() {} 
    // Method to compute the intersection of the object with a ray
    // Returns true if an intersection was found, false otherwise
    // See method implementation in children class for details
    virtual bool intersection(const Vec3f &, const Vec3f &, float &) const = 0; 
    // Method to compute the surface data such as normal and texture coordnates at the intersection point.
    // See method implementation in children class for details
    //virtual void getSurfaceData(const Vec3f &, Vec3f &, Vec2f &) const = 0; 
    //Vec3f color; 
}; 

struct Light {
    Light(const Vec3f &p, const float &i ,const Vec3f &c) : position(p), intensity(i), color(c) {}
    Vec3f position;
    float intensity;
    Vec3f color;
};

enum MaterialType { DIFFUSE, REFLECTION_AND_REFRACTION, REFLECTION , REFRACTION, DIFFUSE_REFLECTION,GLOSSY};

struct Material {
    Material(const Vec3f &color ,const MaterialType &m , const float &s, const float &r) : diffuse_color(color), materialType(m), specular(s),refract(r) {}
    Material() : diffuse_color(), materialType(DIFFUSE), specular(), refract() {}
    Vec3f diffuse_color;
    MaterialType materialType;
    float specular;
    float refract;
};


struct Sphere : public Object
{
    Vec3f center;
    float radius;
    Material material;
    Sphere(const Vec3f &c, const float &r, const Material &m) : center(c), radius(r), material(m){};

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
