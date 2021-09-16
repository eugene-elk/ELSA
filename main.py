import xgboost as xgb
import numpy as np
import sounddevice as sd
from math import atan2, nan, pi, sqrt

from stuff import *
from melody_generator import MelodyGenerator

duration = 10
sound_border = 0.3

# Узнаем сэмплрейт (частота записи звука) дефолтного микрофона
samplerate = sd.query_devices(None, 'input')['default_samplerate']

import serial, time
arduino = serial.Serial('/dev/ttyUSB0', 115200)
time.sleep(1) #give the connection a second to settle

# Загружаем модельку
bst = xgb.Booster()
bst.load_model('0001.model')

a = input()

# Флаг ожидания звука
waiting = True
print('waiting')

# Обработчик данных с микрофона
def callback(indata, frames, time, status):

    if status:
        print(status)

    if any(indata):
        # Если есть доступные данные - считаем среднюю мощность звука
        mean = np.sqrt(np.mean(indata**2)) 
        print(mean)
        
        if (mean > sound_border) :
            # Если звук достаточно громкий - перестаём ждать
            global waiting 
            waiting = False
    else:
        print('no input')


def wait_sound():
    global waiting
    waiting = True

    with sd.InputStream(channels=1, callback=callback, blocksize=int(samplerate * 0.05), samplerate=samplerate):
        while waiting:
            pass
    print('waiting for sound')

wait_sound()

arduino.write([1])
arduino.read()

time.sleep(1)

arduino.write([1])
arduino.read()

wait_sound()

arduino.write([1])
arduino.read()

# Записываем duration секунд
myrecording = sd.rec(int(duration * samplerate), samplerate=samplerate, channels=1)
# Ждём пока запишется
sd.wait()
# Распознаём аккорды
sequence = process_recording(myrecording, samplerate, bst)
print(sequence)

# Создаём генератор, передаём ему первые 4 аккорда, 8 четвертых в одном аккорде (скорее всего)
gen = MelodyGenerator()
melody = gen.generate(sequence[:4], 8)
# Добавляем знак конца мелодии (либо тоника)
melody.extend([20, 0])

# Передача мелодии на Ардуино
for i in range(len(melody) // 2):
    arduino.write([melody[i*2], melody[i*2 + 1]])
    arduino.read()

time.sleep(0.5)

# Ждём начала игры гитариста
wait_sound()
arduino.write([1])

arduino.read()