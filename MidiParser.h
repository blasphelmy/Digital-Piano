#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "minisdl_audio.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "Options.h"
#include "DigitalPiano.h"
#include "MAPPER.h"
#include "key.h"
#include "vectors.h"
#include <iostream>
#include <chrono>
#include <windows.h>
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <signal.h>
#include <thread>
#include <map>
#include <unordered_map>
#include <queue>
#include <mutex>

//class MidiParser {
//    MidiParser() {
//
//    }
//    ~MidiParser() {
//
//    }
//public:
//    smf::MidiFile getMidiFileRoutine(std::string& fileName) {
//        smf::MidiFile newMidiFile(fileName);
//        newMidiFile.doTimeAnalysis();
//        newMidiFile.linkNotePairs();
//        newMidiFile.joinTracks(); // we only care about 1 track right now
//
//        return newMidiFile;
//    }
//    bool playMidi(MAPPER* keyMapper, std::string& fileName) {
//        bool action = false;
//        smf::MidiFile midifile = getMidiFileRoutine(fileName);
//        smf::MidiEvent event;
//        int index = 0;
//        auto start = std::chrono::high_resolution_clock::now();
//        if (midifile[0].size() > 0) std::cout << "Playing...." << std::endl;
//        while (index < midifile[0].size() && !done) {
//            action = true;
//            auto finish = std::chrono::high_resolution_clock::now();
//            long long timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();
//            while (index < midifile[0].size() && midifile[0][index].seconds * 1000.f <= timeSinceStart) {
//                event = midifile[0][index];
//                keyMapper->setKeyState((int)event[0], (int)event[1] - 21, (int)event[2]);
//                index++;
//            }
//        }
//        return action;
//    }
//};