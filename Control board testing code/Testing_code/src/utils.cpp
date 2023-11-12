/** @file utils.cpp
    @brief A Documented file.

    Source file for utilities functions
    For schematic please refer to:
    For additional pin functionality refer to:

*/

#include "utils.h"

volatile uint32_t counter_1us = 0;


int JOINT(int joint__){
  return joint__ - 1;
}

void Turn_on_24V(void)
{
  digitalWrite(SUPPLY_ON_OFF, HIGH);
}

void Turn_off_24V(void)
{
  digitalWrite(SUPPLY_ON_OFF, LOW);
}

/**
 * Inits Ticker
 * Inputs are Timer instance, tick frequency and callback function
 * @param[in] Instance
 * @param[in] frequency
 * @param[in] void (*int_callback)()
 */
void Ticker_init(TIM_TypeDef *Instance, int frequency, void (*int_callback)())
{

  HardwareTimer *MyTim = new HardwareTimer(Instance);
  MyTim->setOverflow(frequency, HERTZ_FORMAT);
  MyTim->attachInterrupt((*int_callback));
  MyTim->resume();
}

void Init_tick_1us()
{
  Ticker_init(TIM3, 1000000, tick_1us);
}

void tick_1us()
{
  counter_1us = counter_1us + 1;
}

uint32_t us_tick()
{
  return counter_1us;
}

/**
 * If power button is pressed start the robot
 * If it is held for more then a 3 seconds power off the power stage of the robot
 * Robot can still communicate when power stage is off if connected with USB but some functions are control functions are disabled
 *
 * @param[in] Instance
 * @param[in] frequency
 * @param[in] void (*int_callback)()
 */

/// primjer kada je upaljen gleda svakih 50 ms da li je nešto stisnuto
/// ako je onda pogleda da li je prošlo 3 sec i ako je onda zgasi
/// za power on, čim se stisne tipka u setupu je da holding pin ode high
/// ako je power button stisnut fet holding je high
/// ako je stisnut duže fet holding je nula
void Power_switch_managment()
{
  static uint32_t previous_milis = 0;
  static uint32_t turn_off_flag = 0;
  uint32_t current_milis = HAL_GetTick();

  if (current_milis - previous_milis >= 100)
  {
    if (turn_off_flag == 1)
    {
      turn_off_flag = 0;
      digitalWrite(SUPPLY_ON_OFF, LOW);
    }
    if (digitalRead(SUPPLY_BUTTON_STATE) == HIGH)
    {
      turn_off_flag = 1;
      digitalWrite(SUPPLY_ON_OFF, HIGH);
    }
  }
}

// in big endian machines, first byte of binary representation of the multibyte data-type is stored first.

/// @brief Convert int to 3 bytes, used for robot positon and speed
/// @param value
/// @param bytes 3 bytes
void intTo3Bytes(int32_t value, byte *bytes)
{
  bytes[0] = (value >> 16) & 0xFF; // extract upper byte
  bytes[1] = (value >> 8) & 0xFF;  // extract middle byte
  bytes[2] = value & 0xFF;         // extract lower byte
}

/// @brief convert signed int to 2 bytes, used for gripper data
/// @param value
/// @param bytes 2 bytes
void intTo2Bytes(int32_t value, byte *bytes)
{
  bytes[0] = (value >> 8) & 0xFF; // extract upper byte
  bytes[1] = value & 0xFF;        // extract lower byte
}

/// @brief convert 3 bytes into a signed int, used for robot position and speed variables
/// @param bytes
/// @return int value of fused bytes
int bytes_to_int(uint8_t *bytes)
{
  int value = ((int)bytes[0] << 16) | ((int)bytes[1] << 8) | (int)bytes[2];
  if (value & 0x00800000) // (value & 0x00800000)
  { // sign extend if needed
    value |= 0xFF000000;
  }
  return value;
}

/// @brief convert 2 bytes into a signed int, used for gripper data
/// @param bytes
/// @return signed int
int two_bytes_to_int(uint8_t *bytes)
{
  int value = ((int)bytes[0] << 8) | (int)bytes[1];
  if (value & 0x00008000)
  { // sign extend if needed
    value |= 0xFFFF0000;
  }
  return value;
}

/// @brief  convert array of bits to byte
/// @param bits
/// @return bits fused into a byte
unsigned char bitsToByte(const bool *bits)
{
  unsigned char byte = 0;
  for (int i = 0; i < 8; ++i)
  {
    if (bits[i])
    {
      byte |= (1 << (7 - i));
    }
  }
  return byte;
}

/// @brief  Convert byte to array of bits
/// @param b byte we want to convert
/// @param bits bits we will get
void byteToBits(byte b, bool* bits) {
  for (int i = 0; i < 8; i++) {
    bits[i] = (b >> i) & 0x01;
  }
}

/// @brief  Convert byte to array of bits
/// @param b byte we want to convert
/// @param bits bits we will get
void byteToBitsBigEndian(byte b, bool* bits) {
  for (int i = 7; i >= 0; i--) {
    bits[i] = (b >> (7 - i)) & 0x01;
  }
}