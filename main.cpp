#include <iostream>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>

#include <string>
#include <vector>
#include <unordered_map>

#include "Bitmap.h"
#include "geometry.h"



const uint32_t RED   = 0x000000FF;
const uint32_t GREEN = 0x0000FF00;
const uint32_t BLUE  = 0x00FF0000  ;

const int   width    = 1240;
const int   height   = 796;
const float fov      = M_PI/2.;

struct Material {
    Material(const Vec3f &a, const Vec3f &color, const float &spec) : albedo(a), diffuse_color(color), specular(spec) {}
    Material() : albedo(1,0,0), diffuse_color(), specular() {}
    Vec3f albedo;
    Vec3f diffuse_color;
    float specular;
};

struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};



struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere() {};
    Sphere(const Vec3f &c, const float r,const Material &m) : center(c), radius(r),material(m) {}

    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
        Vec3f L = center - orig;
        float tca = L*dir;
        float d2 = L*L - tca*tca;
        if (d2 > radius*radius) return false;
        float thc = sqrtf(radius*radius - d2);
        t0       = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
};


Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N*2.f*(I*N);
}

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i=0; i < spheres.size(); i++) {
        float dist_i;
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit = orig + dir*dist_i;
            N = (hit - spheres[i].center).normalize();
            material = spheres[i].material;
        }
    }
    return spheres_dist<1000;
}


Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres,const std::vector<Light> &lights, size_t depth=0) 
{
    Vec3f point, N;
    Material material;

        if (depth > 4 || !scene_intersect(orig, dir, spheres, point, N, material)) 
        {
        //return 0x24C5CC00; // background color
        return Vec3f(0.2, 0.7, 0.8); // background color
        }

    Vec3f reflect_dir = reflect(dir, N).normalize();
     Vec3f reflect_orig;
    
    if (reflect_dir*N <0) // offset the original point to avoid occlusion by the object itself
    {
        reflect_orig = point - N*1e-3;
    }
    else
    {
        reflect_orig = point + N*1e-3;
    } 
    
    Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);
    
    
    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for (size_t i=0; i<lights.size(); i++) {
        Vec3f light_dir      = (lights[i].position - point).normalize();
        float light_distance = (lights[i].position - point).norm();
        
        
         Vec3f shadow_orig;
        if (light_dir*N < 0) // checking if the point lies in the shadow of the lights[i]
        {
          shadow_orig = point - N*1e-3;
        } 
        else
        {
          shadow_orig = point + N*1e-3;
        }

        Vec3f shadow_pt, shadow_N;
        Material tmpmaterial;

        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt-shadow_orig).norm() < light_distance)
            continue;

        diffuse_light_intensity  += lights[i].intensity * std::max(0.f, light_dir*N);
        specular_light_intensity += powf(std::max(0.f, -reflect(-light_dir, N)*dir), material.specular)*lights[i].intensity;
    }
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1] + reflect_color*material.albedo[2];
    //return material.diffuse_color;
}



int main(int argc, const char** argv)
{
  std::unordered_map<std::string, std::string> cmdLineParams;

  for(int i=0; i<argc; i++)
  {
    std::string key(argv[i]);

    if(key.size() > 0 && key[0]=='-')
    {
      if(i != argc-1) // not last argument
      {
        cmdLineParams[key] = argv[i+1];
        i++;
      }
      else
        cmdLineParams[key] = "";
    }
  }

  std::string outFilePath = "zout.bmp";
  if(cmdLineParams.find("-out") != cmdLineParams.end())
    outFilePath = cmdLineParams["-out"];

  int sceneId = 0;
  if(cmdLineParams.find("-scene") != cmdLineParams.end())
    sceneId = atoi(cmdLineParams["-scene"].c_str());


    Material      ivory(Vec3f(0.6,  0.3, 0.1), Vec3f(0.4, 0.4, 0.3),   50.);
    Material red_rubber(Vec3f(0.9,  0.1, 0.0), Vec3f(0.3, 0.1, 0.1),   10.);
    Material     mirror(Vec3f(0.0, 10.0, 0.8), Vec3f(1.0, 1.0, 1.0), 1425.);

    // Material      ivory(0x00CDE5FC);
    // Material red_rubber(0x000E1A74);

    std::vector<Sphere> spheres;
    std::vector<Light> lights;

   if(sceneId == 1)
    {
      spheres.push_back(Sphere(Vec3f(-3,    0,   -16), 2,      ivory));
      spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2,      mirror));
      spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 3, red_rubber));
      spheres.push_back(Sphere(Vec3f( 7,    5,   -18), 4,     mirror));


      lights.push_back(Light(Vec3f(-20, 20,  20), 1.5));
      lights.push_back(Light(Vec3f( 30, 50, -25), 1.8));
      lights.push_back(Light(Vec3f( 30, 20,  30), 1.7));
    }
   else if(sceneId == 2)
   {
     //spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 5, red_rubber));
     spheres.push_back(Sphere(Vec3f( 7,    5,   -18), 4,      ivory));

     lights.push_back(Light(Vec3f(-20, 20,  20), 1.5));
   }
      
   else if(sceneId == 3)
   {
     spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, ivory));
   }
     
    std::vector<uint32_t> image(height*width); 



  for (size_t j = 0; j<height; j++) // actual rendering loop
    {   for (size_t i = 0; i<width; i++) 
        {
            float dir_x =  (i + 0.5) -  width/2.;
            float dir_y = (j + 0.5) - height/2.;    // this flips the image at the same time
            float dir_z = -height/(2.*tan(fov/2.));
            
            Vec3f temp = cast_ray(Vec3f(0,0,0), Vec3f(dir_x, dir_y, dir_z).normalize(),spheres,lights);
            //std:: cout <<std::showbase << std:: hex << (255*temp[0]) << " " << (255*temp[1]) << " "<< (255*temp[2])  << std:: endl;
          
            float max = std::max(temp[0], std::max(temp[1], temp[2]));
            if (max>1) temp = temp*(1./max);
              image[i+j*width] = (uint32_t)(255 * std::max(0.f, std::min(1.f, temp[2])))<<16 
                                 | (uint32_t)(255 * std::max(0.f, std::min(1.f, temp[1])))<<8
                                 | (uint32_t)(255 * std::max(0.f, std::min(1.f, temp[0])));
          
        }
    }
  

  SaveBMP(outFilePath.c_str(), image.data(), width, height);

  std::cout << "end." << std::endl;
  return 0;
}