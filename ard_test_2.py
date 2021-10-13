import serial, time

# Устанавливаем соединение с ардуино, скорость соединения 115200 бод
arduino = serial.Serial('/dev/ttyUSB0', 115200, timeout=.1)

# Задержка для установки соединения
time.sleep(1) 

while True:
	a = input()

	

	# Создаем массив из введённых значений
	seq = [int(x) for x in a.split()]
	
	arduino.write(seq)