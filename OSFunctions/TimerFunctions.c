/**************************************************************************//**
 * @file     TimerFunctions.c
 * @brief    ART Real Time Micro Kernel TimerFunctions.c File
 * @version  V6
 * @date     6. March 2016
 *
 * @note
 * Copyright (C) 2016-2017 Hussam Alshammari. All rights reserved.
 *
 * @par
 * Hussam Alshammari is supplying this software for use with Cortex-M
 * processor based microcontrollers.  This file have Task administration
 * Inter-Process Communication and Timing functions that will work with ARM based processors.
 *
 *
 ******************************************************************************/
 
/*******************************************************************************
*                 Timing functions
******************************************************************************/

#include "TimerFunctions.h"

/** \brief  block task 

    This call will block the calling task until the given
    number of ticks has expired.

    \param [in]    nTicks:  the duration given in number of ticks
    \return        OK: Normal function, no exception occurred.
    \return        DEADLINE_REACHED: This return parameter is given if the receiving
                   tasksí deadline is reached while it is blocked by the receive_wait call.
 */
exception wait( uint nTicks){
  volatile int firstExec = TRUE;
  int x;
  exception status = OK;
  x= set_isr(isr_off); //1-Disable interrupt
  SaveContext(); //2-Save context
  if(firstExec){//3-IF first execution THEN
    firstExec=FALSE;//4-Set: ìnot first execution any more
    readyL->pHead->pNext->nTCnt = TC+nTicks;
    insertTL(timmerL, extractRL(readyL)); //5-Place running task in the Timerlist
    uppdateRunning();
    LoadContext();//6-Load context
  }//ELSE
  else{
    if(Running->DeadLine<=TC){//7-IF deadline is reached 
      status=DEADLINE_REACHED;//8-THEN Status is DEADLINE_REACHED
    }//ELSE
    else{
      status = OK;//9-Status is OK
    }//ENDIF
  }//ENDIF
  //9-Return status
  set_isr(x);
  return status;
}

/** \brief  set the TC

    This call will set the tick counter to the given value.

    \param [in]    nTicks: the new value of the tick counter
    \return        none

 */
void set_ticks( uint no_of_ticks ){
     TC=no_of_ticks;  //Set the tick counter.
}

/** \brief  return the TC

    This call will return the current value of the tick counter

    \param [in]    none
    \return        TC: A 32 bit value of the tick counter

 */
uint ticks( void ){
    return TC;//Return the tick counter
}

/** \brief  return the task deadline

    This call will return the deadline of the specified task

    \param [in]    none
    \return        DeadLine: the deadline of the given task

 */
uint deadline( void ){
    return Running->DeadLine; //Return the deadline of the current task
}

/** \brief  seting new deadline for the task

    This call will set the deadline for the calling task. The
    task will be rescheduled and a context switch might occur.

    \param [in]    nDeadline: the new deadline given in number of ticks.
    \return        none


 */
void set_deadline( uint nDeadline ){
     volatile int firstExec = TRUE;
     set_isr(isr_off); //Disable interrupt
     SaveContext(); //Save context
     if(firstExec){//IF ìfirst executionî THEN
       firstExec=FALSE;//Set: ìnot first execution any moreî
       Running->DeadLine = nDeadline; //Set the deadline field in the calling TCB.
       insertRL(readyL, extractRL(readyL));//Reschedule Readylist
       uppdateRunning();
       LoadContext();//Load context
     }//ENDIF
}



/** \brief  Interrupt Service Routine

    This function is not available for the user to call.
    It is called by an ISR (Interrupt Service Routine)
    invoked every tick. Note, context is automatically saved
    prior to call and automatically loaded on function exit.

    \param [in]      none
    \return          none
 */
void TimerInt(void)
{
  TC++;//Increment tick counter
  //Check the Timerlist for tasks that are ready for execution, move these to Readylist
  //pekare
  listobj *pTobj = timmerL->pHead;
  while(pTobj->pNext != timmerL->pTail){
    if(pTobj->pNext->nTCnt<=TC){
      insertRL(readyL,extractRL(timmerL));//timmerL->pHead->pNext
      uppdateRunning();
    }
    pTobj=pTobj->pNext;
  }
  //Check the Waitinglist for tasks that have expired deadlines,
  //move these to Readylist and clean up their Mailbox entry.
  listobj *pWobj = waitingL->pHead;
  while(pWobj->pNext != waitingL->pTail){
    if(pWobj->pNext->pTask->DeadLine <=TC){
      insertRL(readyL,extractWL(waitingL,waitingL->pHead->pNext));  //waitingL->pHead->pNext
      uppdateRunning();
    }
    pWobj=pWobj->pNext;
  }
}
/** \brief  idle task

    This function let the task stay in while loop untill its something happen

    \param [in]      none
    \return          none
 */
void Idle(void){
    while(1){
       /* SaveContext();
        TimerInt();
        LoadContext();*/
  }
}