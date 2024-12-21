#include <Wire.h>
#include "RTClib.h" //https://github.com/adafruit/RTClib
#include <AccelStepper.h> //http://www.airspayce.com/mikem/arduino/AccelStepper/
#include <StepClock.h>

#define motorHour1Pin1   6    // IN4 on the ULN2003 driver 1
#define motorHour1Pin2    7   // IN3 on the ULN2003 driver 1
#define motorHour1Pin3    8   // IN2 on the ULN2003 driver 1
#define motorHour1Pin4    9  // IN1 on the ULN2003 driver 1

// Minute pin definitions
#define motorMin1Pin1    10   // IN1 on the ULN2003 driver 2
#define motorMin1Pin2     11 // IN2 on the ULN2003 driver 2
#define motorMin1Pin3     12  // IN3 on the ULN2003 driver 2
#define motorMin1Pin4     13 // IN4 on the ULN2003 driver 2
#define hallMin1Pin       A0
#define hallHour1Pin       A1
#define HALFSTEP 8

#define DEBUG 1

#define CLK 2
#define DT 4

AccelStepper stepperHour1(HALFSTEP, motorHour1Pin1, motorHour1Pin3, motorHour1Pin2, motorHour1Pin4);
AccelStepper stepperMin1(HALFSTEP, motorMin1Pin1, motorMin1Pin3, motorMin1Pin2, motorMin1Pin4);
RTC_DS1307 rtc;

Step_Clock my_clock;

static unsigned long msTick =0;

int8_t seconds;

void setup() 
{
  my_clock.Debug(&Serial);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));//this sets time based on the compilation time of the computer
  my_clock.Init(&stepperHour1, &stepperMin1, &rtc);
  my_clock.Calibration(hallHour1Pin,hallMin1Pin);
  my_clock.SetHomePosition();
  my_clock.MoveMotorsToHomePosition();
  my_clock.UpdateTime();
  my_clock.UpdateTimeAndMove(); 
}

void loop() 
{
    //my_clock.UpdateTime();
    my_clock.IncrementTime();
}
