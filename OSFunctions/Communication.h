#ifndef Com_H
#define Com_H
#include "Listor.h"
#include "TimerFunctions.h"

/*******************************************************************************
 *                 Inter-Process Communication
 ******************************************************************************/


mailbox* create_mailbox(uint nof_msg, uint size_of_msg);
exception remove_mailbox( mailbox* mBox );
int no_messages( mailbox* mBox );
exception send_wait( mailbox *mBox, void* pData );
exception receive_wait( mailbox* mBox, void* pData );
exception send_no_wait( mailbox* mBox, void* pData );
int receive_no_wait( mailbox* mBox, void* pData );

#endif