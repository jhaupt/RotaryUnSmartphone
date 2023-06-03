/***************************************************
**********LICENSE***************
Rotary Un-Smartphone original firmware written by Justine Haupt. 
MIT license, this text must be included in any redistribution.
********************************

2/1/2021: v1.0

***Arduino IDE Setup Requirements***
	- The ATmega2560V (with the "V") is not supported by default. To fix this, install the "MegaCore" board configuration (read the "installation instructions", no need to git the repo): https://github.com/MCUdude/MegaCore

*****Needed libraries (install via the library manager)*****
	- adafruit_epd
	- EnableInterrupt
	- GxEPD2 (use the Sky's Edge fork at: , which includes support for the specific ePaper display used on the RUSP)

The board settings under Tools in the Arduino IDE should be as follows (after installing the above MegaCore configuration):
Board : ATmega2560 (in the MegaCore section)
Clock: External 8MHz
BOD: BOD 2.7v
EEPROM: EEPROM retained
Compiler LTO: LTO enabled
Pinout: Arduino MEGA pinout
Bootloader: Yes (UART0)
****************************************************/
#include "RUSP.h"

void setup(){
	enableInterrupt(SW_ROTARY, ISR_rotary, FALLING); // call ISR on rotary switch falling edge (input is internally pulled up).
	enableInterrupt(SW_HALL, ISR_hall, FALLING);

	// Set output pin functions
	pinMode(LED_STAT, OUTPUT);
	pinMode(LED_FILAMENT, OUTPUT);
	pinMode(LED_BELL, OUTPUT);
	pinMode(LED1A, OUTPUT);
	pinMode(LED2A, OUTPUT);
	pinMode(LED2R, OUTPUT);
	pinMode(LED3A, OUTPUT);
	pinMode(LED3R, OUTPUT);
	pinMode(LED4A, OUTPUT);
	pinMode(LED4R, OUTPUT);
	pinMode(LED5A, OUTPUT);
	pinMode(LED5R, OUTPUT);
	pinMode(RINGER_P, OUTPUT);
	pinMode(RINGER_N, OUTPUT);
	pinMode(RELAY_OFF, OUTPUT);
	pinMode(LL_OE, OUTPUT);
	pinMode(EN_3V3, OUTPUT);
	pinMode(EN_12V, OUTPUT);
	pinMode(EN_OUTAMP, OUTPUT);
	pinMode(CELL_ON, OUTPUT);
	pinMode(CHIPSELECT, OUTPUT); //SD card CS

	// Define input pin functions
	pinMode(SW_ROTARY, INPUT_PULLUP); 	//Input handled by ISR, but here we set the pullup resistor
	pinMode(SW_C, INPUT_PULLUP);
	pinMode(SW_Hook, INPUT_PULLUP);
 	pinMode(SW_alpha, INPUT_PULLUP);
 	pinMode(SW_beta, INPUT_PULLUP);
 	pinMode(SW_lambda, INPUT_PULLUP);
 	pinMode(SW_Fn, INPUT_PULLUP);
 	pinMode(SW_Local, INPUT_PULLUP);
 	pinMode(SW_Alt, INPUT_PULLUP);
 	pinMode(SW_Nonlocal, INPUT_PULLUP);
	pinMode(SW_HALL, INPUT_PULLUP);			//Input handled by ISR, but here we set the pullup resistor
	pinMode(OFFSIGNAL, INPUT_PULLUP);
	pinMode(CHG_STAT, INPUT);
	
	digitalWrite(LED_BELL, HIGH);
	Serial.begin(115200);	//try different values

	digitalWrite(LL_OE, HIGH);
	digitalWrite(EN_3V3, HIGH);

	delay(2000);
	digitalWrite(CELL_ON, HIGH); delay(1000);	//Turn on LARA
	digitalWrite(CELL_ON, LOW); 
	digitalWrite(LED_STAT, HIGH);		//Indicate waiting for LARA to power on
	//digitalWrite(EN_OUTAMP, HIGH);
	while (digitalRead(A2) == LOW){
	}	//wait for CELL_PWR_DET to go high. !!! Needa timeout here!

	digitalWrite(LED_STAT, LOW);		//When led goes off, we know LARA is powered on
	delay(1000);

	toby.init();	
	eink.init(9600);
	eink.setRotation(0);
	eink.setTextColor(GxEPD_BLACK);
	SDinit();
	SDgetConfig();
		//SDinfo();
		//SDreadAll();
	n = 0;    //Starting phone number digit value is 0
	k = 0;    //Starting phone number digit position is 0
	oled.init(); 
	digitalWrite(EN_12V, HIGH);
	Serial.println("startup complete");
	digitalWrite(LED_BELL, LOW);
	//SPI.begin(); //handled elsewhere
}

void loop(){
	//toby.rx();	//check for and read UART data from TOBY. Use with serialPassthroughOut(), OR, use serialPassthrough() by itself.
	//serialPassthroughOut();
	serialPassthrough();	//Expose TOBY's UART-in to serial console

	rotary_accumulator();	//Counts pulses from rotary and does debounce. Pulses are initiated by ISR_rotary().
	oled.autoClear(50000);	//Clear the OLED automatically after given time in ms.
	pNumAutoClear(70000);
	ringer_daemon(); 	//Check for incomming call and manage the call alert bell and LEDs

	if (digitalRead(OFFSIGNAL) == LOW){
		shutdown();
	}

	if (digitalRead(SW_Alt) != LOW){	
		Alt = false;
	}

	if (digitalRead(SW_Hook) == LOW){
		if (toby.ringCheck() == true){	//ANSWER INCOMING CALL
			toby.answer(); 
		} 
		else {
			toby.hangup();		//HANG UP
			ringDemo = false;	//stop the ring demo, if it's on.
			oled.clear();
			//oled.print("Call ended",0,30);		//FIX to only show if UCALLSTAT shows active
		}
		delay(500);
		if (digitalRead(SW_Hook) == LOW){ 	//PLACE CALL. NOTE: Speed dial using SW_Hook handled in rotary_accumulator() in Helpers.ino.
			oled.clear();
			oled.print("Dialing out",0,30);
			if (digitalRead(SW_Local) == LOW){
				toby.callLocal(areaCode, PNumber, k);	//k is number of digits in phone number	
			}
			else if (digitalRead(SW_Nonlocal) == LOW){
				toby.call(PNumber, k);
			}
		}
	}

	if (digitalRead(SW_C) == LOW){
		delay(100);	//debounce
		if (digitalRead(SW_C) == LOW){ 
			BackspacePNum();   //Clear whatever number has been entered so far
			digitalWrite(LED_STAT, HIGH);
			delay(100); 
			digitalWrite(LED_STAT, LOW);
		}
		delay(500);
		if(digitalRead(SW_C) == LOW){
			ClearPNum();   //Clear whatever number has been entered so far
			digitalWrite(LED_STAT, HIGH);
			delay(100); 
			digitalWrite(LED_STAT, LOW);
		}
	}

	if (digitalRead(SW_alpha) == LOW){
		/*if (testRing == false){
			testRing = true;
			delay(6000);
		} 
		else {
			testRing = false;
		}*/
		epd_splash();
	}

	if (digitalRead(SW_beta) == LOW){	
		dispBatt();
	}
	
	if (digitalRead(SW_lambda) == LOW){
		dispSignal();
		delay(200);
	}
}
