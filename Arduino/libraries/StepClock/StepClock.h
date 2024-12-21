
#ifndef Step_Clock_h
#define Step_Clock_h

#if (ARDUINO >= 100)
  #include <Arduino.h>
#else
  #include <WProgram.h>
  #include "pins_arduino.h"
#endif

#include <Wire.h>
#include "RTClib.h" //https://github.com/adafruit/RTClib
#include <AccelStepper.h> //http://www.airspayce.com/mikem/arduino/AccelStepper/

#define hourStep5min   (int)4096/144
#define minuteStep1min  (int)4096/60
#define hourStep1hour   (int)4096/12


class Step_Clock {

public:

	Step_Clock();
	void Init(AccelStepper* hour_motor, AccelStepper* minute_motor, RTC_DS1307* rtc);
	void Calibration(int hallHour1Pin, int hallMin1Pin);
	void StepperHome(AccelStepper* stepper, int hallSensorVal, int hallSensorPin, int hallSensorCalib);
	void MoveMotorsToPosition(uint8_t hours, uint8_t minutes);
	void MoveMotorsToHomePosition(void);
	void UpdateTime(void);
	void UpdateTimeAndMove(void);
	void SetHomePosition(void);
	void IncrementTime(void);
	void OneMinuteUp(void);
	void OneMinuteDown(void);
	void OneHourUp(void);
	void OneHourDown(void);
	int hallMin1Pin=2;
	int hallHour1Pin=4;
private:

	AccelStepper* _hour_motor;
    AccelStepper* _minute_motor;
	RTC_DS1307* _rtc;
	DateTime _now;
	long positionHour;
	long positionMin;
	unsigned long msTick =0;
	uint8_t _hours;
	uint8_t _minutes;
	uint8_t _hours24;
	uint8_t _seconds;

};
//extern Step_Clock_Class Step_Clock;

#endif	//



