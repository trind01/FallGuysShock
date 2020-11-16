import serial
import time

#Open ESP Port
ESPserial = serial.Serial('COM8',115200)

#Open Arduion Serial
ArdSerial = serial.Serial('COM11',9600)

#Test Message for testing purposes
# ESPserial.write(b'hello')

#Pass messages between devices
while True:
	try:
		if ESPserial.in_waiting:
			espser = bytearray()
			while ESPserial.in_waiting:
				espser.extend(ESPserial.read(1))
			# print(espser.hex())
			ArdSerial.write(espser)
		if ArdSerial.in_waiting:
			ardser = bytearray()
			while ArdSerial.in_waiting:
				ardser.extend(ArdSerial.read(1))
			# ESPserial.write(ardser)
			# print(ardser.hex())
		time.sleep(0.01)
	except KeyboardInterrupt:
		print("End")
		break

#Close Out Ports
ESPserial.close()
ArdSerial.close()

# #Write and Read to ESP
# ESPserial.write(b'hello')
# s = ESPserial.read(5)
# print(s)
# #Write To Arduino
# ArdSerial.write(b'hello')

