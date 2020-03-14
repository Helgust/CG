#ifndef Lights_h
#define Lights_h

#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include <random>

#include "vectors.h"
#include "functions.h"


class Light
{
    public:
    Light() {}
    virtual ~Light() {}
    virtual void get_LightData(const Vec3f &, Vec3f &, Vec3f &, float &) const = 0;
    
    
};

class PointLight : public Light
{
    public:
    Vec3f position;
    float intensity;
    Vec3f color;
    PointLight(const Vec3f &p, const float &i, const Vec3f &c) : position(p), intensity(i), color(c) {}
    void get_LightData(const Vec3f &P, Vec3f &lightDir, Vec3f &lightIntensity, float &distance) const
    {
        lightDir = (position - P); 
        float r2 = norma(lightDir); 
        distance = sqrt(r2); 
        //lightDir.x /= distance, lightDir.y /= distance, lightDir.z /= distance;
        lightDir = normalize(lightDir);
        // avoid division by 0
        //lightIntensity = color * intensity / (4 * M_PI * r2); 
        lightIntensity = color * intensity; 
    }

};

class AmbientLight : public Light
{   
    private:

    public:
    float intensity;
    Vec3f color;
    AmbientLight( const float &i, const Vec3f &c) : intensity(i), color(c) {}
    void get_LightData(const Vec3f &P, Vec3f &lightDir, Vec3f &lightIntensity, float &distance) const
    {
        lightDir = Vec3f(0,0,0);
        lightIntensity = color * intensity;
        distance = std::numeric_limits<float>::max(); 
    }


};

class DirectLight : public Light
{    

    public:
    Vec3f dir;
    float intensity;
    Vec3f color;
    DirectLight( const Vec3f &d,const float &i, const Vec3f &c) : dir(d), intensity(i), color(c) {}
    void get_LightData(const Vec3f &P, Vec3f &lightDir, Vec3f &lightIntensity, float &distance) const
    {
        lightDir = dir;
        lightIntensity = color * intensity;
        distance = std::numeric_limits<float>::max(); 
    }


};


#endif