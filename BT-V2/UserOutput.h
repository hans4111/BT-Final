#ifndef _UserOutput_h
#define _UserOutput_h

#include<Windows.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include"ConsoleLogger.h"

extern int dOutStatus;
extern int dInStatus;
extern int aInStatus[2];
extern int rOutStatus;
extern int established;
extern int cont;

void UOut(void *param);

#endif