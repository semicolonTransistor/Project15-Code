# include "SDUtilites.h"

File findAnimationFile(SdFat sd, const char* path){
  File dir = sd.open(path);
  if(!dir.isDirectory()){
    return File();
  }
  File currentFile;
  while(true){
    currentFile = dir.openNextFile();
    if(!currentFile){
      return File();
    }

    char fileNameChar[13];
    currentFile.getName(fileNameChar,13);
    String currentFileName = String(fileNameChar);
    Serial.println(currentFileName);
    if(currentFileName.endsWith(".LAF")){
      return currentFile;
    }
    currentFile.close();
  }
}

AnimationConfig getAnimationConfig(SdFat sd,const char* path){
  File file = sd.open(path);
  file.seek(0); //start from the begining
  uint16_t numLeds = ((uint16_t)file.read())<<8;
  numLeds += file.read();
  uint16_t numFrames = ((uint16_t)file.read())<<8;
  numFrames += file.read();
  uint32_t expectedSize = numLeds*(uint32_t)numFrames + 4;
  if(expectedSize <= file.size()){
    return {numLeds, numFrames};
  }else{
    return {0,0};
  }

}
