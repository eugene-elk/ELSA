import serial, time

# Устанавливаем соединение с ардуино, скорость соединения 115200 бод
arduino = serial.Serial('/dev/ttyUSB0', 115200, timeout=.1)

# Задержка для установки соединения
time.sleep(1) 

positions = [
# open / closed
    [0, 0],			#0
    [5, 20, 40],	#1
    [25, 50],  		#2
    [60, 20],   	#3
    [20, 55],   	#4
    [70, 20],   	#5
    [60, 20],   	#6
    [40, 65],   	#7
    [70, 20],   	#8
    [30, 10],   	#9
    [22, 5]  		#10
]

while True:
	a = input()

	seq = []
	# Создаем массив из введённых значений
	arr = [x for x in a.split()]
	# print(arr)

	# Берём значения из массива для открытого или закрытого положения выбранного пальца
	seq.append(arr[0])
	if (arr[1] == "open"):
		seq.append(positions[int(arr[0])][0])
	if (arr[1] == "close"):
		seq.append(positions[int(arr[0])][1])
	
	# print(seq) 
	arduino.write(seq)