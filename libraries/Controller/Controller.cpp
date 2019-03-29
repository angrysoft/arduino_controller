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
	this->red = 0;
	this->green = 0;
	this->blue = 0;
	this->tick = 0;
	this->setRGB(0,0,0);
	wSwitch.enableTransmit(WIRELESS_TRANSMITER_PIN);
	wSwitch.enableReceive(WIRELESS_RECIVER_PIN);
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
int Controller::getTemp(int num) {
	//return T.thermometr_num.temp
	sensors.requestTemperatures();
	delay(50);
	Serial.print("temp:");
	Serial.print(num);
	Serial.print('.');
	Serial.println(sensors.getTempCByIndex(num));
	return 0;
}

void Controller::reportAllTemp() {
    
}

void Controller::tempReport(int temp, int id) {
	Serial.print("'cmd': 'report', 'model': 'dallastemp', 'sid': 'dallasT1', 'data': {'temp': ");
	Serial.print(temp);
	Serial.println("}");
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
void Controller::setColor(String colors) {
	int r = 0;
	int g = 0;
	int b = 0;
	
	int idx = colors.indexOf('.');
	int idxOld = idx+1;
	
	String tmp;
	//Red
	tmp = colors.substring(0, idx);
	r = tmp.toInt();
	
	//Green
	idx = colors.indexOf('.',idxOld);
	tmp = colors.substring(idxOld,idx);
	g = tmp.toInt();
	
	//Blue
	tmp = colors.substring(idx+1);
	b = tmp.toInt();
	
	this->red   = r;
	this->green = g;
	this->blue  = b;
	this->setRGB(r,g,b);
	this->rgbReport(r,g,b);
}

/*
 * Set color on rgb strip with fade
 */

void Controller::setFadeColor (String colors) {
	int r = 0;
	int g = 0;
	int b = 0;
	int idx = colors.indexOf('.');
	int idxOld = idx+1;
	String tmp;
	
	//Red
	tmp = colors.substring(0, idx);
	r = tmp.toInt();
	
	//Green
	idx = colors.indexOf('.',idxOld);
	tmp = colors.substring(idxOld,idx);
	g = tmp.toInt();
	
	//Blue
	tmp = colors.substring(idx+1);
	b = tmp.toInt();

	int change = 0;
	
	while (r != this->red or g != this->green or b != this->blue) {
		if (r != this->red) {
			if (r < this->red) {this->red--;} else {this->red++;}
		}
		if (g != this->green) {
			if (g < this->green) {this->green--;} else {this->green++;}
		}
		if (b != this->blue) {
			if (b < this->blue) {this->blue--;} else {this->blue++;}
		}

		if (change == 4) {
			this->setRGB(this->red,this->green,this->blue);
			change = 0;
		} else {
			change++;
		}

		
	}
	this->setRGB(r,g,b);
	this->rgbReport(r,g,b);
}

void Controller::rgbReport(int r, int g,int b) {
    Serial.print("'cmd': 'report', 'model': 'rgbstrip', 'sid': '0x0000000000000001', 'data': {'red': ");
	Serial.print(r);
	Serial.print(", 'green': ");
	Serial.print(g);
	Serial.print(", 'blue': ");
	Serial.print(b);
	Serial.println("}");
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
	  	
		case 'C':
			this->setColor(code);
      		break;
			
		case 'F':
			this->setFadeColor(code);
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
*/	this->tick++;
    if (tick >= 3600) {
        this->reportAllTemp();
        this->tick = 0;
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

