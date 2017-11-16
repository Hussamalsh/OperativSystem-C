/**
 * @file Listor.h 
 * @author Hussam Alshammari
 * @date 3 mar 2016
 * @brief File containing all the list functions for the data structures and variables.
 *
 * This header file have the important functions to run the program
 */

#ifndef Listor_H
#define Listor_H
#include "kernel.h"

list *create_list();
//TL + WT fuctions
listobj *create_listobj(int num);
void insertTL(list *list, listobj *obj);
listobj *extractWL(list *list, struct l_obj * pBlock);
//RL fuctions
listobj *create_listobjRL(int num);
void insertRL(list *list, listobj *obj);
listobj *extractRL(list *list);
//MialBox fuctions
mailbox * create_mailB();
void insertMB(mailbox *list, msg *obj);
//void insertMB(mailbox *mb, msg *message)

msg * createMsg();

void remove_MBoxmsg(msg *nMsg);
void remove_msgRL(list * RL);
void remove_OldMsg(mailbox *mBox);

void remove_list(list * xList);

#endif

