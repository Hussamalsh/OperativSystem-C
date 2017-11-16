/**
 * @file TaskAdministration.h 
 * @author Hussam Alshammari
 * @date 6 mars 2016
 * @brief File containing all task administration functions and variables.
 *
 * This header file have the important functions to run the program
 */

#ifndef Task_H
#define Task_H

#include "kernel.h"
#include "Listor.h"
#include "TimerFunctions.h"

/*******************************************************************************
*                 Task administration Header
******************************************************************************/
	
extern list  *timmerL;  /**< define timmerL Variable of type list. */
extern list  *waitingL; /**< define waitingL Variable of type list. */
extern list  *readyL;   /**< define readyL Variable of type list. */
extern uint kernelMode; /**< define kernel start up mode Variable  . */       
extern TCB * Running;   /**< define Running Variable of type TCB  . */ 
extern uint TC;         /**< define TC (no_of_ticks) Variable  . */ 


void uppdateRunning();
exception init_kernel(void);
exception create_task(void(*task_body)(), uint deadline);
void run(void);
void terminate(void);


#endif

