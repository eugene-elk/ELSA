from midi_parser import parse_midi
import serial, time

melody = parse_midi('midi/scale3.mid')
print(melody)

# Устанавливаем соединение с ардуино, скорость соединения 115200 бод
arduino = serial.Serial('/dev/ttyUSB1', 115200, timeout=.1)

# Задержка для установки соединения
time.sleep(1) 

print(len(melody))

for x in melody:
    arduino.write([x[0]])
    arduino.write([x[1]])
    arduino.write([x[2] >> 8])
    arduino.write([x[2] & 255])
    arduino.read()