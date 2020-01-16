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

#include "Controller.h"




// RGBdriver Driver(CLK,DIO);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RCSwitch wSwitch = RCSwitch();

/* 
 * Constructor
 */
const byte LED_PIN = 13;


Controller::Controller() {
	pinMode(LED_PIN,OUTPUT);
	//Serial.begin(9600);
	this->inS.reserve(200);
}

void Controller::setupCtrl() {
	this->startTime = 0;
	this->led.report();
	wSwitch.enableTransmit(WIRELESS_TRANSMITER_PIN);
	wSwitch.enableReceive(WIRELESS_RECIVER_PIN);
	sensors.begin();
	sensors.requestTemperatures();
}

/*
 * Sending wireless singal 433Mhz
 */

int Controller::sendWireless(String code) {
	//code.protocol
	String tmp = "";
	//code to send
	unsigned long sendCode;
	//protocol
	int p = 1;
	//bit length
	int bit = 24;
	//get value form code
	int idx = code.indexOf('.');
	tmp = code.substring(0,idx);
	sendCode = tmp.toInt();
	tmp = code.substring(idx+1);
	p = tmp.toInt();

	if ( p == 1) {
		bit = 24;
		wSwitch.setProtocol(1);
		wSwitch.setPulseLength(203);
		wSwitch.setRepeatTransmit(5);
		wSwitch.send(sendCode,24);
	} else if ( p == 2) {
		bit = 32;
		wSwitch.setProtocol(2);
		wSwitch.send(sendCode,32);
	}

	return 0;
}

int Controller::getCode() {
	unsigned long value = wSwitch.getReceivedValue();
	if (value != 0) {
		Serial.print("OW.");
    	Serial.println(value);
	}
	wSwitch.resetAvailable();
	return 0;
}


/*
 * Get temperature from sensor
 */
float Controller::getTemp(int num) {
	sensors.requestTemperatures();
	delay(50);
	return sensors.getTempCByIndex(num);
}

void Controller::reportAllTemp() {
	int count = sensors.getDeviceCount();
	sensors.requestTemperatures();
	delay(50);
	int i;
	for (i = 0; i < count; i++) {
		this->tempReport(sensors.getTempCByIndex(i), i);
	}
}

void Controller::tempReport(float temp, int id) {
	String s_temp = String(temp);
	String s_id = String(id);
	String ret = String("{\"cmd\": \"report\", \"model\": \"dallastemp\", \"sid\": \"dallasDS" + s_id + "\", \"data\": {\"temp\": " + s_temp + "}}");
	Serial.println(ret);
}
 
void Controller::echo(String s) {
	Serial.println(s);
}

int Controller::command(String s) {
	//Serial.println(s);
	if (s == "ping") {
		Serial.println("pong");
		return 0;
	}
	
	int idx = s.indexOf('.');
	if (idx == -1) {return 0;}
	String cmd = "";
	String code = "";
  
	cmd = s.substring(0,idx);
  	code = s.substring(idx+1);
	
	if (cmd == "" or code == "" or cmd.length() > 1) {
  	  return -1;
	}
  	
	switch (cmd[0]) {
		case 'W':
	  		this->sendWireless(code);
	  		break;
	  	
		case 'B':
			this->led.set_bright(code.toInt());
      		break;
			
		case 'C':
			this->led.set_rgb(code.toInt());
			break;
			
		case 'P':
			this->led.set_power(code.toInt());
			break;
			
		case 'T':
	  		this->getTemp(code.toInt());
	  		break;
	}
      
  	return 0;
}

void Controller::listen(bool echo) {
/*	
	if (wSwitch.available()) {
		this->getCode();
	}
*/
	unsigned long currentTime = millis();
	if (currentTime - this->startTime > 60000) {
		this->reportAllTemp();
		this->startTime = currentTime;
    }

	while (Serial.available()) {
		char inC = Serial.read();
		
		if (inC == '\n') {
			if (echo) {
				Serial.println(this->inS);	
			}
			this->command(inS);
			this->inS = "";
			break;
		}
		this->inS += inC;
	}
}

