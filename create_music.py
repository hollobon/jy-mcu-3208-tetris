import os
import sys

import midi


def filter_track(track):
    newtrack = midi.Track([], False)
    last_tick = 0
    for n, e in enumerate(track):
        if isinstance(e, midi.NoteOnEvent) and ((midi.C_4 > e.pitch < midi.C_7) or e.velocity == 0):
            continue
        if isinstance(e, midi.NoteOnEvent) and len(newtrack) > 0 and isinstance(newtrack[-1], midi.NoteOnEvent) and e.tick - last_tick == 0:
            if e.pitch > newtrack[-1].pitch:
                newtrack[-1].pitch = e.pitch
        else:
            newtrack.append(e)
        last_tick = e.tick
    return newtrack


def note_frequency(note):
    return 440 * (2.0 ** ((note - midi.A_4) / 12.0))


TIMER_DIVISOR = 256


def main():
    cpufreq = int(sys.argv[1])
    
    pattern = midi.read_midifile(os.path.join(os.path.dirname(__file__), "Korobeiniki.mid"))
    pattern.make_ticks_abs()
    track = filter_track(pattern[0])
    pattern[0] = midi.Track(track, False)
    pattern.make_ticks_rel()
    midi.write_midifile(os.path.join(os.path.dirname(__file__), "Korobeiniki-filtered.mid"), pattern)

    used_notes = {e.pitch for e in track if isinstance(e, midi.NoteOnEvent)}
    note_map = {note: n for n, note in enumerate(used_notes)}

    print("#ifndef TUNE_H")
    print("#define TUNE_H")
    print("#include <stdint.h>")
    print
    print("/* Autogenerated by %s - do not edit */" % os.path.basename(__file__))
    print
    print("typedef struct {")
    print("    uint8_t note;")
    print("    uint8_t time;")
    print("} note;")
    print
    print("uint8_t note_clocks[] = {")
    print("\n".join("    %d, // %s" % (round((cpufreq / TIMER_DIVISOR / 2) / note_frequency(note)), midi.NOTE_VALUE_MAP_SHARP[note].replace("_", "").replace("s", "#")) for note in note_map))
    print("};")
    print
    print("note tune[] = {")
    track_notes = [event for event in track if isinstance(event, midi.NoteOnEvent)]
    print("\n".join("    {%d, %d}, // %s" % (note_map[event.pitch], nextevent.tick, midi.NOTE_VALUE_MAP_SHARP[event.pitch]) for event, nextevent in zip(track_notes, track_notes[1:])))
    print("};")
    print("#define TUNE_LENGTH %d" % (len(track_notes) - 1))
    print
    print("#endif // TUNE_H")


if __name__ == "__main__":
    sys.exit(main())