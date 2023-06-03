void SDinit(){
	if (card.init(SPI_HALF_SPEED, CHIPSELECT)) {
		Serial.println("SD Card Found");
	} else {
		Serial.println("No SD Card");
	}

	SD.begin(CHIPSELECT);	//Is this eating a lot of power?
}

void SDinfo(){
	Serial.print("Card type:         ");
	switch (card.type()) {
		case SD_CARD_TYPE_SD1:
			Serial.println("SD1");
			break;
		case SD_CARD_TYPE_SD2:
			Serial.println("SD2");
			break;
		case SD_CARD_TYPE_SDHC:
			Serial.println("SDHC");
			break;
		default:
			Serial.println("Unknown");
	}

	// Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
	if (!volume.init(card)) {
		Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
		while (1);
	}
	Serial.print("Clusters:          ");
	Serial.println(volume.clusterCount());
	Serial.print("Blocks x Cluster:  ");
	Serial.println(volume.blocksPerCluster());
	Serial.print("Total Blocks:      ");
	Serial.println(volume.blocksPerCluster() * volume.clusterCount());
	Serial.println();
	// print the type and size of the first FAT-type volume
	uint32_t volumesize;
	Serial.print("Volume type is:    FAT");
	Serial.println(volume.fatType(), DEC);
	volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
	volumesize *= volume.clusterCount();       // we'll have a lot of clusters
	volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
	Serial.print("Volume size (Kb):  ");
	Serial.println(volumesize);
	Serial.print("Volume size (Mb):  ");
	volumesize /= 1024;
	Serial.println(volumesize);
	Serial.print("Volume size (Gb):  ");
	Serial.println((float)volumesize / 1024.0);
	Serial.println("\nFiles found on the card (name, date and size in bytes): ");
	root.openRoot(volume);
	// list all files in the card with date and size
	root.ls(LS_R | LS_DATE | LS_SIZE);
	root.close();
}

void SDreadAll(){
	SD.begin(CHIPSELECT);
	myFile = SD.open("contacts.txt");
	if (myFile){
		while (myFile.available()){
			Serial.write(myFile.read());
		}
		myFile.close();
	} else{
		Serial.println(F("The file is missing from the SD card."));
	}
}

//COPIED CODE ALERT
//Thanks to arduinogetarted.com. What's below was more or less copied from https://arduinogetstarted.com/tutorials/arduino-read-config-from-sd-card
void SDgetConfig(){
	battDisplay = SD_findInt(F("battDisplay"));
	areaCode = SD_findInt(F("areaCode"));
	
//	if (firstBoot == 1){
//		toby.setCODEC();
//		delay(500);
//	}
	//bellMin = SD_findInt(F("bellMin"));
	//bellMax = SD_findInt(F("bellMax"));

	Serial.print(F("battDisplay = "));
	Serial.println(battDisplay);
	Serial.print(F("Local area code = "));
	Serial.println(areaCode);
}

void SDwriteContact(){	//debug only
	SD.begin(CHIPSELECT);
	myFile = SD.open("contacts.txt", FILE_WRITE);
	//contact[0].name = "JaneDoe";
	//contact[0].number = 15; 
	//contact[1].name = "JonSow";
	//contact[1].number = 24; 
	//myFile.print(contact[0].name);
	//myFile.print(" ");
	//myFile.println(contact[0].number); 
	//myFile.print(contact[1].name);
	//myFile.print(" ");
	//myFile.println(contact[1].number); 
	//myFile.close();
	Serial.println("wrote contacts?");
}

void SDgetContact(int line){
	int m = 0;
	char cholder;
	bool preColon = false;
	bool exitNow = false;
	SD.begin(CHIPSELECT);
	myFile = SD.open("contacts.txt", FILE_READ);
	myFile.seek(0);
	kc = 0;		//reset the contact phone number index
	for(int i = 0; i < (line -1 );){		//This block brings myFile.read to the requested contact number
		cholder = myFile.read();
		if (cholder == '\n'){	//Inc i when we find a carriage return.
			i++;
		} else if (cholder == '#'){
			cholder = 0;
			exitNow = true;
			break;
		}
	}
	while(true){	//Now we are at the right line. Loop through each byte in this line until we hit another CR. Infinite loop broken only by break statement if NL.
		if (exitNow == true){
			break;
		}
		cholder = myFile.read();
		if (preColon == false){
			if (cholder == ':'){	//detect colon
				preColon = true;
			} else {
				CName[m] = cholder;
				m++;
			}
		} else {
			if (cholder == ' '){	//detect space
				//Serial.print(": ");
			} else {
				CNumber[kc] = cholder-'0';
				kc++;
			}
		}
		if (cholder == '\n'){
			break;
		}
	}
	while (m <= 30){	//Be sure the remainder of CName is empty
		CName[m] = 0;
		m++;
	}
	m = 11;			//Truncate name to m chars to fit on ePD
	while(m <= 30){
		CName[m] = 0;
		m++;
	}
	myFile.close();
}

int SD_findInt(const __FlashStringHelper * key) {
	char value_string[VALUE_MAX_LENGTH];
	int value_length = SD_findKey(key, value_string);
	return HELPER_ascii2Int(value_string, value_length);
}

String SD_findString(const __FlashStringHelper * key) {
	char value_string[VALUE_MAX_LENGTH];
	int value_length = SD_findKey(key, value_string);
	return HELPER_ascii2String(value_string, value_length);
}

int SD_findKey(const __FlashStringHelper * key, char * value) {
	File configFile = SD.open(CONFIGFILE);

	if (!configFile) {
		Serial.print(F("SD Card: error on opening file "));
		Serial.println(CONFIGFILE);
	}

	char key_string[KEY_MAX_LENGTH];
	char SD_buffer[KEY_MAX_LENGTH + VALUE_MAX_LENGTH + 1]; // 1 is = character
	int key_length = 0;
	int value_length = 0;

	// Flash string to string
	PGM_P keyPoiter;
	keyPoiter = reinterpret_cast<PGM_P>(key);
	byte ch;
	do {
		ch = pgm_read_byte(keyPoiter++);
		if (ch != 0)
			key_string[key_length++] = ch;
	} while (ch != 0);

	// check line by line
	while (configFile.available()) {
		int buffer_length = configFile.readBytesUntil('\n', SD_buffer, 100);
		if (SD_buffer[buffer_length - 1] == '\r')
			buffer_length--; // trim the \r

		if (buffer_length > (key_length + 1)) { // 1 is = character
			if (memcmp(SD_buffer, key_string, key_length) == 0) { // equal
				if (SD_buffer[key_length] == '=') {
					value_length = buffer_length - key_length - 1;
					memcpy(value, SD_buffer + key_length + 1, value_length);
					break;
				}
			}
		}
	}

	configFile.close();  // close the file
	return value_length;
}

int HELPER_ascii2Int(char *ascii, int length) {
  int sign = 1;
  int number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

String HELPER_ascii2String(char *ascii, int length) {
  String str;
  str.reserve(length);
  str = "";

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str += String(c);
  }

  return str;
}
