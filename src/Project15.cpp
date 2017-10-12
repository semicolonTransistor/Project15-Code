#include <Arduino.h>
#include <SdFat.h>
#include <SDUtilites.h>
#include <Adafruit_NeoPixel.h>

#define BAUDRATE 115200
#define NUMLEDS 300
#define FRAME_DURATION 33
#define NEOPIXEL_PIN 13
#define LED_PIN 5
#define V_SENSE_PIN A0
#define LOW_VOLTAGE_TRIP 220
#define LOW_VOLTAGE_TRIP_HYST 10
#define SD_CS 6

#define NO_SD 0
#define RUNING 1

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
SdFat SD;
File rootDir;
char animationFileName[13];
File animationFile;
unsigned int activeLeds = 0;
unsigned int numFrames = 0;
boolean lowVoltageDisable = false;
uint8_t opState = 0;

void setup(){
  Serial.begin(BAUDRATE);
  strip.begin();
  pinMode(LED_PIN,OUTPUT);


}

void loop(){
  switch(opState){
  case NO_SD:{
    delay(5000);
    Serial.println(F("Loading SD Card"));
    if(!SD.begin(SD_CS,SPI_FULL_SPEED)){
      Serial.println(F("Unable to load SD card, is the SD card pluged in?"));
      return;
    }
    Serial.println(F("SD card Loaded."));
    rootDir = SD.open("/");
    Serial.println(F("Serching root for animation files..."));
    animationFile = findAnimationFile(SD,"/");
    if(!animationFile){
      Serial.println(F("Unable to find animation file."));
      return;
    }
    animationFile.getName(animationFileName,13);
    animationFile.close();
    Serial.print(F("Found animation file "));
    Serial.println(animationFileName);
    AnimationConfig config = getAnimationConfig(SD,animationFileName);
    if(config.numFrames == 0 || config.numLed == 0){
      Serial.println(F("Invalid Animation File!"));
      return;
    }
    Serial.print(F("Animation contains "));
    Serial.print(config.numFrames);
    Serial.print(F(" frames at "));
    Serial.print(config.numLed);
    Serial.println(F(" LEDs"));
    if(config.numLed > NUMLEDS){
      activeLeds = NUMLEDS;
    }else{
      activeLeds = config.numLed;
    }
    numFrames = config.numFrames;
    rootDir.close();
    Serial.println("Setup complete, animation will start.");
    opState = RUNING;
    break;
  }
  case RUNING:{
    animationFile = SD.open(animationFileName);
    animationFile.seek(3u);
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
    break;
  }
  }
}
