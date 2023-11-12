/*
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <http://unlicense.org>
 * -------------------------------------------------------------------------
 * 
 * This is example code to 1) format an SPI Flash chip, and 2) copy raw 
 * audio files (mono channel, 16 bit signed, 44100Hz) to it using the 
 * SerialFlash library.  The audio can then be played back using the 
 * AudioPlaySerialflashRaw object in the Teensy Audio library.
 * 
 * To convert a .wav file to the proper .RAW format, use sox:
 * sox input.wav -r 44100 -b 16 --norm -e signed-integer -t raw OUTPUT.RAW remix 1,2
 * 
 * Note that the OUTPUT.RAW filename must be all caps and contain only the following
 * characters: A-Z, 0-9, comma, period, colon, dash, underscore.  (The SerialFlash
 * library converts filenames to caps, so to avoid confusion we just enforce it here).
 * 
 * It is a little difficult to see what is happening; aswe are using the Serial port
 * to upload files, we can't just throw out debug information.  Instead, we use the LED
 * (pin 13) to convey state.
 * 
 * While the chip is being formatted, the LED (pin 13) will toggle at 1Hz rate.  When 
 * the formatting is done, it flashes quickly (10Hz) for one second, then stays on 
 * solid.  When nothing has been received for 3 seconds, the upload is assumed to be 
 * completed, and the light goes off.
 * 
 * Use the 'rawfile-uploader.py' python script (included in the extras folder) to upload
 * the files.  You can start the script as soon as the Teensy is turned on, and the
 * USB serial upload will just buffer and wait until the flash is formatted.
 * 
 * This code was written by Wyatt Olson <wyatt@digitalcave.ca> (originally as part 
 * of Drum Master http://drummaster.digitalcave.ca and later modified into a 
 * standalone sample).
 * 
 * Enjoy!
 */

#include <SerialFlash.h>
#include <SPI.h>

//Buffer sizes
#define USB_BUFFER_SIZE      128
#define FLASH_BUFFER_SIZE    4096

//Max filename length (8.3 plus a null char terminator)
#define FILENAME_STRING_SIZE      13

//State machine
#define STATE_START      0
#define STATE_SIZE      1
#define STATE_CONTENT    2

//Special bytes in the communication protocol
#define BYTE_START      0x7e
#define BYTE_ESCAPE      0x7d
#define BYTE_SEPARATOR    0x7c


//SPI Pins (these are the values on the Audio board; change them if you have different ones)
#define MOSI               7
#define MISO              12
#define SCK               14
#define CSPIN              6
//#define CSPIN           21  // Arduino 101 built-in SPI Flash

void setup(){
  Serial.begin(9600);  //Teensy serial is always at full USB speed and buffered... the baud rate here is required but ignored

  pinMode(13, OUTPUT);
  
  //Set up SPI
  //SPI.setMOSI(MOSI);  // uncomment these if using the alternate pins
  //SPI.setMISO(MISO);
  //SPI.setSCK(SCK);
  if (!SerialFlash.begin(CSPIN)) {
    while (1) {
      Serial.println(F("Unable to access SPI Flash chip"));
      delay(1000);
    }
  }


  //We start by formatting the flash...
  uint8_t id[5];
  SerialFlash.readID(id);
  SerialFlash.eraseAll();
  
  //Flash LED at 1Hz while formatting
  while (!SerialFlash.ready()) {
    delay(500);
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
  }

  //Quickly flash LED a few times when completed, then leave the light on solid
  for(uint8_t i = 0; i < 10; i++){
    delay(100);
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
  }
  digitalWrite(13, HIGH);
  
  //We are now going to wait for the upload program
  while(!Serial.available());
  
  SerialFlashFile flashFile;
  
  uint8_t state = STATE_START;
  uint8_t escape = 0;
  uint8_t fileSizeIndex = 0;
  uint32_t fileSize = 0;
  char filename[FILENAME_STRING_SIZE];
  
  char usbBuffer[USB_BUFFER_SIZE];
  uint8_t flashBuffer[FLASH_BUFFER_SIZE];
  
  uint16_t flashBufferIndex = 0;
  uint8_t filenameIndex = 0;
  
  uint32_t lastReceiveTime = millis();
  
  //We assume the serial receive part is finished when we have not received something for 3 seconds
  while(Serial.available() || lastReceiveTime + 3000 > millis()){
    uint16_t available = Serial.readBytes(usbBuffer, USB_BUFFER_SIZE);
    if (available) {
      lastReceiveTime = millis();
    }

    for (uint16_t usbBufferIndex = 0; usbBufferIndex < available; usbBufferIndex++){
      uint8_t b = usbBuffer[usbBufferIndex];
      
      if (state == STATE_START){
        //Start byte.  Repeat start is fine.
        if (b == BYTE_START){
          for (uint8_t i = 0; i < FILENAME_STRING_SIZE; i++){
            filename[i] = 0x00;
          }
          filenameIndex = 0;
        }
        //Valid characters are A-Z, 0-9, comma, period, colon, dash, underscore
        else if ((b >= 'A' && b <= 'Z') || (b >= '0' && b <= '9') || b == '.' || b == ',' || b == ':' || b == '-' || b == '_'){
          filename[filenameIndex++] = b;
          if (filenameIndex >= FILENAME_STRING_SIZE){
            //Error name too long
            flushError();
            return;
          }
        }
        //Filename end character
        else if (b == BYTE_SEPARATOR){
          if (filenameIndex == 0){
            //Error empty filename
            flushError();
            return;
          }
          
          //Change state
          state = STATE_SIZE;
          fileSizeIndex = 0;
          fileSize = 0;
          
        }
        //Invalid character
        else {
          //Error bad filename
          flushError();
          return;
        }
      }
      //We read 4 bytes as a uint32_t for file size
      else if (state == STATE_SIZE){
        if (fileSizeIndex < 4){
          fileSize = (fileSize << 8) + b;
          fileSizeIndex++;
        }
        else if (b == BYTE_SEPARATOR){
          state = STATE_CONTENT;
          flashBufferIndex = 0;
          escape = 0;
          
          if (SerialFlash.exists(filename)){
            SerialFlash.remove(filename);  //It doesn't reclaim the space, but it does let you create a new file with the same name.
          }
          
          //Create a new file and open it for writing
          if (SerialFlash.create(filename, fileSize)) {
            flashFile = SerialFlash.open(filename);
            if (!flashFile) {
              //Error flash file open
              flushError();
              return;
            }
          }
          else {
            //Error flash create (no room left?)
            flushError();
            return;
          }
        }
        else {
          //Error invalid length requested
          flushError();
          return;
        }
      }
      else if (state == STATE_CONTENT){
        //Previous byte was escaped; unescape and add to buffer
        if (escape){
          escape = 0;
          flashBuffer[flashBufferIndex++] = b ^ 0x20;
        }
        //Escape the next byte
        else if (b == BYTE_ESCAPE){
          //Serial.println("esc");
          escape = 1;
        }
        //End of file
        else if (b == BYTE_START){
          //Serial.println("End of file");
          state = STATE_START;
          flashFile.write(flashBuffer, flashBufferIndex);
          flashFile.close();
          flashBufferIndex = 0;
        }
        //Normal byte; add to buffer
        else {
          flashBuffer[flashBufferIndex++] = b;
        }
        
        //The buffer is filled; write to SD card
        if (flashBufferIndex >= FLASH_BUFFER_SIZE){
          flashFile.write(flashBuffer, FLASH_BUFFER_SIZE);
          flashBufferIndex = 0;
        }
      }
    }
  }

  //Success!  Turn the light off.
  digitalWrite(13, LOW);
}

void loop(){
  //Do nothing.
}

void flushError(){
  uint32_t lastReceiveTime = millis();
  char usbBuffer[USB_BUFFER_SIZE];
  //We assume the serial receive part is finished when we have not received something for 3 seconds
  while(Serial.available() || lastReceiveTime + 3000 > millis()){
    if (Serial.readBytes(usbBuffer, USB_BUFFER_SIZE)){
      lastReceiveTime = millis();
    }
  }
}
