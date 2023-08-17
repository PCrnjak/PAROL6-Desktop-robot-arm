#include <SerialFlash.h>
#include <SPI.h>

const int FlashChipSelect = 6; // digital pin for flash chip CS pin
//const int FlashChipSelect = 21; // Arduino 101 built-in SPI Flash

void setup() {
  //uncomment these if using Teensy audio shield
  //SPI.setSCK(14);  // Audio shield has SCK on pin 14
  //SPI.setMOSI(7);  // Audio shield has MOSI on pin 7

  //uncomment these if you have other SPI chips connected
  //to keep them disabled while using only SerialFlash
  //pinMode(4, INPUT_PULLUP);
  //pinMode(10, INPUT_PULLUP);

  Serial.begin(9600);

  // wait for Arduino Serial Monitor
  while (!Serial) ;
  delay(100);
  Serial.println(F("All Files on SPI Flash chip:"));

  if (!SerialFlash.begin(FlashChipSelect)) {
    while (1) {
      Serial.println(F("Unable to access SPI Flash chip"));
      delay(2500);
    }
  }

  SerialFlash.opendir();
  int filecount = 0;
  while (1) {
    char filename[64];
    uint32_t filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
      Serial.print(F("  "));
      Serial.print(filename);
      Serial.print(F(", "));
      Serial.print(filesize);
      Serial.print(F(" bytes"));
      SerialFlashFile file = SerialFlash.open(filename);
      if (file) {
        unsigned long usbegin = micros();
        unsigned long n = filesize;
        char buffer[256];
        while (n > 0) {
          unsigned long rd = n;
          if (rd > sizeof(buffer)) rd = sizeof(buffer);
          file.read(buffer, rd);
          n = n - rd;
        }
        unsigned long usend = micros();
        Serial.print(F(", read in "));
        Serial.print(usend - usbegin);
        Serial.print(F(" us, speed = "));
        Serial.print((float)filesize * 1000.0 / (float)(usend - usbegin));
        Serial.println(F(" kbytes/sec"));
        file.close();
      } else {
        Serial.println(F(" error reading this file!"));
      }
      filecount = filecount + 1;
    } else {
      if (filecount == 0) {
        Serial.println(F("No files found in SerialFlash memory."));
      }
      break; // no more files
    }
  }
}

void loop() {
}

