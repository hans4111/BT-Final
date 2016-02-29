#include<iostream>
#include<iomanip>
#include<cmath>
#include<Windows.h>
#include<string>
#include<bitset>
#include<atlstr.h>
#include<process.h>
#include<sstream>
#include"SerialClass.h"
#include"BTComm.h"
using namespace std;


void BTComm(void *param) {
	int currentround = 0;
	char receive[4] = "";
	char receive2[4] = "";
	stringstream translate;
	int finR = 999;
	
	Serial SP = Serial(comm);
	while (established != 1) {
		if (SP.IsConnected())
			established = 1;
		else
			SP = Serial(comm);
	}
	while (cont == 1) {
		switch (currentround) {
		case(0) :
			SP.WriteData("I", 1);
			Sleep(50);
			SP.ReadData(receive, 3);
			translate << receive;
			translate >> finR;
			dInStatus = finR;
			currentround++;
			translate.clear();
			Sleep(100);
			break;
		case(1) :
			SP.WriteData("A", 1);
			Sleep(50);
			SP.ReadData(receive, 3);
			SP.ReadData(receive2, 3);
			translate << receive;
			translate >> finR;
			aInStatus[0] = finR;
			translate.clear();
			translate << receive2;
			translate >> finR;
			aInStatus[1] = finR;
			translate.clear();
			currentround++;
			Sleep(100);
			break;
		case(2) :
			SP.WriteData("O", 1);
			translate << dOutStatus;
			translate >> receive;
			translate.clear();
			SP.WriteData(receive, 3);
			Sleep(100);
			SP.ReadData(receive, 1);
			if (receive[0] == 'X') {
				currentround++;
				Sleep(100);
				break;
			}
			else {
				currentround = 99;
				break;
			}
		case(3) :
			SP.WriteData("R", 1);
			translate << rOutStatus;
			translate >> receive;
			SP.WriteData(receive, 2);
			translate.clear();
			Sleep(100);
			SP.ReadData(receive, 1);
			if (receive[0] == 'X') {
				currentround = 0;
				Sleep(100);
				break;
			}
			else {
				currentround = 99;
				break;
			}
		case(98) :
			SP.WriteData("E", 1);
			Sleep(50);
			SP.ReadData(receive, 1);
			if (receive[0] == 'E') {
				currentround = 2;
				Sleep(100);
				break;
			}
			else {
				globError = 1;
				break;
			}
		case(99):
			SP.WriteData("E", 1);
			Sleep(50);
			SP.ReadData(receive, 1);
			if (receive[0] == 'E') {
				currentround = 3;
				Sleep(100);
				break;
			}
			else {
				globError = 1;
				break;
			}
		}
		if (globError == 1) {
			Sleep(300);
			SP.WriteData("E", 1);
			Sleep(300);
			SP.ReadData(receive, 1);
			if (receive[0] == 'E') {
				currentround = 0;
				Sleep(100);
				break;
			}
			else {
				SP.~Serial();
				SP = Serial(comm);
			}
		}
		Sleep(500);
	}
	SP.~Serial();
}