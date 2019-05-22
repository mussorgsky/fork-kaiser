#include "hFramework.h"
#include <stddef.h>
#include <stdio.h>

using namespace hFramework;

void hMain()
{
	sys.setLogDev(&Serial);

	int16_t smallPower = 50;
	int16_t bigPower = 500;

	while(!((hBtn1.isPressed() || hBtn2.isPressed()) && hExt.pin3.read())) {
		sys.delay(250);
	}

	if(hBtn2.isPressed()) {
		smallPower = 100;
		bigPower = 1000;
	}

	hLED3.on();
	sys.delay(1000);

	bool overLine = hExt.pin3.read();
	while(true) {
		overLine = hExt.pin3.read();

		if(overLine) {
			hMot1.setPower(bigPower);
			hMot2.setPower(smallPower);
		} else {
			hMot1.setPower(smallPower);
			hMot2.setPower(bigPower);
		}

		sys.delay(50);
	}
}

void calibrateAndDo90Degs()
{
	while(!(hBtn1.isPressed() && hExt.pin3.read())) {
		sys.delay(500);
	}

	hLED3.on();
	sys.delay(1000);

	uint64_t startTime = sys.getRefTime();
	hMot1.setPower(500);
	hMot2.setPower(-500);

	bool previous = hExt.pin3.read();
	bool current = previous;

	while(true) {
		current = hExt.pin3.read();
		if(current != previous) {
			previous = current;
			if(current == true) {
				break;
			}
		}
		sys.delay(10);
	}

	uint64_t stopTime = sys.getRefTime();

	uint64_t turn90Time = (stopTime - startTime) / 2;

	hMot1.setPower(0);
	hMot2.setPower(0);

	sys.delay(1000);

	while(true) {
		hMot1.setPower(500);
		hMot2.setPower(-500);

		sys.delay(turn90Time);

		hMot1.setPower(0);
		hMot2.setPower(0);

		sys.delay(1000);
	}
}


	// bool overLine = false;

	// for (;;)
	// {
	// 	bool state = hExt.pin3.read();

	// 	if(state != overLine) {
	// 		overLine = state;
	// 		hLED1.set(state);
	// 		printf("Sensor over line? %s\r\n", (state) ? "Yes" : "No");
	// 	}

	// 	sys.delay(10);
	// }

/*
	wait while not over the line and button not pressed
	wait a moment
	start timer
	full power mega turn until back over line
	stop turning and stop timer
	start turning for half of timer time endlessly with pauses
*/
