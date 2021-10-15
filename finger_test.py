import serial, time

# Устанавливаем соединение с ардуино, скорость соединения 115200 бод
arduino = serial.Serial('/dev/ttyUSB0', 115200, timeout=.1)

# Задержка для установки соединения
time.sleep(1) 

# TODO сделать единый settings.json

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

	# Последовательность для отправки (содержит информацию об одном пальце и его положении)
	seq = []

	# Флаг корректности введённого значения
	correct = True

	# Создаем массив из введённых значений
	arr = [x for x in a.split()]
	print(arr)

	# Берём значения из массива для открытого или закрытого положения выбранного пальца
	seq.append(int(arr[0]))
	if (seq[0] == 1): # Палец номер 1 (большой) имеет также среднее положение, рассматриваем его отдельно
		if arr[1] == "open":
			seq.append(positions[int(arr[0])][0])
		elif arr[1] == "mid":
			seq.append(positions[int(arr[0])][1])
		elif arr[1] == "close":
			seq.append(positions[int(arr[0])][2])
		else:
			correct = False

	elif (seq[0] <= 10) and (seq[0] >= 0):
		if arr[1] == "open":
			seq.append(positions[int(arr[0])][0])
		elif arr[1] == "close":
			seq.append(positions[int(arr[0])][1])
		else:
			correct = False
	
	else:
		correct = False

	# Проверка корректности ввода - на случай опечаток
	if correct:
		arduino.write(seq)
		print("sent: ", seq)
	else:
		print("incorrect input")