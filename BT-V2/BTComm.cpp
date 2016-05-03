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

/**************************************************************************************************************************************************
**												EXPLANATION OF THE COMMUNICATION PROTOCOL														 **
**Below is the designed bluetooth protocal being put into effect. As a basic overview there are 4 main commands and one error check command in   **
**this protocol. The four commands are I, A, O, and R. For the inputs I and A no other commands are needed and the uC will return the values of  **
**the 8 bit register represented in a integer. This integer arrives to us as three characters that we then translate using stringstreams into an **
**integer. The integer is then saved to the global variables that are used by the user output. For example, if all of the digital inputs are on, **
**and we send the 'I' command. The uC will return three characters '2''5''5', which we then put into a string stream and dump into an integer for**
**the value 255. 255 is then stored to the global int. For the case of the analog inputs (the 'A' command), since there are two channels the uC  **
**will return six characters, with the first three representing the the first channel and the second three representing the second channel. It   **
**is up to us to parse the information from the controller seperatly. For outputs we follow the same pattern. The 'O' command is the digital out **
**command, and we send the decimal representation of the 8 bit register. IE for all on we send 255, for just pin 7 we send 128 and so on. The    **
**protocol used by windows for serial devices treats the devices like an open file so all data sent is characters. Therefore for the output		 **
**commands we put the integer representation into a string stream and then dump it in to a character array from the stringstream. The uC expects **
**three characters so if our output is only single or double digit we prefix it with '0' to maintain three character output. For the Relay outs	 **
**the 'R' command, we follow the exact same method, EXCEPT it only uses 4 bits and therefore a output of two characters(besides the command).	 **
**The final part of the output commands is the uC's response to our command. After any output command is sent and received the uC should return  **
**an 'X' for successfull write. If this response is not received the code enters an error state. In the error state we send the uC an 'E' command**
**in return we should receive an 'E' in response. When a successful error check and response occurs we return to the previous write case to		 **
**attempt the write the command again. If the error is not returned the user is informed of a packet drop error and the code will restart from   **
**the first case. For future development tracking of dropped package, connection checking, error response will be more fully integrated into this**
**system.																																		 **
**************************************************************************************************************************************************/

/**************************************************************************************************************************************************
**												Use of the Serial Class for communication														 **
**The Serial.cpp and the Serial.h files included were sourced from http://www.codeproject.com/Articles/992/Serial-library-for-C#_articleTop      **
**It was written by Ramon de Klein and released into the public domain. A better explanation can be found at the site above, however I will give **
**a breif overview of its use.																													 **
**																																				 **
**The class comes with built in functions that handle the Windows Serial communication. Windows uses SPP and creates a virtual COM Port for a	 **
**serial bluetooth devices. This is why we have the user connect to the IO suite outside of the program, and then type in the COM Port assigned  **
**by Windows. To open the serial connection we create a new Varaible of type Serial defined in the class. As seen by in line 70 we create a name **
**and then call Serial(comm) where comm is the com port assigned by windows. The definition of that variable is found in Main.cpp on line 29.    **
**The following information about the commands in the class is copied from the Serial.h file:													 **
**	Initialize Serial communication with the given COM port																						 **
**		Serial(char *portName);																													 **
**	Close the connection																														 **
**		~Serial();																																 **
**	Read data in a buffer, if nbChar is greater than the maximum number of bytes available, it will return only the bytes available. The function**
**  return -1 when nothing could be read, or the number of bytes actually read.																	 **
**		int ReadData(char *buffer, unsigned int nbChar);																						 **
**	Writes data from a buffer through the Serial connection return true on success.																 **
**		bool WriteData(char *buffer, unsigned int nbChar);																						 **
**	Check if we are actually connected																											 **
**		bool IsConnected();																														 **
**************************************************************************************************************************************************/



void BTComm(void *param) {
	// initialize variables for state machine and translation
	int currentround = 0;
	char receive[6] = "";
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
		//State machine running until the exit command is given in by the user in main.cpp
		switch (currentround) {
		case(0) :
			//Digital In Case
			//Clear the buffers in case somethin got left in them
			while (SP.ReadData(test, 1) != 0) {};
			//A quick side note, we fill receive with whitespace to clear it because stringstream sees whitespace as end of line, not return characters
			//Also for some reason when a for loop is used to clear receive data corruption tends to occur for some reason, so due to lack of time to continue
			//debugging I just implemented the clear piece by piece, future development will include fixing that issue
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			SP.WriteData("I", 1);
			Sleep(100);
			SP.ReadData(receive, 3);
			//Read characters
			translate << receive;
			//Translate char into int
			translate >> finR;
			//Set dIn to new value
			dInStatus = finR;
			finR = 0;
			//Next case
			currentround=1;
			//Clear the string stream
			translate.str(string());
			translate.clear();
			Sleep(100);
			//Clear the input buffer
			while (SP.ReadData(test, 1) != 0) {};
			//Set receive to new lines for the next info grab
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			break;
		case(1) :
			//Analog Input Code
			SP.WriteData("A", 1);
			Sleep(100);
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
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
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
			Sleep(100);
			//Clear input buffer
			while (SP.ReadData(test, 1) != 0) {};
			//Clear receive
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
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
			Sleep(100);
			//clear receive
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			SP.ReadData(receive, 1);
			//check success response
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
				//if fail we go to a second error case and check communication issues
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
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			Sleep(100);
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
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				break;
			}
			else {
				//if not confirmed continue to global error case
				currentround = 99;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				break;
			}
		case(98) :
			//Global Error Case for Dout
			cout << "Case Dout Error" << endl;
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			//Write E command to check error, expect E response
			SP.WriteData("E", 1);
			Sleep(150);
			SP.ReadData(receive, 1);
			if (receive[0] == 'E') {
				//Return to Dout incase it was written improperly to send write command again
				currentround = 2;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				Sleep(100);
				break;
			}
			else {
				//set global error to 1 for true
				//globError = 1;
				//cout << "Glob Error, receive status: " <<receive<< endl;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				//for now we dont have global error handeling so continue on the next case
				currentround = 3;
				break;
			}
		case(99) :
			//global Error Case for Rout
			cout << "Case Rout Error" << endl;
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			//Write E command to check error, expect E response
			SP.WriteData("E", 1);
			Sleep(150);
			SP.ReadData(receive, 1);
			if (receive[0] == 'E') {
				//Return to Rout case in order to rewrite in case of a write error
				currentround = 3;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				Sleep(100);
				break;
			}
			else {
				//set global error to 1
				//globError = 1;
				//cout << "Glob Error, receive status: " << receive << endl;
				//clear input buffer
				while (SP.ReadData(test, 1) != 0) {};
				//clear receive
				receive[0] = ' ';
				receive[1] = ' ';
				receive[2] = ' ';
				receive[3] = ' ';
				receive[4] = ' ';
				//go to next case since we dont have global error handeling yet
				currentround = 0;
				break;
			}

		}
	}
	//Here is a shutdown sequence to set all outputs to zero. We do this for safety when the application turns off
	//This can be removed IFF you understand that your outputs will retain their values until either A) the application is restarted and defaults are set (right now thats 0)
	//or B) the uC cyles which resets output to default off
	if (cont == 0) {
		//we want to be able to error check the shutdown cycle for both Relay and Digital out
		int shutdown1 = 0;
		int shutdown2 = 0;
		rOutStatus = 0;
		dOutStatus = 0;
		//Set shutdown values to 0 for all off
		while (shutdown1 == 0 || shutdown2 == 0) {
			//Write dOutput command 
			translate << 'O' << '0' << '0' << '0';
			translate >> receive;
			//clear string stream
			translate.str(string());
			translate.clear();
			//write bluetooth
			SP.WriteData(receive, 4);
			//clear receive
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			Sleep(100);
			//we wait to allow bt to respond and then check response
			SP.ReadData(receive, 1);
			//clear input buffer
			while (SP.ReadData(test, 1) != 0) {};
			if (receive[0] == 'X') {
				//successful error check command shutdown of dOut successful
				shutdown1 = 1;
			}
			else
				shutdown1 = 0;
			Sleep(100);
			//Wait so no overlap then generat Rout command
			translate << 'R' << '0' << '0';
			translate >> receive;
			//Write command to BT
			SP.WriteData(receive, 3);
			//Clear String
			translate.str(string());
			translate.clear();
			//clear receive
			receive[0] = ' ';
			receive[1] = ' ';
			receive[2] = ' ';
			receive[3] = ' ';
			receive[4] = ' ';
			Sleep(100);
			SP.ReadData(receive, 1);
			//clear input buffer
			while (SP.ReadData(test, 1) != 0) {};
			if (receive[0] == 'X') {
				//successful error check command shutdown of rOut successful
				shutdown2 = 1;
			}
			else
				shutdown2 = 0;
			Sleep(100);

		}
	}
	//Close serial connection at end of thread.
	SP.~Serial();
}