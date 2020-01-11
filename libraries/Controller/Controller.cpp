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




RGBdriver Driver(CLK,DIO);
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
	this->red = 255;
	this->green = 255;
	this->blue = 255;
	this->status = false;
	this->bright = 0;
	this->startTime = 0;
	this->setRGB(0,0,0);
	this->rgbReport();
	wSwitch.enableTransmit(WIRELESS_TRANSMITER_PIN);
	wSwitch.enableReceive(WIRELESS_RECIVER_PIN);
	sensors.begin();
	sensors.requestTemperatures();
}


/*
 * Sending IR signal
 */

int Controller::sendIR(String code) {
	Serial.print("Sending signal IR : ");
	Serial.println(code);
	/*	code.mode.bits
		mode =
		0 - SONY
		1 - NEC
		2 - OTHER
		3 - JVC
		code = ir code
		bits 
	*/

	String tmp = "";
	unsigned long btnCode;
	int mode = 0;
	int bits = 12;
	//get valu form code
	int idx = code.indexOf('.');
	int idxOld = idx+1;
	tmp = code.substring(0,idx);
	btnCode = tmp.toInt();
	Serial.println(String("code " + btnCode));
	idx = code.indexOf('.', idxOld);
	tmp = code.substring(idxOld, idx);
	mode = tmp.toInt();
	Serial.println(String("mode " + tmp));
	tmp = code.substring(idx+1);
	bits = tmp.toInt();
	Serial.println(String("bits " + tmp));
	switch( mode ) {
		case 0:
		for (int i = 0; i < 3; i++) {
			irsend.sendSony(0x490, bits);
			delay(40);
		}
		//String ret = String(btnCode + "-" + bits);
		break;

		case 1:
		/*
		for (int i = 0; i < 3; i++) {
			irsend.sendNEC(btnCode, bits);
			delay(40);
		}
		*/
		Serial.println("nec");
		break;

		default:
		Serial.println("OI:unsuported");
		break;
    	}

	return 0;
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
	//get valu form code
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
 

/*
 * Get the light sensor state
 */
int Controller::getLight(int num) {
	//L.light_sensor_num
	int light = 0;
	for ( int i = 0; i<=2; i++) {
		light += analogRead(LIGHTPIN);
	}
	light = light/3;
	Serial.print("light:");
	Serial.print(num);
	Serial.print('.');
	Serial.println(light);
	return light; 
}


void Controller::echo(String s) {
	Serial.println(s);
}


/*
 * Set color on rgb stirp used by oteher method.Do not set color var
 */
void Controller::setRGB(int r, int g, int b) {
	Driver.begin();
	Driver.SetColor(r, g, b);
	Driver.end();
}


/*
 * Set color on rgb strip
 */
void Controller::setColor(inr rgb) {
	int r = rgb >> 16 & 255;
    int g = rgb >> 8 & 255;
    int b = rgb & 255;

	int current_r = this->red * this->bright / 100;
	int current_g = this->green * this->bright / 100;
	int current_b = this->blue * this->bright / 100;
	this->red   = r;
	this->green = g;
	this->blue  = b;
	this->bright = d;

	int target_r = r * d / 100;
	int target_g = g * d / 100;
	int target_b = b * d / 100;
	
	int change = 0;

	while (target_r != current_r or target_g != current_g or target_b != current_b) {
		if (current_r != target_r) {
			if (target_r < current_r) {current_r--;} else {current_r++;}
		}
		if (current_g != target_g) {
			if (target_g < current_g) {current_g--;} else {current_g++;}
		}
		if (current_b != target_b) {
			if (target_b < current_b) {current_b--;} else {current_b++;}
		}

		if (change == 4) {
			this->setRGB(current_r,current_g,current_b);
			change = 0;
		} else {
			change++;
		}

		
	}
	
	this->setRGB(target_r,target_g,target_b);
	this->rgbReport();
}

void Controller::rgbReport() {
	String red = String(this->red);
	String green = String(this->green);
	String blue = String(this->blue);
	String bright = String(this->bright);
	String ret = String("{\"cmd\": \"report\", \"model\": \"rgbstrip\", \"sid\": \"rgb01\", \"data\": {\"red\": " + red + ", \"green\": " + green + ", \"blue\": " + blue + ",\"bright\":" + bright + "}}");
	Serial.println(ret);
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
		case 'I':
	  		this->sendIR(code);
	  		break;
	  	
		case 'W':
	  		this->sendWireless(code);
	  		break;
	  	
		case 'D':
			this->setBright(code.toInt());
      		break;
			
		case 'C':
			this->setColor(code,toInt());
			break;
			
		case 'T':
	  		this->getTemp(code.toInt());
	  		break;
		/*	
		case 'L':
			this->getLight(code.toInt());
			break;
	    */
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

