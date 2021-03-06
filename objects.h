#ifndef Objects_h
#define Objects_h

#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include <random>

#include "vectors.h"
#include "functions.h"



enum MaterialType
{
    DIFFUSE,
    REFLECTION_AND_REFRACTION,
    REFLECTION,
    REFRACTION,
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


class Triangle : public Object
{
    public:
    Vec3f v0;
    Vec3f v1;
    Vec3f v2;
    Material material;

    Triangle (const Vec3f &a,const Vec3f &b,const Vec3f &c, const Material &m) : v0(a),v1(b),v2(c),material(m){}

    bool intersection(const Vec3f &orig, const Vec3f &dir, float &tnear) const
    {
        float a = v0.x - v1.x , b = v0.x - v2.x , c = dir.x , d = v0.x - orig.x;
        float e = v0.y - v1.y , f = v0.y - v2.y , g = dir.y , h = v0.y - orig.y;
        float i = v0.z - v1.z , j = v0.z - v2.z , k = dir.z , l = v0.z - orig.z;

        float m = f * k - g * j, n = h * k - g * l, p = f * l - h * j;
        float q = g * i - e * k, s = e * j - f * i;

        float inv_denom = 1.0 / ( a * m + b * q + c * s);

        float e1 = d * m - b * n - c * p;
        float beta = e1 * inv_denom;

        if(beta < 0.0)
            return false;
        float r = e * l - h * i;
        float e2 = a * n + d * q + c * r;
        float gamma = e2 * inv_denom;

        if(gamma < 0.0)
            return false;
        if(beta + gamma > 1.0)
            return false;
        float e3 = a * p - b * r + d * s;
        float t = e3 * inv_denom;

        if (t < 1e-9)
            return false;

        tnear =t;

        return true; 

    }  


    void getData(
        const Vec3f &hit_point,
        Vec3f &N,
        Material &mat) const
    {
        N = crossProduct((v1 - v0),(v2 - v0));
        N = -normalize(N);
        //N = Vec3f(0,1,0);
        mat = material;
    }
};


class Cone : public Object
{
public:
    Vec3f center;
    float radius;
    float height;
    Material material;

    Cone(const Vec3f &c, const float &r, const float &h, const Material &m) : center(c), radius(r), height(h), material(m){};


    bool intersection(const Vec3f &orig, const Vec3f &dir, float &tnear) const
    {
        float tangent = radius/height;
        
        float a = (dir.x * dir.x) + (dir.z * dir.z) - ((tangent * tangent)* (dir.y*dir.y));
        float b = (2*(orig.x-center.x)*dir.x) + (2*(orig.z - center.z)*dir.z) + (2*tangent*tangent*(height - orig.y + center.y)*dir.y);
        float c = ((orig.x - center.x)*(orig.x - center.x)) + ((orig.z - center.z)*(orig.z - center.z)) - (tangent*tangent*(( height - orig.y + center.y)*( height - orig.y + center.y)));
        float t0, t1;

        if (!solveQuadratic(a, b, c, t0, t1))
        return false;

        if (t0 < 0)
            t0 = t1;
        if (t0 < 0)
        return false;

        float r = orig.y + t0 * dir.y;
        if (!((r >= center.y) and (r <= center.y + height)))
        return false;

        tnear = t0;

        return true;
    }

    void getData(
        const Vec3f &hit_point,
        Vec3f &N,
        Material &mat) const
    {
       float r = sqrt((hit_point.x-center.x)*(hit_point.x-center.x) + (hit_point.z-center.z)*(hit_point.z-center.z));
        N = normalize(Vec3f (hit_point.x-center.x, r*(radius/height), hit_point.z-center.z));
        mat = material;
    }


};

class Cylinder : public Object
{
public:
    Vec3f center;
   // Vec3f dir_cyl; TODO will be pretty hard
    float radius;
    float height;
    Material material;

    Cylinder(const Vec3f &c, const float &r, const float &h, const Material &m) : center(c), radius(r), height(h), material(m){};

    bool intersection(const Vec3f &orig, const Vec3f &dir, float &tnear) const
    {
        float a = (dir.x * dir.x) + (dir.z * dir.z);
        float b = 2 * (dir.x * (orig.x - center.x) + dir.z * (orig.z - center.z));
        float c = (orig.x - center.x) * (orig.x - center.x) + (orig.z - center.z) * (orig.z - center.z) - (radius * radius);

        float t0, t1;
        Vec3f p_top = Vec3f(center.x, center.y + height, center.z);

        if (!solveQuadratic(a, b, c, t0, t1))
        {
            if (intersectCylinderCapsTop(Vec3f(0, 1, 0), p_top, orig, dir, tnear))
            {
                return intersectCylinderCapsTop(Vec3f(0, 1, 0), p_top, orig, dir, tnear);
            }
            else
            {
                return (intersectCylinderCapsBottom(Vec3f(0, 1, 0), center, orig, dir, tnear));
            }
        }
        //return false;

        if (t0 < 0)
            t0 = t1;
        if (t0 < 0)
        {
            if (intersectCylinderCapsTop(Vec3f(0, 1, 0), p_top, orig, dir, tnear))
            {
                return intersectCylinderCapsTop(Vec3f(0, 1, 0), p_top, orig, dir, tnear);
            }
            else
            {
                return (intersectCylinderCapsBottom(Vec3f(0, 1, 0), center, orig, dir, tnear));
            }
        }
        //return false;

        float r = orig.y + t0 * dir.y;
        if (!((r >= center.y) and (r <= center.y + height)))
        {
            if (intersectCylinderCapsTop(Vec3f(0, 1, 0), p_top, orig, dir, tnear))
            {
                return intersectCylinderCapsTop(Vec3f(0, 1, 0), p_top, orig, dir, tnear);
            }
            else
            {
                return (intersectCylinderCapsBottom(Vec3f(0, 1, 0), center, orig, dir, tnear));
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
        Vec3f top = Vec3f(center.x, center.y + height, center.z);
        Vec3f axis = normalize(top - center);
        float d1 = -dotProduct(axis, top);
        if (fabs(dotProduct(hit_point, axis) + d1) <= 1e-9)
        {
            N = axis;
        }

        float d2 = -dotProduct(-axis, top);
        if (fabs(dotProduct(hit_point, -axis) + d2) <= 1e-9)
        {
            N = -axis;
        }

        N = Vec3f(hit_point.x - center.x, 0, hit_point.z - center.z);
        N = normalize (N);
        /* Vec3f to_center = hit_point - center; 
        N = normalize((to_center-(Vec3f(0,1,0)*dotProduct(to_center,Vec3f(0,1,0))))); */
        mat = material;
    }

    bool intersectCylinderCapsTop(const Vec3f &n, const Vec3f &p0, const Vec3f &l0, const Vec3f &l, float &t) const
    {
        float reserver = t;
        Vec3f p1 = Vec3f(center.x, center.y + height, center.z);
        if (!(intersectPlane((Vec3f(0, -1, 0)), p0, l0, l, t)))
        {
            return false;
        }
        Vec3f p = l0 + l * t;
        Vec3f v = p - p1;
        float d2 = dotProduct(v, v);
        if ((sqrtf(d2) <= radius))
        {
            return true;
        }
        return false;
    }
    bool intersectCylinderCapsBottom(const Vec3f &n, const Vec3f &p0, const Vec3f &l0, const Vec3f &l, float &t) const
    {
        float reserver = t;
        Vec3f p1 = Vec3f(center.x, center.y, center.z);
        if (!(intersectPlane((Vec3f(0, 1, 0)), p0, l0, l, t)))
        {
            return false;
        }
        Vec3f p = l0 + l * t;
        Vec3f v = p - p1;
        float d2 = dotProduct(v, v);
        if ((sqrtf(d2) <= radius))
        {
            return true;
        }

        return false;
    }
};


class Plane : public Object
{
    public:
    Vec3f v0;
    Vec3f n;
    Material material;

    Plane (const Vec3f &a, const Vec3f &nn ,const Material &m) : v0(a),n(nn),material(m){}

    bool intersection(const Vec3f &orig, const Vec3f &dir, float &tnear) const
    {
        float t = dotProduct((v0 - orig) , n) / dotProduct(dir, n); 
														
        if (t < 1e-9)
        {
            return false;	
        }

        tnear = t;
        return true;
	
    }  


    void getData(
        const Vec3f &hit_point,
        Vec3f &N,
        Material &mat) const
    {
        N = normalize(n);

        if ( dotProduct(N,Vec3f(0,0,-1))) // plane at XY
        {
            mat = material;
            mat.diffuse_color = (int(.85 * hit_point.x + 1000)) & 1 ? Vec3f(1, 1, 1) : Vec3f(0, 0, 0);
            mat.diffuse_color = mat.diffuse_color*0.7;
        }
        else
        {
            mat = material;
            mat.diffuse_color = (int(.25 * hit_point.x + 1000) + int(.25 * hit_point.z)) & 1 ? Vec3f(1, 1, 1) : Vec3f(0, 0, 0);
            mat.diffuse_color = mat.diffuse_color*0.5;
        }
        
        
    }
};

#endif
