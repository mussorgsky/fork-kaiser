#include "hFramework.h"
#include <stddef.h>
#include <stdio.h>

#include <vector>

// #include "ColorSensor.hpp"
// #include "Node.hpp"
// #include "Router.hpp"

using namespace hFramework;
using std::vector;

hGPIO *lineSensor, *markerDetector;
hMotor *leftMotor, *rightMotor, *winchMotor;

enum Decisions { FORWARD };

void hMain()
{
	sys.setLogDev(&Serial);
	printf("Battery voltage: %fV\r\n", sys.getSupplyVoltage());

	// Setting up the motors
	leftMotor = &hMotD;
	rightMotor = &hMotA;
	winchMotor = &hMotB;
	leftMotor->setEncoderPolarity(Polarity::Reversed);
	rightMotor->setEncoderPolarity(Polarity::Reversed);
	winchMotor->setEncoderPolarity(Polarity::Reversed);

	// Setting up the sensors
	lineSensor = &hExt.getPin(7);
	markerDetector = &hExt.getPin(9);
	lineSensor->setIn();
	markerDetector->setIn();

	// Waiting for a button before going
	while(!hBtn1.isPressed()) {
		sys.delay(100);
	}

	//
	bool detecting = true;
	uint64_t detectStopTime = 0;
	uint64_t ignorancePeriod = 3000;
	uint8_t decision = Decisions::FORWARD;

	bool lastSense = lineSensor->read();
	bool sense, marker;
	bool boosted = false;
	uint16_t offset = 0;

	while(true) {
		sense = lineSensor->read();
		marker = markerDetector->read();

		if(detecting && marker) {
			rightMotor->setPower(0);
			leftMotor->setPower(0);
			sys.delay(1500);

			detectStopTime = sys.getRefTime();
			detecting = false;
		}

		if(!detecting && sys.getRefTime() > detectStopTime + ignorancePeriod) {
			detecting = true;
			boosted = false;
		}

		if(sense == lastSense) {
			offset += 5;
		} else {
			offset = 0;
		}

		if(offset > 500) {
			offset = 500;
		}

		if(!boosted && !detecting && offset < 25) {
			rightMotor->setPower(1000);
			leftMotor->setPower(1000);
			sys.delay(500);
			boosted = true;
			continue;
		}

		if(lineSensor->read()) {
			rightMotor->setPower(0 - offset*2);
			leftMotor->setPower(500);
		} else {
			rightMotor->setPower(500);
			leftMotor->setPower(0 - offset*2);
		}

		lastSense = sense;
		sys.delay(10);
	}
}

// void takeTurn(uint8_t target, uint8_t *current, uint16_t power)
// {
// 	if(target == *current) {
// 		return;
// 	}

// 	bool toLeft = false;
// 	if((target == Router::MOVES::NORTH && *current == Router::MOVES::EAST)
// 	|| (target == Router::MOVES::EAST && *current == Router::MOVES::SOUTH)
// 	|| (target == Router::MOVES::SOUTH && *current == Router::MOVES::WEST)
// 	|| (target == Router::MOVES::WEST && *current == Router::MOVES::NORTH)) {
// 		 toLeft = true;
// 	}

// 	*current = target;

// 	// Each wheel goes one quarter of a circle
// 	int16_t angle = (int16_t) (15.6f * 0.125f / 8.16f * 1.666f * 720.0f);
// 	angle *= (toLeft) ? -1 : 1;
// 	leftMotor->rotRel(angle, power);
// 	rightMotor->rotRel(-angle, power, true);

// 	leftMotor->stopRegulation();
// 	rightMotor->stopRegulation();

// 	leftMotor->setPower(power);
// 	rightMotor->setPower(power);

// 	sys.delay(250);

// 	leftMotor->setPower(0);
// 	rightMotor->setPower(0);
// }

// void driveUntilCheckpoint(uint16_t power)
// {
// 	leftMotor->setPower(power);
// 	rightMotor->setPower(power);

// 	sys.delay(250);

// 	while(!(leftSensor->read() && centerSensor->read() && rightSensor->read())) {
// 		if(centerSensor->read()) {
// 			leftMotor->setPower(10);
// 			rightMotor->setPower(500);
// 		} else {
// 			leftMotor->setPower(500);
// 			rightMotor->setPower(10);
// 		}
		
		
// 		sys.delay(25);
// 	}

// 	leftMotor->setPower(0);
// 	rightMotor->setPower(0);

// 	sys.delay(2500);
// }

// void hMain()
// {
// 	sys.setLogDev(&Serial);

// 	printf("Battery voltage: %fV\r\n", sys.getSupplyVoltage());

// 	leftSensor = &hExt.i2c.pinSda;
// 	centerSensor = &hExt.spi.pinMiso;
// 	rightSensor = &hExt.serial.pinTx;
// 	leftSensor->setIn();
// 	centerSensor->setIn();
// 	rightSensor->setIn();

// 	leftMotor = &hMotD;
// 	rightMotor = &hMotA;
// 	winchMotor = &hMotB;
// 	leftMotor->setEncoderPolarity(Polarity::Reversed);
// 	rightMotor->setEncoderPolarity(Polarity::Reversed);
// 	winchMotor->setEncoderPolarity(Polarity::Reversed);
	
// 	// ColorSensor cs = ColorSensor(&hExt.pin2, &hExt.pin1, &hExt.pin4, &hExt.pin3, &hExt.pin5, &hExt.serial.pinRx);
// 	// cs.init();

// 	vector<Node> world = { Node("home"), Node("bayA") };
// 	Node *homeNode = &world.at(0);
// 	Node *target = &world.at(1);

// 	uint8_t bearing = Router::MOVES::NORTH;

// 	Router gps = Router(&world);
// 	vector<uint8_t> *route;

// 	// Main loop
// 	while(true) {
// 		// Wait for button press, so it doesn't run away
// 		while(!hBtn1.isPressed()) {
// 			hLED1.set(leftSensor->read());
// 			hLED2.set(centerSensor->read());
// 			hLED3.set(rightSensor->read());

// 			sys.delay(5);

// 			hLED1.off();
// 			hLED2.off();
// 			hLED3.off();

// 			sys.delay(5);
// 		}

// 		while(true) {
// 			driveUntilCheckpoint(500);
// 		}

// 		route = gps.getRoute(homeNode, target);
// 		for(auto direction : *route) {
			
// 		}

// 		break;
// 	}

// 	sys.delay(1000);
// }

	// cs.setFrequencyMode(ColorSensor::FREQUENCY::LOW);
	// cs.setColorMode(ColorSensor::COLOR::WHITE);

	// float allLight, redLight, greenLight, blueLight;
	// while(true) {
	// 	cs.setColorMode(ColorSensor::COLOR::WHITE);
	// 	allLight = cs.getIntensity();
	// 	sys.delay(15);
	// 	cs.setColorMode(ColorSensor::COLOR::RED);
	// 	redLight = cs.getIntensity();
	// 	sys.delay(15);
	// 	cs.setColorMode(ColorSensor::COLOR::GREEN);
	// 	greenLight = cs.getIntensity() / 0.55f;
	// 	sys.delay(15);
	// 	cs.setColorMode(ColorSensor::COLOR::BLUE);
	// 	blueLight = cs.getIntensity() / 0.45f;

	// 	printf("%f\t%f\t%f\t\t%f\r\n", redLight, greenLight, blueLight, allLight);
	// 	sys.delay(250);
	// }



	// // sys.startCritical();
	// int32_t times[100];
	// bool states[100];
	// for(uint8_t i = 0; i < 100; i++) {
	// 	states[i] = hExt.pin3.read();
	// 	times[i] = sys.getUsTimVal();
	// }
	// // sys.endCritical();

	// for(uint8_t i = 0; i < 100; i++) {
	// 	printf("Time: %d\tState: %s\r\n", times[i], (states[i]) ? "True" : "False");
	// }
	// uint32_t elapsed = times[99] - times[0];
	// double frequency = 100.0 / (double) elapsed * 1000.0;
	// printf("Elapsed time: %dus\r\nFrequency: %fkHz\r\n", elapsed, frequency);


	// // while(true) {
	// // 	// printf("%d\r\n", hMot3.getEncoderCnt());
	// // 	// sys.delay(100);
	// // 	hMot3.rotRel(-5400, 500, true);
	// // 	sys.delay(5000);
	// // 	hMot3.rotRel(5400, 500, true);
	// // 	sys.delay(5000);
	// // }

	// int16_t smallPower = 50;
	// int16_t bigPower = 500;

	// while(!((hBtn1.isPressed() || hBtn2.isPressed()) && hExt.pin3.read())) {
	// 	sys.delay(100);
	// }

	// if(hBtn2.isPressed()) {
	// 	smallPower = 100;
	// 	bigPower = 1000;
	// }

	// hLED3.on();
	// sys.delay(1000);

	// // float circum = 15.6f * 3.1415f;
	// // int16_t angle = (int16_t) (circum / (8.16f * 3.1415f) * 1.6666f * 360.0f * 2.0f);

	// // while(true) {
	// // 	hMot1.rotRel(angle, bigPower);
	// // 	hMot2.rotRel(-angle, bigPower, true);
	// // 	sys.delay(1000);
	// // 	hMot1.rotRel(-angle, bigPower);
	// // 	hMot2.rotRel(angle, bigPower, true);
	// // 	sys.delay(1000);
	// // }

	// bool overLine = hExt.pin3.read();
	// bool side = true;

	// while(true) {
	// 	overLine = hExt.pin3.read();

	// 	bool roadblock = hExt.pin3.read() && hExt.pin4.read() && hExt.pin5.read();

	// 	if(roadblock) {
	// 		hMot1.setPower(0);
	// 		hMot2.setPower(0);

	// 		sys.delay(500);

	// 		// hMot1.rotRel(-234, 250);
	// 		// hMot2.rotRel(-234, 250, true);

	// 		// sys.delay(500);

	// 		hMot3.rotRel(-5400, 250, true);

	// 		sys.delay(500);

	// 		hMot1.rotRel(300, 250);
	// 		hMot2.rotRel(300, 250, true);

	// 		sys.delay(500);

	// 		hMot3.rotRel(5400, 750, true);

	// 		sys.delay(500);

	// 		hMot1.setPower(-bigPower);
	// 		hMot2.setPower(-bigPower);

	// 		while(true) {
	// 			sys.delay(50);
	// 		}
	// 	}

	// 	hMot1.setPower(bigPower);
	// 	hMot2.setPower(bigPower);

	// 	sys.delay(10);
	// }
