import mido

def parse_midi(path_to_midi):
    mid = mido.MidiFile(path_to_midi)
    melody = []
    tempo = 0
    for msg in mid:
        if not msg.is_meta:
            break
        elif msg.type == "set_tempo":
            tempo = mido.tempo2bpm(msg.tempo)
    
    for msg in mid:
        # print(msg)
        if msg.type == 'note_on' or msg.type == 'note_off':
            # print(msg)
            melody.append([(msg.note - 60), msg.velocity, int(msg.time*1000)])

    prefix = [[0, 0, len(melody)]]
    prefix.extend(melody)

    return prefix

        

# from melody_generator import MelodyGenerator
# gen = MelodyGenerator()
# gen.write_midi(melody, "midi/hp.mid")

# from midi2audio import FluidSynth
# FluidSynth('midi/Wii_Grand_Piano.sf2').midi_to_audio('midi/hp.mid', 'midi/hp.wav')


# from melody_generator import MelodyGenerator
# gen = MelodyGenerator()


# print(melody)

