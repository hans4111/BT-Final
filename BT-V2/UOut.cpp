#include<bitset>
#include<stdlib.h>
#include"UserOutput.h"
#include"ConsoleLogger.h"

using namespace std;



void UOut(void *param) {
	CConsoleLoggerEx UI;
	UI.Create("Bluetooth User Interface");
	UI.cls();
	while (cont == 1) {
		char digin[33];
		char digout[33];
		char rout[33];
		char anin[33];
		UI.cls();
		_itoa(dOutStatus, digout, 2);
		UI.printf("Digital Out Status: %08s \n", digout);
		_itoa(dInStatus, digin, 2);
		UI.printf("Digital In status:  %08s \n", digin);
		_itoa(rOutStatus, rout, 2);
		UI.printf("Relay Out Status:   %08s \n", rout	);
		for (int i = 0; i < 2; i++) {
			_itoa(aInStatus[i], anin, 2);
			UI.printf("Ain Channel %d:      %08s \n", i, anin);
		}

		Sleep(200);
	}
	UI.~CConsoleLoggerEx();
	
}