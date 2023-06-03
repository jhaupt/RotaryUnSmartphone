/*
 * Manage control of u-Blox TOBY-R2
 * J. Haupt
 *
 * */

#include <Arduino.h>
#include "TOBY.h"

TOBY::TOBY(byte NET_STAT, byte CELL_CTS, byte CELL_RTS, byte CELL_RESET, byte CELL_PWR_DET){	//The constructor
	_NET_STAT = NET_STAT;
	_CELL_CTS = CELL_CTS;
	_CELL_RTS = CELL_RTS;
	_CELL_RESET = CELL_RESET;
	_CELL_PWR_DET = CELL_PWR_DET;
	pinMode(_NET_STAT, INPUT);
	pinMode(_CELL_CTS, INPUT);
	pinMode(_CELL_PWR_DET, INPUT);
	pinMode(_CELL_RTS, OUTPUT);
	pinMode(_CELL_RESET, OUTPUT);
}

void TOBY::init(){	//Some basic setup for TOBY
	Serial1.begin(115200); delay(500);	//57600 was TOBY, 115200 is LARA. Other options: 230400
	Serial1.write("AT"); Serial.write(13); delay(1000);
	Serial1.write("AT+CMEE=2"); Serial1.write(13); delay(100);	//Turn on verbose error reporting
	Serial1.write("AT+CREG=1"); Serial1.write(13); delay(100);	//Turn on network registration URCs
	Serial1.write("AT+UCALLSTAT=1"); Serial1.write(13); delay(100);	//Turn on call status reporting URCs
	//Serial1.write("AT+UMGC=1,12,16000"); Serial1.write(13); delay(200);	//Set microphone digital gain
	//Serial1.write("AT+CLVL=80"); Serial1.write(13); delay(200);	//Set call volume
	//Serial1.write("AT+UDCONF=31,1"); Serial1.write(13); delay(200);	//Turn on DTMF tones. 31,1 = local only, 31,2 = in-band, 31,0 = off.
}

void TOBY::firstBOOT(){	//deprecated in favor of running
	Serial1.write(13);
	//Serial1.write("AT+CSGT=1,\"LARA is ON and READY\""); Serial1.write(13);delay(200);
	Serial1.write("AT+UEXTDCONF=0,1"); Serial1.write(13); delay(500);	//Autoconfigure MAX9860 codec. Only needs to be done one time ever, as the settings are saved to NVM, although I don't know if the NVM is in the CODEC or in the uBlox module.
	//or
	//Serial1.write("AT+UI2S=14,1,0,3,0"); Serial1.write(13); delay(50);	//part of manual configuration of MAX9860 codec. Only needs to be done one time ever, as the settings are saved to NVM, although I don't know if the NVM is in the CODEC or in the uBlox module.
	//Serial1.write("AT+USPM=1,0"); Serial1.write(13); delay(50);	//part of manual configuration of MAX9860 codec. Only needs to be done one time ever, as the settings are saved to NVM, although I don't know if the NVM is in the CODEC or in the uBlox module.
	//Serial1.write("AT+UMCLK=2,0"); Serial1.write(13); delay(50);	//part of manual configuration of MAX9860 codec. Only needs to be done one time ever, as the settings are saved to NVM, although I don't know if the NVM is in the CODEC or in the uBlox module.
	//Serial1.write("AT+UI2CO=1,0,0,0x10,0"); Serial1.write(13); delay(50);	//part of manual configuration of MAX9860 codec. Only needs to be done one time ever, as the settings are saved to NVM, although I don't know if the NVM is in the CODEC or in the uBlox module.
	//Serial1.write("AT+UI2CW=\"0000000010A000303000063300500000008A\",18"); Serial1.write(13); delay(50);	//part of manual configuration of MAX9860 codec. Only needs to be done one time ever, as the settings are saved to NVM, although I don't know if the NVM is in the CODEC or in the uBlox module.
	//Serial1.write("AT+UI2CC"); Serial1.write(13); delay(50);	//part of manual configuration of MAX9860 codec. Only needs to be done one time ever, as the settings are saved to NVM, although I don't know if the NVM is in the CODEC or in the uBlox module.

//	Serial1.write("AT+CFUN=16"); Serial1.write(13); delay(1000);	//Reset uBlox module.
	Serial.println(F("First-boot configuration complete"));
}

void TOBY::refresh(){	//was used for debugging
	Serial1.write(13); delay(200);
	Serial1.write("AT+CMEE=2"); Serial1.write(13); delay(200);	//Turn on verbose error reporting
	Serial1.write("AT+CREG=1"); Serial1.write(13); delay(50);	//Turn on network registration URCs
}

void TOBY::powerdown(){
	Serial1.write("AT+CPWROFF"); Serial1.write(13); delay(50);
	_stopwatch_shutdown = millis();
	while (digitalRead(_CELL_PWR_DET) == HIGH){ //wait for CELL_PWR_DET to go LOW
		if (millis() - _stopwatch_shutdown > 5000){	//Timeout
			break;
		}
	}	
}

void TOBY::sleep(){
	Serial1.write("AT+UPSV=1,3000"); Serial1.write(13); delay(50);	//Turn on verbose error reporting
}

void TOBY::wake(){
	Serial1.write("AT+UPSV=0"); Serial1.write(13); delay(50);	//Turn on verbose error reporting
}

bool TOBY::ringCheck(){	//There's an RI line now. Switch to that as primary.
	if (_NewData == true){
		//----Look for "RING" embedded in string---
		if (_ReceivedChars[0] == 'R'){	//I think this should be a double quote. Don't know why this (and only this) works.
			if (_ReceivedChars[1] == 'I'){
				if (_ReceivedChars[2] == 'N'){
					if (_ReceivedChars[3] == 'G'){
						_ring = true;
					}
				}
			}
		}
		_NewData = false;
		_stopwatch_ring = millis();
	}
	if (millis() > _stopwatch_ring+5000){
		_ring = false;
	}
	return _ring;
}

void TOBY::answer(){
	Serial1.write("ATA"); Serial1.write(13); delay(500);
	_ring = false;
}

void TOBY::hangup(){
	Serial1.write("ATH"); Serial1.write(13); delay(500);
}

void TOBY::callLocal(int areaCode, int PNumber[30], int k){
	Serial1.write("AT+FCLASS=8"); Serial1.write(13); delay(50);
	Serial1.print("ATD");
	Serial1.print(areaCode);
	for (int j = 0; j < k; j++){	//Dial number in rotary buffer
		Serial1.print(PNumber[j]);
	}
	Serial1.write(13); delay(500);	
}

void TOBY::call(int PNumber[30], int k){
	Serial1.write("AT+FCLASS=8"); Serial1.write(13); delay(50);
	Serial1.print("ATD");
	for (int j = 0; j < k; j++){	//Dial number in rotary buffer
		Serial1.print(PNumber[j]);
	}
	Serial1.write(13); delay(500);	
}

void TOBY::speed1(){
	Serial1.write("AT+FCLASS=8"); Serial1.write(13); delay(50);
	Serial1.write("ATD6315994176"); Serial1.write(13); delay(500);	
}

void TOBY::speed2(){
	Serial1.write("AT+FCLASS=8"); Serial1.write(13); delay(50);
	Serial1.write("ATD6313545680"); Serial1.write(13); delay(500);	
}

void TOBY::speed3(){
	Serial1.write("AT+FCLASS=8"); Serial1.write(13); delay(50);
	Serial1.write("ATD6313756912"); Serial1.write(13); delay(500);	
}

char TOBY::signal(int digitRequest){
	Serial1.write("AT+CESQ"); Serial1.write(13);// delay(50);
	rx();
	Serial.println(_ReceivedChars);
	return _ReceivedChars[digitRequest];
}

char TOBY::callID(int digitRequest){
	Serial1.write("AT+CLCC"); Serial1.write(13);// delay(50);
	rx();
	Serial.println(_ReceivedChars);
	return _ReceivedChars[digitRequest];
}

char TOBY::rx(){
	static byte n = 0;
	char EndMarker = 10;   //ASCII 10 = newline, 13 = carriage return.  
	char rc;
	int cnt;
 	while (Serial1.available() && _NewData == false){	
		rc = Serial1.read();
		if (rc != EndMarker){
			_ReceivedChars[n] = rc;
			n++;
			if (n >= _nChars){
				n = _nChars - 1;
			}
		}
		else{
			_ReceivedChars[n] = '\0'; // terminate the string
			_maxchars = n;
			n = 0;
			_NewData = true;
			Serial.print(rc);
			//Serial.println(_ReceivedChars[64]);
			//return _ReceivedChars;//[64];
		}
	}

}
