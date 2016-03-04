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
	char test[1] = "";
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
			//cout << "Case Din" << endl;
			SP.WriteData("I", 1);
			Sleep(50);
			SP.ReadData(receive, 3);
			translate << receive;
			translate >> finR;
			dInStatus = finR;
			currentround=1;
			translate.str(string());
			translate.clear();
			Sleep(150);
			while (SP.ReadData(test, 1) != 0) {};
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			break;
		case(1) :
			//cout << "Case Ain" << endl;
			SP.WriteData("A", 1);
			Sleep(50);
			SP.ReadData(receive, 3);
			translate << receive;
			translate >> finR;
			aInStatus[0] = finR;
			translate.str(string());
			translate.clear();
			SP.ReadData(receive, 3);
			translate << receive;
			translate >> finR;
			aInStatus[1] = finR;
			translate.str(string());
			translate.clear();
			currentround=2;
			Sleep(150);
			while (SP.ReadData(test, 1) != 0) {};
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			break;
		case(2) :
			//cout << "Case Dout" << endl;
			//SP.WriteData("O", 1);
			translate << 'O'<< dOutStatus;
			translate >> receive;
			translate.str(string());
			translate.clear();
			//cout << "Dout output: **" << receive <<"**"<< endl;
			SP.WriteData(receive, 4);
			Sleep(150);
			SP.ReadData(receive, 1);
			//cout << "Read input for Dout: **" << receive << "**" << endl;
			if (receive[0] == 'X') {
				currentround=3;
				Sleep(100);
				while (SP.ReadData(test, 1) != 0) {};
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				break;
			}
			else {
				currentround = 98;
				while (SP.ReadData(test, 1) != 0) {};
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				break;
			}
		case(3) :
			//cout << "Case Rout" << endl;
			//SP.WriteData("R", 1);
			//translate << rOutStatus;
			//translate >> receive;
			translate << 'R' << rOutStatus;
			translate >> receive;
			//cout << "THIS IS THE RECEIVE OUTPUT: **" << receive << "**" << endl;
			SP.WriteData(receive, 3);
			translate.str(string());
			translate.clear();
			SP.WriteData(receive, 3);
			Sleep(150);
			SP.ReadData(receive, 1);
			//cout << "THIS IS THE READ INPUT FOR ROUT: **" << receive << "**" << endl;
			if (receive[0] == 'X') {
				currentround = 0;
				Sleep(100);
				while (SP.ReadData(test, 1) != 0) {};
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				break;
			}
			else {
				currentround = 99;
				while (SP.ReadData(test, 1) != 0) {};
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				break;
			}
		case(98) :
			cout << "Case Dout Error" << endl;
			SP.WriteData("E", 1);
			Sleep(50);
			SP.ReadData(receive, 1);
			if (receive[0] == 'E') {
				currentround = 2;
				Sleep(100);
				break;
			}
			else {
				globError = 0;
				cout << "Glob Error, receive status: " <<receive<< endl;
				currentround = 3;
				break;
			}
		case(99) :
			cout << "Case Rout Error" << endl;
			SP.WriteData("E", 1);
			Sleep(50);
			SP.ReadData(receive, 1);
			if (receive[0] == 'E') {
				currentround = 3;
				Sleep(100);
				break;
			}
			else {
				globError = 0;
				cout << "Glob Error, receive status: " << receive << endl;
				currentround = 0;
				break;
			}

		}
		//Sleep(200);
	}
	if (cont == 0) {
		int shutdown1 = 1;
		int shutdown2 = 1;
		while (shutdown1 == 0 || shutdown2 == 0) {
			rOutStatus = 0;
			dOutStatus = 0;
			SP.WriteData("O", 1);
			translate << dOutStatus;
			translate >> receive;
			translate.str(string());
			translate.clear();
			SP.WriteData(receive, 3);
			Sleep(100);
			SP.ReadData(receive, 1);
			if (receive[0] != 'X') {
				shutdown1 = 1;
			}
			else
				shutdown1 = 0;
			Sleep(100);
			SP.WriteData("R", 1);
			translate << rOutStatus;
			translate >> receive;
			SP.WriteData(receive, 2);
			translate.str(string());
			translate.clear();
			Sleep(100);
			SP.ReadData(receive, 1);
			if (receive[0] != 'X') {
				shutdown2 = 1;
			}
			else
				shutdown2 = 0;
			Sleep(100);
		}
	}
	
	SP.~Serial();
}