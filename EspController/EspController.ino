#include <Wire.h>
#include "BluetoothSerial.h"

bool start_game = false;
void ButtonInterrupt();

//Controls
void wakeUp();
bool isHolding();
bool isJumping();
bool isDiving();


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

//Relay Vars
const byte RELAY_PIN = 2;
void SetRelay(bool state);
bool relayState = false;

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
    movement(gyro_y,accelerometer_z); 
//    if(isDuck())
//    {
//      Duck();
//    }
//    else if(isJump())
//    {
//      Jump();
//    }
  }
  if (ESP_BT.available()) //Check if we receive anything from Bluetooth
  {
    char incoming = ESP_BT.read(); //Read what we recevive
    ESP_BT.write(incoming); //write what we recevive
//    if(incoming == 'F') //We lost
//    {
//      bleKeyboard.releaseAll();
//      start_game = false;
//      SetRelay(true);
//      delay(2000);
//      SetRelay(false);
//    }
    Serial.println(incoming);
  }
  delay(50);
}

void SetRelay(bool state)
{
  digitalWrite(RELAY_PIN,state);
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
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
  
  // print out data
  Serial.print("aX = "); Serial.print(convert_int16_to_str(accelerometer_x));
  Serial.print(" | aY = "); Serial.print(convert_int16_to_str(accelerometer_y));
  Serial.print(" | aZ = "); Serial.print(convert_int16_to_str(accelerometer_z));
  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
  Serial.print(" | tmp = "); Serial.print(temperature/340.00+36.53);
  Serial.print(" | gX = "); Serial.print(convert_int16_to_str(gyro_x));
  Serial.print(" | gY = "); Serial.print(convert_int16_to_str(gyro_y));
  Serial.print(" | gZ = "); Serial.print(convert_int16_to_str(gyro_z));
  Serial.println();
}

///////////////////////////////////////////////////////////
/////////////////////Control Functions/////////////////////
///////////////////////////////////////////////////////////

//gY will be the turning more positive is left, more negative is right
//aZ will determine if I'm moving forward
void movement(int16_t gY, int16_t aZ)
{
  int16_t x,y;
  //Check if should move forward
  if(aZ < 0){
//    y = min(-1*aZ,10000);
      y = 23000;
  } else
  {
    y = 0;
  }

  //Check if should turn
  if(gY < -10000)
  {
    //turn right
    x = gY * -1;
  } else if (gY > 18000)
  {
    //turn left
    x = gY * -1;
  } else
  {
    x = 0;
  }

  uint8_t mBytes[5] = {'m', uint8_t(x >> 8), (uint8_t)x, int8_t(y >> 8), (uint8_t)y};
  ESP_BT.write(mBytes,sizeof(mBytes));
}

void wakeUp()
{
  
}

bool isHolding()
{
  
}

bool isJumping()
{
  
}

bool isDiving()
{
  
}

bool isDuck()
{
  return (gyro_x > 10000 && accelerometer_z < -1000);
}

bool isJump()
{
  if(isDuck())
  {
    return false;
  }
  return (accelerometer_y > 19000);
}

void Jump()
{
//  if(bleKeyboard.isConnected())
  {
//    bleKeyboard.press(KEY_UP_ARROW);
    while(isJump())
    {
      getMPUReadings();
      delay(10);
    }
//    bleKeyboard.release(KEY_UP_ARROW);
  }
}

void Duck()
{
//    if(bleKeyboard.isConnected())
  {
//    bleKeyboard.press(KEY_DOWN_ARROW);
    while(accelerometer_z < 1000)
    {
      getMPUReadings();
      delay(10);
    }
//    bleKeyboard.release(KEY_DOWN_ARROW);
  }
}
