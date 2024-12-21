#include "StepClock.h"


Step_Clock::Step_Clock() 
{


}

void Step_Clock::Init(AccelStepper* hour_motor, AccelStepper* minute_motor, RTC_DS1307* rtc)
{
	_hour_motor = hour_motor;
	_hour_motor->setMaxSpeed(1000.0);
	_hour_motor->setAcceleration(200.0);
	_hour_motor->setSpeed(500);
	
	_minute_motor = minute_motor;
	_minute_motor->setMaxSpeed(1000.0);
	_minute_motor->setAcceleration(200.0);
	_minute_motor->setSpeed(500);
	
	_rtc = rtc;
	_now = _rtc->now();
}


void Step_Clock::Calibration(int hallHour1Pin, int hallMin1Pin)
{	
	//Calibrate 12 o'clock
	int hallMin1Val=0;
	int hallMin1Calib=3000;
	
	int hallHour1Val=0;
	int hallHour1Calib=2400; //Magnet detection calibration
	
	Serial.println("Calibrating minute motor...");
    StepperHome(_minute_motor, hallMin1Val, hallMin1Pin, hallMin1Calib);// search for magnet
    Serial.println("Calibrating hours motor...");
	StepperHome(_hour_motor, hallHour1Val, hallHour1Pin, hallHour1Calib);// search for magnet
	Serial.println("Calibration done!");
}

void Step_Clock::SetHomePosition(void)
{
	Serial.println("Moving to home position...");
	//Code calibration, change these values if necessary
    _minute_motor->runToNewPosition(3675);//Manual calibration, change this value to achieve 0 position
    _hour_motor->runToNewPosition(3900);//Manual calibration, change this value to achieve 0 position
    
	//Manual calibration, comment if you are using code calibration or hands are already in place
	Serial.println("Insert minute and hour hand on the 12 o'clock position, you have 20 seconds");
    delay(20000);//Time to put the hour and minute hands on the correct position, you can change it
  
	//Set current position 0
    _minute_motor->setCurrentPosition(0);
    _hour_motor->setCurrentPosition(0);
}

void Step_Clock::StepperHome(AccelStepper* stepper, int hallSensorVal, int hallSensorPin, int hallSensorCalib)
{
	//this routine looks for magnet
	hallSensorVal = analogRead(hallSensorPin);
	
	while(hallSensorVal<hallSensorCalib)
	{
		//forward slowly till it detects the magnet
		stepper->move(100);
		stepper->run();
		hallSensorVal = analogRead(hallSensorPin);
		//Serial.println(hallSensorVal,DEC);
	}
	stepper->setCurrentPosition(0); 	
}

void Step_Clock::MoveMotorsToPosition(uint8_t hours, uint8_t minutes){

    //Adjust hours to 12h format
    if(hours>12)
	{
		hours=hours-12;  
    }

	//Calculate position
	positionHour=(hours*4096L/12);
	positionHour+=(minutes*4096L/720);//Calculate position based on the calibration
	positionMin=(minutes*4096L/60);//Calculate position based on the calibrationntln(positionMin);
    
	//Move motors to correct position 
	_minute_motor->enableOutputs();   
	_minute_motor->move(positionMin);
	_minute_motor->runToPosition();
	_minute_motor->stop();
	_minute_motor->disableOutputs();  
	_hour_motor->enableOutputs();
	_hour_motor->move(positionHour);
	_hour_motor->runToPosition();
	_hour_motor->stop();
	_hour_motor->disableOutputs(); 
}//end of moveMotorsToPosition------------------------------------------

void Step_Clock::MoveMotorsToHomePosition(void)
{
	MoveMotorsToPosition(0,0);
}

void Step_Clock::UpdateTime(void)
{
	_now = _rtc->now();
	_hours = _now.hour();
	_minutes = _now.minute();
	_seconds = _now.second();
	_hours24 = _hours;

	if(_hours>12)
	{
    _hours = _hours - 12;  
    }
}

void Step_Clock::UpdateTimeAndMove(void)
{
	UpdateTime();
	MoveMotorsToPosition(_hours,_minutes);
}

void Step_Clock::IncrementTime(void)
{
    if ( millis() - msTick >999) 
	{
	Serial.println("1 s");
	msTick = millis();
	_seconds++;
	Serial.print("Time: ");
	Serial.print(_hours);
	Serial.print(":");
	Serial.print(_minutes);
	Serial.print(":");
	Serial.println(_seconds);
	}
	
	if (_seconds == 60)
	{
		UpdateTime();
		_seconds = 0;
		Serial.println("60 s");
		// increment the time counters keeping care to rollover as required
		Serial.println("60 seconds, move one minute"); //debug message    
		_minute_motor->enableOutputs();
		_minute_motor->move(minuteStep1min);
		_minute_motor->runToPosition();
		_minute_motor->disableOutputs();
	  
		if (++_minutes >= 60) 
		{
			_minutes=0;
			//Compensate lost steps when dividing with integers
			Serial.println("60 minutes, compensate minutes"); //debug message    
			_minute_motor->enableOutputs(); 
			_minute_motor->move(16);// 4096/60=68.26 -> 68*60=4080 ->4096-4080=16
			_minute_motor->runToPosition();
			_minute_motor->disableOutputs();
		  
			if (++_hours >= 12) 
			{
				_hours=0;    
				//Compensate lost steps when dividing with integers
				Serial.println("12 hours, compensate hours"); //debug message   
				_hour_motor->enableOutputs(); 
				_hour_motor->move(32);// 4096/144=28.44 ->28*144=4032 -> 4096-4032=64/2 (because of 12h) = 32
				_hour_motor->runToPosition();
				_hour_motor->disableOutputs();
			}
			//Recalibrate and read time from RTC each 24 hours
			if(++_hours24 == 24)
			{
			_hours24=0;
			  //Recalibratiom
			_minute_motor->runToNewPosition(0);
			_hour_motor->runToNewPosition(0);
			  //move motors to correct position
			MoveMotorsToPosition(_hours,_minutes);
			}    
		}
		if (_minutes==10||_minutes==20||_minutes==30||_minutes==40||_minutes==50||_minutes==0||_minutes==60||_minutes==5||_minutes==15||_minutes==25||_minutes==35||_minutes==45||_minutes==55)
		{
			//Move hour motor each 5 minutes
			Serial.println("Move hour motor each 5 minutes"); //debug message
			_hour_motor->enableOutputs(); 
			_hour_motor->move(hourStep5min);
			_hour_motor->runToPosition();
			_hour_motor->disableOutputs();   
		}
	}
}

void Step_Clock::OneMinuteUp(void)
{
	_minute_motor->enableOutputs();
	_minute_motor->move(minuteStep1min);
	_minute_motor->runToPosition();
	_minute_motor->disableOutputs();	
}

void Step_Clock::OneMinuteDown(void)
{
	_minute_motor->enableOutputs();
	_minute_motor->move(-minuteStep1min);
	_minute_motor->runToPosition();
	_minute_motor->disableOutputs();		
}
  
void Step_Clock::OneHourUp(void)
{
	_hour_motor->enableOutputs(); 
	_hour_motor->move(hourStep1hour);
	_hour_motor->runToPosition();
	_hour_motor->disableOutputs(); 	
}

void Step_Clock::OneHourDown(void)
{
	_hour_motor->enableOutputs(); 
	_hour_motor->move(-hourStep1hour);
	_hour_motor->runToPosition();
	_hour_motor->disableOutputs(); 	
}
