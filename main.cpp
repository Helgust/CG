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



   if(sceneId == 1)
    {

    }
   else if(sceneId == 2)
   {
     
   }
      
   else if(sceneId == 3)
   {
     
   }
     
    std::vector<uint32_t> image(height*width); 



  for (size_t j = 0; j<height; j++) // actual rendering loop
    {   for (size_t i = 0; i<width; i++) 
        {
            float dir_x =  (i + 0.5) -  width/2.;
            float dir_y = (j + 0.5) - height/2.;    // this flips the image at the same time
            float dir_z = -height/(2.*tan(fov/2.));
            
            Vec3f temp; // = here a casting a ray function;          
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