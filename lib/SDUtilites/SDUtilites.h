#ifndef SD_UTILITES_H
#define SD_UTILITES_H
# include <SdFat.h>

struct AnimationConfig
{
  uint16_t numLed;
  uint16_t numFrames;
};

File findAnimationFile(SdFat sd,const char* path);
AnimationConfig getAnimationConfig(SdFat sd,const char* path);

#endif
