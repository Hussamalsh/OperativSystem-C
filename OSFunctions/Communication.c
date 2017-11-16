/*******************************************************************************
 *                 Inter-Process Communication
 ******************************************************************************/

#include "Communication.h"


/** \brief  create a Mailbox

    This call will create a Mailbox. The Mailbox is a FIFO
    communication structure used for asynchronous and
    synchronous communication between tasks.

    \param [in]    nof_msg: Maximum number of Messages the Mailbox can hold.
    \param [in]    Size_of msg: The size of one Message in the Mailbox.
    \return        Mailbox*: a pointer to the created mailbox or NULL.
 */
mailbox* create_mailbox(uint nof_msg, uint size_of_msg)
{
  //Allocate memory for the Mailbox
  mailbox *mailb_obj = create_mailB();
  if(mailb_obj == NULL){
    return FAIL;
  }
  //Initialize Mailbox structure
  mailb_obj->nMaxMessages = nof_msg;
  mailb_obj->nDataSize= size_of_msg;
  return mailb_obj;//Return Mailbox*
}

/** \brief  remove the Mailbox 

    This call will remove the Mailbox if it is empty and return
    OK. Otherwise no action is taken and the call will return
    NOT_EMPTY.

    \param [in]    Mailbox*: A pointer to the Mailbox to be removed.
    \return        OK: The mailbox was removed
    \return        NOT_EMPTY: The mailbox was not removed because it was not empty.
 */
   
exception remove_mailbox( mailbox* mBox ){
  if(mBox->pHead->pNext == mBox->pTail){//IF Mailbox is empty THEN  (NOT_EMPTY =0)
    free(mBox->pHead);//Free the memory for the Mailbox
    mymem_count_free++; 
    free(mBox->pTail);//Free the memory for the Mailbox
    mymem_count_free++; 
    free(mBox);//Free the memory for the Mailbox
    mymem_count_free++; 
    return OK;//Return OK
  }//ELSE
  else{
    return NOT_EMPTY; //Return NOT_EMPTY
  }//ENDIF
}

/** \brief  Return number of messages in MB

    This call will return number of messages in the MailBox

    \param [in]    Mailbox*: A pointer to the Mailbox.
    \return        nMessages: number of the message that Mailbox hold
 */
int no_messages( mailbox* mBox ){
  return mBox->nMessages;
}

/** \brief  send a Message to the Mailbox 

  This call will send a Message to the specified Mailbox.
  If there is a receiving task waiting for a Message on the
  specified Mailbox, send_wait will deliver it and the
  receiving task will be moved to the Readylist.
  Otherwise, if there is not a receiving task waiting for a
  Message on the specified Mailbox, the sending task will
  be blocked. In both cases (blocked or not blocked) a
  new task schedule is done and possibly a context
  switch. During the blocking period of the task its
  deadline might be reached. At that point in time the
  blocked task will be resumed with the exception:
  DEADLINE_REACHED. Note: send_wait and
  send_no_wait Messages shall not be mixed in the
  same Mailbox.

  \param [in]    Mailbox*: A pointer to the Mailbox.
  \param [in]    *Data: a pointer to a memory area where the data of
                        the communicated Message is residing.
  \return        OK: Normal behavior, no exception occurred.
  \return        DEADLINE_REACHED: This return parameter is given if the 
  sending tasksí deadline is reached while it is blocked by
  the send_wait call.
*/
exception send_wait( mailbox *mBox, void* pData ){//recieve -
  volatile int firstExec = TRUE;
  int x = set_isr(isr_off); //Disable interrupt
  SaveContext(); //Save context
  if(firstExec){//IF ìfirst executionî THEN
    firstExec=FALSE;//Set: ìnot first execution any moreî
    if(mBox->nMessages<0 /*&& mBox->nBlockedMsg<0*/ ){//IF receiving task is waiting THEN
      //Copy senderís data to the data area of the receivers Message
      memcpy(mBox->pHead->pNext->pData,pData , mBox->nDataSize);//(DEST,SRS,SIZE)
      //str1(pData) -- This is pointer to the destination array where the content
      // is to be copied, type-casted to a pointer of type void*.
      //*Remove receiving taskís Message struct from the mailbox
      struct l_obj  *list_pobj = mBox->pHead->pNext->pBlock;
      remove_MBoxmsg(mBox->pHead->pNext);
      mBox->nMessages += SENDER; //+1
      mBox->nBlockedMsg += SENDER; //+1
      //Move receiving task to Readylist
      insertRL(readyL,extractWL(waitingL, list_pobj));
      uppdateRunning();
      
    }//ELSE
    else{
      if(mBox->nMessages > 0 && mBox->nBlockedMsg == 0 ) // return fail if there  are 
        return FAIL;                                 //send_no_wait in mailbox
      if(mBox->nMaxMessages == mBox->nMessages) //return fail if mailbox is full
        return FAIL;
      //Allocate a Message structure
      msg *msg_Obj = createMsg();
      if(msg_Obj==NULL){//return fail if the MSG_obj is not allocated
        return FAIL;
      }
      //Set data pointer
      msg_Obj->pData=pData;
      msg_Obj->pBlock = readyL->pHead->pNext;
      readyL->pHead->pNext->pMessage = msg_Obj;
      //Add Message to the Mailbox
      insertMB(mBox, msg_Obj);
      mBox->nMessages += SENDER;
      mBox->nBlockedMsg += SENDER; //+1
      //Move sending task from Readylist to Waitinglist
      insertRL(waitingL,extractRL(readyL));
      uppdateRunning();
    }//ENDIF
    LoadContext();//Load context
  }//ELSE
  else{
    if(Running->DeadLine<=TC){//IF deadline is reached THEN
      x = set_isr(isr_off); //Disable interrupt
      //Remove send Message       
      remove_msgRL(readyL);
      mBox->nMessages   += RECEIVER; //-1
      mBox->nBlockedMsg += RECEIVER; //-1
      set_isr(x);  //isr_on();      //Enable interrupt
      return DEADLINE_REACHED;//Return DEADLINE_REACHED
    }//ELSE
    else{
      return OK;//Return OK
    }//ENDIF
  }//ENDIF
  
}


/** \brief  receive a Message from Mailbox

      This call will attempt to receive a Message from the
      specified Mailbox. If there is a send_wait or a
      send_no_wait Message waiting for a receive_wait or a
      receive_no_wait Message on the specified Mailbox,
      receive_wait will collect it. If the Message was of
      send_wait type the sending task will be moved to the
      Readylist. Otherwise, if there is not a send Message (of
      either type) waiting on the specified Mailbox, the
      receiving task will be blocked. In both cases (blocked or
      not blocked) a new task schedule is done and possibly
      a context switch. During the blocking period of the task
      its deadline might be reached. At that point in time the
      blocked task will be resumed with the exception:
      DEADLINE_REACHED.

    \param [in]    *mBox: a pointer to the specified Mailbox.
    \param [in]    *Data: a pointer to a memory area where the data of
                               the communicated Message is to be stored.
    \return        OK: Normal function, no exception occurred.
    \return        DEADLINE_REACHED: This return parameteris given if the receiving
                  tasksí deadline is reached while it is blocked by the receive_waitcall.
 */                  //recieve                      //sendData
exception receive_wait( mailbox* mBox, void* pData ){
  volatile int firstExec = TRUE;
  int x = set_isr(isr_off); //Disable interrupt
  SaveContext(); //Save context
  if(firstExec){//IF ìfirst executionî THEN
    firstExec=FALSE;//Set: ìnot first execution any more
    if(mBox->nMessages>0 /*&& mBox->nBlockedMsg>0*/ ){//IF send Message is waiting THEN
      //Copy senderís data to receiving taskís data area
      memcpy(pData,mBox->pHead->pNext->pData , mBox->nDataSize);//(DEST,SRS(copyfrom),SIZE)
      //Remove sending taskís Message struct from the Mailbox
      void *pdata_temp = mBox->pHead->pNext->pData;
      // remove_MBoxmsg(mBox->pHead->pNext);
      //IF Message was of wait type THEN Move sending task to Ready list        (pblock?)
      int typewait=0;//if block
      
      if (mBox->pHead->pNext->pBlock->pMessage !=NULL && mBox-> nBlockedMsg != 0)  {
        typewait = 1;
        mBox->nMessages += RECEIVER; //-1
        mBox->nBlockedMsg += RECEIVER; //-1
        insertRL(readyL,extractWL(waitingL, mBox->pHead->pNext->pBlock));
        remove_MBoxmsg(mBox->pHead->pNext); //Remove Message struct
        uppdateRunning();
      }else{
        if(typewait==0){// if send_no_wait
          remove_MBoxmsg(mBox->pHead->pNext);
          free(pdata_temp);//Free senders data area
          mBox->nMessages+= RECEIVER;
        }
      }//ENDIF
    }//ELSE
    else{
      //Allocate a Message structure
      msg *msg_Obj = createMsg();
      
      msg_Obj->pData = pData; //
      msg_Obj->pBlock = readyL->pHead->pNext; //
      readyL->pHead->pNext->pMessage = msg_Obj;
      //Add Message to the Mailbox
      insertMB(mBox, msg_Obj);
      mBox->nMessages--; //-1   
      mBox->nBlockedMsg--; //-1
      //Move receiving task from Readylist to Waitinglist
      insertRL(waitingL,extractRL(readyL));
      uppdateRunning();
    }//ENDIF
    LoadContext();//Load context
  }//ELSE
  else {
    if(Running->DeadLine<=TC){//IF deadline is reached THEN
       x = set_isr(isr_off);   //Disable interrupt
      //Remove receive Message
      remove_msgRL(readyL);
      //remove_MBoxmsg(readyL->pHead->pNext->pMessage);
      mBox->nMessages += SENDER; //-1
      mBox->nBlockedMsg += SENDER; //-1
      set_isr(x);  //isr_on();//Enable interrupt
      return DEADLINE_REACHED;//Return DEADLINE_REACHED
    }//ELSE
    else{
      return OK;//Return OK
    }//ENDIF
  }//ENDIF
}




/** \brief  send a Message to the Mailbox

    This call will send a Message to the specified Mailbox.
    The sending task will continue execution after the call.
    When the Mailbox is full, the oldest Message will be
    overwritten. The send_no_wait call will imply a new
    scheduling and possibly a context switch.
    Note: send_wait and send_no_wait Messages shall not be
    mixed in the same Mailbox.

    \param [in]    *mBox: a pointer to the specified Mailbox
    \param [in]    *pData: a pointer to a memory area where the data of
                          the communicated Message is residing.
    \return        FAIL/OK: Description of the functionís status. 
 */
exception send_no_wait( mailbox* mBox, void* pData ){
  volatile int firstExec = TRUE;
  int x = set_isr(isr_off); //Disable interrupt
  SaveContext(); //Save context
  if(firstExec){//IF ìfirst executionî THEN
    firstExec=FALSE;//Set: ìnot first execution any more
    if(/*mBox->nMessages<0 &&*/ mBox->nBlockedMsg<0 ){//IF receiving task is waiting THEN
      //Copy senderís data to the data area of the receivers Message
      memcpy(mBox->pHead->pNext->pData,pData , mBox->nDataSize);//(DEST,SRS,SIZE)
      //*Remove receiving taskís Message struct from the mailbox
      struct l_obj  *list_pobj = mBox->pHead->pNext->pBlock;
      remove_MBoxmsg(mBox->pHead->pNext);
      mBox->nMessages += SENDER; //+1
      mBox->nBlockedMsg += SENDER; //+1
      //Move receiving task to Readylist
      insertRL(readyL,extractWL(waitingL, list_pobj));
      uppdateRunning();
      LoadContext();//Load context
    }//ELSE
    else{
      if(mBox->nBlockedMsg > 0) //return fail if there is send_wait in mailbox
        return FAIL;
      //Allocate a Message structure
      msg *msg_Obj = createMsg();
      if (msg_Obj== NULL) {
        return FAIL;
      }
      //Copy Data to the Message
      msg_Obj->pData = pData;
      memcpy(msg_Obj->pData,pData,mBox->nDataSize);
      //IF mailbox is full THEN
      if(mBox->nMessages == mBox->nMaxMessages){
        //Remove the oldest Message struct
        remove_OldMsg(mBox);
        mBox->nMessages--;
      }//ENDIF
      //Add Message to the Mailbox
      insertMB(mBox, msg_Obj);
      msg_Obj->pBlock = NULL;
      mBox->nMessages++;
    }//ENDIF
  }//ENDIF
  set_isr(x);
  return OK;//Return status
}


/** \brief  send a Message to the Mailbox

    This call will attempt to receive a Message from the
    specified Mailbox. The calling task will continue
    execution after the call. When there is no send
    Message available, or if the Mailbox is empty, the
    function will return FAIL. Otherwise, OK is returned.
    The call might imply a new scheduling and possibly a
    context switch.

    \param [in]    *mBox: a pointer to the specified Mailbox
    \param [in]    *pData:*Data: a pointer to the Message.
    \return        OK/FAIL  Integer indicating whether or not a Message wasreceived.

 */
int receive_no_wait( mailbox* mBox, void* pData ){
  
  volatile int firstExec = TRUE;
  int x = set_isr(isr_off); //Disable interrupt
  SaveContext(); //Save context
  if(firstExec){//IF ìfirst executionî THEN
    firstExec=FALSE;//Set: ìnot first execution any more
    
    
    if(mBox->nMessages>0 /*&& mBox->nBlockedMsg>0*/){//IF send Message is waiting THEN
      //Copy senderís data to receiving taskís data area
      memcpy(pData,mBox->pHead->pNext->pData , mBox->nDataSize);//(DEST,SRS(copyfrom),SIZE)
      //Remove sending taskís Message struct from the Mailbox
      void *pdata_temp = mBox->pHead->pNext->pData;
      // remove_MBoxmsg(mBox->pHead->pNext);
      //IF Message was of wait type THEN Move sending task to Ready list        (pblock?)
      int typewait=0;//if block
      
      if (mBox->pHead->pNext->pBlock->pMessage !=NULL && mBox-> nBlockedMsg != 0)  {
        typewait = 1;
        mBox->nMessages += RECEIVER; //-1
        mBox->nBlockedMsg += RECEIVER; //-1
        insertRL(readyL,extractWL(waitingL, mBox->pHead->pNext->pBlock));
        remove_MBoxmsg(mBox->pHead->pNext);
        uppdateRunning();
      }else{
        if(typewait==0){ //if send_no_wait
          remove_MBoxmsg(mBox->pHead->pNext);
          free(pdata_temp);//Free senders data area
          mBox->nMessages+= RECEIVER;
        }
      }//ENDIF
    }//ELSE
    LoadContext();//Load context
  }//ENDIF
  //Return status on received Message
  set_isr(x);
  return OK; 
}


void     isr_off(void){

}
void     isr_on(void){



}
