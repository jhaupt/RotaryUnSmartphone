#include <avr/io.h>
#include <GxEPD2_BW.h>
#include <SPI.h>
#include <EnableInterrupt.h>
//#include <Flash.h>
#include <SD.h>
#include "TOBY.h"
#include "OLED.h"
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerifItalic9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSansOblique9pt7b.h>

#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= 800 / (EPD::WIDTH / 8) ? EPD::HEIGHT : 800 / (EPD::WIDTH / 8))

//Define output pins
#define ENABLE_GxEPD2_GFX 0
#define SW_ROTARY 2 
#define LED_STAT 47
#define LED_FILAMENT 46
#define LED_BELL 4
#define LED1A 8		
#define LED2A 6	
#define LED2R 16		
#define LED3A 17
#define LED3R 40
#define LED4A 42
#define LED4R A6
#define LED5A A7
#define LED5R A8
#define RINGER_P 21
#define RINGER_N 20
#define OFFSIGNAL 27
#define RELAY_OFF 35
#define LL_OE 38	//output enable for logic level converters
#define EN_3V3 34
#define EN_12V 33
#define EN_OUTAMP A15
#define CELL_ON A0
#define CHIPSELECT 24	//SD card CS

//Define input pins
#define SW_C 15
#define SW_Hook 14
#define SW_alpha 10
#define SW_beta 12
#define SW_lambda 11
#define SW_Fn 13
#define SW_Local 32
#define SW_Alt 31
#define SW_Nonlocal 30
#define SW_HALL 3
#define CHG_STAT 44

//Battery constants
#define LIPOMAX 4150	//in mV
#define LIPOMIN 3500	//Go as low as 2.75? Batt failsafe cutoff is ~2.5

//SD constants
#define CONFIGFILE "config.txt"
#define CONTACTFILE "contacts.txt"
#define KEY_MAX_LENGTH    30 // change it if key is longer
#define VALUE_MAX_LENGTH  30 // change it if value is longer

//Ring pattern constants
#define ledRingPattern_t1 2000
#define ledRingPattern_t2 3000
#define ledRingPattern_t3 5000
#define ledRingPattern_t4 11000

//For bell hammer frequency calibration routine
#define bellMin 30	
#define bellMax 55
#define bellPanDwell 100
#define bellDutyDelay 68
#define bellDelay 47	//sets bell hammer frequency

//For SD config and contacts management. String types are required by a copied section of code. Switching to char arrays will require work.
int battDisplay = 0;
//Faied attempts to define strings to pass key IDs to SD functions:
//const __FlashStringHelper *cNameID = "cName";
//const __FlashStringHelper *cNumID = "cNum";
//FLASH_STRING(cNameID, "cName");
//FLASH_STRING(cNumID, "cNum");
//const String cNameID PROGMEM = "cName";	//contact name ID
//const String cNumID PROGMEM = "cNum";	//contact number ID.
int areaCode = 123;	//area code for local calling mode
int firstBoot = 0;	//Flag that determines whether the audio codec should be configured, which only needs to happen once.
String cName;	//contact name. 
int cNum = 0;	//contact number

typedef struct {
		int number[30]; //array for recalling contact phone numbers stored on SD card
	} contactBook;

contactBook contact;

//Define global variables:
int n = 0;   //For counting up numbers from the rotary dial for entering a digit in the phone number
int k = 0;   //Index for current/buffered phone number
int kc = 0;	//Index for contact phone number

const byte lspace = 12;	//Applies to KLUGE sections to print ints to OLED in Helpers.ino
const byte sspace = 6; //""
int pg=1;	//Contacts list page number
int PNumber[30]; // an array to store phone numbers as they're dialed with the rotary dial
int CNumber[30]; // To store phone numbers recalled from the contacts list
char CName[30];	// To store contact names recalled from contacts list
char cbuf[64];	//buffer for characters converted from ints with itoa()
int ibuf[20];
int no1s = 0;	//Applies to sections labeled "A KLUGE" in rotary_accumulator. Counts number of "1s" in PNumber.
bool CallOn = false;   //Set to "true" when a call is in progress, to determine the function of the "call_startedn" pin.
bool pulseOn = false;
bool spinning = false;
bool testRing = false;
bool Fn = false;	//flag for function mode (used in function called by rotary ISR)
bool Alt = false;
bool Speed = false;	//flag for speed dial mode (used in function called by rotary ISR)
bool newNum = false;
bool ledToggle = true;	//used to rapidly alternate certain LEDs on and off 
bool dingFlag = true;
bool bellOn = false;
bool bellCalOn = false;
bool ringDemo = false;
int ringCnt = 0;
int ledCounter = 0;	//also used for the above
int bellDelayCounter = 0;	//for the bell calibration function
uint32_t stopwatch_bellCal;
uint32_t stopwatch_pNum;
uint32_t interrupt_rot;
int lhlf;
int rhlf;
int pagenum;		//holder for page number
int mode;	

//Instantiate objects. Note that a trailing "()" confuses C++, so if there are no arguments it must be omitted. The class name is therefore like a type in a variable definition.
Sd2Card card;
SdVolume volume;
SdFile root;
TOBY toby(A5, A3, A4, A1, A2);	//Pins = (NET_STAT, CELL_CTS, CELL_RTS, CELL_RESET, CELL_PWR_DET)
GxEPD2_BW<GxEPD2_290_flex, MAX_HEIGHT(GxEPD2_290_flex)> eink(GxEPD2_290_flex(25, 26, 28, 29)); //Pin order is ECS, D/C, RESET, BUSY. For Adafruit 2.13" Flexible Monochrome EPD (AKA GDEW0213I5F)
OLED oled; 
File myFile;

