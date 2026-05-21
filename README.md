# Arduino-powered-smoke-detecting-watch
It's an program using Arudion UNO (I used R3 version). Basic items you need include OLED, MQ-2, Button, and LED(1-color).
About the OLED:

The code is adjusted for 0.96-inch OLED, feel free to customize the OLED display layout based on your preferences.

About the ClockBasic.ino:

It should be always connected to the PC since the time is not saved, which means you need to set up a new time whenever connect the Arduino to computer.(You need to type on the Serial Monitor like 05:06:00(hh/mm/ss))

About the ClockPlus.ino

It uses EEPROM(which is built-in in the arduino board) to store the time you typed on the Serial Monitor. I didn't make the saving process automatic because frequent saving would reduce the lifespan of the EEPROM.


You can change the threshold of the MQ-2. When smoke level exceeds the threshold, the LED will be on, and you can press the button to see the recorded time, when the button is pressed, the LED will be automatically off.

When you want to check smoke level for the second time, you might need to run the code again since I haven't written any code to record for the second time yet.

The pin locations: 

OLED:
OLED/Arduino UNO
VCC --> 3.3V
 SCL --> A5(you cannot change this one)
 SDA --> A4(you cannot change this one)
 GND --> GND

 MQ2:
 MQ-2 / Arduino UNO
 VCC  -->   5V
 A0   -->   A0
 GND  -->   GND
 I didn't use D0

 LED:
 D8

 Button:
 3~
