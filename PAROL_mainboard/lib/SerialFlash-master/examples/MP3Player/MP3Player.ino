// Simple MP3 file player example
//
// https://forum.pjrc.com/threads/27059-MP3-Player-Lib-with-example?p=101537&viewfull=1#post101537
//
// Requires the prop-shield and Teensy 3.2 or 3.1
// This example code is in the public domain.

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <play_sd_mp3.h> // https://github.com/FrankBoesing/Arduino-Teensy-Codec-lib
//#include <play_sd_aac.h>


// GUItool: begin automatically generated code
//AudioPlaySdWav           playSdWav1;     //xy=154,422
AudioPlaySdMp3           playMp31; //xy=154,422
AudioMixer4              mixer1;         //xy=327,432
AudioOutputAnalog        dac1;           //xy=502,412
AudioConnection          patchCord1(playMp31, 0, mixer1, 0);
AudioConnection          patchCord2(playMp31, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, dac1);
// GUItool: end automatically generated code


#define PROP_AMP_ENABLE    5
#define FLASH_CHIP_SELECT  6
//#define FLASH_CHIP_SELECT 21  // Arduino 101 built-in SPI Flash

void setup() {
  Serial.begin(9600);
  AudioMemory(8); //4
  delay(2000);

  // Start SerialFlash
  if (!SerialFlash.begin(FLASH_CHIP_SELECT)) {
    while (1) {
      Serial.println(F("Cannot access SPI Flash chip"));
      delay (1000);
    }
  }

  //Set Volume
  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);

  //Start Amplifier
  pinMode(PROP_AMP_ENABLE, OUTPUT);
  digitalWrite(PROP_AMP_ENABLE, HIGH);    // Enable Amplifier
}

void playFile(const char *filename)
{
  SerialFlashFile ff = SerialFlash.open(filename);
  Serial.print(F("Playing file: "));
  Serial.println(filename);

  uint32_t sz = ff.size();
  uint32_t pos = ff.getFlashAddress();

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playMp31.play(pos,sz);

  // Simply wait for the file to finish playing.
  while (playMp31.isPlaying()) {
    yield();
  }
}


void loop() {
  playFile("rain.mp3");
  delay(1000);
}
