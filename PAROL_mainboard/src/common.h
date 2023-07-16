/** @file common.h
    @brief A Documented file.
    
    Here declare existance of global variables
    Define them in main.cpp --> ex volatile unsignet tick_1ms
    For schematic please refer to:
    For additional pin functionality refer to:

*/

#ifndef COMMON_H
#define COMMON_H


extern volatile unsigned reset_cnt;
extern volatile unsigned tick_count;
extern volatile unsigned tick_1ms;
extern volatile unsigned tick_10ms;
extern volatile unsigned tick_100ms;


#endif
