import librosa
import numpy as np
import xgboost as xgb



note_codes = {
    'C' : 0,
    'C#': 1,
    'D' : 2,
    'D#': 3,
    'E' : 4,
    'F' : 5,
    'F#': 6,
    'G' : 7,
    'G#': 8,
    'A' : 9,
    'A#': 10,
    'B' : 11,
    } 
note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
classes = {
    'C'  : 0,
    'Cm' : 1,
    'C#' : 2,
    'Cm#': 3,
    'D'  : 4,
    'Dm' : 5,
    'D#' : 6,
    'Dm#': 7,
    'E'  : 8,
    'Em' : 9,
    'F'  : 10,
    'Fm' : 11,
    'F#' : 12,
    'Fm#': 13,
    'G'  : 14,
    'Gm' : 15,
    'G#' : 16,
    'Gm#': 17,
    'A'  : 18,
    'Am' : 19,
    'A#' : 20,
    'Am#': 21,
    'B'  : 22,
    'Bm' : 23
    }

names = ['C', 'Cm', 'C#', 'Cm#', 'D', 'Dm', 'D#', 'Dm#', 'E', 'Em', 'F', 'Fm', 'F#', \
'Fm#', 'G', 'Gm', 'G#', 'Gm#', 'A', 'Am', 'A#', 'Am#', 'B', 'Bm']


name_map = {
    1:'zhmih',
    2:'nya'
}

color_map = {
    1:(0,255,255),
    2:(0,0,255),
    3:(255,0,0)
}

MINDIST = 20 # minimal deviation in pix for NXT to move
COMMAND_FREQ = 0.2 # 1 sec between commands to NXTs
SPEED_COEF = 0.4 # speed per pixel of dist

def process_recording(recording, samplerate, booster):
    y = np.array([x[0] for x in recording])
    print(y)

    ind = 0
    while (y[ind] < 0.001):
        ind += 1
    y = y[ind:]
    ind = len(y)
    while (y[ind - 1] < 0.001):
        ind -= 1
    y = y[:ind]

    print(y)

    chroma_orig = librosa.feature.chroma_cqt(y=y, sr=samplerate)

    onset_env = librosa.onset.onset_strength(y, sr=samplerate)
    tempo = librosa.beat.tempo(onset_envelope=onset_env, sr=samplerate)

    print(tempo)

    l = len(chroma_orig[0])

    features = []

    for i in range(l//5):
        buf = []
        for j in range(i*5, min(l, (i+1)*5)):
            buf.extend([chroma_orig[x][j] for x in range(12)])
        features.append(buf)

    dfeats = xgb.DMatrix(features)
    y_pred = booster.predict(dfeats)

    chords_in_recording = []

    for x in y_pred:
            
        chord = (x.argmax(axis=0) + 18) % len(classes)
            
        if (max(x) > 0.9) : 
            
            print("{} {}".format(max(x), names[chord]))
            chords_in_recording.append(chord)

        else : 
            
            print("{} {}?".format(max(x), names[chord]))
            chords_in_recording.append(-1)

    amount = 0
    counts = {}

    print(chords_in_recording)

    for chord in chords_in_recording:
        if chord > - 1:
            if chord in counts.keys():
                counts[chord] += 1
            else :
                counts[chord] = 1
        amount += 1
    print(counts)

    s = ''
    for c in chords_in_recording:
        if (c > -1) : s += names[c] + ' '

    print(s)

    sequence = []
    scores = {}

    for chord in chords_in_recording:    
        if chord > -1 :
            if chord in scores.keys():
                scores[chord] += 1
            else :
                scores[chord] = 1
            if scores[chord] == 6:
                for key in scores.keys():
                    if key != chord:
                        scores[key] = 0
                sequence.append(chord)
    return sequence
