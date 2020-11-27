#include <Wire.h>
#include "BluetoothSerial.h"

bool start_game = false;
void ButtonInterrupt();

//Controls
bool Jumping = false;
unsigned long jumpDebounce = 0;
int jumpCount = 0;

//Accelerometer vars
const byte MPU_ADDR = 0x68;
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data
char tmp_str[7]; // temporary variable used in convert function
char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

BluetoothSerial ESP_BT; //Object for Bluetooth

//Button vars. Button is active low
const byte BOOT_BUTTON_PIN = 0;

//Relay Vars/ Rumble Controls
const byte RELAY_PIN = 2;
void SetRelay(bool state);
bool relayState = false;
unsigned long rumbleDebounce = 0;

void setup() {

  //Set up Button and relay
  pinMode(BOOT_BUTTON_PIN,INPUT);
  pinMode(RELAY_PIN,OUTPUT);
  
  //Set Up Serial
  Serial.begin(115200);

  //Set up Accelerometer
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  //Set up and Serial
  ESP_BT.begin("ESP Bluetooth Controller"); //Name of your Bluetooth Signal

  //Set up Interrupt for button
  attachInterrupt(BOOT_BUTTON_PIN, ButtonInterrupt, FALLING);
}

void loop() {
  if(start_game)
  {
    getMPUReadings(); //Update Movement Variables
    movement(accelerometer_x,accelerometer_z); 
    Jump();
    Dive();
  }
  if (ESP_BT.available()) //Check if we receive anything from Bluetooth
  {
    char incoming = ESP_BT.read(); //Read what we recevive
    if(incoming == 'r') //We lost
    {
      uint8_t rumbleLeft = ESP_BT.read(); //Read what we recevive
      uint8_t rumbleRight = ESP_BT.read(); //Read what we recevive
      SetRelay(true);
    }
  }
  rumbleCheck();
delay(50);
}

void rumbleCheck()
{
  if(relayState && (millis() - rumbleDebounce > 100)) 
  {
    SetRelay(false);
  }
}

void SetRelay(bool state)
{
  digitalWrite(RELAY_PIN,state);
  relayState = state;
  rumbleDebounce = millis();
}

void ButtonInterrupt()
{
  start_game = !start_game;
//  relayState = !relayState;
//  SetRelay(relayState);
  SetRelay(false);
  uint8_t w = 'w';
  ESP_BT.write(&w,1);
}

void getMPUReadings()
{
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
//   "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
  
  // print out data
//  Serial.print("aX = "); Serial.print(convert_int16_to_str(accelerometer_x));
//  Serial.print(" | aY = "); Serial.print(convert_int16_to_str(accelerometer_y));
//  Serial.print(" | aZ = "); Serial.print(convert_int16_to_str(accelerometer_z));
//  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
//  Serial.print(" | tmp = "); Serial.print(temperature/340.00+36.53);
//  Serial.print(" | gX = "); Serial.print(convert_int16_to_str(gyro_x));
//  Serial.print(" | gY = "); Serial.print(convert_int16_to_str(gyro_y));
//  Serial.print(" | gZ = "); Serial.print(convert_int16_to_str(gyro_z));
//  Serial.println();
}

///////////////////////////////////////////////////////////
/////////////////////Control Functions/////////////////////
///////////////////////////////////////////////////////////

//aX will be the turning more negative is left, more positive is right
//aZ will determine if I'm moving forward
void movement(int16_t aX, int16_t aZ)
{
  int16_t x,y;
  //Check if should move forward
  if(aZ < 0){
    y = abs((int16_t)map(aZ, 0, -5500, 23000, 32767));
  } else
  {
    y = 0;
  }

  //Check if should turn
  if(aX < -2000)
  {
    //turn left
    x = -16000;
  } else if (aX > 4000)
  {
    //turn right
    x = 16000;
  } else
  {
    x = 0;
  }

  uint8_t mBytes[5] = {'m', uint8_t(x >> 8), (uint8_t)x, int8_t(y >> 8), (uint8_t)y};
  ESP_BT.write(mBytes,sizeof(mBytes));
}

void Hold()
{
  
}


void Dive()
{
  if(Jumping && (gyro_x > 7000))
  {
    uint8_t d = 'd';
//    Serial.println("dive");
    ESP_BT.write(&d,1);
  }
}

void Jump()
{
  if( (accelerometer_y > 20000) && !Jumping && ((millis() - jumpDebounce) > 500) && (accelerometer_y != 0xFFFF) )
  {
    jumpCount++;
    if(jumpCount >= 3)
    {
      Jumping = true;
      uint8_t j = 'j';
      jumpCount = 0;
      ESP_BT.write(&j,1);
//      Serial.println("JUMP");
    }
  }
  else if( Jumping && (accelerometer_y < 17000) && (accelerometer_y > 4000) )
  {
    Jumping = false;
    uint8_t f = 'f';
    ESP_BT.write(&f,1);
    jumpDebounce = millis();
//    Serial.println("Fall");
  }
}
