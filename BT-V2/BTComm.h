#ifndef _BTComm_h
#define _BTComm_h

#include<Windows.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include"SerialClass.h"

extern int dOutStatus;
extern int dInStatus;
extern int aInStatus[2];
extern int rOutStatus;
extern int established;
extern int cont;
extern int globError;
extern char* comm;

void BTComm(void *param);

#endif