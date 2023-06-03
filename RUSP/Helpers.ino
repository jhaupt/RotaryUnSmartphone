void serialPassthrough(){	//Taken from Erik Nyquist's example
	if (Serial.available()) {     //Pass anything entered in Serial off to the TOBY 
		Serial1.write(Serial.read());   
	}
	if (Serial1.available()) {     // Pass anything received from TOBY to serial console
		Serial.write(Serial1.read()); 
	}
}

void serialPassthroughOut(){	
	if (Serial.available()) {     //Pass anything entered in Serial off to the TOBY 
		Serial1.write(Serial.read());   
	}
}

void ISR_hall(){	
	spinning = true;
	digitalWrite(LED_FILAMENT, HIGH);
	delay(10);
	digitalWrite(LED_FILAMENT, LOW);
}

void ISR_rotary(){	//Interrupt Service Routine for the rotary dial's "pulse" switch, which is a tied-to-GND normlally-open limit switch that rolls against a cam. 	
	interrupt_rot = millis();	//record time of interrupt
	pulseOn = true;
}

void rotary_accumulator(){ 	// Accumulates digits from rotary dial
	if (spinning == true && pulseOn == true && (millis() - interrupt_rot  > 30)) {	//Debounce detection
		pulseOn = false;
		digitalWrite(LED_STAT, HIGH);
		digitalWrite(LED5A, HIGH);
		digitalWrite(LED5R, HIGH);
		delay(10);
		n++;
		if (digitalRead(SW_Alt) == LOW){	//See if we're in alt mode 
			if (digitalRead(SW_Hook) == LOW){	//See if the Function button is depressed while in alt mode
				Speed = true;
			} else {
				Alt = true;
			}
		}
		else if (digitalRead(SW_Fn) == LOW){	//See if the Fn button is being depressed while the dial is turning.
			Fn = true;
		} 
	}
	digitalWrite(LED_STAT, LOW);
	digitalWrite(LED5A, LOW);
	digitalWrite(LED5R, LOW);
	//ACCUMULATE PULSES
	if (spinning == true && (millis() - interrupt_rot > 700)){	//If more than this many ms have elapsed since the last pulse, assume a number is complete
		PNumber[k] = n-2; //X: THIS NUMBER sometimes depends on the build?
		if (PNumber[k] >= 10){	//The tenth position on the dial is 0
			PNumber[k] = 0;
		}
		oled.clear();
		for (int j = 0; j <= k; j++){	//DISPLAY ON OLED
			//Serial.print(j);
			Serial.print(PNumber[j]);
			oled.print(itoa(PNumber[j],cbuf,10),(j*lspace),30); //A KLUGE. Should be able to send the whole number as a char array straight to oled.print and let that handle the character spacing. Instead, I'm using the loop index to advance the characters
		}

		Serial.println("");
		if (Fn == true){
			////ADD STAR/POUND/Ring demo DIAL FUNCTIONS
			Fn = false;
			if (PNumber[k] == 1){
				oled.clear();
				oled.print("*",0,30);
			}
			else if (PNumber[k] == 2){
				oled.clear();
				oled.print("#",0,30);
			}
			else if (PNumber[k] == 3){
				oled.clear();
				oled.print("Ring Demo",0,30);		//FIX to only show if UCALLSTAT shows active
				ringDemo = true;
			}
		}
		if (Alt == true){
			pg = epd_displayContacts(n);	
			oled.clear();
			ClearPNum();   //If in Alt mode, don't accumalate these numbers in the PNum buffer
			Alt = false;
		}
		if (Speed == true){
			SDgetContact((pg*9)-9+n);	//Get contact phone number based on the page number from the last time epd_showContacts was called and the rotary dial input
			ClearPNum();   //Clear whatever number has been entered so far
			k = kc;			//Set the current max phone number index to be that of the Contact Numbre
			for (int j = 0; j <= (k-2); j++){	//Y
				PNumber[j] = CNumber[j];	//Set current buffered phone number to be the contact number that SDgetContact provided 
				Serial.print(PNumber[j]);
			}
			Serial.println("");
			oled.clear();
			for (int j = 0; j <= k-2; j++){	//Y

				Serial.print(PNumber[j]);
				oled.print(itoa(PNumber[j],cbuf,10),(j*lspace),30); //A KLUGE. Should be able to send the whole number as a char array straight to oled.print and let that handle the character spacing. Instead, I'm using the loop index to advance the characters
			}
			toby.call(PNumber, k);
			Speed = false;
		}
		else {
			newNum = true;
			k++;	//advance to next number position
		}
		stopwatch_pNum = millis();
		n = 0;	//reset digit
		spinning = false;
		digitalWrite(LED5A, LOW);
		digitalWrite(LED5R, LOW);
	}
}

void pNumAutoClear(uint32_t pndelay){
	if (millis() > (stopwatch_pNum + pndelay)){
		ClearPNum();   //Clear whatever number has been entered so far
		;stopwatch_pNum = millis();
		toby.refresh();		//for good measure
	}
}

void ClearPNum(){  //Clear the currently entered phone number
	for (int j = 0; j < 30; j++){
		PNumber[j] = 0;  //Reset the phone number
	}
	no1s = 0;
	oled.clear();
	newNum = false;
	k = 0;    //Reset the position of the phone number
}

void BackspacePNum(){  //Clear the previously entered digit
	k--;
	PNumber[k] = 0;  //delete prevoius entry
}

long readVcc() {	//Read 1.1V reference against AVcc. Thank you to Scott: https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
	ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); // set the reference to Vcc and the measurement to the internal 1.1V reference
	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)); // measuring
	uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
	uint8_t high = ADCH; // unlocks both
	long mV = (high<<8) | low;
	
	mV = 1125300L / mV; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	return mV;
}

int readBatt() {	//Convert charge in mV (reported from readVcc) to percent	
	int chargeLevel;
	int LiPoRange = LIPOMAX - LIPOMIN;
	
	chargeLevel = ((readVcc() - LIPOMIN)*100)/LiPoRange;
	if (chargeLevel > 100){chargeLevel = 100;}
	if (chargeLevel < 0){chargeLevel = 0;}
	return chargeLevel; 
}

void dispSignal(){
	int k = 24;	//digit in chars returned from TOBY to pull out. KLUGE! Will probably fail in other regions.
	int len = 1;
	oled.clear();
	for (int j = 0; j <= len; j++){
		ibuf[j] = toby.signal(k) - '0';		//convert char to int
		k++;
	}
	oled.print("Signal:",0,30);
	for (int j = 0; j <= len; j++){	//DISPLAY ON OLED
		oled.print(itoa(ibuf[j],cbuf,10),75+(j*lspace),30); //A KLUGE. Should be able to send the whole number as a char array straight to oled.print and let that handle the character spacing. Instead, I'm using the loop index to advance the characters
	}
}

void dispCallID(){
	int k = 20;
	int len = 9;
	for (int j = 0; j <= 20; j++){
		ibuf[j] = 0;
	}
	oled.clear();
		for (int j = 0; j <= len; j++){
			ibuf[j] = toby.callID(k) - '0';		//convert char to int
			k++;
		}
		for (int j = 0; j <= len; j++){	//DISPLAY ON OLED
			oled.print(itoa(ibuf[j],cbuf,10),(j*lspace),30); //A KLUGE. Should be able to send the whole number as a char array straight to oled.print and let that handle the character spacing. Instead, I'm using the loop index to advance the characters
		}
}

void dispBatt(){
	oled.clear();
	Serial.print(F("BATT: "));
	Serial.print(readBatt());
	Serial.println(F("%"));
	oled.print("BATT:",0,30);
	if (battDisplay == 0){
		oled.print(itoa(readBatt(),cbuf,10),65,30);	//use itoa to display readBatt
		oled.print("%",100,30);
	} 
	else {
		oled.print(itoa(readVcc(),cbuf,10),65,30);	//use itoa to display readBatt
	}
}

void blinkRingLEDs() {
	if (ledToggle == true){		//make filament and bell LED flash alternatingly
		digitalWrite(LED_FILAMENT, HIGH);
		digitalWrite(LED_BELL, LOW);
		if (ledCounter % 100 == 0){
			ledToggle = false;
		}
	} else {
		digitalWrite(LED_FILAMENT, LOW);
		digitalWrite(LED_BELL, HIGH);
		if (ledCounter % 100 == 0){
			ledToggle = true;
		}
	}
}

void ringPattern(){
	ledCounter++;
	if (ledCounter <= ledRingPattern_t1){
		blinkRingLEDs();
		if (bellOn == true){	//ring bell only every other cycle
			dingBell();
		}	
	} 
	else if (ledCounter > ledRingPattern_t1 && ledCounter <= ledRingPattern_t2){
		analogWrite(LED_FILAMENT, 30);
		digitalWrite(LED_BELL, LOW);
		digitalWrite(RINGER_P, LOW);
		digitalWrite(RINGER_N, LOW);
	}
	else if (ledCounter > ledRingPattern_t2 && ledCounter <= ledRingPattern_t3){
		blinkRingLEDs();
		if (bellOn == true){
			dingBell();
		}
	}
	else if (ledCounter > ledRingPattern_t3 && ledCounter<= ledRingPattern_t4){
		analogWrite(LED_FILAMENT, 30);
		digitalWrite(LED_BELL, LOW);
		digitalWrite(RINGER_P, LOW);
		digitalWrite(RINGER_N, LOW);
	}
	else if (ledCounter > ledRingPattern_t4){
		ledCounter = 0;
		if (bellOn == true){ 	//toggle this bool
			bellOn = false;
		} else {
			bellOn = true;
		}
		ringCnt++;
		if (ringCnt > 3){
			ringDemo = false;
		}
	}
}

void dingBell(){	//Pulse the bell hammer (once per call)
	bellDelayCounter++;
	if (bellDelayCounter == bellDutyDelay){ 	//Turn solenoid off for part of the cycle
		digitalWrite(RINGER_P, LOW);
		digitalWrite(RINGER_N, LOW);
	}
	if (bellDelayCounter >= bellDelay){	//Turn solenoid on every full cycle (where bellDelayCounter == the bellDelay
		if (dingFlag == true){			//...and the solenoid polarity will depend on the previous state
			digitalWrite(RINGER_P, HIGH);
			digitalWrite(RINGER_N, LOW);
			dingFlag = false;
		}
		else {
			digitalWrite(RINGER_P, LOW);
			digitalWrite(RINGER_N, HIGH);
			dingFlag = true;
		}
		bellDelayCounter = 0;
	}
}

/*void calibrateBell(){	//pan through a range of bell hammer frequencies to find resonance
	if (bellCalOn == false){	//initialize some stuff the first time we're in this function
		bellCalOn = true;
		stopwatch_bellCal = millis();
		bellDelay = bellMin;
	}

	if (millis() < stopwatch_bellCal + bellPanDwell){
		dingBell();
	}
	else if (bellDelay > bellMax) {	//finish calibration routine at this value for bellDelay 
		bellDelay = bellMin;
		bellCalOn = false;	//main loop won't re-enter calibrateBell if this is false
	}
	else {		//when the bellCalSettleTime is exceeded, reset stopwatch_bellCal, increment the bellDelay, and print the value to the console
		stopwatch_bellCal = millis();	
		bellDelay++;
		Serial.println(bellDelay);
	}
}*/

void ringer_daemon(){
	if(toby.ringCheck() == true || ringDemo == true){
		ringPattern();
		if (ringCnt == 0 || ringCnt == 1){	//Run this twice per ring. For some reason the very first time it runs the text is garbled. 
			//dispCallID();		//doesn't work with ringDemo. Uncomment for normal use.
		}
	}
	else{
		digitalWrite(LED_FILAMENT, LOW);
		digitalWrite(LED_BELL, LOW);
		digitalWrite(RINGER_P, LOW);
		digitalWrite(RINGER_N, LOW);
		ringCnt = 0;
	}
}

void shutdown(){
	Serial.println("Shutdown signal sent. Waiting for cell powerdown...");
	oled.clear();
	oled.print("Turning off",0,30);
	digitalWrite(LED1A, HIGH);
	toby.powerdown();
	digitalWrite(RELAY_OFF, HIGH);	
	//POWER KILLED
}


