#include<iostream>
#include<iomanip>
#include<Windows.h>
#include<string>
#include<bitset>
#include<atlstr.h>
#include<sstream>
#include"SerialClass.h"
#include"BTComm.h"
using namespace std;


void BTComm(void *param) {
	// initialize variables for state machine and translation
	int currentround = 0;
	char receive[4] = "";
	char test[1] = "";
	stringstream translate;
	int finR = 0;
	
	//POLL while finding communication. Written like this in order to change established which is an external int
	Serial SP = Serial(comm);
	while (established != 1) {
		if (SP.IsConnected())
			established = 1;
		else
			SP = Serial(comm);
	}
	while (cont == 1) {
		//State machine running until 
		switch (currentround) {
		case(0) :
			//Digital In Case
			SP.WriteData("I", 1);
			Sleep(50);
			SP.ReadData(receive, 3);
			//Read characters
			translate << receive;
			//Translate char into int
			translate >> finR;
			//Set dIn to new value
			dInStatus = finR;
			//Next case
			currentround=1;
			//Clear the string stream
			translate.str(string());
			translate.clear();
			Sleep(150);
			//Clear the input buffer
			while (SP.ReadData(test, 1) != 0) {};
			//Set receive to new lines for the next info grab
			receive[0] = (char)'/0';
			receive[1] = (char)'/0';
			receive[2] = (char)'/0';
			receive[3] = (char)'/0';
			receive[4] = (char)'/0';
			break;
		case(1) :
			//Analog Input Code
			SP.WriteData("A", 1);
			Sleep(50);
			SP.ReadData(receive, 3);
			//translate char to int using string stream
			translate << receive;
			translate >> finR;
			//fill first aIn status
			aInStatus[0] = finR;
			//clear string stream
			translate.str(string());
			translate.clear();
			//clear receive
			receive[0] = (char)'/0';
			receive[1] = (char)'/0';
			receive[2] = (char)'/0';
			receive[3] = (char)'/0';
			receive[4] = (char)'/0';
			//analog in port 2
			SP.ReadData(receive, 3);
			//translate char to int
			translate << receive;
			translate >> finR;
			aInStatus[1] = finR;
			//clear string
			translate.str(string());
			translate.clear();
			//iterate case
			currentround=2;
			Sleep(150);
			//Clear input buffer
			while (SP.ReadData(test, 1) != 0) {};
			//Clear receive
			receive[0] = (char)'/0';
			receive[1] = (char)'/0';
			receive[2] = (char)'/0';
			receive[3] = (char)'/0';
			receive[4] = (char)'/0';
			break;
		case(2) :
			//Digital Out Case
			//Create ouput to uC in the format of O123 where O is output command and 123 is data settings, mask single and double digits with extra zeros
			if (dOutStatus < 100 && dOutStatus > 10) {
				translate << 'O' << '0' << dOutStatus;
			}
			else if (dOutStatus < 10) {
				translate << 'O' << '0' << '0' << dOutStatus;
			}
			else
				translate << 'O'<< dOutStatus;
			translate >> receive;
			//clear the string stream
			translate.str(string());
			translate.clear();
			//write output to BT
			SP.WriteData(receive, 4);
			Sleep(150);
			//clear receive
			receive[0] = (char)'/0';
			receive[1] = (char)'/0';
			receive[2] = (char)'/0';
			receive[3] = (char)'/0';
			receive[4] = (char)'/0';
			SP.ReadData(receive, 1);
			//check success response
			if (receive[0] == 'X') {
				currentround=3;
				Sleep(100);
				while (SP.ReadData(test, 1) != 0) {};
				receive[0] = (char)'/0';
				receive[1] = (char)'/0';
				receive[2] = (char)'/0';
				receive[3] = (char)'/0';
				receive[4] = (char)'/0';
				break;
			}
			else {
				//if fail we go to a second error case and check communication issues
				currentround = 98;
				while (SP.ReadData(test, 1) != 0) {};
				receive[0] = (char)'/0';
				receive[1] = (char)'/0';
				receive[2] = (char)'/0';
				receive[3] = (char)'/0';
				receive[4] = (char)'/0';
				break;
			}
		case(3) :
			//Relay Output case
			//Create the output command RXX where R is relay command and XX is the numeral representation given by user, if less than 10 mask with a 0 character
			if (rOutStatus < 10) {
				translate << 'R' << '0' << rOutStatus;
			}
			else
				translate << 'R' << rOutStatus;
			translate >> receive;
			//Write command to BT
			SP.WriteData(receive, 3);
			//clear string stream
			translate.str(string());
			translate.clear();
			//clear receive
			receive[0] = (char)'/0';
			receive[1] = (char)'/0';
			receive[2] = (char)'/0';
			receive[3] = (char)'/0';
			receive[4] = (char)'/0';
			Sleep(150);
			//receive confirmation
			SP.ReadData(receive, 1);
			//Error check for confirmation
			if (receive[0] == 'X') {
				//restart case machine
				currentround = 0;
				Sleep(100);
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = (char)'/0';
				receive[1] = (char)'/0';
				receive[2] = (char)'/0';
				receive[3] = (char)'/0';
				receive[4] = (char)'/0';
				break;
			}
			else {
				//if not confirmed continue to global error case
				currentround = 99;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = (char)'/0';
				receive[1] = (char)'/0';
				receive[2] = (char)'/0';
				receive[3] = (char)'/0';
				receive[4] = (char)'/0';
				break;
			}
		case(98) :
			//Global Error Case for Dout
			cout << "Case Dout Error" << endl;
			//Write E command to check error, expect E response
			SP.WriteData("E", 1);
			Sleep(50);
			SP.ReadData(receive, 1);
			if (receive[0] == 'E') {
				//Return to Dout incase it was written improperly to send write command again
				currentround = 2;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = (char)'/0';
				receive[1] = (char)'/0';
				receive[2] = (char)'/0';
				receive[3] = (char)'/0';
				receive[4] = (char)'/0';
				Sleep(100);
				break;
			}
			else {
				//set global error to 1 for true
				globError = 1;
				cout << "Glob Error, receive status: " <<receive<< endl;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = (char)'/0';
				receive[1] = (char)'/0';
				receive[2] = (char)'/0';
				receive[3] = (char)'/0';
				receive[4] = (char)'/0';
				//for now we dont have global error handeling so continue on the next case
				currentround = 3;
				break;
			}
		case(99) :
			//global Error Case for Rout
			cout << "Case Rout Error" << endl;
			//Write E command to check error, expect E response
			SP.WriteData("E", 1);
			Sleep(50);
			SP.ReadData(receive, 1);
			if (receive[0] == 'E') {
				//Return to Rout case in order to rewrite in case of a write error
				currentround = 3;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = (char)'/0';
				receive[1] = (char)'/0';
				receive[2] = (char)'/0';
				receive[3] = (char)'/0';
				receive[4] = (char)'/0';
				Sleep(100);
				break;
			}
			else {
				//set global error to 1
				globError = 1;
				cout << "Glob Error, receive status: " << receive << endl;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = (char)'/0';
				receive[1] = (char)'/0';
				receive[2] = (char)'/0';
				receive[3] = (char)'/0';
				receive[4] = (char)'/0';
				//go to next case since we dont have global error handeling yet
				currentround = 0;
				break;
			}

		}
		//Sleep(200);
	}
	//Here is a shutdown sequence to set all outputs to zero. We do this for safety when the application turns off
	//This can be removed IFF you understand that your outputs will retain their values until either A) the application is restarted and defaults are set (right now thats 0)
	//or B) the uC cyles which resets output to default off
	if (cont == 0) {
		//we want to be able to error check the shutdown cycle for both Relay and Digital out
		int shutdown1 = 1;
		int shutdown2 = 1;
		//Set shutdown values to 0 for all off
		rOutStatus = 0;
		dOutStatus = 0;
		while (shutdown1 == 0 || shutdown2 == 0) {
			//Write dOutput command 
			translate << 'O' << dOutStatus;
			translate >> receive;
			//clear string stream
			translate.str(string());
			translate.clear();
			//write bluetooth
			SP.WriteData(receive, 4);
			//clear receive
			receive[0] = (char)'/0';
			receive[1] = (char)'/0';
			receive[2] = (char)'/0';
			receive[3] = (char)'/0';
			receive[4] = (char)'/0';
			Sleep(100);
			//we wait to allow bt to respond and then check response
			SP.ReadData(receive, 1);
			//clear input buffer
			while (SP.ReadData(test, 1) != 0) {};
			if (receive[0] != 'X') {
				//successful error check command shutdown of dOut successful
				shutdown1 = 1;
			}
			else//fail rerun
				shutdown1 = 0;
			Sleep(100);
			//Wait so no overlap then generat Rout command
			translate << 'R' << rOutStatus;
			translate >> receive;
			//Write command to BT
			SP.WriteData(receive, 3);
			//Clear String
			translate.str(string());
			translate.clear();
			//clear receive
			receive[0] = (char)'/0';
			receive[1] = (char)'/0';
			receive[2] = (char)'/0';
			receive[3] = (char)'/0';
			receive[4] = (char)'/0';
			Sleep(100);
			SP.ReadData(receive, 1);
			//clear input buffer
			while (SP.ReadData(test, 1) != 0) {};
			if (receive[0] != 'X') {
				//successful error check command shutdown of rOut successful
				shutdown2 = 1;
			}
			else//fail rerun
				shutdown2 = 0;
			Sleep(100);
		}
	}
	//Close serial connection at end of thread.
	SP.~Serial();
}