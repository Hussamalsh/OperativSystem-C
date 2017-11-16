/**************************************************************************//**
 * @file     main.c
 * @brief    ART Real Time Micro Kernel main c File
 * @version  v6
 * @date     6. Mars 2016
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

#include "kernel.h"



/*******************************************************************************
*                 ART Real Time Micro Kernel by Hussam
******************************************************************************/
mailbox *mb;
void task1();
int main(void)
{
  if (init_kernel() != OK ) {
    /* Memory allocation problems */
    while(1);
  }
  
  if (create_task(task1, 2000 ) != OK ) {
    /* Memory allocation problems */
    while(1);
  }
  
  if ((mb=create_mailbox(1,sizeof(int))) == NULL){
    /* Memory allocation problems */
    while(1);
  }
  
  run();
  
  return 1;
}

void task1(){
  
  
  terminate();
  
}

