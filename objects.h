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

class Object 
{ 
 public: 
    Object() {} 
    virtual ~Object() {}
    virtual bool intersection(const Vec3f &, const Vec3f &, float &) const=0; 
    virtual void getData(const Vec3f &, Vec3f &, Material &) const=0; 
}; 

class Sphere : public Object
{

    public:
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

    void getData(
        const Vec3f &hit_point, 
        Vec3f &N,
        Material &mat ) const
    {
        N = normalize(hit_point - center); 
        mat = material;
    }

    Vec3f center;
    float radius;
    Material material;

};

class Cylinder : public Object 
{
    public:
	Vec3f center;
	Vec3f direction;
	float radius;
	float height;
    Material material;

    bool intersection(const Vec3f &orig, const Vec3f &dir, float &tnear) const
    {
		
        const double SELF_AVOID_T = 1e-2;


        Vec3f L = orig - center;

		const float a = 1 - dotProduct(dir,dir)*dotProduct(dir,dir);
		//const float b = 2 * (rel_origin.dot(ray.direction) - directions_dot *rel_origin.dot(direction));
        const float b = 2 * ( dotProduct(L,dir)-dotProduct(dir,dir)*dotProduct(L,dir));
		//const float c = rel_origin.dot(rel_origin) -rel_origin.dot(direction)* rel_origin.dot(direction) - radius * radius;
        const float c = dotProduct(L,L) - dotProduct(L,dir) * dotProduct(L,dir) - radius * radius ;
		double delta = b * b - 4 * a * c;

		if (delta < 0) { 
			tnear = std::numeric_limits<float>::max(); // no intersection, at 'infinity'
			return false;
		}

		const float sqrt_delta_2a = sqrt(delta) / (2 * a);
		float t1 = (-b) / (2*a);
		const float t2 = t1 + sqrt_delta_2a;
		t1 -= sqrt_delta_2a;

		if (t2 < SELF_AVOID_T) { // the cylinder is behind us
			tnear = std::numeric_limits<float>::max(); // no intersection, at 'infinity'
			return false;
		}
        float center_proj = dotProduct(center,dir);
        float t1_proj =  dotProduct(dir,(orig + dir*t1));
		if (t1 >= SELF_AVOID_T && t1_proj > center_proj && t1_proj < center_proj+height) {
			tnear = t1;
			return true;
		}
        float  t2_proj =  dotProduct(dir,(orig + dir*t2));
		if (t2 >= SELF_AVOID_T && t2_proj > center_proj && t2_proj < center_proj+height) {
			tnear = t2;
			return true;
		}
		tnear = std::numeric_limits<float>::max(); // no intersection, at 'infinity'
		return false;
	}

    void getData(
        const Vec3f &hit_point, 
        Vec3f &N,
        Material &mat ) const
    {
        N = normalize(hit_point - center); 
        mat = material;
    }



};

#endif
