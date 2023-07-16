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
    error("Unable to access SPI Flash chip");
  }

  SerialFlash.opendir();
  while (1) {
    char filename[64];
    uint32_t filesize;

    if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
      Serial.print(F("  "));
      Serial.print(filename);
      spaces(20 - strlen(filename));
      Serial.print(F("  "));
      Serial.print(filesize);
      Serial.print(F(" bytes"));
      Serial.println();
    } else {
      break; // no more files
    }
  }
}

void spaces(int num) {
  for (int i=0; i < num; i++) {
    Serial.print(' ');
  }
}

void loop() {
}

void error(const char *message) {
  while (1) {
    Serial.println(message);
    delay(2500);
  }
}
