/** @file motor_init.h
    @brief A Documented file.
    
    Header file of motor initialization.
    For schematic please refer to:
    For additional pin functionality refer to:

*/

#ifndef MOTOR_INIT
#define MOTOR_INIT

#include "structs.h"
#include "hw_init.h"
#include "common.h"
#include "iodefs.h"
#include "constants.h"
/// HERE INIT STRUCTURES??


void Init_motor_struct(struct MotorStruct *Joint__);
void Init_Joint_1(struct MotorStruct *Joint__);
void Init_Joint_2(struct MotorStruct *Joint__);
void Init_Joint_3(struct MotorStruct *Joint__);
void Init_Joint_4(struct MotorStruct *Joint__);
void Init_Joint_5(struct MotorStruct *Joint__);
void Init_Joint_6(struct MotorStruct *Joint__);


#endif