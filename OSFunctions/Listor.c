/**************************************************************************//**
 * @file     Listor.c
 * @brief    ART Real Time Micro Kernel Listor.c File
 * @version  V5
 * @date     3. March 2016
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

#include "listor.h"

list * create_list()
{
  list * mylist = (list *)calloc(1, sizeof(list));
  if (mylist == NULL) {
    return NULL;
  }
  mymem_count_alloc++;
  mylist->pHead = (listobj *)calloc(1, sizeof(listobj));
  if (mylist->pHead == NULL) {
    free(mylist);
    mymem_count_free++;  
    return NULL;
  }
  mymem_count_alloc++;
  mylist->pTail = (listobj *)calloc(1, sizeof(listobj));
  if (mylist->pTail == NULL) {
    free(mylist->pHead);
    free(mylist);
    mymem_count_free++;  
    mymem_count_free++;  
    return NULL;
  }
  mymem_count_alloc++;
  mylist->pHead->pPrevious = mylist->pHead;
  mylist->pHead->pNext = mylist->pTail;
  mylist->pTail->pPrevious = mylist->pHead;
  mylist->pTail->pNext = mylist->pTail;
  return mylist;
}

listobj *create_listobj(int num)
{
  listobj * myobj = (listobj *)calloc(1, sizeof(listobj));
  myobj->pTask 	= (TCB *)calloc(1, sizeof(TCB));
  if (myobj == NULL || myobj->pTask ==NULL)
  {
    return NULL;
  }
  myobj->nTCnt = num;
  return (myobj);
}

listobj *create_listobjRL(int num)
{
  listobj * myobj = (listobj *)calloc(1, sizeof(listobj));//+1
  myobj->pTask = (TCB *)calloc(1, sizeof(TCB));//+1
  if (myobj == NULL || myobj->pTask ==NULL)
  {
    return NULL;
  }
  //myobj->nTCnt = num;
  myobj->pTask->DeadLine = num;
  return (myobj);
}
//The element with the lowest value of TCB-> Deadline is first placed first in the list
void insertRL(list *list, listobj *obj) {
       
    listobj *templist = list->pHead;
    while (templist->pNext != list->pTail) {
        if (templist->pNext->pTask->DeadLine > obj->pTask->DeadLine) {
            break;
        }
        templist = templist->pNext;
    }
    obj->pPrevious = templist;
    obj->pNext = templist->pNext;
    templist->pNext->pPrevious = obj;
    templist->pNext = obj;
}

//• Extraction is always done from the front, i.e. at the "head-element". 
listobj *extractRL(list *list){
  listobj *obj;
  obj= list->pHead->pNext;
  if(list->pHead->pNext != list->pTail){ 
      list->pHead->pNext = list->pHead->pNext->pNext;
      list->pHead->pNext->pPrevious->pNext =NULL;
      list->pHead->pNext->pPrevious->pPrevious = NULL;
      list->pHead->pNext->pPrevious= list->pHead;
  }
  return obj;
}


//Add obj to list
void insertTL(list *list, listobj *obj) {
   
  // insert first in list 
  listobj *pTimertemp = list->pHead;
    while (pTimertemp->pNext != list->pTail) {
        if ((pTimertemp->pNext->nTCnt > obj->nTCnt)) {
            break;
        }
        pTimertemp = pTimertemp->pNext;
    }
    /* Position found, insert element */
    obj->pPrevious = pTimertemp;
    obj->pNext = pTimertemp->pNext;
    pTimertemp->pNext->pPrevious = obj;
    pTimertemp->pNext = obj;
}

//Extraction made by the use of a pointer to the list element, struct l_obj * pBlock
listobj *extractWL(list *list, struct l_obj * pBlock ){
    int found = 0;
    listobj *obj_founded;
    //listobj *obj = list->pHead->pNext;
    listobj *ptemplist = list->pHead;
    while((ptemplist->pNext != list->pTail) && (found!=1)){
        ptemplist = ptemplist->pNext;
        if(ptemplist == pBlock){
            found = 1;
            obj_founded = ptemplist;
        } 
    }
    
    if(found == 1){
          pBlock->pPrevious->pNext = pBlock->pNext;
          pBlock->pNext->pPrevious = pBlock->pPrevious;
          pBlock->pNext = NULL;
          pBlock->pPrevious = NULL;
    }
    return obj_founded;
}


//MialBox fuctions
mailbox* create_mailB()
{
  mailbox *mailb_list = (mailbox*)calloc(1,sizeof(mailbox));
  if (mailb_list == NULL) {
    return NULL;
  }
  mymem_count_alloc++;
  mailb_list->pHead = (msg*)calloc(1, sizeof(msg));
  if (mailb_list->pHead == NULL) {
    free(mailb_list);
    mymem_count_free++;  
    return NULL;
  }
  mymem_count_alloc++;
  mailb_list->pTail = (msg *)calloc(1, sizeof(msg));
  if (mailb_list->pTail == NULL) {
    free(mailb_list->pHead);
    mymem_count_free++; 
    free(mailb_list);
    mymem_count_free++; 
    return NULL;
  }
  mymem_count_alloc++;
  mailb_list->pHead->pPrevious = mailb_list->pHead;
  mailb_list->pHead->pNext = mailb_list->pTail;
  mailb_list->pTail->pPrevious = mailb_list->pHead;
  mailb_list->pTail->pNext = mailb_list->pTail;
  return mailb_list;
}


void insertMB(mailbox *list, msg *obj){

    // insert first in list 
    msg *ptemp = list->pHead;
    // Position found, insert element 
    obj->pPrevious = ptemp;
    obj->pNext = ptemp->pNext;
    ptemp->pNext->pPrevious = obj;
    ptemp->pNext = obj;

}

//MSG
msg * createMsg(){ 
  msg *msg_Obj = (msg*)calloc(1,sizeof(msg));
  msg_Obj->pBlock = (listobj*)calloc(1,sizeof(listobj));
  if (msg_Obj== NULL || msg_Obj->pBlock==NULL) {
    return FAIL;
  }
  mymem_count_alloc++;
  mymem_count_alloc++;
  return msg_Obj;
}

/** \brief  Remove receiving tasks Message

    If receiving Task is waiting then remove receiving task message struct from the mailbox 

    \param [in]      nMsg : a pointer to the specified Mailbox
    \return          none
 */
void remove_MBoxmsg(msg *nMsg){
  nMsg->pPrevious->pNext = nMsg->pNext;
  nMsg->pNext->pPrevious = nMsg->pPrevious;
  nMsg->pNext=nMsg->pPrevious=NULL;
  free(nMsg->pBlock);
  mymem_count_free++;
  free(nMsg);
  mymem_count_free++; 
}

void remove_msgRL(list * RL){

    RL->pHead->pNext->pMessage->pPrevious->pNext = RL->pHead->pNext->pMessage->pNext; //REMOVE MSG
    RL->pHead->pNext->pMessage->pNext->pPrevious = RL->pHead->pNext->pMessage->pPrevious;
    RL->pHead->pNext->pMessage->pNext = NULL;
    RL->pHead->pNext->pMessage->pPrevious = NULL;
    RL->pHead->pNext->pMessage->pBlock = NULL;
    free(RL->pHead->pNext->pMessage->pData);
    free(RL->pHead->pNext->pMessage->pBlock);
    mymem_count_free++;
    free(RL->pHead->pNext->pMessage);
    mymem_count_free++;
    RL->pHead->pNext->pMessage = NULL;
  //free(nMsg);
}


/** \brief  Remove the oldest Message struct

    If mailbox is full then remove the oldest Message struct 

    \param [in]      nMsg : a pointer to the specified Mailbox
    \return          none
 */
void remove_OldMsg(mailbox *mBox){
  msg *msg_Obj = mBox->pTail->pPrevious;
  
  mBox->pTail->pPrevious = mBox->pTail->pPrevious->pPrevious;
  mBox->pTail->pPrevious->pNext= mBox->pTail->pPrevious->pNext->pNext;
  msg_Obj->pNext = msg_Obj->pPrevious=NULL;
  free(msg_Obj->pBlock);
  mymem_count_free++;
  free(msg_Obj);
  mymem_count_free++;
}


void remove_list(list * xList){
  if(xList->pHead->pNext->pNext != NULL)
  {
      return;
  }
  else{
    xList->pTail->pPrevious = NULL;
    xList->pHead->pNext = NULL;
    free(xList->pHead);
    free(xList->pTail);
    free(xList);
    mymem_count_free++;  
    mymem_count_free++;  
    mymem_count_free++;  
  }
}