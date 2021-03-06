// Copyright 2014 Sebastian Zwierzchowski <sebastian.zwierzchowski<at>gmail<dot>com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef Controller_h
#define Controller_h
//Led strip Driver Pins

//Light sensor
#define LIGHTPIN A0

//OneWire thermometr pin
#define ONE_WIRE_BUS 7

//Arduino Uno pin 3
//Arduino Mega pin 5
// to change edit IRremoteIn.h
//#define IROUTPIN 3
//Irda modes
/*
#define OTHER 0;
#define NEC  1;
#define SONY 2;
#define PANASONIC 3;
#define JVC 4;
*/
//
#define WIRELESS_TRANSMITER_PIN 11
#define WIRELESS_RECIVER_PIN 0 //Reciver on inerrupt 0 => that is on pin #2

#include <IRremote.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RCSwitch.h>
#include "RgbStrip.h"


//#include <Wiring.h>

class Controller {
	public:
		Controller();
		//~Controller();
		void setupCtrl();
		
		void listen(bool echo);
	private:
		RgbStrip led;
		String inS;
		String offCode;
		IRrecv irrecv(int outPin);
		IRsend irsend;
		unsigned long startTime;
		int sendIR(String code);
		int sendWireless(String code);
		float getTemp(int num);
		void reportAllTemp();
		void tempReport(float temp, int id);
		int getLight(int num);
		void echo(String s);
		int command(String s);
		int getCode();
};
#endif
