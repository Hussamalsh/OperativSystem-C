/**
 * @file TimerFunctions.h 
 * @author Hussam Alshammari
 * @date 3 mar 2016
 * @brief File containing all timer functions and variables.
 *
 * This header file have the important functions to run the program
 */
 
#ifndef Timer_H
#define Timer_H
#include "kernel.h"
#include "Listor.h"
#include "TaskAdministration.h"

exception wait(uint nTicks);
void set_ticks(uint no_of_ticks);
uint ticks(void);
uint deadline(void);
void set_deadline(uint nDeadline);

void TimerInt(void);
void Idle(void);


#endif

