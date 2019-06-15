/*
    Kod do obsługi czujnika kolorów TCS3200

    Sposób użycia:
        Utworzyć obiekt klasy ColorSensor
        Zainicjować go
        Pobrać kolor

    Przykład:
        ColorSensor cs = ColorSensor(&hExt.pin2, &hExt.pin1, &hExt.pin4, &hExt.pin3, &hExt.pin5, &hExt.serial.pinRx);
	    cs.init();
        while(true) {
            uint8 color = cs.getColor();
            printf("%d\r\n", color);
            sys.delay(1000);
        }

    Przy tworzeniu obiektu należy podać, do których pinów hExt zostały podpięte odpowiednie piny na module czujnika

    Aby kolory były wykrywane sprawnie i niezawodnie, należy skalibrować funkcję getColor
    Kalibracja polega na zmodyfikowaniu warunków w if-ach odpowiednio do warunków w których kolory mają być rozpoznawane

    Tips & Tricks:
    Czujnik najlepiej działa gdy kolorowy obszar jest duży
    Czujnik dobrze mieści się w ramce Lego Technic 5x7 (nr części 64179)
    Lego Łącznik Osi #2 (nr części 32034) umieszczony na środku ramki może pełnić funkcję czegoś w stylu obiektywu,
        sprawiając, że do czujnika światło odbite dochodzi wyłącznie od przodu
*/

#pragma once

#include "hFramework.h"

using hFramework::hGPIO;

class ColorSensor
{
    private:
    hGPIO *s0, *s1, *s2, *s3, *oe, *out;
    uint8_t frequencyMode;
    uint8_t colorMode;
    void setFrequencyMode(uint8_t mode);
    void setColorMode(uint8_t color);
    uint16_t getPeriod();

    public:
    enum FREQUENCY { FULL, MEDIUM, LOW };
    enum COLOR { RED, GREEN, BLUE, WHITE, UNKNOWN };
    ColorSensor(hGPIO *s0, hGPIO *s1, hGPIO *s2, hGPIO *s3, hGPIO *oe, hGPIO *out) : s0(s0), s1(s1), s2(s2), s3(s3), oe(oe), out(out) {};
    void init();
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

        /*
            Poniższa linjka pomaga w kalibracji
        */
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