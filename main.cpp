#include <iostream>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>
#include <ctime>

#include <string>
#include <vector>
#include <unordered_map>

#include "Bitmap.h"
#include "vectors.h"
#include "objects.h"
#include "functions.h"

const uint32_t RED = 0x000000FF;
const uint32_t GREEN = 0x0000FF00;
const uint32_t BLUE = 0x00FF0000;

struct Settings
{
  int width;
  int height;
  float fov;
  int maxDepth;
  Vec3f backgroundColor;
  float bias;
};

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material)
{
  float spheres_dist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < spheres.size(); i++)
  {
    float dist_i;
    if (spheres[i].intersection(orig, dir, dist_i) && dist_i < spheres_dist)
    {
      spheres_dist = dist_i;
      hit = orig + dir * dist_i;
      N = normalize(hit - spheres[i].center);
      material = spheres[i].material;
    }
  }

  return spheres_dist < 1000;
}


Vec3f newcast_ray(
      const Vec3f &orig, const Vec3f &dir,
      const std::vector<Sphere> &spheres,
      const std::vector<Light> &lights,
      const Settings &settings)
{
  //if (depth > options.maxDepth) return options.backgroundColor; 
    
    Vec3f hit_point, N;
    Material material; 
    Vec3f PhongColor = settings.backgroundColor; 
    if (scene_intersect(orig, dir, spheres, hit_point, N , material)) 
    {
      Vec3f diffuse = 0, specular = 0; 
      Vec3f shadow_orig = (dotProduct(dir, N) < 0) ? hit_point + N * 1e-3 : hit_point - N * 1e-3;

      for (uint32_t i = 0; i < lights.size(); ++i) 
      {
        Vec3f light_dir = normalize(lights[i].position - hit_point);
        float light_dist = norma(lights[i].position - hit_point);
        Vec3f shadow_point, shadow_N;
        Material temp_material;
        bool inShadow = scene_intersect(shadow_orig, light_dir, spheres, shadow_point, shadow_N,temp_material) && 
                        norma(shadow_point-shadow_orig) < light_dist;
        diffuse += (1 - inShadow) * lights[i].intensity * std::max(0.f, dotProduct(light_dir, N));
        Vec3f R = reflect(-light_dir, N);
        specular += powf(std::max(0.f, -dotProduct(R, dir)), material.specular) * lights[i].intensity;
      }
      PhongColor = diffuse * material.diffuse_color * 0.8 + specular*0.2; //Kd = 0.8 Ks = 0.2 
      //PhongColor = specular*0.2; //Kd = 0.8 Ks = 0.2 
    }
    else
    {
      PhongColor = settings.backgroundColor;
    }
    
      return PhongColor; 
}

int main(int argc, const char **argv)
{
  clock_t begin = clock();
  std::unordered_map<std::string, std::string> cmdLineParams;

  for (int i = 0; i < argc; i++)
  {
    std::string key(argv[i]);

    if (key.size() > 0 && key[0] == '-')
    {
      if (i != argc - 1) // not last argumen
      {
        cmdLineParams[key] = argv[i + 1];
        i++;
      }
      else
        cmdLineParams[key] = "";
    }
  }

  std::string outFilePath = "zout.bmp";
  if (cmdLineParams.find("-out") != cmdLineParams.end())
    outFilePath = cmdLineParams["-out"];

  int sceneId = 0;
  if (cmdLineParams.find("-scene") != cmdLineParams.end())
    sceneId = atoi(cmdLineParams["-scene"].c_str());

  Settings settings;

   /* settings.width = 16;
  settings.height = 16; */ 

   settings.width = 1920;
  settings.height = 1080;

  settings.fov = 90;
  settings.maxDepth = 4;
  settings.backgroundColor = Vec3f(0, 0, 0); // light blue Vec3f(0.2, 0.7, 0.8);

  Material orange(Vec3f(1, 0.4, 0.3), Vec3f(0.18, 0.18, 0.18), 10.0);
  Material ivory(Vec3f(0.4, 0.4, 0.3), Vec3f(1, 1, 1), 2.0);

  std::vector<Sphere> spheres;
  std::vector<Light> lights;

  if (sceneId == 1)
  {
    spheres.push_back(Sphere(Vec3f(0, -3, -12), 4, ivory));
    spheres.push_back(Sphere(Vec3f(0, 5, -12), 2, orange));
    spheres.push_back(Sphere(Vec3f(-5, -3, -12), 2, orange));

     lights.push_back(Light(Vec3f(-20, 70, 20), 0.5));
     lights.push_back(Light(Vec3f(30, 50, -12), 1));
    // lights.push_back(Light(Vec3f(0, 0, 200), 3)); 
     
  }

  else if (sceneId == 2)
  {
    spheres.push_back(Sphere(Vec3f(-5, 0, -5), 3, ivory));
    spheres.push_back(Sphere(Vec3f(4, 2, -4), 2, orange));

    lights.push_back(Light(Vec3f(1, 3, -4), 0.5));
    lights.push_back(Light(Vec3f(0, 5, -6), 1));
  }

  else if (sceneId == 3)
  {
    spheres.push_back(Sphere(Vec3f( 7,    5,   -18), 4,     ivory)); 
    spheres.push_back(Sphere(Vec3f(-3,    0,   -20), 6,      ivory));
    spheres.push_back(Sphere(Vec3f(-3, 6, -12), 2,      orange));
    spheres.push_back(Sphere(Vec3f( 15, -2, -18), 3, orange));

    lights.push_back(Light(Vec3f(-20, 20,  20), 0.8));
    lights.push_back(Light(Vec3f( 30, 50, -25), 1.2));
    lights.push_back(Light(Vec3f( 30, 20,  30), 2.2));
  }

  std::vector<uint32_t> image(settings.height * settings.width);

  float scale = tan(deg2rad(settings.fov * 0.5));
  float imageAspectRatio = settings.width / (float)settings.height;

  for (size_t j = 0; j < settings.height; j++) // actual rendering loop
  {
    for (size_t i = 0; i < settings.width; i++)
    {
      float x = (2 * (i + 0.5) / (float)settings.width - 1) * imageAspectRatio * scale ;
      float y = (1 - 2 * (j + 0.5) / (float)settings.height) * scale ;

      Vec3f dir = normalize(Vec3f(x, -y, -1));
      Vec3f temp = newcast_ray(Vec3f(0, 0, 0), dir, spheres, lights, settings);
      ;
      float max = std::max(temp.x, std::max(temp.y, temp.z));
      if (max > 1)
        temp = temp * (1. / max);
      image[i + j * settings.width] = (uint32_t)(255 * std::max(0.f, std::min(1.f, temp.z))) << 16 | (uint32_t)(255 * std::max(0.f, std::min(1.f, temp.y))) << 8 | (uint32_t)(255 * std::max(0.f, std::min(1.f, temp.x)));
    }
  }

  SaveBMP(outFilePath.c_str(), image.data(), settings.width, settings.height);

  std::cout << "end." << std::endl;
  clock_t end = clock();
  double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
  std::cout << "runtime= " << time_spent << std::endl;
  return 0;
}