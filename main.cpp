#define __WINDOWS_MM__
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngineGL.h"
#define TSF_IMPLEMENTATION
#include "tsf.h"
#include "minisdl_audio.h"
#include "RtMidi.h"
#include <iostream>
#include <windows.h>
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>
#include <signal.h>
#include <thread>
#include <map>
#include <queue>

class key {
public:
    olc::vd2d position;
    olc::vd2d size;
    std::string name;
    bool isWhite;
    //bool active = false;
    int velocity = 0;
public :
    key(
        bool isWhite
    ) {
        this->isWhite = isWhite;
    };
    key(
        bool isWhite,
        olc::vd2d size
        )
    {
        this->size = size;
        this->isWhite = isWhite;
    };
    key(
        std::string name,
        bool isWhite,
        olc::vd2d position
    ) {
        this->isWhite = isWhite;
        this->position = position;
        this->name = name;
    }
    key(
        std::string name,
        bool isWhite,
        olc::vd2d position,
        olc::vd2d size
    ) {
        this->isWhite = isWhite;
        this->position = position;
        this->name = name;
        this->size = size;
    }
    ~key() {

    }

};

class MAPPER {
public:
    std::vector<key> keyMap;
    std::map<int, int> keyIdMap;
    std::map<int, key * > activelyDrawing;
    std::queue<key> onScreenNoteElements;
    tsf* soundFile = nullptr;
    bool pedal = false;


    struct activeNotes {
        activeNotes(int index, int keyId) {
            this->index = index;
            this->keyId = keyId;
        }
        int index;
        int keyId;
    };

    std::queue<activeNotes> activeNotesPool;
private :
    void flushActiveNote() {
        std::queue<activeNotes> newqueue;
        while (!activeNotesPool.empty()) {
            activeNotes note = activeNotesPool.front();
            activeNotesPool.pop();
            if (keyMap[keyIdMap[note.keyId]].velocity > 0) {
                newqueue.push(note);
            }
            else {
                tsf_note_off(soundFile, 0, note.keyId+21);
            }
        }
        activeNotesPool = newqueue;
    }

public : 
    MAPPER() {
        //set up white keys
        //index 0-51 : white keys
        //index 52-87 : black keys
        double slice = 1920.f / 52.f;
        std::string abc = "ABCDEFG";
        olc::vd2d initialPos(0, 880);
        olc::vd2d offSet(slice, 0);
        int i = 0;
        keyIdMap =
        {
            //white keys
            {0, 0},{2, 1},{3, 2},{5, 3},{7, 4},{8, 5},{10, 6},{12, 7},{14, 8},{15, 9},{17, 10},{19, 11},
            {20, 12},{22, 13},{24, 14},{26, 15},{27, 16},{29, 17},{31, 18},{32, 19},{34, 20},{36, 21},
            {38, 22},{39, 23},{41, 24},{43, 25},{44, 26},{46, 27},{48, 28},{50, 29},{51, 30},{53, 31},
            {55, 32},{56, 33},{58, 34},{60, 35},{62, 36},{63, 37},{65, 38},{67, 39},{68, 40},{70, 41},
            {72, 42},{74, 43},{75, 44},{77, 45},{79, 46},{80, 47},{82, 48},{84, 49},{86, 50},{87, 51},
            
            //black keys
            {1, 52},{4, 53},{6, 54},{9, 55},{11, 56},{13, 57},{16, 58},{18, 59},{21, 60},{23, 61},{25, 62},
            {28, 63},{30, 64},{33, 65},{35, 66},{37, 67},{40, 68},{42, 69},{45, 70},{47, 71},{49, 72},{52, 73},
            {54, 74},{57, 75},{59, 76},{61, 77},{64, 78},{66, 79},{69, 80},{71, 81},{73, 82},{76, 83},{78, 84},
            {81, 85},{83, 86},{85, 87},

        };
        olc::vd2d whiteKeySize(1920.f / 53.f, 200);
        olc::vd2d blackKeySize (1920.f / 65.f, 120);
        while (i < 52) {
            key newKey(std::string(1, abc.at(i % 7)), true, initialPos, whiteKeySize);
            i++;
            initialPos = initialPos + offSet;
            keyMap.push_back(newKey);
        }
        i = 0;
        //create first a# key
        key newKey(false, blackKeySize);
        newKey.position = olc::vd2d(slice * .5, 880);
        keyMap.push_back(newKey);
        //create the subsequent groups of 5 black keys
        initialPos = olc::vd2d(slice * 2.5, 880);
        for (int y = 0; y < 35; y = y + 5) {
            key k1(false, blackKeySize);
            key k2(false, blackKeySize);
            key k3(false, blackKeySize);
            key k4(false, blackKeySize);
            key k5(false, blackKeySize);

            k1.position = initialPos;
            
            initialPos.x = initialPos.x + slice;
            k2.position = initialPos;

            initialPos.x = initialPos.x + (2 * slice);
            k3.position = initialPos;

            initialPos.x = initialPos.x + slice;
            k4.position = initialPos;

            initialPos.x = initialPos.x + slice;
            k5.position = initialPos;
            initialPos.x = initialPos.x + (2 * slice);

            keyMap.push_back(k1);
            keyMap.push_back(k2);
            keyMap.push_back(k3);
            keyMap.push_back(k4);
            keyMap.push_back(k5);

        }
    }
    ~MAPPER() {

    }

public : 
    void setKeyState(int cat, int keyId, int command) {
        if (cat == 144) {
            if (command != 0) {
                tsf_note_on(soundFile, 0, keyId + 21, static_cast<float>(command) / 100.f);
                activeNotesPool.push(activeNotes(0, keyId));
                key thisKey = keyMap[keyIdMap[keyId]];
                key * newKey = new key(thisKey.isWhite);
                newKey->name = thisKey.name;
                newKey->position = thisKey.position;
                newKey->position.y = (880 - 110);
                activelyDrawing.emplace(std::make_pair(keyIdMap[keyId], newKey));
            }
            else {
                if(activelyDrawing.count(keyIdMap[keyId]) > 0)
                    onScreenNoteElements.push(*(activelyDrawing.find(keyIdMap[keyId])->second));
                delete activelyDrawing.find(keyIdMap[keyId])->second;
                activelyDrawing.erase(keyIdMap[keyId]);
                if (!pedal) tsf_note_off(soundFile, 0, keyId + 21);
            }
            activelyDrawing;
            keyMap[keyIdMap[keyId]].velocity = command;
        }
        else {
            switch (pedal) {
            case true: pedal = false; flushActiveNote();  break;
            case false: pedal = true; break;
            }
        }
    }

};

class DigitalPiano : public olc::PixelGameEngine {
public :
    DigitalPiano() {
        sAppName = "Digital Piano";
        whiteKeySize = olc::vd2d(1920.f / 53.f, 200);
        blackKeySize = olc::vd2d(1920.f / 65.f, 120);
    }
    ~DigitalPiano() {
    }
public:
    MAPPER* keyMapper = nullptr;
private: 
    olc::vd2d whiteKeySize;
    olc::vd2d blackKeySize;

public:
    bool OnUserCreate() override {
        return true;
    }
    bool OnUserUpdate(float felaspedTime) override {
        Clear(olc::Pixel(143, 139, 123));
        DrawOnScreenNotes(felaspedTime);
        drawKeys(felaspedTime);
        SetPixelMode(olc::Pixel::NORMAL); // Draw all pixels
        return true;
    }

    
private :
    olc::Pixel getColor(bool isWhite, int velocity) {
        if (isWhite) {
            if (velocity == 0) {
                return olc::Pixel(255, 255, 255);
            }
            else {
                return olc::Pixel(63, 119, 209);
            }
        }
        else {
            if (velocity == 0) {
                return olc::Pixel(50, 50, 50);
            }
            else {
                return olc::Pixel(98, 142, 217);
            }
        }
        return olc::RED;
    }
    void DrawOnScreenNotes(float felaspedTime) {
        
    }
    void drawKeys(double timeElasped) {
        double yOffSet = timeElasped * 300.f;
        for (int i = 0; i < keyMapper->keyMap.size(); i++) {
            if ((*keyMapper).activelyDrawing.count(i) > 0) {
                key* drawnKey = keyMapper->activelyDrawing.find(i)->second;
                olc::vd2d size;
                if (drawnKey->isWhite) size = whiteKeySize;
                else size = blackKeySize;
                size.y = blackKeySize.y;
                FillRect(drawnKey->position, size - olc::vd2d(1, 1), olc::Pixel(72, 124, 207));
            }

            key thisKey = (*keyMapper).keyMap[i];
            if (thisKey.isWhite) FillRect(thisKey.position, whiteKeySize - olc::vd2d(1, 1), getColor(thisKey.isWhite, thisKey.velocity));
            else FillRect(thisKey.position, thisKey.size - olc::vd2d(1, 1), getColor(thisKey.isWhite, thisKey.velocity));
        }
        std::queue<key> newOnScreenElementsQueue;
        while (!keyMapper->onScreenNoteElements.empty()) {
            key onscreenKey = keyMapper->onScreenNoteElements.front();
            keyMapper->onScreenNoteElements.pop();

            olc::vd2d size;
            if (onscreenKey.isWhite) size = whiteKeySize;
            else size = blackKeySize;
            size.y = blackKeySize.y;
            FillRect(onscreenKey.position, size - olc::vd2d(1, 1), olc::Pixel(72, 124, 207));

            onscreenKey.position.y -= yOffSet;

            if(onscreenKey.position.y > 0) newOnScreenElementsQueue.push(onscreenKey);
        }
        keyMapper->onScreenNoteElements = newOnScreenElementsQueue;
    }
};

std::string hex(uint32_t n, uint8_t d)
{
    std::string s(d, '0');
    for (int i = d - 1; i >= 0; i--, n >>= 4)
        s[i] = "0123456789ABCDEF"[n & 0xF];
    return s;
};

int guiRenderThread(MAPPER * keyMapper) {
    DigitalPiano app;
    app.keyMapper = keyMapper;
    if (app.Construct(1920, 1080, 1, 1))
        app.Start();
    return 0;
}
bool done;
static void finish(int ignore) { done = true; }
int inputThread(MAPPER * keyMapper) {
    RtMidiIn* midiin = new RtMidiIn();
    std::vector<unsigned char> message;
    int nBytes, i;
    double stamp;
    // Check available ports.
    unsigned int nPorts = midiin->getPortCount();
    if (nPorts == 0) {
        std::cout << "No ports available!\n";
        goto cleanup;
    }
    midiin->openPort(0);
    midiin->ignoreTypes(false, false, false);
    done = false;
    (void)signal(SIGINT, finish);
    std::cout << "Reading MIDI from port ... quit with Ctrl-C.\n";
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

tsf* soundFile;
static void AudioCallback(void* data, Uint8* stream, int len)
{
    int SampleCount = (len / (2 * sizeof(short))); //2 output channels
    tsf_render_short(soundFile, (short*)stream, SampleCount, 0);
}
int main(int argc, char* argv[])
{
    SDL_AudioSpec OutputAudioSpec;
    OutputAudioSpec.freq = 24000;
    OutputAudioSpec.format = AUDIO_S16MSB;
    OutputAudioSpec.channels = 2;
    OutputAudioSpec.samples = 512;
    OutputAudioSpec.callback = AudioCallback;

    SDL_AudioInit(NULL);
    soundFile = tsf_load_filename("px860.sf2");

    tsf_set_output(soundFile, TSF_STEREO_INTERLEAVED, OutputAudioSpec.freq, 0);

    SDL_OpenAudio(&OutputAudioSpec, NULL);

    SDL_PauseAudio(0);


    MAPPER* keyMapper = new MAPPER();
    keyMapper -> soundFile = soundFile;
    std::thread guiThreadObject(guiRenderThread, keyMapper);
    std::thread inputThreadObject(inputThread, keyMapper);
    inputThreadObject.join();
    guiThreadObject.join();

    delete keyMapper;
    delete soundFile;
    return 0;
}