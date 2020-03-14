#define _USE_MATH_DEFINES
#include <iostream>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>
#include <ctime>

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

#include "Bitmap.h"
#include "vectors.h"
#include "objects.h"
#include "lights.h"
#include "functions.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb/stb_image.h"

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
  float Kd;
  float Ks;
  float Kg;
  float AA;
  int envmap_ineed;
  int envmap_width;
  int envmap_height;
};

bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<std::unique_ptr<Object>> &objects, Vec3f &hit, Vec3f &N, Material &material)
{
  float objects_dist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < objects.size(); i++)
  {
    float dist_i;
    if (objects[i]->intersection(orig, dir, dist_i) && dist_i < objects_dist)
    {
      objects_dist = dist_i;
      hit = orig + dir * dist_i;
      objects[i]->getData(hit, N, material);
    }
  }
  return objects_dist < 1000;
}

Vec3f newcast_ray(
    const Vec3f &orig, const Vec3f &dir,
    const std::vector<std::unique_ptr<Object>> &objects,
    const std::vector<std::unique_ptr<Light>> &lights,
    const std::vector<Vec3f> &envmap,
    const Settings &settings,
    size_t depth = 0)
{

  Vec3f hit_point, N;
  Material material;
  Vec3f PhongColor = 0;

  if ((depth > settings.maxDepth) || (!(scene_intersect(orig, dir, objects, hit_point, N, material))))
  {

     if (settings.envmap_ineed == 0)
    {
      return settings.backgroundColor;
    } 
    Sphere env(Vec3f(0, 0, 0), 1000, Material());
    float dist = 0;
    env.intersection(orig, dir, dist);
    Vec3f p = orig + dir * dist;
    int a = (atan2(p.z, p.x) / (-2 * M_PI) + .5) * settings.envmap_width;
    int b = acos(p.y / 1000) / M_PI * settings.envmap_height;
    return envmap[a + b * settings.envmap_width];
    //return settings.backgroundColor;
  }

  if (scene_intersect(orig, dir, objects, hit_point, N, material))
  {

    switch (material.materialType)
    {

     case GLOSSY:
    {

      float kr;
      fresnel(dir, N, material.refract, kr);
      Vec3f reflect_dir = normalize(reflect(dir, N));
      Vec3f reflect_orig = (dotProduct(reflect_dir, N) < 0) ? hit_point - N * 1e-4 : hit_point + N * 1e-4; // offset the original point to avoid occlusion by the object itself
      Vec3f reflect_color = newcast_ray(reflect_orig, reflect_dir, objects, lights, envmap, settings, depth + 1);
     

      Vec3f diffuse = 0, specular = 0;
      Vec3f shadow_orig = (dotProduct(dir, N) < 0) ? hit_point + N * 1e-4 : hit_point - N * 1e-4;
      for (uint32_t i = 0; i < lights.size(); ++i)
      {
        Vec3f shadow_orig = (dotProduct(dir, N) < 0) ? hit_point + N * 1e-4 : hit_point - N * 1e-4;
        Vec3f light_dir, light_intensity;
        Vec3f shadow_pt, shadow_N;
        float light_dist;
        
        Material tmp_mat;


        lights[i]->get_LightData(hit_point, light_dir, light_intensity, light_dist);

        if (scene_intersect(shadow_orig, light_dir, objects, shadow_pt, shadow_N, tmp_mat) && norma(shadow_pt-shadow_orig) < light_dist)
            continue;
        Vec3f reflectionDirection = reflect(-light_dir, N);
        diffuse  += light_intensity * std::max(0.f, dotProduct(light_dir,N));
        specular += light_intensity * powf(std::max(0.f, -dotProduct(reflectionDirection, dir)), material.specular) ;
      }
      PhongColor = diffuse*material.diffuse_color*settings.Kd +
                   material.diffuse_color*specular +
                    material.diffuse_color*reflect_color*settings.Kg;

      //PhongColor = specular*0.2;
      break;
    }

     case REFLECTION_AND_REFRACTION:
    {
      float kr;
      fresnel(dir, N, material.refract, kr);
      Vec3f reflect_dir = normalize(reflect(dir, N));
      Vec3f refract_dir = normalize(refract(dir, N, material.refract));
      Vec3f reflect_orig = (dotProduct(reflect_dir, N) < 0) ? hit_point - N * 1e-4 : hit_point + N * 1e-4; // offset the original point to avoid occlusion by the object itself
      Vec3f refract_orig = (dotProduct(refract_dir, N) < 0) ? hit_point - N * 1e-4 : hit_point + N * 1e-4;
      Vec3f reflect_color = newcast_ray(reflect_orig, reflect_dir, objects, lights, envmap, settings, depth + 1);
      Vec3f refract_color = newcast_ray(refract_orig, refract_dir, objects, lights, envmap, settings, depth + 1);
      PhongColor = reflect_color * kr + refract_color * (1 - kr);
      break;
    }
    

    case REFLECTION:
    {
      Vec3f reflect_dir = normalize(reflect(dir, N));
      Vec3f reflect_orig = (dotProduct(reflect_dir, N) < 0) ? hit_point - N * 1e-4 : hit_point + N * 1e-4; // offset the original point to avoid occlusion by the object itself
      Vec3f reflect_color = newcast_ray(reflect_orig, reflect_dir, objects, lights, envmap, settings, depth + 1);
      PhongColor += reflect_color * 0.8;
      break;
    } 

    

      default:
    {

      Vec3f diffuse = 0, specular = 0;
      Vec3f shadowPointOrig = (dotProduct(dir, N) < 0) ? 
                    hit_point + N * 1e-4 : 
                    hit_point - N * 1e-4; 
      for (uint32_t i = 0; i < lights.size(); ++i)
      {
        Vec3f shadow_orig = (dotProduct(dir, N) < 0) ? hit_point + N * 1e-4 : hit_point - N * 1e-4;
        Vec3f light_dir, light_intensity;
        Vec3f shadow_pt, shadow_N;
        float light_dist;
        
        Material tmp_mat;


        lights[i]->get_LightData(hit_point, light_dir, light_intensity, light_dist);

        if (scene_intersect(shadow_orig, light_dir, objects, shadow_pt, shadow_N, tmp_mat) && norma(shadow_pt-shadow_orig) < light_dist)
            continue;
        Vec3f reflectionDirection = reflect(-light_dir, N);
        diffuse  += light_intensity * std::max(0.f, dotProduct(light_dir,N));
        specular += light_intensity * powf(std::max(0.f, -dotProduct(reflectionDirection, dir)), material.specular) ;
      }
      PhongColor = diffuse*material.diffuse_color * settings.Kd + specular * settings.Ks ; //Kd = 0.8 Ks = 0.2
      //PhongColor = specular*0.2; //Kd = 0.8 Ks = 0.2 albedo[0] + Vec3f(1., 1., 1.)*specular_light_intensity * material.albedo[1] + reflect_color*material.albedo[2];
      break;
      }
    }
  }

  return PhongColor;
}

int main(int argc, const char **argv)
{

  Settings settings;

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

  int threads = 1;
  if (cmdLineParams.find("-threads") != cmdLineParams.end())
    threads = atoi(cmdLineParams["-threads"].c_str());

  //std::string envFilePath = "../envmap4.jpg";
  settings.envmap_ineed = 0;
  if (cmdLineParams.find("-envmap") != cmdLineParams.end())
  {
    settings.envmap_ineed = 1;
    //envFilePath = cmdLineParams["-envmap"];
  }

  std::vector<Vec3f> envmap;

  int n = -1;
  unsigned char *pixmap = stbi_load("../envmap4.jpg", &settings.envmap_width, &settings.envmap_height, &n, 0);
  if (!pixmap || 3 != n)
  {
    std::cerr << "Error: can not load the environment map" << std::endl;
    return -1;
  }
  envmap = std::vector<Vec3f>(settings.envmap_width * settings.envmap_height);
#pragma omp parallel for num_threads(threads)
  for (int j = settings.envmap_height - 1; j >= 0; j--)
  {
    for (int i = 0; i < settings.envmap_width; i++)
    {
      envmap[i + j * settings.envmap_width] = Vec3f(pixmap[(i + j * settings.envmap_width) * 3 + 0], pixmap[(i + j * settings.envmap_width) * 3 + 1], pixmap[(i + j * settings.envmap_width) * 3 + 2]) * (1 / 255.);
    }
  }
  stbi_image_free(pixmap);

  /*    settings.width = 512;
  settings.height = 512; */

/*   settings.width = 1024;
  settings.height = 796; */

/*        settings.width = 1920;
  settings.height = 1080; */
          settings.width = 3840;
  settings.height = 2160;

  settings.fov = 90;
  settings.maxDepth = 6;
  settings.backgroundColor = Vec3f(0.0, 0.0, 0.0); // light blue Vec3f(0.2, 0.7, 0.8);
  settings.AA = 1;
  settings.Kd = 0.8;
  settings.Ks = 0.2;
  settings.Kg = 0.4;

  Material orange(Vec3f(1, 0.4, 0.3), DIFFUSE, 1.0, 1.5);
  Material red(Vec3f(0.40, 0.0, 0.0), GLOSSY, 3.0, 1.5);
  Material green(Vec3f(0.0, 0.40, 0.0), GLOSSY, 5.0, 1.5);
  Material blue(Vec3f(0.0, 0.00, 0.4), GLOSSY, 5.0, 1.5);
  Material ivory(Vec3f(0.4, 0.4, 0.3), DIFFUSE, 5.0, 1.5);
  Material checker(Vec3f(0.0, 0.0, 0.0), GLOSSY, 5.0, 1.5);
  Material gold(Vec3f(0.5, 0.4, 0.1), GLOSSY, 8.0, 1.5);
  Material mirror(Vec3f(0.0, 10.0, 0.8), REFLECTION, 1.0, 1.5);
  Material glass(Vec3f(0.0, 10.0, 0.8), REFLECTION_AND_REFRACTION, 1.0, 1.5);

  std::vector<std::unique_ptr<Object>> objects;
  std::vector<std::unique_ptr<Light>> lights; 

  Vec3f ta = Vec3f(-2, -4, -12);
  Vec3f tb = Vec3f(-5, -4, -16);
  Vec3f tc = Vec3f(1, -4, -16);
  Vec3f top = Vec3f(-2, 2, -14);

  if (sceneId == 1)
  {

    //objects.push_back(std::unique_ptr<Object>(new Cylinder (Vec3f(0,-4, -7), 1 ,4, blue)));
    //objects.push_back(std::unique_ptr<Object>(new Cone (Vec3f(-4,-4, -7), 2 ,2, gold)));
    //objects.push_back(std::unique_ptr<Object>(new Plane (Vec3f(0,-4, 0),Vec3f(0,1,0 ),checker)));

    //objects.push_back(std::unique_ptr<Object>( new Triangle(Vec3f(0,0,0),Vec3f(-20,0,0),Vec3f(0,0,-20), gold)));
    // objects.push_back(std::unique_ptr<Object>(new Triangle(ta, top, tc, orange)));
    // objects.push_back(std::unique_ptr<Object>(new Triangle(ta, tb, top, orange)));
    // objects.push_back(std::unique_ptr<Object>(new Triangle(tc, tb, top, orange)));
    // objects.push_back(std::unique_ptr<Object>(new Triangle(ta, tb, tc, orange)));

    //objects.push_back(std::unique_ptr<Object>( new Sphere(Vec3f(-5, 0, -5), 3, ivory)));

    lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(-20, 20, 20), 1, Vec3f(1, 1, 1))));
    // lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(-30, 20, -25), 1, Vec3f(1, 1, 1))));
    // lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(30, 20, -20), 2.3, Vec3f(1, 1, 1))));
  }

  else if (sceneId == 2)
  {
    objects.push_back(std::unique_ptr<Object>(new Sphere(Vec3f(-5, 0, -5), 3, ivory)));
    objects.push_back(std::unique_ptr<Object>(new Sphere(Vec3f(4, 2, -4), 2, gold)));

    lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(-20, 20, 20), 1, Vec3f(1, 1, 1))));
    // lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(-30, 20, -25), 1, Vec3f(1, 1, 1))));
    // lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(30, 20, -20), 2.3, Vec3f(1, 1, 1))));
  }

  else if (sceneId == 3)
  {
    objects.push_back(std::unique_ptr<Object>( new Sphere(Vec3f(-2, 0, -2), 0.5, red)));
    objects.push_back(std::unique_ptr<Object>(new Sphere(Vec3f(2, 0, -5), 1, glass)));
    objects.push_back(std::unique_ptr<Object>(new Sphere(Vec3f(-3, 4, -20), 3, gold)));
     objects.push_back(std::unique_ptr<Object>(new Sphere(Vec3f(-3, 6, -12), 2, ivory)));
     objects.push_back(std::unique_ptr<Object>(new Sphere(Vec3f(15, -2, -18), 3, orange)));
     objects.push_back(std::unique_ptr<Object>(new Sphere(Vec3f(1.25, 12, -40), 10, mirror)));

    objects.push_back(std::unique_ptr<Object>(new Cylinder (Vec3f(5,0, -15), 1 ,6, red)));
    objects.push_back(std::unique_ptr<Object>(new Cone (Vec3f(-4,-4, -7), 1 ,2, ivory)));
    objects.push_back(std::unique_ptr<Object>(new Plane (Vec3f(0,-4, 0),Vec3f(0,1,0 ),checker)));

    objects.push_back(std::unique_ptr<Object>( new Triangle(ta,top,tc, green)));
    objects.push_back(std::unique_ptr<Object>( new Triangle(ta,tb,top, blue)));
    objects.push_back(std::unique_ptr<Object>( new Triangle(tc,tb,top, orange)));
    objects.push_back(std::unique_ptr<Object>( new Triangle(ta,tb,tc, orange)));
    
    lights.push_back(std::unique_ptr<Light>( new DirectLight(Vec3f(-0.5, 0.5, 1), 1.0, Vec3f(0.89, 0.73, 0.53))));
    lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(-20, 20, 20), 0.5, Vec3f(1, 1, 1))));
    lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(-30, 20, -25), 1.5, Vec3f(0.89, 0.73, 0.53))));
  }

  else if (sceneId == 4)
  {
    objects.push_back(std::unique_ptr<Object>( new Sphere(Vec3f(0, 12, -40), 10, mirror)));
    objects.push_back(std::unique_ptr<Object>( new Sphere(Vec3f(0, -2, -10), 1.5, gold)));
    objects.push_back(std::unique_ptr<Object>(new Cylinder (Vec3f(6,-4, -10), 1 ,3, gold)));
    objects.push_back(std::unique_ptr<Object>(new Cone (Vec3f(-6,-4, -10), 2 ,3, gold)));
     objects.push_back(std::unique_ptr<Object>(new Plane (Vec3f(0,-4, 0),Vec3f(0,1,0 ),checker)));


    objects.push_back(std::unique_ptr<Object>( new Triangle(ta,top,tc, gold)));
    objects.push_back(std::unique_ptr<Object>( new Triangle(ta,tb,top, orange)));
    objects.push_back(std::unique_ptr<Object>( new Triangle(tc,tb,top, orange)));
    objects.push_back(std::unique_ptr<Object>( new Triangle(ta,tb,tc, orange)));


    //lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(-20, 20, 20), 0.5, Vec3f(1, 1, 1))));
    //lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(-30, 20, -25), 1.5, Vec3f(0.89, 0.73, 0.53))));
    //lights.push_back(std::unique_ptr<Light>( new DirectLight(Vec3f(-0.4, 0, -1), 1.5, Vec3f(0.89, 0.73, 0.53))));
    //lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(20, 20, -13), 0.5, Vec3f(1, 1, 1))));
    //lights.push_back(std::unique_ptr<Light>( new PointLight(Vec3f(0, 0, 0), 2.5, Vec3f(1, 1, 1))));
  }

  std::vector<uint32_t> image(settings.height * settings.width * 3);

  float scale = tan(deg2rad(settings.fov * 0.5));
  float imageAspectRatio = settings.width / (float)settings.height;

  std::cout << threads << std::endl;
#pragma omp parallel for num_threads(threads)
  for (size_t j = 0; j < settings.height; j++) // actual rendering loop
  {
    for (size_t i = 0; i < settings.width; i++)
    {
      Vec3f temp = Vec3f(0, 0, 0);
      for (size_t k = 0; k < settings.AA; k++)
      {
        float x = (2 * (i + 0.5 + k * 0.25) / (float)settings.width - 1) * imageAspectRatio * scale;
        float y = (2 * (j + 0.5 - k * 0.25) / (float)settings.height - 1) * scale;

        Vec3f dir = normalize(Vec3f(x, y, -1));
        temp += newcast_ray(Vec3f(0, 0, 1.5), dir, objects, lights, envmap, settings);
      }
      temp = temp * (1.0 / settings.AA);
      float max = std::max(temp.x, std::max(temp.y, temp.z));
      if (max > 1)
        temp = temp / max;
      image[i + j * settings.width] = (uint32_t)(255 * std::max(0.f, std::min(1.f, temp.z))) << 16 | (uint32_t)(255 * std::max(0.f, std::min(1.f, temp.y))) << 8 | (uint32_t)(255 * std::max(0.f, std::min(1.f, temp.x)));
    }
  }

  //stbi_write_bmp(outFilePath.c_str(), settings.width, settings.height, 3, image.data());
  SaveBMP(outFilePath.c_str(), image.data(), settings.width, settings.height);

  //std::cout << "end." << std::endl;

  return 0;
}