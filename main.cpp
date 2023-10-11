#include "GLOBALVARIABLES.h"
#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "minisdl_audio.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "Options.h"
#include "DigitalPiano.h"
#include "MAPPER.h"
#include <windows.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <thread>

tsf* soundfile;

int GuiRenderThread                 (DigitalPianoController * newPiano) { return newPiano->ConstructGUI(); }
int CreateSongListenerThread        (DigitalPianoController * newPiano) { return newPiano->SongCommandListener(); }
int CreateMemoryManagementThread    (DigitalPianoController * newPiano) { return newPiano->MemoryManagementThread(); }
int CeateMidiInstrumentInputThread  (DigitalPianoController * newPiano) { return newPiano->MidiInputThread(); }
int CreateMidiFileListenerThread    (DigitalPianoController * newPiano) { return newPiano->MidiFileListener(); }

static void AudioCallback(void* data, Uint8* stream, int len) { int SampleCount = (len / (2 * sizeof(short))); tsf_render_short(soundfile, (short*)stream, SampleCount, 0); }

void setUp() {
    //https://stackoverflow.com/questions/54912038/querying-windows-display-scaling
    auto activeWindow           = GetActiveWindow();
    HMONITOR monitor            = MonitorFromWindow(activeWindow, MONITOR_DEFAULTTONEAREST);

    // Get the logical width and height of the monitor
    MONITORINFOEX monitorInfoEx;
    monitorInfoEx.cbSize        = sizeof(monitorInfoEx);
    GetMonitorInfo              (monitor, &monitorInfoEx);
    float cxLogical             = (float)monitorInfoEx.rcMonitor.right - (float)monitorInfoEx.rcMonitor.left;
    float cyLogical             = (float)monitorInfoEx.rcMonitor.bottom - (float)monitorInfoEx.rcMonitor.top;

    _WINDOW_W                   = (int)((float)cxLogical * .75f);
    _WINDOW_H                   = (int)((float)cyLogical * .75f);

    if ((int)_WINDOW_W % 52 != 0) _WINDOW_W += 52 - ((int)_WINDOW_W % 52);
    _KEYSIZE                    = (float)(_WINDOW_H / 1.2272727273);
    _TEXT_SCALE                 = 1;

    SDL_AudioSpec OutputAudioSpec;
    OutputAudioSpec.freq        = 32000;
    OutputAudioSpec.format      = AUDIO_S16;
    OutputAudioSpec.channels    = 2;
    OutputAudioSpec.samples     = 1024;
    OutputAudioSpec.callback    = AudioCallback;
    float dcbGain               = 0.f;

    SDL_AudioInit               (NULL);
    soundfile                   = tsf_load_filename("soundfile_1.sf2");
    tsf_set_output              (soundfile, TSF_STEREO_INTERLEAVED, OutputAudioSpec.freq, dcbGain);
    tsf_set_max_voices          (soundfile, 312);
    SDL_OpenAudio               (&OutputAudioSpec, NULL);
    SDL_PauseAudio              (0);
}
int main()
{
    setUp                               ();

    MAPPER * keyMapper                  = new MAPPER(soundfile);
    DigitalPiano * app                  = new DigitalPiano();
    DigitalPianoController * newPiano   = new DigitalPianoController(app, keyMapper, soundfile);

    std::thread guiThreadObject         (GuiRenderThread, newPiano);
    std::thread inputThreadObject       (CeateMidiInstrumentInputThread, newPiano);
    std::thread loadSongInputThread     (CreateSongListenerThread, newPiano);
    std::thread memoryManagementThread  (CreateMemoryManagementThread, newPiano);
    std::thread midiFileListeners       (CreateMidiFileListenerThread, newPiano);
    
    guiThreadObject                     .join();
    inputThreadObject                   .join();
    loadSongInputThread                 .join();
    memoryManagementThread              .join();
    midiFileListeners                   .join();

    delete keyMapper;
    delete soundfile;
    delete app;
    delete newPiano;
    return 0;
}