#include <iostream>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>

#include <string>
#include <vector>
#include <unordered_map>

#include "Bitmap.h"
#include "vectors.h"
#include "objects.h"

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




  Vec3f cast_ray(const Vec3f &origin, const Vec3f &dir, Sphere &sphere, const Settings &settings)
  {

    float sphere_dist = std::numeric_limits<float>::max();
    if (!sphere.intersection(origin, dir, sphere_dist))
    {
      return settings.backgroundColor;
    }
    else
    {
       return Vec3f(1, 0.4, 0.3);
    }
    
  }


int main(int argc, const char **argv)
{
  std::unordered_map<std::string, std::string> cmdLineParams;

  for (int i = 0; i < argc; i++)
  {
    std::string key(argv[i]);

    if (key.size() > 0 && key[0] == '-')
    {
      if (i != argc - 1) // not last argument
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

  settings.width = 1024;
  settings.height = 768;
  settings.fov = 90;
  settings.maxDepth = 4;
  settings.backgroundColor = Vec3f(0.2, 0.7, 0.8);
  

  //std::vector<Sphere> spheres;

  Sphere sphere(Vec3f(-1, 0, -4), 2);

  if (sceneId == 1)
  {
   //spheres.push_back(Sphere(Vec3f(-1, 0, -12), 2));
  }
  else if (sceneId == 2)
  {
  }

  else if (sceneId == 3)
  {
  }

  std::vector<uint32_t> image(settings.height * settings.width);

  float scale = tan(deg2rad(settings.fov * 0.5)); 
  float imageAspectRatio = settings.width / (float)settings.height;


  for (size_t j = 0; j < settings.height; j++) // actual rendering loop
  {
    for (size_t i = 0; i < settings.width; i++)
    {
      float x = (2 * (i + 0.5) / (float)settings.width - 1) * imageAspectRatio * scale; 
      float y = (1 - 2 * (j + 0.5) / (float)settings.height) * scale; 

      Vec3f dir = normalize(Vec3f(x, y, -1));
      Vec3f temp = cast_ray(Vec3f(0,0,0), dir, sphere, settings);;
      float max = std::max(temp.x, std::max(temp.y, temp.z));
      if (max > 1)
        temp = temp * (1. / max);
      image[i + j * settings.width] = (uint32_t)(255 * std::max(0.f, std::min(1.f, temp.z))) << 16 | (uint32_t)(255 * std::max(0.f, std::min(1.f, temp.y))) << 8 | (uint32_t)(255 * std::max(0.f, std::min(1.f, temp.x)));
    }
  }

  SaveBMP(outFilePath.c_str(), image.data(), settings.width, settings.height);

  std::cout << "end." << std::endl;
  return 0;
}