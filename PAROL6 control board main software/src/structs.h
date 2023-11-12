/** @file common.h
    @brief A Documented file.

    Here declare structs for the stuff that will change all the time and is called by multiple instances
    Ex: motor coms struct --> stuff that we receive from USB
    For schematic please refer to:
    For additional pin functionality refer to:

*/

#ifndef STRUCTS
#define STRUCTS

/// Put all declarations of pins and stuff like that that is specific to the motors
/// That way they can all be addressed thru indexing?
struct MotorStruct
{

  /// @brief maximum motor current. Rated as RMS current by stepper manufacturer
  int motor_max_current = 100;
  /// @brief  Speed at what motor homes
  int homing_speed = 100;

  /// @brief Max speed of the motor
  int motor_max_speed = 0;
  /// @brief maximum motor acceleration
  int motor_max_acceleration = 0;
  /// @brief minimum motor acceleration
  int motor_min_speed = 0;
  /// @brief  minimum motor acceleration
  int motor_min_acceleration = 0;

  /// @brief robot joint range in positive direction (in radians)
  float joint_range_positive;
  /// @brief robot joint range in negative direction (in radians)
  float joint_range_negative;

  /// @brief robot joint range in positive direction in steps (changes with microstep setting)
  float joint_range_positive_steps;
  /// @brief robot joint range in negative direction (changes with microstep setting)
  float joint_range_negative_steps;

  /// @brief standby position of the robot
  int standby_position;
  /// @brief how much steeps need to be made from when limit is hit to standby position
  float homed_position;
  /// @brief is the joint homed or not? homed is when full procedure is done
  int homed;
  /// @brief is the joint currently homing?
  int homing;
  /// @brief  stage 1 of homing
  int homing_stage_1 = 0;
  /// @brief  stage 2 of homing
  int homing_stage_2 = 0;

  /// @brief temperature error flag
  int temperature_error;
  /// @brief temperature warrning flag
  int temperature_warrning;

  /// @brief microstep value used
  int microstep;
  /// @brief at what state is limit switch when triggered
  int limit_switch_trigger;

  /// @brief reduction ratio used in joint
  float reduction_ratio;

  /// @brief current position of the joint
  long position;
  /// @brief current speed of the joint
  int speed;
  /// @brief current current of the joint
  int current;

  /// @brief  limit switch pin of the joint
  int LIMIT;
  /// @brief  direction pin of the joint
  int DIR;
  /// @brief step pin of the joint
  int STEP;
  /// @brief SPI select pin of the joint
  int SELECT;
  /// @brief what driver is controlling this motor
  int driver_chip_on_board;
  /// @brief is the robot moving in the right direction?
  int direction_reversed;

  /// @brief scale factor for holding current based on set rms current
  /// max value is 31 but of the available current defined by sense resistor in driver datasheet
  int ihold;
  /// @brief scale factor for running current based on set rms current
  /// max value is 31 but of the available current defined by sense resistor in driver datasheet
  int irun;
  /// @brief sets how much curret is scaled when motors are holding position (should be čles then 1, usually 0.95)
  float hold_multiplier;

  /// @brief pre over temperature warning, recomend to reduce current or somehow cool the drivers
  int over_temp_pre_warning;
  /// @brief over temperature warning, drivers get disabled. (should never happen, dangerous)
  int over_temp_warning;
  /// @brief TODO check this
  int diag0;
  /// @brief nothing connected on phase B
  int open_load_B;
  /// @brief nothing connected on phase A
  int open_load_A;
  /// @brief Short on phase B
  int short_2_gnd_B;
  /// @brief Short on phase A
  int short_2_gnd_A;

  int commaned_mode;
  int commanded_position;
  int commanded_velocity;
  int commanded_current;

  /// @brief Robot tried to go to position it is not allowed to go
  int position_error = 0;

  int error;
};

/// @brief general robot data
struct Robot
{

  int CRC_value = 212;  
  int command;
  int affected_joint;
  int commanded_OUT1;
  int commanded_OUT2;
  int In1;
  int In2;
  int Out1;
  int Out2;
  int Estop;
  unsigned int time_between_commands = 0;
  int Out1_commanded;
  int Out2_commanded;
  int xtr2_byte = 8;
  /// @brief  Timeout represents time passed between 2 commands sent from PC
  /// 255 = 255ms, If we set it to 200 and 200ms passes between 2 commands 
  /// robot goes to timeout error. If set to 0 it is disabled
  /// it is sent from the PC to the robot to set it
  int Timeout = 0;
  /// @brief  Represents if the robots timeout is triggered or not
  /// Robot sends this 
  int timeout_error = 100; 
  int Affected_joint;
  int disabled = 0;


};

/// @brief All commanded and current data for the gripper
struct Gripper
{
  /// @brief data that is being sent from the gripper to the robot
  int current_position = -100;
  int current_speed = 2000;
  int current_current = -3000;
  int current_status = 123;
  int object_detection = 69;
  int Gripper_ID = 255;

  /// @brief data that is being sent from the robot to the gripper
  int commanded_position;
  int commanded_speed;
  int commanded_current;
  int command;
  int mode;
  int commanded_ID;
};

struct data_send
{
};

struct CommsStruct
{
};

#endif