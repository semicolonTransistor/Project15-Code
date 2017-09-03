#include <Arduino.h>
#include <avr/pgmspace.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define BAUDRATE 115200
#define NUMLEDS 150
#define FRAME_DURATION 33
#define NEOPIXEL_PIN 6
#define LED_PIN 9
#define V_SENSE_PIN A0
#define LOW_VOLTAGE_TRIP 220
#define LOW_VOLTAGE_TRIP_HYST 10
#define SD_CS 10

const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS,NEOPIXEL_PIN,NEO_GRB);
File rootDir;
char* animationFileName;
unsigned int activeLeds = 0;
unsigned int numFrames = 0;
boolean lowVoltageDisable = false;

File findAnimationFile(File dir){
  if(!dir.isDirectory()){
    return File();
  }
  File currentFile;
  while(true){
    currentFile = dir.openNextFile();
    if(!currentFile){
      return File();
    }
    String currentFileName = String(currentFile.name());
    if(currentFileName.endsWith(".LAF")){
      return currentFile;
    }
    currentFile.close();
  }
}

void listDir(File dir){
  unsigned int numFiles = 0;
  if(dir.isDirectory()){
    File currentFile;
    while(true){
      currentFile = dir.openNextFile();
      if(! currentFile){
        break;
      }
      if(!currentFile.isDirectory()){
        Serial.println(currentFile.name());
        numFiles++;
      }
      currentFile.close();
    }
  }
  Serial.print(F("Found "));
  Serial.print(numFiles);
  Serial.println(F(" Files"));
}

void setup(){
  Serial.begin(BAUDRATE);
  strip.begin();
  pinMode(LED_PIN,OUTPUT);
  Serial.println(F("Loading SD Card"));
  if(!SD.begin(SD_CS)){
    Serial.println(F("Unable to load SD card, is the SD card pluged in?"));
    Serial.println(F("Program will halt."));
    while(true); //halt
  }
  Serial.println(F("SD card Loaded."));
  rootDir = SD.open(F("/"));
  Serial.println(F("Serching root for animation files..."));
  File animationFile = findAnimationFile(rootDir);
  if(!animationFile){
    Serial.println(F("Unable to find animation file."));
    Serial.println(F("Program will halt."));
    while(true); //halt
  }
  Serial.print(F("Found animation file "));
  Serial.println(animationFile.name());
  unsigned int animationNumLeds = ((unsigned int)animationFile.read())<<8;
  animationNumLeds += animationFile.read();
  unsigned int animationNumFrames = ((unsigned int)animationFile.read())<<8;
  animationNumFrames += animationFile.read();
  Serial.print(F("Animation contains "));
  Serial.print(animationNumFrames);
  Serial.print(F(" frames at "));
  Serial.print(animationNumLeds);
  Serial.println(F(" LEDs"));
  unsigned long expectedSize = animationNumLeds*animationNumFrames*3+4;
  Serial.print(F("Checking file size, Expects "));
  Serial.print(expectedSize);
  Serial.println(F(" bytes."));
  if(expectedSize > animationFile.size()){
    Serial.println(F("Size check failed."));
    Serial.println(F("Program will halt."));
    while(true); //halt
  }
  Serial.println(F("Size check passed"));
  activeLeds = animationNumLeds;
  numFrames = animationNumFrames;
  animationFileName = animationFile.name();
  animationFile.close();
  Serial.println("Setup complete, animation will start.");
}

void loop(){
  File animationFile = SD.open(animationFileName,FILE_READ);
  //skip though the num frame and num led fields
  animationFile.read();
  animationFile.read();
  animationFile.read();
  animationFile.read();
  byte red;
  byte green;
  byte blue;
  for(unsigned int frame = 0; frame < numFrames;frame ++){
    unsigned long timeStarted = millis();

    if(analogRead(V_SENSE_PIN) <= LOW_VOLTAGE_TRIP){
      lowVoltageDisable = true;
    }else if(analogRead(V_SENSE_PIN) > (LOW_VOLTAGE_TRIP + LOW_VOLTAGE_TRIP_HYST)){
      lowVoltageDisable = false;
    }
    digitalWrite(LED_PIN, HIGH);
    for(unsigned int led = 0; led < activeLeds; led++){
      red = animationFile.read();
      green = animationFile.read();
      blue = animationFile.read();
      if(led < NUMLEDS){
        if(lowVoltageDisable){
          strip.setPixelColor(led, 0, 0, 0);
        }else{
          strip.setPixelColor(led, pgm_read_byte(&gamma8[red]),
                                   pgm_read_byte(&gamma8[green]),
                                   pgm_read_byte(&gamma8[blue]));
        }
      }
    }
    strip.show();
    digitalWrite(LED_PIN, LOW);
    while((millis() - timeStarted) < FRAME_DURATION); //wait for fram time to end
  }
  animationFile.close();
}
