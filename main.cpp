#include "GLOBALVARIABLES.h"
#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "minisdl_audio.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "Options.h"
#include "DigitalPiano.h"
#include "MAPPER.h"
#include "key.h"
#include <windows.h>
#include "vectors.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <signal.h>
#include <thread>
#include <map>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <set>

tsf* soundFile;

static void AudioCallback(void* data, Uint8* stream, int len) { int SampleCount = (len / (2 * sizeof(short))); tsf_render_short(soundFile, (short*)stream, SampleCount, 0); }

int guiRenderThread                 (DigitalPianoController * newPiano) { return newPiano->ConstructGUI(); }
int createSongListenerThread        (DigitalPianoController * newPiano) { return newPiano->songCommandListener(); }
int createMemoryManagementThread    (DigitalPianoController* newPiano) { return newPiano->memoryManagementThread(); }
int ceateMidiInstrumentInputThread  (DigitalPianoController* newPiano) { return newPiano->MidiInputThread(); }
int CreateMidiFileListenerThread    (DigitalPianoController* newPiano) { return newPiano->MidiFileListener(); }

void setUp() {
    //https://stackoverflow.com/questions/54912038/querying-windows-display-scaling
    auto activeWindow           = GetActiveWindow();
    HMONITOR monitor            = MonitorFromWindow(activeWindow, MONITOR_DEFAULTTONEAREST);

    // Get the logical width and height of the monitor
    MONITORINFOEX monitorInfoEx;
    monitorInfoEx.cbSize        = sizeof(monitorInfoEx);
    GetMonitorInfo              (monitor, &monitorInfoEx);
    float cxLogical             = monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left;
    float cyLogical             = monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top;

    _WINDOW_W                   = (float)cxLogical - (int)(cxLogical * .3);
    _WINDOW_H                   = (float)cyLogical - (int)(cyLogical * .3);

    _KEYSIZE                    = _WINDOW_H / 1.2272727273;
    _TEXT_SCALE                 = 1;

    //if (_WINDOW_W < 1080)       _TEXT_SCALE = 2;

    SDL_AudioSpec OutputAudioSpec;
    OutputAudioSpec.freq        = 32000;
    OutputAudioSpec.format      = AUDIO_S16;
    OutputAudioSpec.channels    = 2;
    OutputAudioSpec.samples     = 1024;
    OutputAudioSpec.callback    = AudioCallback;
    int dcbGain                 = 0;

    SDL_AudioInit               (NULL);
    soundFile                   = tsf_load_filename("soundfile_1.sf2");
    tsf_set_output              (soundFile, TSF_STEREO_INTERLEAVED, OutputAudioSpec.freq, dcbGain);
    tsf_set_max_voices          (soundFile, 312);
    SDL_OpenAudio               (&OutputAudioSpec, NULL);
    SDL_PauseAudio              (0);
}
int main()
{
    setUp                               ();

    MAPPER* keyMapper                   = new MAPPER(soundFile);
    DigitalPiano * app                  = new DigitalPiano();
    DigitalPianoController * newPiano   = new DigitalPianoController(app, keyMapper, soundFile);

    std::thread guiThreadObject         (guiRenderThread, newPiano);
    std::thread inputThreadObject       (ceateMidiInstrumentInputThread, newPiano);
    std::thread loadSongInputThread     (createSongListenerThread, newPiano);
    std::thread memoryManagementThread  (createMemoryManagementThread, newPiano);
    std::thread midiFileListeners       (CreateMidiFileListenerThread, newPiano);
    
    guiThreadObject                     .join();
    inputThreadObject                   .join();
    loadSongInputThread                 .join();
    memoryManagementThread              .join();
    midiFileListeners                   .join();

    delete keyMapper;
    delete soundFile;
    delete app;
    delete newPiano;
    return 0;
}