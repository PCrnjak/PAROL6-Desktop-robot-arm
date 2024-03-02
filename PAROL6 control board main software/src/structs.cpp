#include "structs.h"
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


/// Global structure declaration
Gripper Comp_gripper;
