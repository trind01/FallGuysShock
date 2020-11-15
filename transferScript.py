import serial

#Open ESP Port
ESPserial = serial.Serial('COM8',115200)

#Open Arduion Serial
ArdSerial = serial.Serial('COM11',9600)

#Write and Read to ESP
ESPserial.write(b'hello')
s = ESPserial.read(5)
print(s)

#Write To Arduino
ArdSerial.write(b'hello')

ESPserial.close()
ArdSerial.close()