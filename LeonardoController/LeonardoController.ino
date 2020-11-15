#include <XInput.h>

String inputString = "";
int LEDPIN = 8;
bool LEDLevel = false;

void setup() {
	XInput.begin();
  Serial1.begin(9600);
  pinMode(LEDPIN,OUTPUT);
  digitalWrite(LEDPIN,LOW);
}

void loop() {
//	XInput.press(BUTTON_A);
//	delay(1000);
//
//	XInput.release(BUTTON_A);
//	delay(1000);
  if(Serial1.available()){
    while(Serial1.available())
    {
      char inChar = (char)Serial1.read(); //read the input
      if(inChar == 'e')
      {
        LEDLevel = !LEDLevel;
        digitalWrite(LEDPIN,LEDLevel);
      }
    }
  }
  delay(100);
}
