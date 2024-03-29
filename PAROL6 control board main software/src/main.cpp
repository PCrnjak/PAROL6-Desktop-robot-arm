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
#include "CAN.h"
#include "coms_CAN.h"

// HardwareSerial Serial2(USART2); // compiles
#define Serial SerialUSB
#define DEBUG 0      // For stepper drivers debug
#define DEBUG_COMS 0 // comms debug

volatile unsigned tick_count;

#define NUM 6

// Init structures
TMC5160Stepper driver[] = {TMC5160Stepper(SELECT1, R_SENSE), TMC5160Stepper(SELECT6, R_SENSE), TMC5160Stepper(SELECT5, R_SENSE),
                           TMC5160Stepper(SELECT4, R_SENSE), TMC5160Stepper(SELECT2, R_SENSE), TMC5160Stepper(SELECT3, R_SENSE)};

AccelStepper stepper[] = {AccelStepper(stepper[0].DRIVER, PUL1, DIR1), AccelStepper(stepper[5].DRIVER, PUL6, DIR6),
                          AccelStepper(stepper[4].DRIVER, PUL5, DIR5), AccelStepper(stepper[3].DRIVER, PUL4, DIR4), AccelStepper(stepper[1].DRIVER, PUL2, DIR2), AccelStepper(stepper[2].DRIVER, PUL3, DIR3)};

struct MotorStruct Joint[NUMBER_OF_JOINTS];
struct Robot PAROL6;
// struct Gripper Comp_gripper;

// variable for mesuring elapsed time between 2 received commands
int current_tick = 0;
int prev_tick = 0;

// variables for robot homing
int robot_homed = 0;
int homed = 0;
int repet_flag = 0;
int home_command = 0;

void Init_motor_drivers(int num);
int home_all();
void Init_motor_direction();
void disable_motors();
void enable_motors();

// Input helper variables
byte input_byte = 0; // Here save incoming bytes from serial

byte start_cond1_byte = 255;
byte start_cond2_byte = 255;
byte start_cond3_byte = 255;

byte end_cond1_byte = 1;
byte end_cond2_byte = 2;

byte start_cond1 = 0; // Flag if start_cond1_byte is received
byte start_cond2 = 0; // Flag if start_cond2_byte is received
byte start_cond3 = 0; // Flag if start_cond3_byte is received

byte good_start = 0; // Flag if we got all 3 start condition bytes
byte data_len = 0;   // Length of the data after -3 start condition bytes and length byte, so -4 bytes

byte data_buffer[255]; // Here save all data after data length byte
byte data_counter = 0; // Data counter for incoming bytes; compared to data length to see if we have correct length

// 255 255 255 5 2 3 3 0 1
/// @brief Data for output stuff
uint8_t start_bytes[] = {0xff, 0xff, 0xff};
uint8_t data_led = 0x05;
// uint8_t test_data[] = {0x02, 0x03, 0x03};
uint8_t test_data[] = {2, 3, 3};
uint8_t end_bytes[] = {0x01, 0x02};

TIM_TypeDef *Instance = TIM2;
HardwareTimer *MyTim = new HardwareTimer(Instance);

// FOR PNEUMATIC GRIPPER 8900
int j5_homing_offset = 8900; // for SSG48 GRIPPER 8035;

// Home commands
int run_once = 0;
int joint123_stage1 = 0;
int joint123_stage2 = 0;
int joint123_stage3 = 0;
int joint123_done = 0;

int J4_stage2 = 0;
int J4_stage3 = 0;
int J4_stage4 = 0;
int J4_done = 0;

int J6_stage2 = 0;
int J6_stage3 = 0;
int J6_stage4 = 0;
int J6_done = 0;

int J5_stage2 = 0;
int J5_stage3 = 0;
int J5_stage4 = 0;
int J5_done = 0;

int joint456_stage1 = 0;
int joint456_stage2 = 0;
int joint456_stage3 = 0;

/// demo stage test
int setup_var = 0;
int move1 = 0;
int move2 = 0;
int move3 = 0;
int move4 = 0;
int start_stop = 0;

/// @brief Called when timer for mesuring time between commands overflows, does nothing
/// @param
void Update_IT_callback(void)
{
}

void Unpack_data(uint8_t *data_buffer);
void Pack_data();
void Pack_data_TEST();
void Get_data();
void reset_homing();
void Handle_gripper();

void setup()
{

  /// Init Joint sturctures
  Init_Joint_1(&Joint[0]);
  Init_Joint_2(&Joint[1]);
  Init_Joint_3(&Joint[2]);
  Init_Joint_4(&Joint[3]);
  Init_Joint_5(&Joint[4]);
  Init_Joint_6(&Joint[5]);

  /// Init motor directions
  Init_motor_direction();
  /// Init Inputs
  Init_Digital_Inputs();
  /// Init Outputs
  Init_Digital_Outputs();

  // enable supply pin
  digitalWrite(SUPPLY_ON_OFF, HIGH);

  // Disable all stepper drivers with hardware enable pin
  digitalWrite(GLOBAL_ENABLE, HIGH);
// Delay so you can catch serial
#if (DEBUG > 0)
  delay(3000);
#endif

  // SPI1, and SPI4 can communicate at up to 45 Mbits/s, SPI2 and
  // SPI3 can communicate at up to 22.5 Mbit/s.
  // Init SPI (SPI 1 is used)
  SPI.setMOSI(MOSI);
  SPI.setMISO(MISO);
  SPI.setSCLK(SCK);
  SPI.setClockDivider(SPI_CLOCK_DIV4); // High speed (180 / 4 = 45 MHz SPI_1 speed)
  SPI.begin();
  // NOTE this delay is needed for normal operation of Power_switch_managment
  delay(200);

  /// Init ADC
  HAL_Init();
  MX_ADC1_Init();
  HAL_ADC_MspInit_(&hadc1);

  // Enable all stepper drivers with hardware enable pin
  digitalWrite(GLOBAL_ENABLE, LOW);
  // Sets some initial parameters to stepper drivers

  for (int i = 0; i < 6; i++)
  {
    Init_motor_drivers(i);
    delay(90);
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

  /// Freq is 90Mhz, with 128 prescale we get 703125, timer is 16bit
  /// It counts to 65535. 1 Tick is then equal to 1/703125 = 1.422222e-6
  /// To get 10 ms we need 7031 ticks
  MyTim->setPrescaleFactor(128);
  MyTim->attachInterrupt(Update_IT_callback);
  MyTim->resume();
  Serial.begin(3000000);
  Setup_CAN_bus();
}

void loop()
{

  /*
  int state1 = digitalRead(INPUT1);
  int state2 = digitalRead(INPUT2);
  Serial.println("inputs 1 is:");
  Serial.println(state1);
  Serial.println("inputs 2 is:");
  Serial.println(state2);
  delay(2000);
  */

  Power_switch_managment();
  static uint32_t last_time = 0;
  int ms = HAL_GetTick();

  if ((ms - last_time) > 3000) // run every 3s
  {
    last_time = ms;
  }

  /// Robot repetability

  if (PAROL6.command == 69)
  {
    if (setup_var == 0)
    {
      setup_var = 1;

      stepper[0].setMaxSpeed(6000);
      stepper[0].setAcceleration(6000);
      stepper[0].moveTo(6179);

      stepper[1].setMaxSpeed(4000);
      stepper[1].setAcceleration(4000);
      stepper[1].moveTo(-22222);

      stepper[2].setMaxSpeed(3500);
      stepper[2].setAcceleration(3500);
      stepper[2].moveTo(49812);

      stepper[3].setMaxSpeed(3500);
      stepper[3].setAcceleration(3500);
      stepper[3].moveTo(0);

      stepper[4].setMaxSpeed(3500);
      stepper[4].setAcceleration(3500);
      stepper[4].moveTo(4059);

      stepper[5].setMaxSpeed(7500);
      stepper[5].setAcceleration(7500);
      stepper[5].moveTo(46075);

      /// init accels
      move1 = 1;
    }

    if (move1 == 1)
    {

      for (int i = 0; i < 6; i++)
      {
        stepper[i].run();
      }

      if (stepper[0].isRunning() || stepper[1].isRunning() || stepper[2].isRunning() || stepper[3].isRunning() || stepper[4].isRunning() || stepper[5].isRunning())
      {
        move1 = 1;
      }
      else
      {
        move1 = 0;
        move2 = 1;
        stepper[0].moveTo(10240);
        stepper[1].moveTo(-32000);
        stepper[2].moveTo(57905);
        stepper[3].moveTo(0);
        stepper[4].moveTo(0);
        stepper[5].moveTo(32000);
        delay(2000);
      }
    }

    if (move2 == 1)
    {

      for (int i = 0; i < 6; i++)
      {
        stepper[i].run();
      }

      if (stepper[0].isRunning() || stepper[1].isRunning() || stepper[2].isRunning() || stepper[3].isRunning() || stepper[4].isRunning() || stepper[5].isRunning())
      {
        move2 = 1;
      }
      else
      {
        move2 = 0;
        setup_var = 1;
      }
    }
  }

  // Dummy command
  if (PAROL6.command == 255)
  {
    setup_var = 0;
    move1 = 0;
    move2 = 0;
    if (home_command == 0)
    {
      for (int i = 0; i < 6; i++)
      {
        stepper[i].setSpeed(Joint[i].commanded_velocity);
        //  provjera da li smo na većim pozicijama, ako da baci error
      }
    }
  }

  /// Enable robot
  if (PAROL6.command == 101)
  {
    reset_homing();
    home_command = 0;
    PAROL6.disabled = 0;
  }

  /// Disable robot
  if (PAROL6.command == 102)
  {
    home_command = 0;
    PAROL6.disabled = 1;
  }

  /// Clear error
  if (PAROL6.command == 103)
  {
    home_command = 0;
    reset_homing();
  }

  // If robot is disabled, disable all move commands
  if (PAROL6.disabled == 0)
  {
    /// Home robot
    if (PAROL6.command == 100)
    {
      home_command = 1;
      homed = 0;
      if (homed == 0)
      {
        home_all();
      }
      else
      {
        home_command = 0;
      }
    }
    else if (PAROL6.command == 255 && home_command == 1)
    {
      if (homed == 0)
      {
        home_all();
      }
      else
      {
        home_command = 0;
      }
    }

    /// JOG
    if (PAROL6.command == 123)
    {
      home_command = 0;
      homed = 1;
      for (int i = 0; i < 6; i++)
      {
        stepper[i].setSpeed(Joint[i].commanded_velocity);
        stepper[i].runSpeed();
      }
      // Joint[i].speed = Joint[i].commanded_velocity;
      //  provjera da li smo na većim pozicijama, ako da baci error

      /// Pročitaj koji je od joint speeds nije nula i miči taj joint tom brzinom
      // If (current position != commanded) // ovo vrijedi za one position command tip naredbe
      // Set speed
      // Run speed
      // Dummy data sets sppeds to zero
    }

    /// GO 2 POSITION
    /// Input is needed position and speed
    if (PAROL6.command == 156)
    {
      home_command = 0;
      homed = 1;

      for (int i = 0; i < 6; i++)
      {

        int speed_set = int(((Joint[i].commanded_position - Joint[i].position) / 0.01));
        speed_set = int(((Joint[i].commanded_velocity + speed_set) / 2));
        stepper[i].setSpeed(speed_set);
        stepper[i].runSpeed();
      }
    }
  }

  /***************************************************/
  /***************************************************/
  /// Here check robot position and speed and save to struct
  for (int i = 0; i < 6; i++)
  {
    Joint[i].position = stepper[i].currentPosition();
    Joint[i].speed = stepper[i].speed();
    // provjera da li smo na većim pozicijama, ako da baci error
  }
  /***************************************************/
  /***************************************************/

  Get_data();
}

/// @brief  Here we receive data from the PC and unpack it.
/// If the data is good we also send a packet to the PC
void Get_data()
{

  // Get data from CAN gripper
  CAN_protocol(Serial);

  // Get data from serial
  while (Serial.available() > 0)
  {

    input_byte = Serial.read();
    //  When data len is received start is good and after that put all data in receive buffer
    //  Data len is ALL data after it; that includes input buffer, end bytes and CRC
    if (good_start != 1)
    {
      // All start bytes are good and next byte is data len
      if (start_cond1 == 1 && start_cond2 == 1 && start_cond3 == 1)
      {
        good_start = 1;
        data_len = input_byte;
        // Serial.println("Robot good start");
      }
      // Third start byte is good
      if (input_byte == start_cond3_byte && start_cond2 == 1 && start_cond1 == 1)
      {
        start_cond3 = 1;
        // Serial.println("Robot good cond 3");
      }
      // Third start byte is bad, reset all flags
      else if (start_cond2 == 1 && start_cond1 == 1)
      {
        start_cond1 = 0;
        start_cond2 = 0;
      }
      // Second start byte is good
      if (input_byte == start_cond2_byte && start_cond1 == 1)
      {
        start_cond2 = 1;
        // Serial.println("Robot good cond 2");
      }
      // Second start byte is bad, reset all flags
      else if (start_cond1 == 1)
      {
        start_cond1 = 0;
      }
      // First start byte is good
      if (input_byte == start_cond1_byte)
      {
        start_cond1 = 1;
        // Serial.println("Robot good cond 1");
      }
    }
    else
    {
      // Here data goes after good start
      data_buffer[data_counter] = input_byte;
      if (data_counter == data_len - 1)
      {
        // Here if last 2 bytes are end condition bytes we process the data
        if (data_buffer[data_len - 2] == end_cond1_byte && data_buffer[data_len - 1] == end_cond2_byte)
        {

          // Serial.println(data_buffer[data_len-2]); // 1
          // Serial.println(data_buffer[data_len-1]); // 1
          //  Serial.println(data_buffer[data_len]); // 0
          // Serial.println("ROBOT GOOD END CONDITION ");
          current_tick = MyTim->getCount();
          PAROL6.time_between_commands = current_tick - prev_tick;
          prev_tick = current_tick;
          // Serial.println("ROBOT DATA UNPACK ");
          Unpack_data(data_buffer);
          // Serial.println("ROBOT DATA PACK");

          // Read estop and inputs and write outputs
          
          PAROL6.In1 = digitalRead(INPUT1);
          PAROL6.In2 = digitalRead(INPUT2);
          PAROL6.Estop = digitalRead(ESTOP);
          digitalWrite(OUTPUT1, PAROL6.commanded_OUT1);
          digitalWrite(OUTPUT2, PAROL6.commanded_OUT2);
          PAROL6.Out1 = PAROL6.commanded_OUT1;
          PAROL6.Out2 = PAROL6.commanded_OUT2;

          Handle_gripper();
          Pack_data();
          // digitalWrite(DIR6,HIGH);
          // digitalWrite(DIR6,LOW);
          //   ako su dobri izračunaj crc
          //   if crc dobar raspakiraj podatke
          //   ako je dobar paket je dobar i spremi ga u nove variable!
          //  Pošalji nove podatke od robota
        }

        // Serial.println("podaci u data bufferu su: ");
        // for (int i = 0; i < data_len; i++)
        // Serial.println(data_buffer[i]);

        good_start = 0;
        start_cond1 = 0;
        start_cond3 = 0;
        start_cond2 = 0;
        data_len = 0;
        data_counter = 0;
      }
      else
      {
        data_counter = data_counter + 1;
      }
    }
  }
}

void Handle_gripper()
{

  
    /// Here unpack gripper command data to bits and see what needs to be sent to the gripper
    bool bitArray[8]; // 0 - activation, 1 - action status, 2 - estop status, 3 - release dir
  // Comp_gripper.mode; 1 - calibration, 0 - operation mode

  byteToBitsBigEndian(Comp_gripper.command, bitArray);

  if (Comp_gripper.mode == 1)
  {
    Send_gripper_cal();
  }
  else if (Comp_gripper.mode == 2)
  {
    Send_clear_error();
  }
  else if (Comp_gripper.prev_commanded_position == Comp_gripper.commanded_position && Comp_gripper.prev_commanded_speed == Comp_gripper.commanded_speed 
  && Comp_gripper.prev_commanded_current == Comp_gripper.commanded_current && Comp_gripper.prev_command == Comp_gripper.command && Comp_gripper.prev_commanded_ID == Comp_gripper.commanded_ID
  && Comp_gripper.mode == 0)
  {
    Send_gripper_pack_empty();
  }
  else
  {
    Send_gripper_pack();
  }

  Comp_gripper.prev_commanded_position = Comp_gripper.commanded_position;
  Comp_gripper.prev_commanded_speed = Comp_gripper.commanded_speed;
  Comp_gripper.prev_commanded_current = Comp_gripper.commanded_current;
  Comp_gripper.prev_command = Comp_gripper.command;
  Comp_gripper.prev_commanded_ID = Comp_gripper.commanded_ID;
}

/// @brief  Unpack data packet we got from the PC
/// @param data_buffer array of bytes we get thru serial
void Unpack_data(uint8_t *data_buffer)
{

  int temp[6][3];
  int Joints[6];
  int Speed[6];
  int Command;
  int Affected_joint;
  int InOut;
  int Timeout;
  int Gripper_position;
  int Gripper_speed;
  int Gripper_current;
  int Gripper_command;
  int Gripper_mode;
  int Gripper_ID;
  int CRC_byte;

  int i, j;
  /// Unpack position data
  for (i = 0, j = 0; i < 18; i += 3, j++)
  {
    temp[j][0] = data_buffer[i];
    temp[j][1] = data_buffer[i + 1];
    temp[j][2] = data_buffer[i + 2];
    uint8_t buf_test[] = {temp[j][0], temp[j][1], temp[j][2]};
    Joints[j] = bytes_to_int(buf_test);
    Joint[j].commanded_position = Joints[j];

    // Serial.println(Joints[j]);
  }
  /// Unpack speed data
  for (i = 18, j = 0; i < 36; i += 3, j++)
  {
    temp[j][0] = data_buffer[i];
    temp[j][1] = data_buffer[i + 1];
    temp[j][2] = data_buffer[i + 2];
    uint8_t buf_test[] = {temp[j][0], temp[j][1], temp[j][2]};
    Speed[j] = bytes_to_int(buf_test);
    Joint[j].commanded_velocity = Speed[j];
    // Serial.println(Speed[j]);
  }

  Command = data_buffer[36];
  PAROL6.command = Command;
  // Serial.println(Command);
  Affected_joint = data_buffer[37];
  PAROL6.Affected_joint = Affected_joint;
  // Serial.println(Affected_joint);
  InOut = data_buffer[38];
  bool bitArray[8];
  byteToBitsBigEndian(InOut, bitArray);
  PAROL6.commanded_OUT1 = bitArray[2];
  PAROL6.commanded_OUT2 = bitArray[3];
  // Serial.println(InOut);
  Timeout = data_buffer[39];
  PAROL6.Timeout = Timeout;
  // Serial.println(Timeout);

  uint8_t buf_test[2] = {data_buffer[40], data_buffer[41]};
  Gripper_position = two_bytes_to_int(buf_test);
  Comp_gripper.commanded_position = Gripper_position;
  // Serial.println(Gripper_position);

  buf_test[0] = data_buffer[42];
  buf_test[1] = data_buffer[43];
  Gripper_speed = two_bytes_to_int(buf_test);
  Comp_gripper.commanded_speed = Gripper_speed;
  // Serial.println(Gripper_speed);

  buf_test[0] = data_buffer[44];
  buf_test[1] = data_buffer[45];
  Gripper_current = two_bytes_to_int(buf_test);
  Comp_gripper.commanded_current = Gripper_current;
  // Serial.println(Gripper_current);

  Gripper_command = data_buffer[46];
  Comp_gripper.command = Gripper_command;
  // Serial.println(Gripper_command);
  Gripper_mode = data_buffer[47];
  Comp_gripper.mode = Gripper_mode;
  // Serial.println(Gripper_mode);
  Gripper_ID = data_buffer[48];
  Comp_gripper.commanded_ID = Gripper_ID;
  // Serial.println(Gripper_ID);
  CRC_byte = data_buffer[49];
  // Serial.println(CRC_byte);

#if (DEBUG_COMS > 0)
  for (i = 0, j = 0; i < 18; i += 3, j++)
  {
    Serial.println(Joints[j]);
  }
  for (i = 0, j = 0; i < 18; i += 3, j++)
  {
    Serial.println(Speed[j]);
  }
  Serial.println(Command);
  Serial.println(Affected_joint);
  Serial.println(InOut);
  Serial.println(Timeout);
  Serial.println(Gripper_position);
  Serial.println(Gripper_speed);
  Serial.println(Gripper_current);
  Serial.println(Gripper_command);
  Serial.println(Gripper_mode);
  Serial.println(Gripper_ID);
  Serial.println(CRC_byte);

#endif
}

/// @brief  Pack data packet that we will send to the PC
/// Test function
void Pack_data_TEST()
{

  // Len is defined by all bytes EXCEPT start bytes and len
  uint8_t start_bytes[] = {0xff, 0xff, 0xff}; // 3
  int len = 56;
  int Position_out[] = {255, 254, 253, 252, 251, 250}; // 18
  int Speed_out[] = {245, 244, 243, 242, 241, 240};    // 18
  bool Homed[] = {1, 1, 1, 1, 1, 1, 1, 1};             // 1
  bool IO_var[] = {0, 0, 0, 0, 0, 0, 0, 0};            // 1
  bool temp_error[] = {1, 1, 1, 1, 1, 1, 1, 1};        // 1
  bool position_error[] = {0, 0, 0, 0, 0, 0, 0, 0};    // 1
  int timing_data = 255;                               // 2 byte
  int timeout_error = 244;                             // 1
  int xtr2 = 255;                                      // 1
  int gripper_ID = 200;                                // 1
  int gripper_position = 300;                          // 2 byte
  int gripper_speed = 300;                             // 2 byte
  int gripper_current = 300;                           // 2 byte
  int gripper_status = 200;                            // 1
  int object_detection = 1;                            // 1
  int CRC_byte = 212;                                  // 1
  uint8_t end_bytes[] = {0x01, 0x02};                  // 2

  byte data_buffer_send[3];

  // Send start bytes
  Serial.write(start_bytes[0]);
  Serial.write(start_bytes[1]);
  Serial.write(start_bytes[2]);

  Serial.write(len);

  /// Position data
  for (int i = 0; i < 6; i++)
  {
    intTo3Bytes(Position_out[i], data_buffer_send);
    Serial.write(data_buffer_send[0]);
    Serial.write(data_buffer_send[1]);
    Serial.write(data_buffer_send[2]);
  }
  /// Speed data
  for (int i = 0; i < 6; i++)
  {
    intTo3Bytes(Speed_out[i], data_buffer_send);
    Serial.write(data_buffer_send[0]);
    Serial.write(data_buffer_send[1]);
    Serial.write(data_buffer_send[2]);
  }
  /// Homed data
  Serial.write(bitsToByte(Homed));

  /// I/O data
  Serial.write(bitsToByte(IO_var));

  /// temperature error
  Serial.write(bitsToByte(temp_error));

  /// positon error
  Serial.write(bitsToByte(position_error));

  // Dummy timing data
  Serial.write(255);
  Serial.write(255);

  // Timeout error
  Serial.write(timeout_error);

  /// Aditional data slot
  Serial.write(xtr2);

  /// Gripper ID
  Serial.write(gripper_ID);

  // Send gripper positon
  intTo2Bytes(gripper_position, data_buffer_send);
  Serial.write(data_buffer_send[0]);
  Serial.write(data_buffer_send[1]);

  // Send gripper speed
  intTo2Bytes(gripper_speed, data_buffer_send);
  Serial.write(data_buffer_send[0]);
  Serial.write(data_buffer_send[1]);

  // Send gripper current
  intTo2Bytes(gripper_current, data_buffer_send);
  Serial.write(data_buffer_send[0]);
  Serial.write(data_buffer_send[1]);

  Serial.write(gripper_status);

  // Send gripper object detected
  Serial.write(object_detection);

  // Send CRC
  Serial.write(CRC_byte);

  // Send end bytes
  Serial.write(end_bytes[0]);
  Serial.write(end_bytes[1]);
}

/// @brief  Pack data packet that we will send to the PC
void Pack_data()
{

  // Len is defined by all bytes EXCEPT start bytes and len
  uint8_t start_bytes[] = {0xff, 0xff, 0xff}; // 3
  int len = 56;
  int Position_out[] = {Joint[0].position, Joint[1].position, Joint[2].position, Joint[3].position, Joint[4].position, Joint[5].position};                                                            // 18
  int Speed_out[] = {Joint[0].speed, Joint[1].speed, Joint[2].speed, Joint[3].speed, Joint[4].speed, Joint[5].speed};                                                                                 // 18
  bool Homed[] = {Joint[0].homed, Joint[1].homed, Joint[2].homed, Joint[3].homed, Joint[4].homed, Joint[5].homed, 1, 1};                                                                              // 1
  bool IO_var[] = {PAROL6.In1, PAROL6.In2, PAROL6.Out1, PAROL6.Out2, PAROL6.Estop, 1, 1, 1};                                                                                                          // 1
  bool temp_error[] = {Joint[0].temperature_error, Joint[1].temperature_error, Joint[2].temperature_error, Joint[3].temperature_error, Joint[4].temperature_error, Joint[5].temperature_error, 1, 1}; // 1
  bool position_error[] = {Joint[0].position_error, Joint[1].position_error, Joint[2].position_error, Joint[3].position_error, Joint[4].position_error, Joint[5].position_error, 1, 1};               // 1
  unsigned int timing_data = PAROL6.time_between_commands;                                                                                                                                            // 2 byte
  int timeout_error = PAROL6.timeout_error;                                                                                                                                                           // 1
  int xtr2 = PAROL6.command;
  // int xtr2 = PAROL6.xtr2_byte;                                                                                                                                                                        // 1
  int gripper_ID = Comp_gripper.Gripper_ID;             // 1
  int gripper_position = Comp_gripper.current_position; // 2 byte
  int gripper_speed = Comp_gripper.current_speed;       // 2 byte
  int gripper_current = Comp_gripper.current_current;   // 2 byte
  int gripper_status = Comp_gripper.current_status;     // 1
  int object_detection = Comp_gripper.object_detection; // 1
  int CRC_byte = PAROL6.CRC_value;                      // 1
  uint8_t end_bytes[] = {0x01, 0x02};                   // 2

  byte data_buffer_send[3];

  // Send start bytes
  Serial.write(start_bytes[0]);
  Serial.write(start_bytes[1]);
  Serial.write(start_bytes[2]);

  Serial.write(len);

  /// Position data
  for (int i = 0; i < 6; i++)
  {
    intTo3Bytes(Position_out[i], data_buffer_send);
    Serial.write(data_buffer_send[0]);
    Serial.write(data_buffer_send[1]);
    Serial.write(data_buffer_send[2]);
  }
  /// Speed data
  for (int i = 0; i < 6; i++)
  {
    intTo3Bytes(Speed_out[i], data_buffer_send);
    Serial.write(data_buffer_send[0]);
    Serial.write(data_buffer_send[1]);
    Serial.write(data_buffer_send[2]);
  }
  /// Homed data
  Serial.write(bitsToByte(Homed));

  /// I/O data
  Serial.write(bitsToByte(IO_var));

  /// temperature error
  Serial.write(bitsToByte(temp_error));

  /// positon error
  Serial.write(bitsToByte(position_error));

  // Dummy timing data
  intTo2Bytes(timing_data, data_buffer_send);
  Serial.write(data_buffer_send[0]);
  Serial.write(data_buffer_send[1]);

  // Timeout error
  Serial.write(timeout_error);

  /// Aditional data slot
  Serial.write(xtr2);

  /// Gripper ID
  Serial.write(gripper_ID);

  // Send gripper positon
  intTo2Bytes(gripper_position, data_buffer_send);
  Serial.write(data_buffer_send[0]);
  Serial.write(data_buffer_send[1]);

  // Send gripper speed
  intTo2Bytes(gripper_speed, data_buffer_send);
  Serial.write(data_buffer_send[0]);
  Serial.write(data_buffer_send[1]);

  // Send gripper current
  intTo2Bytes(gripper_current, data_buffer_send);
  Serial.write(data_buffer_send[0]);
  Serial.write(data_buffer_send[1]);

  // Send gripper status
  Serial.write(gripper_status);

  // Send gripper object detected
  Serial.write(object_detection);

  // Send CRC
  Serial.write(CRC_byte);

  // Send end bytes
  Serial.write(end_bytes[0]);
  Serial.write(end_bytes[1]);
}

void reset_homing()
{
  run_once = 0;
  joint123_stage1 = 0;
  joint123_stage2 = 0;
  joint123_stage3 = 0;
  joint123_done = 0;

  J4_stage2 = 0;
  J4_stage3 = 0;
  J4_stage4 = 0;
  J4_done = 0;

  J6_stage2 = 0;
  J6_stage3 = 0;
  J6_stage4 = 0;
  J6_done = 0;

  J5_stage2 = 0;
  J5_stage3 = 0;
  J5_stage4 = 0;
  J5_done = 0;

  joint456_stage1 = 0;
  joint456_stage2 = 0;
  joint456_stage3 = 0;
}
/// @brief  Homes whole robot using specific sequence
/// @return If the robot is homed or not
int home_all()
{
  /*
    static int run_once = 0;
    static int joint123_stage1 = 0;
    static int joint123_stage2 = 0;
    static int joint123_stage3 = 0;
    static int joint123_done = 0;

    static int J4_stage2 = 0;
    static int J4_stage3 = 0;
    static int J4_stage4 = 0;
    static int J4_done = 0;

    static int J6_stage2 = 0;
    static int J6_stage3 = 0;
    static int J6_stage4 = 0;
    static int J6_done = 0;

    static int J5_stage2 = 0;
    static int J5_stage3 = 0;
    static int J5_stage4 = 0;
    static int J5_done = 0;

    static int joint456_stage1 = 0;
    static int joint456_stage2 = 0;
    static int joint456_stage3 = 0;
    */

  if (homed == 0)
  {
    if (run_once == 0)
    {
      for (int i = 0; i < 6; i++)
      {
        // set initial homing speeds, these change
        Joint[i].homed = 0;
        stepper[0].setSpeed(-2050);
        stepper[1].setSpeed(-2550);
        stepper[2].setSpeed(-2550);
        stepper[3].setSpeed(5550);
        stepper[4].setSpeed(-5050);
        stepper[5].setSpeed(-9550);
        Joint[0].homed = 0;
        Joint[1].homed = 0;
        Joint[2].homed = 0;
        Joint[3].homed = 0;
        Joint[4].homed = 0;
        Joint[5].homed = 0;
      }
      run_once = 1;
    }

    if (joint123_done == 1)
    {

      if (J6_done == 1)
      {

        /// accelerate to standby position for joint 5 and 6
        if (Joint[4].homing_stage_2 == 1 && J5_stage4 == 0)
        {

          stepper[4].setAcceleration(5500);
          stepper[4].moveTo(j5_homing_offset);

          stepper[5].setAcceleration(5500);
          stepper[5].moveTo(0);

          stepper[4].run();
          stepper[5].run();
          if (stepper[4].currentPosition() == j5_homing_offset && stepper[5].currentPosition() == 0)
          {
            J5_stage4 = 1;
            stepper[4].setSpeed(1050);
            J5_done = 1;
            Joint[4].homed = 1;
            Joint[5].homed = 1;
          }
        }

        /// hit limit and use this position as a reference!
        if (J5_stage3 == 1 && Joint[4].homing_stage_2 == 0)
        {

          if (digitalRead(Joint[4].LIMIT) == Joint[4].limit_switch_trigger)
          {

            Joint[4].homing_stage_2 = 1;
            stepper[4].setSpeed(0);
            stepper[4].setCurrentPosition(0);
          }
          stepper[4].runSpeed();
        }

        // move away from the first limit hit
        if (Joint[4].homing_stage_1 == 1 && J5_stage2 == 0)
        {
          stepper[4].moveTo(1550);
          stepper[4].setSpeed(4050);

          stepper[4].runSpeedToPosition();

          if (stepper[4].currentPosition() == 1550)
          {
            J5_stage2 = 1;
            J5_stage3 = 1;
            stepper[4].setSpeed(-4050);
          }
        }

        // is the limit hit? dont use this position since limit can be pressed to much and robot can be touching joint blockers.
        if (Joint[4].homing_stage_1 == 0)
        {

          if (digitalRead(Joint[4].LIMIT) == Joint[4].limit_switch_trigger)
          {

            Joint[4].homing_stage_1 = 1;
            stepper[4].setSpeed(0);
            stepper[4].setCurrentPosition(0);
          }
          stepper[4].runSpeed();
        }
      }

      if (J4_done == 1)
      {
        /// accelerate joint 6 to specified position so you can home joint 5
        if (Joint[5].homing_stage_2 == 1 && J6_stage4 == 0)
        {

          stepper[5].setAcceleration(10500);
          stepper[5].moveTo(Joint[5].homed_position);
          stepper[5].run();
          if (stepper[5].currentPosition() == Joint[5].homed_position)
          {
            J6_stage4 = 1;
            stepper[5].setSpeed(1050);
            J6_done = 1;
          }
        }

        /// hit limit and use this position as a reference!
        if (J6_stage3 == 1 && Joint[5].homing_stage_2 == 0)
        {
          if (digitalRead(Joint[5].LIMIT) == Joint[5].limit_switch_trigger)
          {

            Joint[5].homing_stage_2 = 1;
            stepper[5].setSpeed(0);
            stepper[5].setCurrentPosition(0);
          }
          stepper[5].runSpeed();
        }

        // move away from the first limit hit
        if (Joint[5].homing_stage_1 == 1 && J6_stage2 == 0)
        {
          stepper[5].moveTo(1550);
          stepper[5].setSpeed(4050);

          stepper[5].runSpeedToPosition();

          if (stepper[5].currentPosition() == 1550)
          {
            J6_stage2 = 1;
            J6_stage3 = 1;
            stepper[5].setSpeed(-8050);
          }
        }

        // is the limit hit? dont use this position since limit can be pressed to much and robot can be touching joint blockers.
        if (Joint[5].homing_stage_1 == 0)
        {

          if (digitalRead(Joint[5].LIMIT) == Joint[5].limit_switch_trigger)
          {
            Joint[5].homing_stage_1 = 1;
            stepper[5].setSpeed(0);
            stepper[5].setCurrentPosition(0);
          }
          stepper[5].runSpeed();
        }
      }
      /////

      /// accelerate to standby position for joint 4
      if (Joint[3].homing_stage_2 == 1 && J4_stage4 == 0)
      {

        stepper[3].setAcceleration(5500);
        stepper[3].moveTo(Joint[3].homed_position);
        stepper[3].run();
        if (stepper[3].currentPosition() == Joint[3].homed_position)
        {
          J4_stage4 = 1;
          stepper[3].setSpeed(2050);
          J4_done = 1;
          Joint[3].homed = 1;
        }
      }

      /// hit limit and use this position as a reference!
      if (J4_stage3 == 1 && Joint[3].homing_stage_2 == 0)
      {

        if (digitalRead(Joint[3].LIMIT) == Joint[3].limit_switch_trigger)
        {

          Joint[3].homing_stage_2 = 1;
          stepper[3].setSpeed(0);
          stepper[3].setCurrentPosition(0);
        }
        stepper[3].runSpeed();
      }

      // move away from the first limit hit
      if (Joint[3].homing_stage_1 == 1 && J4_stage2 == 0)
      {
        stepper[3].moveTo(-1550);
        stepper[3].setSpeed(-4050);

        stepper[3].runSpeedToPosition();

        if (stepper[3].currentPosition() == -1550)
        {
          J4_stage2 = 1;
          J4_stage3 = 1;
          stepper[3].setSpeed(4050);
        }
      }

      // is the limit hit? dont use this position since limit can be pressed to much and robot can be touching joint blockers.
      if (Joint[3].homing_stage_1 == 0)
      {

        if (digitalRead(Joint[3].LIMIT) == Joint[3].limit_switch_trigger)
        {

          Joint[3].homing_stage_1 = 1;
          stepper[3].setSpeed(0);
          stepper[3].setCurrentPosition(0);
        }
        stepper[3].runSpeed();
      }
    }

    /// Homing of the joints 1, 2 and 3 is done
    if (joint123_done == 0)
    {
      /// accelerate to standby position for joints 1, 2 and 3
      if (Joint[0].homing_stage_2 == 1 && Joint[1].homing_stage_2 == 1 && Joint[2].homing_stage_2 == 1 && joint123_stage3 == 0)
      {
        joint123_stage2 == 0;

        stepper[0].setAcceleration(4500);
        stepper[0].moveTo(Joint[0].homed_position);
        stepper[1].setAcceleration(3000);
        stepper[1].moveTo(Joint[1].homed_position);
        stepper[2].setAcceleration(5000);
        stepper[2].moveTo(Joint[2].homed_position);

        for (int i = 0; i < 3; i++)
        {
          stepper[i].run();
        }
        if (stepper[0].currentPosition() == Joint[0].homed_position && stepper[1].currentPosition() == Joint[1].homed_position && stepper[2].currentPosition() == Joint[2].homed_position)
        {
          Joint[0].homed = 1;
          Joint[1].homed = 1;
          Joint[2].homed = 1;
          joint123_stage3 = 1;
          joint123_done = 1;
        }
      }

      /// hit limit and use this position as a reference!
      if (joint123_stage2 == 1)
      {
        for (int i = 0; i < 3; i++)
        {
          if (Joint[i].homing_stage_2 == 0)
          {
            if (digitalRead(Joint[i].LIMIT) == Joint[i].limit_switch_trigger)
            {
              Joint[i].homing_stage_2 = 1;
              stepper[i].setSpeed(0);
              stepper[i].setCurrentPosition(0);
            }
            stepper[i].runSpeed();
          }
        }
      }

      // move away from the first limit hit
      if (Joint[0].homing_stage_1 == 1 && Joint[1].homing_stage_1 == 1 && Joint[2].homing_stage_1 == 1 && joint123_stage2 == 0)
      {
        joint123_stage1 == 1;

        stepper[0].moveTo(850);
        stepper[0].setSpeed(-2050);

        stepper[1].moveTo(1050);
        stepper[1].setSpeed(2050);

        stepper[2].moveTo(1050);
        stepper[2].setSpeed(2050);

        for (int i = 0; i < 3; i++)
        {
          stepper[i].runSpeedToPosition();
        }
        if (stepper[0].currentPosition() == 850 && stepper[1].currentPosition() == 1050 && stepper[2].currentPosition() == 1050)
        {
          joint123_stage2 = 1;
          stepper[0].setSpeed(-750);
          stepper[1].setSpeed(-750);
          stepper[2].setSpeed(-750);
        }
      }

      // is the limit hit? dont use this position since limit can be pressed to much and robot can be touching joint blockers.
      // this can be seen on joints 2 and 3
      if (joint123_stage1 == 0)
      {
        for (int i = 0; i < 3; i++)
        {
          if (Joint[i].homing_stage_1 == 0)
          {
            if (digitalRead(Joint[i].LIMIT) == Joint[i].limit_switch_trigger)
            {
              Joint[i].homing_stage_1 = 1;
              stepper[i].setSpeed(0);
              stepper[i].setCurrentPosition(0);
            }
            stepper[i].runSpeed();
          }
        }
      }
    }
  }

  /// Homing is done here
  /// reset all flags, robot is in standby position so set its joint variables to those defined in DH table and diagram
  if (J5_done == 1)
  {
    for (int i = 0; i < 6; i++)
    {
      stepper[i].setCurrentPosition(Joint[i].standby_position);
      Joint[i].homing_stage_1 = 0;
      Joint[i].homing_stage_2 = 0;
    }

    reset_homing();

    homed = 1;
  }
  return homed;
}

void Init_motor_direction()
{
  for (int i = 0; i < 6; i++)
  {
    stepper[i].setPinsInverted(Joint[i].direction_reversed);
  }
}

void Init_motor_drivers(int num)
{
  digitalWrite(Joint[num].SELECT, HIGH);
  driver[num].begin();                                                                      // Initiate pins and registeries
  driver[num].rms_current(Joint[num].motor_max_current * 0.85, Joint[num].hold_multiplier); // Set stepper current, second parameter is hold_multiplier
  driver[num].en_pwm_mode(1);                                                               // Enable extremely quiet stepping
  driver[num].toff(4);                                                                      // off time
  driver[num].blank_time(24);                                                               // blank tim
  driver[num].pwm_autoscale(1);
  driver[num].microsteps(Joint[num].microstep); // What microstep range to use
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
  Serial.println(var0);

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

  if (Joint[num].over_temp_pre_warning)
  {
    Serial.println(F("Overtemp. Pre Warning")); // pre warning flag
  }

  if (Joint[num].over_temp_warning)
  {
    Serial.println(F("Overtemp."));
  }

  if (driver[num].diag0_error())
  {
    Serial.println(F("DIAG0 error"));
  }

#endif

  digitalWrite(Joint[num].SELECT, LOW);
}
