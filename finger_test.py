import serial, time

# Устанавливаем соединение с ардуино, скорость соединения 115200 бод
arduino = serial.Serial('/dev/ttyUSB0', 115200, timeout=.1)

# Задержка для установки соединения
time.sleep(1) 

# TODO сделать единый settings.json



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
		if (arr[1] == "open") or (arr[1] == "o"):
			seq.append(200)
		elif (arr[1] == "mid") or (arr[1] == "m"):
			seq.append(201)
		elif (arr[1] == "close") or (arr[1] == "c"):
			seq.append(202)
		elif (int(arr[1]) >= 3) and (int(arr[1]) <= 100): # Проверка на адекватность числового значения
			seq.append(int(arr[1]))
		else:
			correct = False

	elif (seq[0] <= 10) and (seq[0] >= 0):
		if (arr[1] == "open") or (arr[1] == "o"):
			seq.append(200)
		elif (arr[1] == "close") or (arr[1] == "c"):
			seq.append(201)
		elif (int(arr[1]) >= 3) and (int(arr[1]) <= 100): # Проверка на адекватность числового значения
			seq.append(int(arr[1]))
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