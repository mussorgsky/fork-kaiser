#pragma once

#include "hFramework.h"

using hFramework::hGPIO;

class ColorSensor
{
    private:
    hGPIO *s0, *s1, *s2, *s3, *oe, *out;
    uint8_t frequencyMode;
    uint8_t colorMode;

    public:
    enum FREQUENCY { FULL, MEDIUM, LOW };
    enum COLOR { RED, GREEN, BLUE, WHITE, UNKNOWN };
    ColorSensor(hGPIO *s0, hGPIO *s1, hGPIO *s2, hGPIO *s3, hGPIO *oe, hGPIO *out) : s0(s0), s1(s1), s2(s2), s3(s3), oe(oe), out(out) {};
    void init();
    void setFrequencyMode(uint8_t mode);
    void setColorMode(uint8_t color);
    uint16_t getPeriod();
    uint8_t getColor();
};

// #include "ColorSensor.hpp"

void ColorSensor::init()
{
    s0->setOut();
    s1->setOut();
    s2->setOut();
    s3->setOut();
    oe->setOut();
    out->setIn();
    setFrequencyMode(FREQUENCY::LOW);
    setColorMode(COLOR::WHITE);
    oe->write(false);
}

void ColorSensor::setFrequencyMode(uint8_t mode)
{
    frequencyMode = mode;
    switch(frequencyMode) {
        case FREQUENCY::LOW:
            s0->write(false);
            s1->write(true);
        break;

        case FREQUENCY::MEDIUM:
            s0->write(true);
            s1->write(false);
        break;

        case FREQUENCY::FULL:
            s0->write(true);
            s1->write(true);
        break;
    }
}

void ColorSensor::setColorMode(uint8_t color)
{
    colorMode = color;
    switch(colorMode) {
        case COLOR::RED:
            s2->write(false);
            s3->write(false);
        break;

        case COLOR::GREEN:
            s2->write(true);
            s3->write(true);
        break;

        case COLOR::BLUE:
            s2->write(false);
            s3->write(true);
        break;

        case COLOR::WHITE:
            s2->write(true);
            s3->write(false);
        break;
    }
}

uint16_t ColorSensor::getPeriod()
{
    bool start = out->read();
    uint64_t hmm = 0;
    while(out->read() == start) {
        hmm += 1;
    }
    uint32_t begin = sys.getUsTimVal();
    while(out->read() != start) {
        hmm += 1;
    }
    uint32_t period = (sys.getUsTimVal() - begin);

    return period;
}

uint8_t ColorSensor::getColor() {
        uint16_t r, g, b;
		setColorMode(ColorSensor::COLOR::RED);
		r = getPeriod();

		setColorMode(ColorSensor::COLOR::GREEN);
		g = getPeriod();

		setColorMode(ColorSensor::COLOR::BLUE);
		b = getPeriod();

        // printf("R:%d\tG:%d\tB:%d\r\n", r, g, b);

		uint8_t guess = COLOR::UNKNOWN;

		if(g < 8000) {
			guess = COLOR::GREEN;
		}

		if(b < 1600) {
			guess = COLOR::BLUE;
		}

		if(r < 3000) {
			guess = COLOR::RED;
		}

        if(r < 2700 && g < 2900 && b < 1000) {
            guess = COLOR::WHITE;
        }
		
        return guess;
}