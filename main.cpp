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

/*   float checkerboard_dist = std::numeric_limits<float>::max();

  if (fabs(dir.y) > 1e-3)
  {
    float d = -(orig.y + 4) / dir.y; // the checkerboard plane has equation y = -4
    Vec3f pt = orig + dir * d;
    if (d > 0 && d < spheres_dist)
    {
      checkerboard_dist = d;
      hit = pt;
      N = Vec3f(0, 1, 0);
      material.diffuse_color = (int(0.25 * hit.x + 10000) + int(0.25 * hit.z)) & 1 ? Vec3f(1, 1, 1) : Vec3f(0, 0.0, 0.0);
      material.diffuse_color = material.diffuse_color * 0.3;
    }
  }
  return std::min(spheres_dist, checkerboard_dist) < 1000; */
  return spheres_dist < 1000;
}

Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, const Settings &settings)
{
  Vec3f point, N;
  Material material;

  if (!scene_intersect(orig, dir, spheres, point, N, material))
  {
    return settings.backgroundColor; // background color
  }

  float diffuse_light_intensity = 0;
  float specular_light_intensity = 0;
  for (size_t i = 0; i < lights.size(); i++)
  {
    Vec3f light_dir = normalize(lights[i].position - point);

    float light_distance = norma(lights[i].position - point);

    Vec3f shadow_orig = dotProduct(light_dir,N) < 0 ? point - N * 1e-3 : point + N * 1e-3; // checking if the point lies in the shadow of the lights[i]
    Vec3f shadow_pt, shadow_N;
    Material tmpmaterial;

    if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && norma(shadow_pt - shadow_orig)< light_distance)
      continue;
    
    diffuse_light_intensity += lights[i].intensity * std::max(0.f, dotProduct(light_dir, N));
    Vec3f R = reflect(light_dir, N);
    specular_light_intensity += lights[i].intensity * powf(std::max(0.f, dotProduct(R, -dir)), material.specular);
  }
  //return material.diffuse_color * diffuse_light_intensity * material.albedo.x + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo.y;
   return Vec3f(1., 1., 1.)*specular_light_intensity*material.albedo.y;
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

  Material orange(Vec3f(1, 0.4, 0.3), Vec2f(0.6, 0.3), 50.0);
  Material ivory(Vec3f(0.4, 0.4, 0.3), Vec2f(0.9, 0.1), 20.0);

  std::vector<Sphere> spheres;
  std::vector<Light> lights;

  if (sceneId == 1)
  {
    spheres.push_back(Sphere(Vec3f(-3, -6, -11), 3, ivory));
    spheres.push_back(Sphere(Vec3f(0, 0, -12), 2, orange));

    lights.push_back(Light(Vec3f(5, 20, 2), 0.7));
    lights.push_back(Light(Vec3f(-5, 20, 4), 1.0));
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
      Vec3f temp = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights, settings);
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