#include<iostream>
#include<iomanip>
#include<Windows.h>
#include<string>
#include<sstream>
#include<atlstr.h>
#include<process.h>
#include"SerialClass.h"
#include"BTComm.h"
#include"UserOutput.h"
#include"ConsoleLogger.h"

using namespace std;

//The are the declarations of functions used in main. fully defined following the main()
int init(void);
void HelpMessages();
int parseinput(int input);

//external ints used to pass data betweeen our threads. Not the cleanest or most stable method, but it works for this situation
extern int dOutStatus = 0;
extern int dInStatus = 0;
extern int aInStatus[2] = { 0,0};
extern int rOutStatus = 0;
extern int established = 0;
extern int cont = 1;
extern int globError = 0;

/*************************************************************************************************
**The variable below is where the COM port in use by windows is defined.						**
**In order to make this application work, pair with the I/O device through Windows BT manager   **
**Then Windows will automatically set up a Virtual Com Port. Find which port was assigned		**
**and enter the number when requested by the program. We assign the appropirate "COMX" and if   **
**the port assigned is in double digits then we follow this syntax: "\\\\.\\COM10" Yes windows  **
**is weird. No we dont know why it does this.													**
*************************************************************************************************/
extern char* comm = "\\\\.\\COM13";

HANDLE hThread;
HANDLE gThread;

char help;
char command;
int change;
int comport;
string s1;
stringstream portTrans;

int main() {
	//Lets start the program with a help message! or let the user skip it if they are experienced
	cout << "View Help before we begin? (Y,N)" << endl;
	cout << "Note that help will not be available while the application is running." << endl;
	cin >> help;
	//we are just going to sanity check the user, hopefully we never have this issue or its an accident (dear god i hope they can figure it out)
	if (help != 'Y' && help != 'y' && help != 'N' && help != 'n') {
		do{
			cin.ignore();
			cout << "That's not an option, please try again." << endl;
			cin >> help;
		} while (help != 'Y' && help != 'y' && help != 'N' && help !=  'n');
	}
	if (help == 'Y' || help == 'y')
		HelpMessages();

	cout << "Please enter the COM port assigned by windows. The Number only, no spaces or characters. IE: 9" << endl;
	cin >> comport;
	if (comport > 9)
		portTrans << "\\\\.\\COM" << comport;
	else
		portTrans << "COM" << comport;
	s1 = portTrans.str();
	comm = _strdup(s1.c_str());

	//initialize second thread for communications and the user interface output window. comms will spin until established
	if (init() != 0) {
		//this is literally an unknown errror, probably some sort of memory fault which would be of no fault of this program probably hardware
		cout << "Unknown System Error, failure to initialize. \n End of line." << endl;
		system("pause");
		return 0;
	}
	//spinning while waiting for connection
	while (established == 0) {}
	//alert user of connection begin user interface after 1 second
	cout << "Connection Established" << endl;
	Sleep(1000);
	system("cls");
	while (cont == 1) {
		//User Input
		cout << "What would you like to do? (O -> Dout, R -> Relay, E -> terminate application)\n";		
		cin >> command;
		cin.ignore();
		if (command == 'o' || command == 'O') {
			//Digital Output interface
			cout << "Please input the new port setting in binary (Ex: 01010101):" << endl;
			cin >> change;
			cin.ignore();
			change = parseinput(change);
			if (change > 255 || change < 0)
				cout << "Invalid port setting." << endl;
			else
				dOutStatus = change;
		}
		else if (command == 'R' || command == 'r') {
			//Relay Output interface
			cout << "Please input the new port setting in binary (Ex: 1001):" << endl;
			cin >> change;
			cin.ignore();
			change = parseinput(change);
			if (change > 15 || change < 0)
				cout << "Invalid port setting." << endl;
			else
				rOutStatus = change;
		}
		else if (command == 'e' || command == 'E')
			//Exit command
			cont = 0;
	}
	//We need to wait for the other threads to end before we close the porgram. They are given an infinite wait time because there shouldnt be any loops that continue to run
	WaitForSingleObject(hThread, INFINITE);
	WaitForSingleObject(gThread, INFINITE);
	//Let the user press enter at the end. may remove this
	system("pause");
}

int init(void) {
	//Start the comm and user interface threads
	hThread = (HANDLE)_beginthread(BTComm, 0, NULL);
	gThread = (HANDLE)_beginthread(UOut, 0, NULL);
	return 0;
}

void HelpMessages(void) {
	//This is the awesome help message!
	system("cls");
	cin.ignore(26,'\n');
	cout << "Congratulations and Welcome to the WYSIWYG Bluetooth IO Suite.\n" << endl;
	cout << "The PC application you are currently using is a text based interface that will open two windows for your convienience." << endl;
	cout << "One window will display real time data from the IO suite. The formatting for each port except for the Analog Input are" << endl;
	cout << "shown in binary representation of the port status. The Digital In, Digital Out, and Relay Out will be represented as 1" << endl;
	cout << "if the pin is ON, and as 0 if the pin is OFF. The Analog Input Ports are displayed seperately and are formatted to    " << endl;
	cout << "show the measured voltage level on the port. This value is referenced to the device ground that should be shared with " << endl;
	cout << "sensor ground for the best accuracy. If you would like to change these output values they can be changed in UOut.cpp. \n" << endl;
	cout << "The Input window will have a prompt up asking for a command. You as the user have 3 possible commands. These commands " << endl;
	cout << "are based on a single character input. 'O' for Digital Outputs, 'R' for Relay Outputs, and 'E' for exit.              \n" << endl;
	cout << "After either the 'O' or 'R' commands you will be prompted for the new PORT setting. Meaning you must input a new PORT " << endl;
	cout << "status. This should be done in the binary format, IE 11001100 for turning on ports 0,1,4, and 5. Since there are only " << endl;
	cout << "four relays you should only input a 4 bit line. IE 1001, to activate relay 0 and 3.                                   \n" << endl;
	cout << "When the E command is sent to exit the system will close the connection safely and shutdown the user output and then  " << endl;
	cout << "it will close the main window. Please note that the IO Suite will will set all outputs to zero when the application   " << endl;
	cout << "ends using the exit command. Please take appropriate caution with the device.\n" << endl;
	cout << "After this help message closes the program will start immediately\n" << endl;
	system("pause");
}

int parseinput(int input) {
	//Parse binary input from the user into the number it represents for communication simplicity.
	int fin = 0;
	int bin;
	int rem;
	int base = 1;

	bin = input;
	while (input > 0)
	{
		rem = input % 10;
		fin = fin + rem * base;
		base = base * 2;
		input = input / 10;
	}
	
	return fin;
}