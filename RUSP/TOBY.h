#ifndef TOBY_H
#define TOBY_H
#include <Arduino.h>

class TOBY {
	public:
		TOBY(byte NET_STAT, byte CELL_CTS, byte CELL_RTS, byte CELL_RESET, byte CELL_PWR_DET);
		char rx();
		void init();
		void firstBOOT();
		void refresh();
		bool ringCheck();
		char signal(int digitRequest);
		char callID(int digitRequest);
		void powerdown();
		void answer();
		void hangup();
		void speed1();
		void speed2();
		void speed3();
		void callLocal(int areaCode, int PNumber[30], int k);
		void call(int PNumber[30], int k);
		void sleep();
		void wake();
	private:
		const byte _nChars = 64;	//Number of characters available in ReceivedChars array
		char _ReceivedChars[64];
		bool _NewData = false;  // a flag to indicate whether a string has been received over RS232
		bool _ring = false;
		bool _poff = false;
		int _maxchars;
		int _sigStrength;
		unsigned long _stopwatch_ring;
		unsigned long _stopwatch_shutdown;
		byte _NET_STAT;
		byte _CELL_CTS;
		byte _CELL_RTS;
		byte _CELL_RESET;
		byte _CELL_PWR_DET;
};

#endif
