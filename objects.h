#ifndef Objects_h
#define Objects_h

#include <cmath>
#include <limits>
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

struct Light
{
    Light(const Vec3f &p, const float &i, const Vec3f &c) : position(p), intensity(i), color(c) {}
    Vec3f position;
    float intensity;
    Vec3f color;
};

enum MaterialType
{
    DIFFUSE,
    REFLECTION_AND_REFRACTION,
    REFLECTION,
    REFRACTION,
    DIFFUSE_REFLECTION,
    GLOSSY
};

struct Material
{
    Material(const Vec3f &color, const MaterialType &m, const float &s, const float &r) : diffuse_color(color), materialType(m), specular(s), refract(r) {}
    Material() : diffuse_color(), materialType(DIFFUSE), specular(), refract() {}
    Vec3f diffuse_color;
    MaterialType materialType;
    float specular;
    float refract;
};

class Object
{
public:
    Object() {}
    virtual ~Object() {}
    virtual bool intersection(const Vec3f &, const Vec3f &, float &) const = 0;
    virtual void getData(const Vec3f &, Vec3f &, Material &) const = 0;
};

class Sphere : public Object
{
public:
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
        float c = dotProduct(L, L) - (radius * radius);
        float t0, t1;
        if (!solveQuadratic(a, b, c, t0, t1))
            return false;
        if (t0 < 0)
            t0 = t1;
        if (t0 < 0)
            return false;
        tnear = t0;

        return true;
    }

    void getData(
        const Vec3f &hit_point,
        Vec3f &N,
        Material &mat) const
    {
        N = normalize(hit_point - center);
        mat = material;
    }
};

class Cylinder : public Object
{
public:
    Vec3f center;
    Vec3f dir_cyl;
    float radius;
    float height;
    Material material;

    Cylinder(const Vec3f &c, const float &r, const float &h, const Material &m) : center(c), radius(r), height(h), material(m){};

    bool intersection(const Vec3f &orig, const Vec3f &dir, float &tnear) const
    {

        /* float reserv = tnear;
        Vec3f p0 = Vec3f(center.x,center.y+height,center.z);
        if (!(intersectPlane((Vec3f(0,-1,0)), p0, orig, dir, tnear))) { 
            return false;
        } 
         Vec3f p = orig + dir * tnear; 
         Vec3f v = p - p0; 
         float d2 = dotProduct(v, v);
         if((sqrtf(d2)<=radius))
         {
             return true;
         }

         //tnear = reserv;
        p0 = Vec3f(center.x,center.y,center.z);
        if (!(intersectPlane((Vec3f(0,-1,0)), p0, orig, dir, tnear))) { 
            return false;
        } 
            p = orig + dir * tnear;
            v = p - p0;
            d2 = dotProduct(v, v);
         if((sqrtf(d2)<=radius))
         {
             return true;
         }

         tnear = reserv;  */






        float a = (dir.x * dir.x) + (dir.z * dir.z);
        float b = 2 * (dir.x * (orig.x - center.x) + dir.z * (orig.z - center.z));
        float c = (orig.x - center.x) * (orig.x - center.x) + (orig.z - center.z) * (orig.z - center.z) - (radius * radius);

        float t0,t1;
        Vec3f p_top = Vec3f(center.x,center.y+height,center.z);

        if (!solveQuadratic(a, b, c, t0, t1))
        {
            if(intersectCylinderCapsTop(Vec3f(0,1,0),p_top,orig,dir,tnear))
            {
                return intersectCylinderCapsTop(Vec3f(0,1,0),p_top,orig,dir,tnear);
            }
            else
            {
                return (intersectCylinderCapsBottom(Vec3f(0,1,0),center,orig,dir,tnear));
            }
            
            
        }
            //return false;

        if (t0 < 0)
            t0 = t1;
        if (t0 < 0)
        {
            if(intersectCylinderCapsTop(Vec3f(0,1,0),p_top,orig,dir,tnear))
            {
                return intersectCylinderCapsTop(Vec3f(0,1,0),p_top,orig,dir,tnear);
            }
            else
            {
                return (intersectCylinderCapsBottom(Vec3f(0,1,0),center,orig,dir,tnear));
            }
        }
            //return false;
        
        float r = orig.y + t0*dir.y;
        if (!((r >= center.y) and (r <= center.y + height)))
        {
            if(intersectCylinderCapsTop(Vec3f(0,1,0),p_top,orig,dir,tnear))
            {
                return intersectCylinderCapsTop(Vec3f(0,1,0),p_top,orig,dir,tnear);
            }
            else
            {
                return (intersectCylinderCapsBottom(Vec3f(0,1,0),center,orig,dir,tnear));
            }
        }
        //return false;
        
        tnear = t0;

        return true; 
        
            
    }

    void getData(
        const Vec3f &hit_point,
        Vec3f &N,
        Material &mat) const
    {
        Vec3f top = Vec3f(center.x,center.y+height,center.z);
        Vec3f axis = normalize(top-center);
        float d1 = -dotProduct(axis,top);
        if(fabs(dotProduct(hit_point,axis) + d1) <= 1e-9)
        {
            N=axis;
        }

        float d2 = -dotProduct(-axis,top);
        if(fabs(dotProduct(hit_point,-axis) + d2) <= 1e-9)
        {
            N=-axis;
        }

        N = Vec3f(hit_point.x-center.x ,0,hit_point.z-center.z);
        /* Vec3f to_center = hit_point - center; 
        N = normalize((to_center-(Vec3f(0,1,0)*dotProduct(to_center,Vec3f(0,1,0))))); */
        mat = material;
    }

    bool intersectCylinderCapsTop(const Vec3f &n, const Vec3f &p0, const Vec3f &l0, const Vec3f &l, float &t)const
    {
            float reserver = t;
            Vec3f p1 = Vec3f(center.x,center.y+height,center.z);
            if (!(intersectPlane((Vec3f(0,-1,0)), p0, l0, l, t))) { 
                return false;
            } 
            Vec3f p = l0 + l * t; 
            Vec3f v = p - p1; 
            float d2 = dotProduct(v, v);
            if((sqrtf(d2)<=radius))
            {
                return true;
            }
            return false;
    }
    bool intersectCylinderCapsBottom(const Vec3f &n, const Vec3f &p0, const Vec3f &l0, const Vec3f &l, float &t) const
    {
            float reserver = t;
            Vec3f p1 = Vec3f(center.x,center.y,center.z);
            if (!(intersectPlane((Vec3f(0,1,0)), p0, l0, l, t))) { 
                return false;
            } 
                Vec3f p = l0 + l * t;
                Vec3f v = p - p1;
                float d2 = dotProduct(v, v);
            if((sqrtf(d2)<=radius))
            {
                return true;
            }

            return false;
 
    }
};

#endif
