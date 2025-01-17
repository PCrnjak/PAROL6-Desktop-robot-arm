/** @file main.cpp
    @brief A Documented file.

    Main file of PAROL6 mainboard firmware.
    Author: Petar Crnjak
    license:
    Date:
    GUI compatible version:
    Robot compatible version:

*/

#include <TMCStepper.h>
#include <Arduino.h>
#include <AccelStepper.h>
#include <Arduino.h>
#include "hw_init.h"
#include "iodefs.h"
#include "utils.h"
#include "common.h"
#include "constants.h"
#include "adc_init.h"
#include "stm32f4xx_hal.h"
#include "motor_init.h"
#include <SerialPacketParser.h>
#include <SerialFlash.h>
#include <SPI.h>

SerialPacketParser parser;
char command[20];
char argument[20];

// HardwareSerial Serial2(USART2); // compiles
#define Serial SerialUSB

// #define Serial Serial2

// #HardwareSerial Serial2(USART2);
int COMMAND = 0;
#define DEBUG 1      // For stepper drivers debug
#define DEBUG_COMS 0 // comms debug

// Init structures
TMC5160Stepper driver[] = {TMC5160Stepper(SELECT1, R_SENSE), TMC5160Stepper(SELECT6, R_SENSE), TMC5160Stepper(SELECT5, R_SENSE),
                           TMC5160Stepper(SELECT4, R_SENSE), TMC5160Stepper(SELECT2, R_SENSE), TMC5160Stepper(SELECT3, R_SENSE)};

AccelStepper stepper[] = {AccelStepper(stepper[0].DRIVER, PUL1, DIR1), AccelStepper(stepper[5].DRIVER, PUL6, DIR6),
                          AccelStepper(stepper[4].DRIVER, PUL5, DIR5), AccelStepper(stepper[3].DRIVER, PUL4, DIR4), AccelStepper(stepper[1].DRIVER, PUL2, DIR2), AccelStepper(stepper[2].DRIVER, PUL3, DIR3)};

struct MotorStruct Joint[NUMBER_OF_JOINTS];
struct Robot PAROL6;
struct Gripper Comp_gripper;

volatile unsigned tick_count;
bool LED_status = HIGH;

TIM_TypeDef *Instance = TIM2;
HardwareTimer *MyTim = new HardwareTimer(Instance);

/// @brief Called when timer for mesuring time between commands overflows, does nothing
/// @param
void Update_IT_callback(void)
{
}

// Flash testing stuff
void Init_motor_drivers(int num);
void Init_motor_drivers_no_print(int num);
SerialFlashFile file;

const unsigned long testIncrement = 4096;

void printbuf(const void *buf, uint32_t len);
bool is_erased(const unsigned char *data, unsigned int len);
bool equal_signatures(const unsigned char *data1, const unsigned char *data2);
void create_signature(unsigned long address, unsigned char *data);
void print_signature(const unsigned char *data);
const char *id2chip(const unsigned char *id);

bool test();

void setup()
{



  /// Init Joint sturctures
  
  Init_Joint_1(&Joint[0]);
  Init_Joint_2(&Joint[1]);
  Init_Joint_3(&Joint[2]);
  Init_Joint_4(&Joint[3]);
  Init_Joint_5(&Joint[4]);
  Init_Joint_6(&Joint[5]);  
  

#if (DEBUG > 0)
  delay(3000);
#endif

  /// Freq is 90Mhz, with 128 prescale we get 703125, timer is 16bit
  /// It counts to 65535. 1 Tick is then equal to 1/703125 = 1.422222e-6
  /// To get 10 ms we need 7031 ticks
  MyTim->setPrescaleFactor(128);
  MyTim->attachInterrupt(Update_IT_callback);
  MyTim->resume();

  Init_Digital_Inputs();
  /// Init Outputs
  Init_Digital_Outputs();

  // enable supply pin
  digitalWrite(SUPPLY_ON_OFF, HIGH);
  // NOTE this delay is needed for normal operation of Power_switch_managment
  delay(200);

  // Disable all stepper drivers with hardware enable pin
  digitalWrite(GLOBAL_ENABLE, HIGH);

  /// Init ADC
  HAL_Init();
  MX_ADC1_Init();
  HAL_ADC_MspInit_(&hadc1);

  Serial.begin(1000000);

  SPI.setMOSI(MOSI);
  SPI.setMISO(MISO);
  SPI.setSCLK(SCK);
  SPI.setClockDivider(SPI_CLOCK_DIV4); // High speed (180 / 4 = 45 MHz SPI_1 speed)
  SPI.begin();
  delay(100);
  digitalWrite(GLOBAL_ENABLE, LOW);

  delay(100);
  for (int i = 0; i < 6; i++)
  {
    Init_motor_drivers_no_print(i);
    delay(100);
  }

    for (int i = 0; i < 6; i++)
  {
    Init_motor_drivers(i);
    delay(100);
  }


  stepper[5].setMaxSpeed(50000);
  stepper[5].setAcceleration(100);
  stepper[5].setSpeed(0);

  stepper[0].setMaxSpeed(50000);
  stepper[0].setAcceleration(1000);
  stepper[0].setSpeed(0);

  stepper[1].setMaxSpeed(50000);
  stepper[1].setAcceleration(1000);
  stepper[1].setSpeed(0);

  stepper[2].setMaxSpeed(50000);
  stepper[2].setAcceleration(500);
  stepper[2].setSpeed(0);

  stepper[3].setMaxSpeed(50000);
  stepper[3].setAcceleration(500);
  stepper[3].setSpeed(0);

  stepper[4].setMaxSpeed(50000);
  stepper[4].setAcceleration(500);
  stepper[4].setSpeed(0);


}

void loop()
{

  Power_switch_managment();
  static uint32_t last_time = 0;
  int ms = HAL_GetTick();

  if ((ms - last_time) > 1000) // run every 3s
  {
    LED_status = !LED_status;
    digitalWrite(LED1, !LED_status);
    digitalWrite(LED2, LED_status);
    digitalWrite(OUTPUT1, !LED_status);
    digitalWrite(OUTPUT2, LED_status);
    if(COMMAND == 1){
      Serial.print("State of ESTOP is: ");
      Serial.println(digitalRead(ESTOP));
      Serial.print("State of INPUT1 is: ");
      Serial.println(digitalRead(INPUT1));
      Serial.print("State of INPUT2 is: ");
      Serial.println(digitalRead(INPUT2));
      Serial.print("Voltage supply is: ");
      int supply_voltage = BUS_voltage();
      Serial.println(supply_voltage);
      //Init_motor_drivers(0);
    }

      if(COMMAND == 2){

      
      //Serial.println(digitalRead(Joint[0].LIMIT));
      //Serial.println(digitalRead(Joint[1].LIMIT));
       //Serial.println(digitalRead(Joint[2].LIMIT));
       //Serial.println(digitalRead(Joint[3].LIMIT));
       //Serial.println(digitalRead(Joint[4].LIMIT));
       //Serial.println(digitalRead(Joint[5].LIMIT));
       //Serial.println("");

        
        if (digitalRead(Joint[0].LIMIT) == 0){
            Serial.println("Joint 1 is pressed ");
        }
                if (digitalRead(Joint[1].LIMIT) == 1){
            Serial.println("Joint 2 is pressed ");
        }
                if (digitalRead(Joint[2].LIMIT) == 1){
            Serial.println("Joint 3 is pressed ");
        }
                if (digitalRead(Joint[3].LIMIT) == 0){
            Serial.println("Joint 4 is pressed ");
        }
                if (digitalRead(Joint[4].LIMIT) == 1){
            Serial.println("Joint 5 is pressed ");
        }
                if (digitalRead(Joint[5].LIMIT) == 0){
            Serial.println("Joint 6 is pressed ");
        }
          

      //Init_motor_drivers(0);
    }




    last_time = ms;
  }
    stepper[0].setSpeed(1000);
    stepper[0].runSpeed();
    stepper[1].setSpeed(1000);
    stepper[1].runSpeed();
    stepper[2].setSpeed(1000);
    stepper[2].runSpeed();
    stepper[3].setSpeed(1000);
    stepper[3].runSpeed();
    stepper[4].setSpeed(1000);
    stepper[4].runSpeed();
    stepper[5].setSpeed(1000);
    stepper[5].runSpeed();

  // digitalWrite(OUTPUT1,HIGH);
  // digitalWrite(OUTPUT2,HIGH);
  while (Serial.available())
  {
    char c = Serial.read();

    if (parser.parse(c, command, argument))
    {

      if (strcmp(command, "start") == 0)
      {

        LED_status = !LED_status;
        digitalWrite(LED1, !LED_status);
        digitalWrite(LED2, LED_status);
        Serial.println("Received 'start' command");
      }
      /*
      else if (strcmp(command, "FLASH") == 0)
      {



        Serial.println(F("Raw SerialFlash Hardware Test"));
        SerialFlash.begin(FLASH_SELECT); // proceed even if begin() fails

        if (test())
        {
          Serial.println();
          Serial.println(F("All Tests Passed  :-)"));
          Serial.println();
          Serial.println(F("Test data was written to your chip.  You must run"));
          Serial.println(F("EraseEverything before using this chip for files."));
        }
        else
        {
          Serial.println();
          Serial.println(F("Tests Failed  :{"));
          Serial.println();
          Serial.println(F("The flash chip may be left in an improper state."));
          Serial.println(F("You might need to power cycle to return to normal."));
        }

        Serial.println("Received 'end' command");
      }
      	*/
      else if (strcmp(command, "IO") == 0)
      {
        COMMAND = 1;
        Serial.println("Received 'IO' command");
      }
      else if (strcmp(command, "STOP") == 0)
      {
        COMMAND = 0;
        Serial.println("Received 'STOP' command");
      }

      else if (strcmp(command, "LIMIT") == 0)
      {
        COMMAND = 2;
        Serial.println("Received 'LIMIT' command");
      }
      else
      {

        Serial.println("Unknown command");
      }
      Serial.print("Received command: ");
      Serial.println(command); // Print the received command

      Serial.print("Received argument: ");
      Serial.println(argument); // Print the received argument

      parser.resetCommandAndArgument(command, argument);
      break;
    }
  }
}

bool test()
{
  unsigned char buf[256], sig[256], buf2[8];
  unsigned long address, count, chipsize, blocksize;
  unsigned long usec;
  bool first;

  // Read the chip identification
  Serial.println();
  Serial.println(F("Read Chip Identification:"));
  SerialFlash.readID(buf);
  Serial.print(F("  JEDEC ID:     "));
  Serial.print(buf[0], HEX);
  Serial.print(' ');
  Serial.print(buf[1], HEX);
  Serial.print(' ');
  Serial.println(buf[2], HEX);
  Serial.print(F("  Part Number: "));
  Serial.println(id2chip(buf));
  Serial.print(F("  Memory Size:  "));
  chipsize = SerialFlash.capacity(buf);
  Serial.print(chipsize);
  Serial.println(F(" bytes"));
  if (chipsize == 0)
    return false;
  Serial.print(F("  Block Size:   "));
  blocksize = SerialFlash.blockSize();
  Serial.print(blocksize);
  Serial.println(F(" bytes"));

  // Read the entire chip.  Every test location must be
  // erased, or have a previously tested signature
  Serial.println();
  Serial.println(F("Reading Chip..."));
  memset(buf, 0, sizeof(buf));
  memset(sig, 0, sizeof(sig));
  memset(buf2, 0, sizeof(buf2));
  address = 0;
  count = 0;
  first = true;
  while (address < chipsize)
  {
    SerialFlash.read(address, buf, 8);
    // Serial.print("  addr = ");
    // Serial.print(address, HEX);
    // Serial.print(", data = ");
    // printbuf(buf, 8);
    create_signature(address, sig);
    if (is_erased(buf, 8) == false)
    {
      if (equal_signatures(buf, sig) == false)
      {
        Serial.print(F("  Previous data found at address "));
        Serial.println(address);
        Serial.println(F("  You must fully erase the chip before this test"));
        Serial.print(F("  found this: "));
        printbuf(buf, 8);
        Serial.print(F("     correct: "));
        printbuf(sig, 8);
        return false;
      }
    }
    else
    {
      count = count + 1; // number of blank signatures
    }
    if (first)
    {
      address = address + (testIncrement - 8);
      first = false;
    }
    else
    {
      address = address + 8;
      first = true;
    }
  }

  // Write any signatures that were blank on the original check
  if (count > 0)
  {
    Serial.println();
    Serial.print(F("Writing "));
    Serial.print(count);
    Serial.println(F(" signatures"));
    memset(buf, 0, sizeof(buf));
    memset(sig, 0, sizeof(sig));
    memset(buf2, 0, sizeof(buf2));
    address = 0;
    first = true;
    while (address < chipsize)
    {
      SerialFlash.read(address, buf, 8);
      if (is_erased(buf, 8))
      {
        create_signature(address, sig);
        // Serial.printf("write %08X: data: ", address);
        // printbuf(sig, 8);
        SerialFlash.write(address, sig, 8);
        while (!SerialFlash.ready())
          ; // wait
        SerialFlash.read(address, buf, 8);
        if (equal_signatures(buf, sig) == false)
        {
          Serial.print(F("  error writing signature at "));
          Serial.println(address);
          Serial.print(F("  Read this: "));
          printbuf(buf, 8);
          Serial.print(F("  Expected:  "));
          printbuf(sig, 8);
          return false;
        }
      }
      if (first)
      {
        address = address + (testIncrement - 8);
        first = false;
      }
      else
      {
        address = address + 8;
        first = true;
      }
    }
  }
  else
  {
    Serial.println(F("  all signatures present from prior tests"));
  }

  // Read all the signatures again, just to be sure
  // checks prior writing didn't corrupt any other data
  Serial.println();
  Serial.println(F("Double Checking All Signatures:"));
  memset(buf, 0, sizeof(buf));
  memset(sig, 0, sizeof(sig));
  memset(buf2, 0, sizeof(buf2));
  count = 0;
  address = 0;
  first = true;
  while (address < chipsize)
  {
    SerialFlash.read(address, buf, 8);
    create_signature(address, sig);
    if (equal_signatures(buf, sig) == false)
    {
      Serial.print(F("  error in signature at "));
      Serial.println(address);
      Serial.print(F("  Read this: "));
      printbuf(buf, 8);
      Serial.print(F("  Expected:  "));
      printbuf(sig, 8);
      return false;
    }
    count = count + 1;
    if (first)
    {
      address = address + (testIncrement - 8);
      first = false;
    }
    else
    {
      address = address + 8;
      first = true;
    }
  }
  Serial.print(F("  all "));
  Serial.print(count);
  Serial.println(F(" signatures read ok"));

  // Read pairs of adjacent signatures
  // check read works across boundaries
  Serial.println();
  Serial.println(F("Checking Signature Pairs"));
  memset(buf, 0, sizeof(buf));
  memset(sig, 0, sizeof(sig));
  memset(buf2, 0, sizeof(buf2));
  count = 0;
  address = testIncrement - 8;
  first = true;
  while (address < chipsize - 8)
  {
    SerialFlash.read(address, buf, 16);
    create_signature(address, sig);
    create_signature(address + 8, sig + 8);
    if (memcmp(buf, sig, 16) != 0)
    {
      Serial.print(F("  error in signature pair at "));
      Serial.println(address);
      Serial.print(F("  Read this: "));
      printbuf(buf, 16);
      Serial.print(F("  Expected:  "));
      printbuf(sig, 16);
      return false;
    }
    count = count + 1;
    address = address + testIncrement;
  }
  Serial.print(F("  all "));
  Serial.print(count);
  Serial.println(F(" signature pairs read ok"));

  // Write data and read while write in progress
  Serial.println();
  Serial.println(F("Checking Read-While-Write (Program Suspend)"));
  address = 256;
  while (address < chipsize)
  { // find a blank space
    SerialFlash.read(address, buf, 256);
    if (is_erased(buf, 256))
      break;
    address = address + 256;
  }
  if (address >= chipsize)
  {
    Serial.println(F("  error, unable to find any blank space!"));
    return false;
  }
  for (int i = 0; i < 256; i += 8)
  {
    create_signature(address + i, sig + i);
  }
  Serial.print(F("  write 256 bytes at "));
  Serial.println(address);
  Serial.flush();
  SerialFlash.write(address, sig, 256);
  usec = micros();
  if (SerialFlash.ready())
  {
    Serial.println(F("  error, chip did not become busy after write"));
    return false;
  }
  SerialFlash.read(0, buf2, 8); // read while busy writing
  while (!SerialFlash.ready())
    ; // wait
  usec = micros() - usec;
  Serial.print(F("  write time was "));
  Serial.print(usec);
  Serial.println(F(" microseconds."));
  SerialFlash.read(address, buf, 256);
  if (memcmp(buf, sig, 256) != 0)
  {
    Serial.println(F("  error writing to flash"));
    Serial.print(F("  Read this: "));
    printbuf(buf, 256);
    Serial.print(F("  Expected:  "));
    printbuf(sig, 256);
    return false;
  }
  create_signature(0, sig);
  if (memcmp(buf2, sig, 8) != 0)
  {
    Serial.println(F("  error, incorrect read while writing"));
    Serial.print(F("  Read this: "));
    printbuf(buf2, 256);
    Serial.print(F("  Expected:  "));
    printbuf(sig, 256);
    return false;
  }
  Serial.print(F("  read-while-writing: "));
  printbuf(buf2, 8);
  Serial.println(F("  test passed, good read while writing"));

  // Erase a block and read while erase in progress
  if (chipsize >= 262144 + blocksize + testIncrement)
  {
    Serial.println();
    Serial.println(F("Checking Read-While-Erase (Erase Suspend)"));
    memset(buf, 0, sizeof(buf));
    memset(sig, 0, sizeof(sig));
    memset(buf2, 0, sizeof(buf2));
    SerialFlash.eraseBlock(262144);
    usec = micros();
    delayMicroseconds(50);
    if (SerialFlash.ready())
    {
      Serial.println(F("  error, chip did not become busy after erase"));
      return false;
    }
    SerialFlash.read(0, buf2, 8); // read while busy writing
    while (!SerialFlash.ready())
      ; // wait
    usec = micros() - usec;
    Serial.print(F("  erase time was "));
    Serial.print(usec);
    Serial.println(F(" microseconds."));
    // read all signatures, check ones in this block got
    // erased, and all the others are still intact
    address = 0;
    first = true;
    while (address < chipsize)
    {
      SerialFlash.read(address, buf, 8);
      if (address >= 262144 && address < 262144 + blocksize)
      {
        if (is_erased(buf, 8) == false)
        {
          Serial.print(F("  error in erasing at "));
          Serial.println(address);
          Serial.print(F("  Read this: "));
          printbuf(buf, 8);
          return false;
        }
      }
      else
      {
        create_signature(address, sig);
        if (equal_signatures(buf, sig) == false)
        {
          Serial.print(F("  error in signature at "));
          Serial.println(address);
          Serial.print(F("  Read this: "));
          printbuf(buf, 8);
          Serial.print(F("  Expected:  "));
          printbuf(sig, 8);
          return false;
        }
      }
      if (first)
      {
        address = address + (testIncrement - 8);
        first = false;
      }
      else
      {
        address = address + 8;
        first = true;
      }
    }
    Serial.print(F("  erase correctly erased "));
    Serial.print(blocksize);
    Serial.println(F(" bytes"));
    // now check if the data we read during erase is good
    create_signature(0, sig);
    if (memcmp(buf2, sig, 8) != 0)
    {
      Serial.println(F("  error, incorrect read while erasing"));
      Serial.print(F("  Read this: "));
      printbuf(buf2, 256);
      Serial.print(F("  Expected:  "));
      printbuf(sig, 256);
      return false;
    }
    Serial.print(F("  read-while-erasing: "));
    printbuf(buf2, 8);
    Serial.println(F("  test passed, good read while erasing"));
  }
  else
  {
    Serial.println(F("Skip Read-While-Erase, this chip is too small"));
  }

  return true;
}

const char *id2chip(const unsigned char *id)
{
  if (id[0] == 0xEF)
  {
    // Winbond
    if (id[1] == 0x40)
    {
      if (id[2] == 0x14)
        return "W25Q80BV";
      if (id[2] == 0x15)
        return "W25Q16DV";
      if (id[2] == 0x17)
        return "W25Q64FV";
      if (id[2] == 0x18)
        return "W25Q128FV";
      if (id[2] == 0x19)
        return "W25Q256FV";
    }
  }
  if (id[0] == 0x01)
  {
    // Spansion
    if (id[1] == 0x02)
    {
      if (id[2] == 0x16)
        return "S25FL064A";
      if (id[2] == 0x19)
        return "S25FL256S";
      if (id[2] == 0x20)
        return "S25FL512S";
    }
    if (id[1] == 0x20)
    {
      if (id[2] == 0x18)
        return "S25FL127S";
    }
  }
  if (id[0] == 0xC2)
  {
    // Macronix
    if (id[1] == 0x20)
    {
      if (id[2] == 0x18)
        return "MX25L12805D";
    }
  }
  if (id[0] == 0x20)
  {
    // Micron
    if (id[1] == 0xBA)
    {
      if (id[2] == 0x20)
        return "N25Q512A";
      if (id[2] == 0x21)
        return "N25Q00AA";
    }
    if (id[1] == 0xBB)
    {
      if (id[2] == 0x22)
        return "MT25QL02GC";
    }
  }
  if (id[0] == 0xBF)
  {
    // SST
    if (id[1] == 0x25)
    {
      if (id[2] == 0x02)
        return "SST25WF010";
      if (id[2] == 0x03)
        return "SST25WF020";
      if (id[2] == 0x04)
        return "SST25WF040";
      if (id[2] == 0x41)
        return "SST25VF016B";
      if (id[2] == 0x4A)
        return "SST25VF032";
    }
    if (id[1] == 0x25)
    {
      if (id[2] == 0x01)
        return "SST26VF016";
      if (id[2] == 0x02)
        return "SST26VF032";
      if (id[2] == 0x43)
        return "SST26VF064";
    }
  }
  if (id[0] == 0x1F)
  {
    // Adesto
    if (id[1] == 0x89)
    {
      if (id[2] == 0x01)
        return "AT25SF128A";
    }
  }
  return "(unknown chip)";
}

void print_signature(const unsigned char *data)
{
  Serial.print(F("data="));
  for (unsigned char i = 0; i < 8; i++)
  {
    Serial.print(data[i]);
    Serial.print(' ');
  }
  Serial.println();
}

void create_signature(unsigned long address, unsigned char *data)
{
  data[0] = address >> 24;
  data[1] = address >> 16;
  data[2] = address >> 8;
  data[3] = address;
  unsigned long hash = 2166136261ul;
  for (unsigned char i = 0; i < 4; i++)
  {
    hash ^= data[i];
    hash *= 16777619ul;
  }
  data[4] = hash;
  data[5] = hash >> 8;
  data[6] = hash >> 16;
  data[7] = hash >> 24;
}

bool equal_signatures(const unsigned char *data1, const unsigned char *data2)
{
  for (unsigned char i = 0; i < 8; i++)
  {
    if (data1[i] != data2[i])
      return false;
  }
  return true;
}

bool is_erased(const unsigned char *data, unsigned int len)
{
  while (len > 0)
  {
    if (*data++ != 255)
      return false;
    len = len - 1;
  }
  return true;
}

void printbuf(const void *buf, uint32_t len)
{
  const uint8_t *p = (const uint8_t *)buf;
  do
  {
    unsigned char b = *p++;
    Serial.print(b >> 4, HEX);
    Serial.print(b & 15, HEX);
    // Serial.printf("%02X", *p++);
    Serial.print(' ');
  } while (--len > 0);
  Serial.println();
}


void Init_motor_drivers(int num)
{
  //digitalWrite(Joint[num].SELECT, HIGH);
  driver[num].begin();                                                                      // Initiate pins and registeries
  driver[num].rms_current(260 * 0.85, Joint[num].hold_multiplier); // Set stepper current, second parameter is hold_multiplier
  driver[num].en_pwm_mode(1);                                                               // Enable extremely quiet stepping
  driver[num].toff(4);                                                                      // off time
  driver[num].blank_time(24);                                                               // blank tim
  driver[num].pwm_autoscale(1);
  driver[num].microsteps(32); // What microstep range to use
  // driver[num].ihold(25);
  // driver[num].irun(25);

  int var0 = driver[num].test_connection();
  Joint[num].current = driver[num].rms_current();
  Joint[num].hold_multiplier = driver[num].hold_multiplier();
  Joint[num].ihold = driver[num].ihold();
  Joint[num].irun = driver[num].irun();
  Joint[num].open_load_B = driver[num].olb();
  Joint[num].open_load_A = driver[num].ola();
  Joint[num].short_2_gnd_B = driver[num].s2gb();
  Joint[num].short_2_gnd_A = driver[num].s2ga();
  Joint[num].over_temp_pre_warning = driver[num].otpw();
  Joint[num].over_temp_warning = driver[num].ot();
  Joint[num].diag0 = driver[num].diag0_error();

#if (DEBUG > 0)

  Serial.print("joint ");
  Serial.print(num + 1);
  Serial.print(" Connection is: ");
  if(var0 == 0){
     Serial.println("GOOD");
  }else{
    Serial.println("BAD");
  }


  /*
  Serial.print("joint ");
  Serial.print(num + 1);
  Serial.print(" current is: ");
  Serial.println(Joint[num].current);

  Serial.print("joint ");
  Serial.print(num + 1);
  Serial.print(" multipler is: ");
  Serial.println(Joint[num].hold_multiplier);

  Serial.print("joint ");
  Serial.print(num + 1);
  Serial.print(" Ihold is: ");
  Serial.println(Joint[num].ihold);

  Serial.print("joint ");
  Serial.print(num + 1);
  Serial.print(" Irun is: ");
  Serial.println(Joint[num].irun);
  Serial.println("");
  */

/*
  if (Joint[num].open_load_B)
  {
    Serial.println(F("Open Load B"));
  }

  if (Joint[num].open_load_A)
  {
    Serial.println(F("Open Load A"));
  }

  if (Joint[num].short_2_gnd_B)
  {
    Serial.println(F("Short to Gnd B"));
  }

  if (driver[num].s2ga())
  {
    Serial.println(F("Short to Gnd A"));
  }
*/
  if (Joint[num].over_temp_pre_warning)
  {
    Serial.println(F("Overtemp. Pre Warning")); // pre warning flag
  }

  if (Joint[num].over_temp_warning)
  {
    Serial.println(F("Overtemp."));
  }

  //if (driver[num].diag0_error())
 // {
  //  Serial.println(F("DIAG0 error"));
  //}

#endif

 // digitalWrite(Joint[num].SELECT, LOW);
}





void Init_motor_drivers_no_print(int num)
{
  //digitalWrite(Joint[num].SELECT, HIGH);
  driver[num].begin();                                                                      // Initiate pins and registeries
  driver[num].rms_current(260 * 0.85, Joint[num].hold_multiplier); // Set stepper current, second parameter is hold_multiplier
  driver[num].en_pwm_mode(1);                                                               // Enable extremely quiet stepping
  driver[num].toff(4);                                                                      // off time
  driver[num].blank_time(24);                                                               // blank tim
  driver[num].pwm_autoscale(1);
  driver[num].microsteps(32); // What microstep range to use
  // driver[num].ihold(25);
  // driver[num].irun(25);

  int var0 = driver[num].test_connection();
  Joint[num].current = driver[num].rms_current();
  Joint[num].hold_multiplier = driver[num].hold_multiplier();
  Joint[num].ihold = driver[num].ihold();
  Joint[num].irun = driver[num].irun();
  Joint[num].open_load_B = driver[num].olb();
  Joint[num].open_load_A = driver[num].ola();
  Joint[num].short_2_gnd_B = driver[num].s2gb();
  Joint[num].short_2_gnd_A = driver[num].s2ga();
  Joint[num].over_temp_pre_warning = driver[num].otpw();
  Joint[num].over_temp_warning = driver[num].ot();
  Joint[num].diag0 = driver[num].diag0_error();

#if (DEBUG > 0)

  //Serial.print("joint ");
 // Serial.print(num + 1);
 // Serial.print(" Connection is: ");
 // if(var0 == 0){
  //   Serial.println("GOOD");
 // }else{
 //   Serial.println("BAD");
 // }


  /*
  Serial.print("joint ");
  Serial.print(num + 1);
  Serial.print(" current is: ");
  Serial.println(Joint[num].current);

  Serial.print("joint ");
  Serial.print(num + 1);
  Serial.print(" multipler is: ");
  Serial.println(Joint[num].hold_multiplier);

  Serial.print("joint ");
  Serial.print(num + 1);
  Serial.print(" Ihold is: ");
  Serial.println(Joint[num].ihold);

  Serial.print("joint ");
  Serial.print(num + 1);
  Serial.print(" Irun is: ");
  Serial.println(Joint[num].irun);
  Serial.println("");
  */

/*
  if (Joint[num].open_load_B)
  {
    Serial.println(F("Open Load B"));
  }

  if (Joint[num].open_load_A)
  {
    Serial.println(F("Open Load A"));
  }

  if (Joint[num].short_2_gnd_B)
  {
    Serial.println(F("Short to Gnd B"));
  }

  if (driver[num].s2ga())
  {
    Serial.println(F("Short to Gnd A"));
  }
*/
  //if (Joint[num].over_temp_pre_warning)
  //{
  //  Serial.println(F("Overtemp. Pre Warning")); // pre warning flag
  //}

  //if (Joint[num].over_temp_warning)
  //{
  //  Serial.println(F("Overtemp."));
  //}

  //if (driver[num].diag0_error())
 // {
  //  Serial.println(F("DIAG0 error"));
  //}

#endif

 // digitalWrite(Joint[num].SELECT, LOW);
}
