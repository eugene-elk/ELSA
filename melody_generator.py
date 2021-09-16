import random
from stuff import *

pentatonic_scales = [
    [0, 2, 4, 7, 9], #maj
    [0, 3, 5, 7, 10] #min
]

scales = [
    [0, 2, 4, 5, 7, 9, 11], #maj
    [0, 2, 3, 5, 7, 8, 10] #min
]


tension_border = 12

class MelodyGenerator():
    
    def init_chords(self, chords):
        for i, name in enumerate(names):

            n = i // 2

            if (i % 2 == 0):
                chords[name] = [n % 12, (n+4) % 12, (n+7) % 12]
            else :
                chords[name] = [n % 12, (n+3) % 12, (n+7) % 12]
    
    def __init__(self):
        self.chords = {}
        self.init_chords(self.chords)      

    def gen_phrase(self, step, scale, semiqs_left, tone):
        type_probs = [1, 10, 1]
        phrase_type = self.get_random_index(type_probs)
        phrase = []
        _phrase_length = 0
        last_step = 0

        if (phrase_type == 0):
            phrase_length = min(8, semiqs_left)
            _phrase_length = phrase_length
            
            probs = [1, 3, 0, 7, 3]
            length_probs = [1, 8, 3]

        elif (phrase_type == 1):
            phrase_length = min(6, semiqs_left)
            _phrase_length = phrase_length
            # phrase_length -= 2
            
            probs = [4, 3, 0.2, 3, 4]
            length_probs = [0, 0, 0, 2, 5, 2]
        
        if (phrase_type == 2):
            phrase_length = min(8, semiqs_left)
            _phrase_length = phrase_length
            
            probs = [3, 7, 0, 3, 1]
            length_probs = [1, 8, 3]

        # elif (phrase_type == 2):
        #     phrase_length = min(6, semiqs_left)
        #     _phrase_length = phrase_length
            
        #     probs = [5, 2, 0.2, 3, 7]
        #     length_probs = [3, 2]


        current_step = (step + self.get_random_index(probs) - len(scale) // 2) % len(scale)
        # while (scale[current_step] + tone in [1, 3]) :
        #     current_step = (step + self.get_random_index(probs) - len(scale) // 2) % len(scale)

        length = min(self.get_random_index(length_probs) + 1, phrase_length)
        phrase.append((scale[current_step] + tone) % 12)
        phrase.append(length)
        phrase_length -= length

        while phrase_length > 0:
            current_step = (current_step + self.get_random_index(probs) - len(probs) // 2) % len(scale)

            while (scale[current_step] + tone in [1, 3]) :
                current_step = (current_step + self.get_random_index(probs) - len(probs) // 2) % len(scale)

            if semiqs_left % 16 < 3:
                current_step = 0

            length = min(self.get_random_index(length_probs) + 1, phrase_length)
            phrase.append((scale[current_step] + tone) % 12)
            phrase.append(length)
            phrase_length -= length
            last_step = current_step
            

        print(phrase_type)
        return (_phrase_length, last_step, phrase)
            
    def get_nearest_stable(self, step):
        
        a = 0
        m = step
        if (abs(step - 3) < m):
            m = abs(step - 3)
            a = 3      
    
        return a    

    def get_next(self, current, probs) :  
        return (self.get_random_index(probs) + current - 2)
        
    def get_random_index(self, probabillities):
        total = sum(probabillities)
        chosen = random.uniform(0, total)
        cumulative = 0

        for i, x in enumerate(probabillities):
            cumulative += x
            if cumulative > chosen:
                return i


    def process_chord(self, chord, beats_per_chord=16):

        prev_step = 0
        melody = [chord // 2, 8]
        semiqs_left = (beats_per_chord - 3) * 4
        

        while semiqs_left > 0:
            
            scale = pentatonic_scales[chord % 2]
            phrase_length, prev_step, phrase = self.gen_phrase(prev_step, scale, semiqs_left, chord // 2)
            semiqs_left -= phrase_length
            melody.extend(phrase)
            print(phrase)
        
        melody.extend([12, 4])
        return melody

    def generate(self, sequence, beats_per_chord=16):
        melody = []
        for chord in sequence:
            part = self.process_chord(chord, beats_per_chord)
            melody.extend(part)
            # part = self.process_chord(chord)
            # melody.extend(part)
        return melody

    def write_midi(self, melody, name = 'melody.mid'):
        from mido import Message, MidiFile, MidiTrack, MetaMessage, tempo2bpm, second2tick

        tempo = 200


        mid = MidiFile(ticks_per_beat=120)
        track = MidiTrack()
        mid.tracks.append(track)

        track.append(MetaMessage('set_tempo', tempo = int(tempo2bpm(tempo)), time=0))
        

        

        beat = 60/tempo
        semiq = beat/4
        pause = 0

        for i in range(len(melody)//2):
            pitch, duration = melody[2*i], melody[1+2*i]

            if (pitch < 12):
                track.append(Message('note_on', note=pitch + 60, velocity=80, time=pause))
                pause = 0
                track.append(Message('note_on', note=pitch + 60, velocity=0, time=int(second2tick(semiq*duration, 120, tempo2bpm(tempo)))))
            else :
                pause=int(second2tick(semiq*duration, 120, tempo2bpm(tempo)))

        mid.save(name)    
        