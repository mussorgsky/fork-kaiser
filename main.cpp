#include "hFramework.h"
#include <stddef.h>
#include <stdio.h>

#include <vector>

#include "ColorSensor.hpp"
// #include "Node.hpp"
// #include "Router.hpp"

using namespace hFramework;
using std::vector;

hGPIO *lineSensor, *markerDetector;
hMotor *leftMotor, *rightMotor, *winchMotor;
int8_t checkpointsPassed = -1;

const uint8_t ticksPerCM = 49;
const int16_t ticksWinchTop = 8920;
const int16_t ticksWinchUp = 8000;
const int16_t ticksWinchDown = -3100;

uint8_t jumps[] = { 15, 15, 8, 8, 8, 8, 8, 8, 15, 15, 0 };

enum Decisions { FORWARD, LEFT, RIGHT };
enum Winch { HOME, RAISED, TOP, LOWERED };
enum Tasks { PICKUP_FROM_DOCK, DELIVER_TO_RACK, RETURN_TO_BASE/*, PICKUP_FROM_RACK, DELIVER_TO_DOCK*/ };
enum Shelf { LOW, HIGH };

uint8_t task = Tasks::PICKUP_FROM_DOCK;
uint8_t winchState = Winch::HOME;
uint8_t target = 1;
uint8_t targetShelf = Shelf::LOW;
uint8_t heldColor = ColorSensor::COLOR::UNKNOWN;

uint8_t stockedRed = 0;
uint8_t stockedGreen = 0;
uint8_t stockedBlue = 0;

void moveWinch(uint8_t targetState)
{
	if(winchState == Winch::HOME) {
		if(targetState == Winch::RAISED) {
			winchMotor->rotRel(ticksWinchUp, 1000, true);
		}
		if(targetState == Winch::TOP) {
			winchMotor->rotRel(ticksWinchTop, 1000, true);
		}
		if(targetState == Winch::LOWERED) {
			winchMotor->rotRel(ticksWinchDown, 1000, true);
		}
	}

	if(winchState == Winch::RAISED) {
		if(targetState == Winch::HOME) {
			winchMotor->rotRel(-ticksWinchUp, 1000, true);
		}
		if(targetState == Winch::TOP) {
			winchMotor->rotRel(ticksWinchTop - ticksWinchUp, 1000, true);
		}
		if(targetState == Winch::LOWERED) {
			winchMotor->rotRel(ticksWinchDown - ticksWinchUp, 1000, true);
		}
	}

	if(winchState == Winch::TOP) {
		if(targetState == Winch::HOME) {
			winchMotor->rotRel(-ticksWinchTop, 1000, true);
		}
		if(targetState == Winch::RAISED) {
			winchMotor->rotRel(-ticksWinchTop + ticksWinchUp, 1000, true);
		}
		if(targetState == Winch::LOWERED) {
			winchMotor->rotRel(ticksWinchDown - ticksWinchTop, 1000, true);
		}
	}

	if(winchState == Winch::LOWERED) {
		if(targetState == Winch::HOME) {
			winchMotor->rotRel(-ticksWinchDown, 1000, true);
		}
		if(targetState == Winch::TOP) {
			winchMotor->rotRel(-ticksWinchDown + ticksWinchTop, 1000, true);
		}
		if(targetState == Winch::RAISED) {
			winchMotor->rotRel(-ticksWinchDown + ticksWinchUp, 1000, true);
		}
	}

	winchState = targetState;
}

void pickUpCrate(uint8_t shelf)
{
	moveWinch((shelf == Shelf::LOW) ? Winch::LOWERED : Winch::RAISED);

	rightMotor->rotRel(ticksPerCM * 7, 500);
	leftMotor->rotRel(ticksPerCM * 7, 500, true);

	sys.delay(250);

	moveWinch((shelf == Shelf::LOW) ? Winch::HOME : Winch::TOP);
}

void putCrateDown(uint8_t shelf)
{
	if(shelf == Shelf::HIGH) {
		moveWinch(Winch::TOP);

		rightMotor->rotRel(ticksPerCM * 7, 500);
		leftMotor->rotRel(ticksPerCM * 7, 500, true);

		sys.delay(250);

		moveWinch(Winch::RAISED);
	}

	if(shelf == Shelf::LOW) {
		rightMotor->rotRel(ticksPerCM * 7, 500);
		leftMotor->rotRel(ticksPerCM * 7, 500, true);

		moveWinch(Winch::LOWERED);
		sys.delay(250);
	}
}

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
	winchMotor->resetEncoderCnt();

	// Setting up the sensors
	lineSensor = &hExt.getPin(7);
	markerDetector = &hExt.getPin(9);
	lineSensor->setIn();
	markerDetector->setIn();

	ColorSensor cs = ColorSensor(&hExt.pin2, &hExt.pin1, &hExt.pin4, &hExt.pin3, &hExt.pin5, &hExt.serial.pinRx);
	cs.init();

	// sys.delay(2000);

	// while(!hBtn1.isPressed()) {
	// 	std::string colors[] = { "RED", "GREEN", "BLUE", "WHITE", "UNKNOWN" };
	// 	uint8_t color = cs.getColor();

	// 	printf("%s\r\n", colors[color].c_str());
	// 	sys.delay(1500);
	// }

	// while(!hBtn1.isPressed()) {
	// 	moveWinch(Winch::RAISED);
	// 	sys.delay(2000);
	// 	moveWinch(Winch::TOP);
	// 	sys.delay(2000);
	// 	moveWinch(Winch::HOME);
	// 	sys.delay(2000);
	// 	moveWinch(Winch::LOWERED);
	// 	sys.delay(2000);
	// 	moveWinch(Winch::HOME);
	// 	sys.delay(2000);
	// }

	// Waiting for a button before going
	while(!hBtn1.isPressed()) {
		sys.delay(100);
	}

	//
	bool detecting = true;
	uint64_t detectStopTime = 0;
	uint64_t ignorancePeriod = 450;
	uint8_t decision = Decisions::RIGHT;

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

			checkpointsPassed += 1;

			detectStopTime = sys.getRefTime();
			detecting = false;
		}

		if(checkpointsPassed == target - 1 && task == Tasks::DELIVER_TO_RACK) {
			decision = Decisions::RIGHT;
		}

		if(checkpointsPassed == target) {
			rightMotor->stop();
			leftMotor->stop();

			if(task == Tasks::RETURN_TO_BASE) {
				decision = Decisions::RIGHT;

				rightMotor->rotRel(ticksPerCM * 25, 500);
				leftMotor->rotRel(-ticksPerCM * 25, 500, true);

				rightMotor->stopRegulation();
				leftMotor->stopRegulation();
				
				moveWinch(Winch::HOME);

				target = 1;
				targetShelf = Shelf::LOW;
				task = Tasks::PICKUP_FROM_DOCK;
				heldColor = ColorSensor::COLOR::UNKNOWN;
			}else if(task == Tasks::PICKUP_FROM_DOCK) {
				pickUpCrate(Shelf::LOW);

				sys.delay(250);

				uint8_t color = cs.getColor();
				heldColor = color;

				if(color == ColorSensor::COLOR::RED) {
					hLED1.on();
					target = 2 + stockedRed + 1;
					targetShelf = Shelf::HIGH;
				}
				if(color == ColorSensor::COLOR::GREEN) {
					hLED2.on();
					target = 2 + stockedGreen + 1;
					targetShelf = Shelf::LOW;
				}
				if(color == ColorSensor::COLOR::BLUE) {
					hLED3.on();
					target = 5 + stockedBlue + 1;
					targetShelf = Shelf::HIGH;
				}

				if(color == ColorSensor::COLOR::UNKNOWN) {
					while(true) {
						hLED1.toggle();
						hLED2.toggle();
						hLED3.toggle();
						sys.delay(250);
					}
				} else {
					task = Tasks::DELIVER_TO_RACK;
				}

				rightMotor->rotRel(ticksPerCM * -7, 500);
				leftMotor->rotRel(ticksPerCM * -7, 500, true);

				moveWinch(Winch::HOME);

				rightMotor->rotRel(ticksPerCM * 25, 500);
				leftMotor->rotRel(-ticksPerCM * 25, 500, true);

				rightMotor->stopRegulation();
				leftMotor->stopRegulation();

				decision = Decisions::FORWARD;
			} else if(task == Tasks::DELIVER_TO_RACK) {
				moveWinch(Winch::HOME);

				putCrateDown(targetShelf);

				rightMotor->rotRel(ticksPerCM * -7, 500);
				leftMotor->rotRel(ticksPerCM * -7, 500, true);

				moveWinch(Winch::HOME);

				rightMotor->rotRel(ticksPerCM * -24, 500);
				leftMotor->rotRel(ticksPerCM * -24, 500, true);

				rightMotor->rotRel(ticksPerCM * 16, 500);
				leftMotor->rotRel(-ticksPerCM * 16, 500, true);

				rightMotor->stopRegulation();
				leftMotor->stopRegulation();

				while(!lineSensor->read()) {
					rightMotor->setPower(300);
					leftMotor->setPower(300);
				}
				rightMotor->setPower(0);
				leftMotor->setPower(0);

				if(heldColor == ColorSensor::COLOR::RED) {
					stockedRed += 1;
				}

				if(heldColor == ColorSensor::COLOR::GREEN) {
					stockedGreen += 1;
				}

				if(heldColor == ColorSensor::COLOR::BLUE) {
					stockedBlue += 1;
				}

				task = Tasks::RETURN_TO_BASE;
				decision = Decisions::FORWARD;
				target = 10;
			}
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

		if(decision == Decisions::FORWARD) {
			if(!boosted && !detecting && offset < 25) {
				rightMotor->rotRel(ticksPerCM * jumps[checkpointsPassed], 500);
				leftMotor->rotRel(ticksPerCM * jumps[checkpointsPassed], 500, true);

				rightMotor->stopRegulation();
				leftMotor->stopRegulation();
				sys.delay(500);
				boosted = true;
				continue;
			}
		}

		if(decision == Decisions::LEFT) {
			if(!boosted && !detecting) {
			int32_t angle = 140;
			rightMotor->rotRel(angle, 500);
			leftMotor->rotRel(-angle, 500, true);

			rightMotor->rotRel(angle*3, 500);
			leftMotor->rotRel(angle*3, 500, true);

			rightMotor->stopRegulation();
			leftMotor->stopRegulation();
			sys.delay(500);
			boosted = true;
			continue;
			}
		}

		if(decision == Decisions::RIGHT) {

		}

		if(lineSensor->read()) {
			rightMotor->setPower(0 - offset);
			leftMotor->setPower(500);
		} else {
			rightMotor->setPower(500);
			leftMotor->setPower(0 - offset);
		}

		lastSense = sense;
		sys.delay(10);
	}

	rightMotor->setPower(0);
	leftMotor->setPower(0);
}
