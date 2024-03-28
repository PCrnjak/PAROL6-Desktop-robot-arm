/// HERE INIT STRUCTURES??
#include "motor_init.h"

void Init_motor_struct(struct MotorStruct *Joint__)
{

    Joint__->LIMIT = 200;
}

void Init_Joint_1(struct MotorStruct *Joint__)
{
    // J1 is special because we usually go to the position that 
    // is +90 deg from its Denevit Hartenberg standby positon 
    // + 90 deg is equal to 9600 steps in 32microstep range
    Joint__->standby_position = 10240; // 10240   // 9600
    Joint__->homed_position = 13500; // 13653  // 13500
    Joint__->motor_max_current = MOTOR1_MAX_CURRENT;
    Joint__->motor_max_speed = 0;
    Joint__->motor_max_acceleration = 0;
    Joint__->motor_min_speed = 0;
    Joint__->motor_min_acceleration = 0;
    Joint__->joint_range_positive = 0;
    Joint__->joint_range_negative = 0;
    Joint__->joint_range_positive_steps = 14000;
    Joint__->joint_range_negative_steps = -14000;
    Joint__->homed = 0;
    Joint__->homing = 0;
    Joint__->temperature_error = 0;
    Joint__->temperature_warrning = 0;
    Joint__->microstep = MICROSTEP;
    Joint__->limit_switch_trigger = 0; // temp since sensor is faulty?
    Joint__->reduction_ratio = 6.4; // 96 / 15
    Joint__->position = -200;
    Joint__->speed = -100;
    Joint__->current = 0;
    Joint__->LIMIT = LIMIT6;
    Joint__->DIR = DIR1;
    Joint__->STEP = PUL1;
    Joint__->SELECT = SELECT1;
    Joint__->driver_chip_on_board = 0;
    Joint__->direction_reversed = 1;
    Joint__->irun = 16;
    Joint__->ihold = 16;
    Joint__->hold_multiplier = 0.7;
    Joint__->over_temp_pre_warning = 0;
    Joint__->over_temp_warning = 0;
    Joint__->diag0 = 0;
    Joint__->open_load_B = 0;
    Joint__->open_load_A = 0;
    Joint__->short_2_gnd_B = 0;
    Joint__->short_2_gnd_A = 0;
    Joint__->position_error = 0;
}

void Init_Joint_2(struct MotorStruct *Joint__)
{
    Joint__->standby_position = -32000; 
    Joint__->homed_position = 19588;
    Joint__->motor_max_current = MOTOR2_MAX_CURRENT;
    Joint__->motor_max_speed = 0;
    Joint__->motor_max_acceleration = 0;
    Joint__->motor_min_speed = 0;
    Joint__->motor_min_acceleration = 0;
    Joint__->joint_range_positive = 0;
    Joint__->joint_range_negative = 0;
    Joint__->joint_range_positive_steps = -1200;
    Joint__->joint_range_negative_steps = -51587;
    Joint__->homed = 0;
    Joint__->homing = 0;
    Joint__->temperature_error = 0;
    Joint__->temperature_warrning = 0;
    Joint__->microstep = MICROSTEP;
    Joint__->limit_switch_trigger = 1;
    Joint__->reduction_ratio = 20;
    Joint__->position = 0;
    Joint__->speed = 0;
    Joint__->current = 0;
    Joint__->LIMIT = LIMIT2;
    Joint__->DIR = DIR6;
    Joint__->STEP = PUL6;
    Joint__->SELECT = SELECT6;
    Joint__->driver_chip_on_board = 0;
    Joint__->direction_reversed = 0;
    Joint__->irun = 16;
    Joint__->ihold = 16;
    Joint__->hold_multiplier = 0.8;
    Joint__->over_temp_pre_warning = 0;
    Joint__->over_temp_warning = 0;
    Joint__->diag0 = 0;
    Joint__->open_load_B = 0;
    Joint__->open_load_A = 0;
    Joint__->short_2_gnd_B = 0;
    Joint__->short_2_gnd_A = 0;
    Joint__->position_error = 0;
}

void Init_Joint_3(struct MotorStruct *Joint__)
{
    Joint__->standby_position = 57905;
    Joint__->homed_position = 23020;
    Joint__->motor_max_current = MOTOR3_MAX_CURRENT;
    Joint__->motor_max_speed = 0;
    Joint__->motor_max_acceleration = 0;
    Joint__->motor_min_speed = 0;
    Joint__->motor_min_acceleration = 0;
    Joint__->joint_range_positive = 0;
    Joint__->joint_range_negative = 0;
    Joint__->joint_range_positive_steps = 92605;
    Joint__->joint_range_negative_steps = 34700;
    Joint__->homed = 0;
    Joint__->homing = 0;
    Joint__->temperature_error = 0;
    Joint__->temperature_warrning = 0;
    Joint__->microstep = MICROSTEP;
    Joint__->limit_switch_trigger = 1;
    Joint__->reduction_ratio = 18.0952381; // 20 * (38 / 42)
    Joint__->position = 0;
    Joint__->speed = 0;
    Joint__->current = 0;
    Joint__->LIMIT = LIMIT3;
    Joint__->DIR = DIR5;
    Joint__->STEP = PUL5;
    Joint__->SELECT = SELECT5;
    Joint__->driver_chip_on_board = 0;
    Joint__->direction_reversed = 1;
    Joint__->irun = 16;
    Joint__->ihold = 16;
    Joint__->hold_multiplier = 0.8;
    Joint__->over_temp_pre_warning = 0;
    Joint__->over_temp_warning = 0;
    Joint__->diag0 = 0;
    Joint__->open_load_B = 0;
    Joint__->open_load_A = 0;
    Joint__->short_2_gnd_B = 0;
    Joint__->short_2_gnd_A = 0;
    Joint__->position_error = 0;
}

void Init_Joint_4(struct MotorStruct *Joint__)
{

    Joint__->standby_position = 0;
    Joint__->homed_position = -10200;
    Joint__->motor_max_current = MOTOR4_MAX_CURRENT;
    Joint__->motor_max_speed = 0;
    Joint__->motor_max_acceleration = 0;
    Joint__->motor_min_speed = 0;
    Joint__->motor_min_acceleration = 0;
    Joint__->joint_range_positive = 0;
    Joint__->joint_range_negative = 0;
    Joint__->joint_range_positive_steps = 7500;
    Joint__->joint_range_negative_steps = -7500;
    Joint__->homed = 0;
    Joint__->homing = 0;
    Joint__->temperature_error = 0;
    Joint__->temperature_warrning = 0;
    Joint__->microstep = MICROSTEP;
    Joint__->limit_switch_trigger = 0;
    Joint__->reduction_ratio = 4;
    Joint__->position = 0;
    Joint__->speed = 0;
    Joint__->current = 0;
    Joint__->LIMIT = LIMIT4;
    Joint__->DIR = DIR4;
    Joint__->STEP = PUL4;
    Joint__->SELECT = SELECT4;
    Joint__->driver_chip_on_board = 0;
    Joint__->direction_reversed = 0;
    Joint__->irun = 16;
    Joint__->ihold = 16;
    Joint__->hold_multiplier = 0.8;
    Joint__->over_temp_pre_warning = 0;
    Joint__->over_temp_warning = 0;
    Joint__->diag0 = 0;
    Joint__->open_load_B = 0;
    Joint__->open_load_A = 0;
    Joint__->short_2_gnd_B = 0;
    Joint__->short_2_gnd_A = 0;
    Joint__->position_error = 0;
}

void Init_Joint_5(struct MotorStruct *Joint__)
{
    Joint__->standby_position = 0;
    Joint__->homed_position = 8900;
    Joint__->motor_max_current = MOTOR5_MAX_CURRENT;
    Joint__->motor_max_speed = 0;
    Joint__->motor_max_acceleration = 0;
    Joint__->motor_min_speed = 0;
    Joint__->motor_min_acceleration = 0;
    Joint__->joint_range_positive = 0;
    Joint__->joint_range_negative = 0;
    Joint__->joint_range_positive_steps = 6400;
    Joint__->joint_range_negative_steps = -6400;
    Joint__->homed = 0;
    Joint__->homing = 0;
    Joint__->temperature_error = 0;
    Joint__->temperature_warrning = 0;
    Joint__->microstep = MICROSTEP;
    Joint__->limit_switch_trigger = 1;
    Joint__->reduction_ratio = 4;
    Joint__->position = 0;
    Joint__->speed = 0;
    Joint__->current = 0;
    Joint__->LIMIT = LIMIT5;
    Joint__->DIR = DIR2;
    Joint__->STEP = PUL2;
    Joint__->SELECT = SELECT2;
    Joint__->driver_chip_on_board = 0;
    Joint__->direction_reversed = 0;
    Joint__->irun = 16;
    Joint__->ihold = 16;
    Joint__->hold_multiplier = 0.8;
    Joint__->over_temp_pre_warning = 0;
    Joint__->over_temp_warning = 0;
    Joint__->diag0 = 0;
    Joint__->open_load_B = 0;
    Joint__->open_load_A = 0;
    Joint__->short_2_gnd_B = 0;
    Joint__->short_2_gnd_A = 0;
    Joint__->position_error = 0;
}

void Init_Joint_6(struct MotorStruct *Joint__)
{
    Joint__->standby_position = 32000;
    Joint__->homed_position = 15900;
    Joint__->motor_max_current = MOTOR6_MAX_CURRENT;
    Joint__->motor_max_speed = 0;
    Joint__->motor_max_acceleration = 0;
    Joint__->motor_min_speed = 0;
    Joint__->motor_min_acceleration = 0;
    Joint__->joint_range_positive = 0;
    Joint__->joint_range_negative = 0;
    Joint__->joint_range_positive_steps = 64000;
    Joint__->joint_range_negative_steps = 0;
    Joint__->homed = 0;
    Joint__->homing = 0;
    Joint__->temperature_error = 0;
    Joint__->temperature_warrning = 0;
    Joint__->microstep = MICROSTEP;
    Joint__->limit_switch_trigger = 0;
    Joint__->reduction_ratio = 10;
    Joint__->position = 0;
    Joint__->speed = 0;
    Joint__->current = 0;
    Joint__->LIMIT = LIMIT1;
    Joint__->DIR = DIR3;
    Joint__->STEP = PUL3;
    Joint__->SELECT = SELECT3;
    Joint__->driver_chip_on_board = 0;
    Joint__->direction_reversed = 1;
    Joint__->irun = 16;
    Joint__->ihold = 16;
    Joint__->hold_multiplier = 0.88;
    Joint__->over_temp_pre_warning = 0;
    Joint__->over_temp_warning = 0;
    Joint__->diag0 = 0;
    Joint__->open_load_B = 0;
    Joint__->open_load_A = 0;
    Joint__->short_2_gnd_B = 0;
    Joint__->short_2_gnd_A = 0;
    Joint__->position_error = 0;
}