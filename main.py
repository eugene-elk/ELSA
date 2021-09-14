import xgboost as xgb
import numpy as np
import sounddevice as sd
from math import atan2, pi, sqrt

from bbox_ops import *
from stuff import *

duration = 10
sound_border = 0.1

samplerate = sd.query_devices(None, 'input')['default_samplerate']

import serial, time
arduino = serial.Serial('/dev/ttyUSB0', 115200)
time.sleep(1) #give the connection a second to settle

bst = xgb.Booster()
bst.load_model('0001.model')

a = input()

waiting = True
print('waiting')

def callback(indata, frames, time, status):
    if status:
        print(status)
    if any(indata):
        
        mean = np.sqrt(np.mean(indata**2)) 
        print(mean)

        if (mean > sound_border) :
            global waiting 
            waiting = False

    else:
        print('no input')

def wait_sound(a = 0.3):
    global waiting, sound_border
    waiting = True
    sound_border = a

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

myrecording = sd.rec(int(duration * samplerate), samplerate=samplerate, channels=1)
sd.wait()
sequence = process_recording(myrecording, samplerate, bst)
print(sequence)


from melody_generator import MelodyGenerator

gen = MelodyGenerator()
melody = gen.generate(sequence[:4], 8)

melody.extend([20, 0])

for i in range(len(melody) // 2):
    
    arduino.write([melody[i*2], melody[i*2 + 1]])
    arduino.read()

time.sleep(0.5)

wait_sound()
arduino.write([1])

arduino.read()

def wait_a(a):
    global waiting
    waiting = True
    a.read()
    waiting = False
    print('!!!!')