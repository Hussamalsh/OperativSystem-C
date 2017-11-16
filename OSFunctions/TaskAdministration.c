/**************************************************************************//**
 * @file     TaskAdministration.c
 * @brief    ART Real Time Micro Kernel TaskAdministration.c File
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

// Function prototypes

/*******************************************************************************
*                 Task administration
******************************************************************************/

#include "TaskAdministration.h"


/*********************************************************/
/** Global variabels and definitions                     */
/*********************************************************/

list  *timmerL;   	/**< define timmerL Variable of type list. */
list  *waitingL;        /**< define waitingL Variable of type list. */
list  *readyL;          /**< define readyL Variable of type list. */
uint  kernelMode;       /**< define kernel start up mode Variable  . */    
TCB   *Running;         /**< define Running Variable of type TCB  . */ 
uint  TC;               /**< define TC (no_of_ticks) Variable  . */ 
int mymem_count_alloc;  /**< define mymem_count_alloc Variable to see allocated memory . */ 
int mymem_count_free;   /**< define mymem_count_free Variable to see free memory . */ 

/** \brief  Update the running pointer

    This function keep the running pointer up to date by uppdating it as soon as
    the readylist was modified 

    \param [in]      none
    \return          none
*/
void uppdateRunning(){
 Running = readyL->pHead->pNext->pTask;
}



/** \brief  initializes the kernel 

  This function initializes the kernel and its data structures and leaves
  the kernel in start-up mode. The init_kernel call must be made before any 
  other call is made to the kernel.

\param [in]         none
\return             FAIL/OK.  Int: Description of the functions status
*/
exception init_kernel(void){
  if(kernelMode==RUNNING)  //return fail if the kernal is already running.
    return FAIL;
  set_ticks(0);			//1-Set tick counter to zero
  timmerL=create_list(); 		//2-Create necessary data structures
  waitingL=create_list();
  readyL=create_list();
  void (*pIdle)(void) = &Idle;	//3-Create an idle task
  uint status = create_task(pIdle,5000 );//INT_MAX
  kernelMode =INIT;		//4-Set the kernel in start up mode
  if(timmerL == NULL || waitingL == NULL ||  readyL == NULL || status == FAIL){
    free(timmerL->pHead);
    free(timmerL->pTail);
    free(timmerL);
    free(waitingL->pHead);
    free(waitingL->pTail);
    free(waitingL);
    free(readyL->pHead);
    free(readyL->pTail);
    free(readyL);
    return FAIL; //5-Return status
  }
  
  return OK; //5-Return status
}

/** \brief  creates a task.

    This function creates a task. If the call is made in startup
    mode, i.e. the kernel is not running, only the
    necessary data structures will be created. However, if
    the call is made in running mode, it will lead to a
    rescheduling and possibly a context switch.

    \param [in]    *task_body   A pointer to the C function holding the code of the task.
		            deadline	The kernel will try to schedule the task so it will meet this deadline
    \return         FAIL/OK.    Int: Description of the functionís status
 */
exception create_task(void(*task_body)(), uint deadline){
  volatile int firstExec = TRUE;
  if(!deadline || (* task_body)==NULL) {
    return FAIL;
  }
  //1-Allocate memory for TCB 
  listobj *pObj = create_listobjRL(deadline); //2-Set deadline in TCB
  if(pObj==NULL){
    return FAIL;
  }
  mymem_count_alloc++;    //created memory for listobj
  mymem_count_alloc++;    //created memory for TCB
  pObj->pTask->PC =task_body;   //3-Set the TCBís PC to point to the task body
  pObj->pTask->SP= &(pObj->pTask->StackSeg[STACK_SIZE-1]);//4-Set TCBís SP to point to the stack segment     //newTCB->SP =  &(newTCB->StackSeg[STACK_SIZE-1]);
  pObj->pTask->SPSR = 0;
  if(kernelMode ==INIT){	//5-IF start-up mode THEN 
    insertRL(readyL, pObj); //6-Insert new task in Readylist
    uppdateRunning();
    return OK;//7-Return status
  }//ELSE
  else{
    set_isr(isr_off); //isr_off();	   //8-Disable interrupts
    SaveContext();  //9-Save context
    if(firstExec){//10-IF ìfirst executionî THEN
      firstExec=FALSE;//11-Set: ìnot first execution any moreî
      insertRL(readyL, pObj);//12-Insert new task in Readylist
      uppdateRunning();
      LoadContext();//13-Load context
    }//ENDIF
  }//ENDIF
  return OK;	//14-Return status
}

/** \brief  starts the kernel 

    This function starts the kernel and thus the system of
    created tasks. Since the call will start the kernel it will
    leave control to the task with tightest deadline.
    Therefore, it must be placed last in the application
    initialization code. After this call the system will be in
    running mode.

    \param [in]    none
    \return        none
 */
void run( void ){
	timer0_start();      //1-Initialize interrupt timer
	kernelMode=RUNNING;   //2-Set the kernel in running mode
	isr_on();	      //3-Enable interrupts
	LoadContext();	      //4-Load context
}

/** \brief  terminate the running task

    This call will terminate the running task. All data
    structures for the task will be removed. Thereafter,
    another task will be scheduled for execution.

    \param [in]    none
    \return        none
*/
void terminate( void ){
  //1-Remove running task from Readylist
  listobj *temp_obj=extractRL(readyL); 
  free(temp_obj->pTask);
  mymem_count_free++;  
  //free(temp_obj->pMessage);
  free(temp_obj);
  mymem_count_free++;
  uppdateRunning();//2-Set next task to be the running task
  LoadContext();	//3-Load context
}


