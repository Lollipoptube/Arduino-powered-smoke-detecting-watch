# Arduino-powered-smoke-detecting-watch
It's an program using Arudion UNO (I used R3 version). Basic items you need include OLED, MQ-2, Button, and LED(1-color).
About the OLED:
The code is adjusted for 0.96-inch OLED, feel free to customize the OLED display layout based on your preferences.
The pin locations: 
OLED:
OLED   Arduino UNO
VCC --> 3.3V
SCL --> A5(you cannot change this one)
SDA --> A4(you cannot change this one)
GND --> GND

MQ-2:   Arduino UNO
VCC  -->   5V
A0   -->   A0
GND  -->   GND
I didn't use D0

LED:
D8

Button:
3~
