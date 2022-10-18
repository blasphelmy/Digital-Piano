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

bool done;
tsf* soundFile;
static void finish(int ignore) { done = true; }

int guiRenderThread(MAPPER * keyMapper, DigitalPiano * digitalPiano) {
    digitalPiano->connectMapper(keyMapper);
    if (digitalPiano->Construct(1920, 1080, 1, 1))
        digitalPiano->Start();
    return 0;
}

int inputThread(MAPPER * keyMapper) {
    RtMidiIn* midiin = new RtMidiIn();
    std::vector<unsigned char> message;
    int nBytes, i;
    double stamp;
    // Check available ports.
    unsigned int nPorts = midiin->getPortCount();

    if (nPorts == 0) {
        //std::cout << "No ports available!\n";
        goto cleanup;
    }
    midiin->openPort(0);
    midiin->ignoreTypes(false, false, false);
    done = false;
    (void)signal(SIGINT, finish);
    //std::cout << "Reading MIDI from port ... quit with Ctrl-C.\n";

    while (!done) {
        stamp = midiin->getMessage(&message);
        nBytes = message.size();
        if (nBytes > 1) {
            //144 keys 176 pedals
            keyMapper->setKeyState((int)message[0],(int)message[1] - 21, (int)message[2]);
        }   
    }
    // Clean up
cleanup:
    delete midiin;
    return 0;
}
static void AudioCallback(void* data, Uint8* stream, int len)
{
    int SampleCount = (len / (2 * sizeof(short))); //2 output channels
    tsf_render_short(soundFile, (short*)stream, SampleCount, 0);
}
smf::MidiFile getMidiFileRoutine(std::string& fileName) {
    smf::MidiFile newMidiFile(fileName);
    newMidiFile.doTimeAnalysis();
    newMidiFile.linkNotePairs();
    newMidiFile.joinTracks(); // we only care about 1 track right now

    return newMidiFile;
}
bool playMidi(MAPPER* keyMapper, std::string& fileName, DigitalPiano* digitalPiano) {
    bool action = false;
    smf::MidiFile midifile = getMidiFileRoutine(fileName);
    smf::MidiEvent event;
    MidiTimer &midiTimer = digitalPiano->midiTimer;
    midiTimer.index = 0;
    midiTimer.timeSinceStart = 0.0;
    midiTimer.qNotePerSec = midifile.getFileDurationInSeconds() / midifile.getFileDurationInTicks() * midifile.getTicksPerQuarterNote();
    midiTimer.duration = midifile.getFileDurationInSeconds();

    if (midifile[0].size() > 0) std::cout << "Playing...." << std::endl;
    digitalPiano->playSignal();
    midiTimer.start = std::chrono::high_resolution_clock::now();
    while (midiTimer.index < midifile[0].size() && !done) {
        action = true;
        midiTimer.tick();
        midiTimer.ticks = midifile.getAbsoluteTickTime(event.seconds);
        while (midiTimer.flag == 1 && midifile[0][midiTimer.index].seconds * 1000.f >= midiTimer.timeSinceStart) {
            midiTimer.index--;
            if (midiTimer.index < 0) {
                midiTimer.index = 0;
                break;
            }
            if (midifile[0][midiTimer.index].seconds * 1000.f < midiTimer.timeSinceStart) {
                break;
            }
        }

        while (midiTimer.flag == 2 && midifile[0][midiTimer.index].seconds * 1000.f >= midiTimer.timeSinceStart) {
            midiTimer.index++;
            if (midiTimer.index > midifile[0].size() - 1) {
                midiTimer.index = midifile[0].size() - 1;
                break;
            }
            if (midifile[0][midiTimer.index].seconds * 1000.f > midiTimer.timeSinceStart) {
                break;
            }
        }
        while (midiTimer.index < midifile[0].size() && midiTimer.flag == 0 && midifile[0][midiTimer.index].seconds * 1000.f <= midiTimer.timeSinceStart) {
            event = midifile[0][midiTimer.index];
            keyMapper->setKeyState((int)event[0], (int)event[1] - 21, (int)event[2]);
            midiTimer.index++;
        }
        Sleep(1);
    }
    return action;
}
int playSongInputThread(MAPPER* keyMapper, DigitalPiano * digitalPiano) {
    std::string selection;
    do {
        std::cout << "Enter in midi file directory or 'exit' to quit" << std::endl << "file directory > ";
        std::cin >> selection;
        if (selection != "exit") {
            if (playMidi(keyMapper, selection, digitalPiano)) {
                std::cout << "File not found... try arab2.mid | clairedelune.mid | SOSPIRO.mid" << std::endl;
            }
        }
    } while (selection != "exit");
    done = true;
}

int main(int argc, char* argv[])
{
    SDL_AudioSpec OutputAudioSpec;
    OutputAudioSpec.freq        = 44000;
    OutputAudioSpec.format      = AUDIO_S16;
    OutputAudioSpec.channels    = 2;
    OutputAudioSpec.samples     = 1024;
    OutputAudioSpec.callback    = AudioCallback;
    int dcbGain = 5;

    SDL_AudioInit(NULL);
    soundFile = tsf_load_filename("soundfile_1.sf2");
    tsf_set_output(soundFile, TSF_STEREO_INTERLEAVED, OutputAudioSpec.freq, dcbGain);
    SDL_OpenAudio(&OutputAudioSpec, NULL);
    SDL_PauseAudio(0);


    MAPPER* keyMapper = new MAPPER();
    DigitalPiano* app = new DigitalPiano();
    keyMapper -> soundFile = soundFile;

    std::thread guiThreadObject(guiRenderThread, keyMapper, app);
    std::thread inputThreadObject(inputThread, keyMapper);
    std::thread loadSongInputThread(playSongInputThread, keyMapper, app);

    inputThreadObject.join();
    guiThreadObject.join();
    loadSongInputThread.join();

    delete keyMapper;
    delete soundFile;
    delete app;
    return 0;
}