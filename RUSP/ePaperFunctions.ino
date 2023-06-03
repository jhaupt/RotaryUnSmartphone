//Make the ePaper eink do various things

int epd_displayContacts(int n){
	int col = 0;
	int row = 12;
	int ctcID = 1;
	n = n-2;	//Don't understand why this is needed, again.
	eink.fillScreen(GxEPD_WHITE); // set the background to white (fill the buffer with value for white)
	eink.setFont();	//default font
	eink.firstPage();	//this function is called before every time ePaper is updated. Has nothing to do with what I call page numbers in this section of the program.
	do {
		for(int line = (n*9)-8; line <= (n*9); line++){		//loop through 10 lines/contacts (1 "page") starting with the page dialed.
			SDgetContact(line);	//Fills CName[] and CNumber[] for the current line
			eink.setFont(&FreeSans9pt7b);	//Font for contact name
			eink.setCursor(col, row);
			eink.print(ctcID);
			eink.setCursor(col+10, row);
			eink.print(".");	
			eink.setCursor(col+16, row);
			eink.print(CName);
			eink.setCursor(col+25, row+12);	//space to phone number below contact name
			eink.setFont();		//Font for contact phone number
			for (int j = 0; j < (kc-2); j++){	//Count up to the highest stored numbre in CNumber
				eink.print(CNumber[j]);
				if (j == 2 || j == 5){
					eink.print("-");	//Format the phone number with dashes
				}
			}
			row = row + 33;	//Space to next contact name
			ctcID++;
		}
		row = 12;	//reset row for the next iteration of the do/while loop
		ctcID = 1;	//reset contact ID
	} while (eink.nextPage()); 
	eink.hibernate();
	return n;
}

void epd_splash(){
	eink.firstPage();	//this function is called before every time ePaper is updated. Has nothing to do with what I call page numbers in this section of the program.
  	eink.setFullWindow();
	do {
		eink.fillScreen(GxEPD_WHITE); // set the background to white (fill the buffer with value for white)
		eink.setFont(&FreeSans9pt7b);
		eink.setCursor(12, 30); 
		eink.print("R");
		eink.setCursor(31, 35); 
		eink.print("o");
		eink.setCursor(50, 40); 
		eink.print("t");
		eink.setCursor(69, 45); 
		eink.print("a");
		eink.setCursor(88, 50); 
		eink.print("r");
		eink.setCursor(107, 55); 
		eink.print("y");
		eink.setFont(&FreeSerifItalic9pt7b);
		eink.setCursor(15, 70); 
		eink.print("UnSmArT");
		eink.setFont(&FreeSerif9pt7b);
		eink.setCursor(30, 90); 
		eink.print("PHONE");
		eink.setCursor(8, 110); 
		eink.setFont();
		eink.print("(for making calls)");
		eink.setCursor(42, 150); 
		eink.setFont(&FreeMono9pt7b);
		eink.print("AKA");
		eink.setFont(&FreeSerifItalic9pt7b);
		eink.setCursor(10, 180); 
		eink.print("an electronic,");
		eink.setFont(&FreeSans9pt7b);
		eink.setCursor(35, 200); 
		eink.print("portable,");
		eink.setFont(&FreeMono9pt7b);
		eink.setCursor(15, 220); 
		eink.print("digital,");
		eink.setFont(&FreeSerifItalic9pt7b);
		eink.setCursor(30, 240); 
		eink.print("wireless,");
		eink.setFont(&FreeMonoBold9pt7b);
		eink.setCursor(10, 260); 
		eink.print("TELEPHONE");
		delay(50);
	} while (eink.nextPage());
	eink.hibernate();	//If this isn't here, wonky behavior ensues.
}

void epd_splashOld(){
	eink.firstPage();	//this function is called before every time ePaper is updated. Has nothing to do with what I call page numbers in this section of the program.
  	eink.setFullWindow();
	do {
		eink.fillScreen(GxEPD_WHITE); // set the background to white (fill the buffer with value for white)
		eink.setFont();	//default font
		eink.setCursor(2, 185); 
		eink.print("Missed call:");
		eink.setFont(&FreeMonoBold9pt7b);
		eink.setCursor(8, 208); 
		eink.print("???-????");
		eink.setFont(&FreeSans9pt7b);
		eink.setCursor(5, 60); 
		eink.print("Wireless");
		eink.setFont(&FreeSerifItalic9pt7b);
		eink.setCursor(30, 80); 
		eink.print("Electronic");
		eink.setFont(&FreeMono9pt7b);
		eink.setCursor(10, 100); 
		eink.print("Digital");
		eink.setFont(&FreeSerif9pt7b);
		eink.setCursor(33, 120); 
		eink.print("Portable");
		eink.setFont(&FreeSans9pt7b);
		eink.setCursor(0, 145); 
		eink.print("TeLePhOnE");
		delay(50);
	} while (eink.nextPage());
	eink.hibernate();	//If this isn't here, wonky behavior ensues.
}
