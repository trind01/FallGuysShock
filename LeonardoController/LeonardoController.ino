#include <XInput.h>

//For Debugging
int LEDPIN = 8;
bool LEDLevel = false;

//Control Parsing
void parseControl(char ctrl);
void rumbleCallback(uint8_t packetType);

void setup() {
	XInput.begin();
  Serial1.begin(9600);
  pinMode(LEDPIN,OUTPUT);
  digitalWrite(LEDPIN,LOW);

  // Set callback function. Function must have a 'void' return type
  // and take a single uint8_t as an argument
  XInput.setReceiveCallback(rumbleCallback);
}

void loop() {
  if(Serial1.available()){
    char inChar = (char)Serial1.read(); //read the input
    parseControl(inChar);
  }
  delay(10);
}

void parseControl(char ctrl)
{
  switch(ctrl)
  {
    case 'w': //Wake Up Press
    {
      XInput.press(BUTTON_A);
      XInput.release(BUTTON_A);
      break;
    }
    case 'j': //Jump
    {
      XInput.press(BUTTON_A);
      break;
    }
    case 'f': //Stop Jump/fall
    {
      XInput.release(BUTTON_A);
    }
    case 'd': //Dive
    {
      XInput.press(BUTTON_X);
      XInput.release(BUTTON_X);
      break;
    }
    case 'h': //Hold
    {
      while(!Serial1.available()){}
      uint8_t trigger_set = Serial1.read(); //read the input
      XInput.setTrigger(TRIGGER_RIGHT ,trigger_set);
      break;
    }
    case 'm': //Movement
    {
      int16_t x = 0,y = 0;
      while(!Serial1.available()){}
      uint8_t xHi = Serial1.read(); //read the input
      while(!Serial1.available()){}
      uint8_t xLo = Serial1.read(); //read the input
      while(!Serial1.available()){}
      uint8_t yHi = Serial1.read(); //read the input
      while(!Serial1.available()){}
      uint8_t yLo = Serial1.read(); //read the input
      x = (xHi << 8) | xLo;
      y = (yHi << 8) | yLo;
      XInput.setJoystick(JOY_LEFT,x,y);
      XInput.setJoystick(JOY_RIGHT,x,y);
//      Serial1.write((uint8_t *)x,2);
//      Serial1.write((uint8_t *)y,2);
      break;
    }
    default:
    {
      break;
    }
  }
}

void rumbleCallback(uint8_t packetType) {
  // If we have an LED packet (0x01), do nothing
  if (packetType == (uint8_t) XInputReceiveType::LEDs) {
    return;
  }

  // If we have a rumble packet (0x00), see our rumble data on the LED
  else if (packetType == (uint8_t) XInputReceiveType::Rumble) {
      uint8_t rumbleLeft = XInput.getRumbleLeft();
      uint8_t rumbleRight = XInput.getRumbleRight();
      if(rumbleLeft)
      {
        LEDLevel = !LEDLevel;
        digitalWrite(LEDPIN,LEDLevel);
      }
  }
}
