#ifndef Vectors_h
#define Vectors_h

#include <cmath>
#include <vector>
#include <iostream>

class Vec3f
{
public:
    float x;
    float y;
    float z;

    Vec3f() : x(0), y(0), z(0) {}
    Vec3f(float xx) : x(xx), y(xx), z(xx) {}
    Vec3f(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
    Vec3f operator*(const float &r) const { return Vec3f(x * r, y * r, z * r); }
    Vec3f operator*(const Vec3f &v) const { return Vec3f(x * v.x, y * v.y, z * v.z); }
    Vec3f operator-(const Vec3f &v) const { return Vec3f(x - v.x, y - v.y, z - v.z); }
    Vec3f operator+(const Vec3f &v) const { return Vec3f(x + v.x, y + v.y, z + v.z); }
    Vec3f operator-() const { return Vec3f(-x, -y, -z); }
    Vec3f &operator+=(const Vec3f &v)
    {
        x += v.x, y += v.y, z += v.z;
        return *this;
    }
    /* friend Vec3f operator*(const float &r, const Vec3f &v)
    {
        return Vec3f(v.x * r, v.y * r, v.z * r);
    } */
    friend std::ostream &operator<<(std::ostream &os, const Vec3f &v)
    {
        return os << v.x << ", " << v.y << ", " << v.z;
    }
    
};

#endif
