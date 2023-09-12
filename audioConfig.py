import serial, time
ser = serial.Serial("/dev/ttyACM0", 38400)

#NOT WORKING. Send AT commands manually for initial config.

while True:
    ser.write("AT")
    bs = ser.readline()
    print(bs)
    time.sleep(2000)
    
    
    AT+UEXTDCONF=0,1

#echo -e "AT+UEXTDCONF=0,1" > /dev/ttyACM0
#screen /dev/ttyACM0 test "AT+UEXTDCONF=0,1"		#Configure audio codec
#screen /dev/ttyACM0 test "AT+CFUN=16"	#Reset module
#screen /dev/ttyACM0 test "AT+UTGN=1000,1000,100,0"		#Generate test tone
