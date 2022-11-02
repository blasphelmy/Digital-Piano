#pragma once
#include "GLOBALVARIABLES.h"
#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "minisdl_audio.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "Options.h"
#include "DigitalPiano.h"
#include "key.h"
#include "vectors.h"
#include <iostream>
#include <chrono>
#include <signal.h>
#include <thread>
#include <map>
#include <unordered_map>
#include <queue>
#include <mutex>

struct activeNotes {
    activeNotes(int index, int keyId) {
        this->index = index;
        this->keyId = keyId;
    }
    int index;
    int keyId;
};

class MAPPER {
public:
    std::array<key, 88>                     keyMap;
    std::unordered_map<int, int>            keyIdMap_PIANO;
    std::unordered_map<int, FlyingNotes* >  activelyDrawing;
    std::queue<FlyingNotes>                 onScreenNoteElements;
    std::queue<activeNotes>                 activeNotesPool;
    tsf* soundFile = nullptr;
    bool                                    pedal = false;
    std::mutex                              threadLock;

public:
    MAPPER() {
        //set up white keys
        //index 0-51 : white keys
        //index 52-87 : black keys
        double          slice = _WINDOW_W / 52.f;
        olc::vd2d       whiteKeySize(_WINDOW_W / 54.f, _WINDOW_H / 5.4);
        olc::vd2d       blackKeySize(_WINDOW_W / 70.f, _WINDOW_H / 8.3076923);
        std::string     abc = "ABCDEFG";
        olc::vd2d       initialPos(0, _KEYSIZE);
        olc::vd2d       offSet(slice, 0);
        keyIdMap_PIANO = {
            //white keys
            {0, 0},{2, 1},{3, 2},{5, 3},{7, 4},
            {8, 5},{10, 6},{12, 7},{14, 8},{15, 9},
            {17, 10},{19, 11},{20, 12},{22, 13},
            {24, 14},{26, 15},{27, 16},{29, 17},
            {31, 18},{32, 19},{34, 20},{36, 21},
            {38, 22},{39, 23},{41, 24},{43, 25},
            {44, 26},{46, 27},{48, 28},{50, 29},
            {51, 30},{53, 31},{55, 32},{56, 33},
            {58, 34},{60, 35},{62, 36},{63, 37},
            {65, 38},{67, 39},{68, 40},{70, 41},
            {72, 42},{74, 43},{75, 44},{77, 45},
            {79, 46},{80, 47},{82, 48},{84, 49},
            {86, 50},{87, 51},

            //black keys
            {1, 52},{4, 53},{6, 54},{9, 55},{11, 56},
            {13, 57},{16, 58},{18, 59},{21, 60},
            {23, 61},{25, 62},{28, 63},{30, 64},
            {33, 65},{35, 66},{37, 67},{40, 68},
            {42, 69},{45, 70},{47, 71},{49, 72},
            {52, 73},{54, 74},{57, 75},{59, 76},
            {61, 77},{64, 78},{66, 79},{69, 80},
            {71, 81},{73, 82},{76, 83},{78, 84},
            {81, 85},{83, 86},{85, 87}

        };
        int i = 0;
        while (i < 52) {
            key newKey(std::string(1, abc.at(i % 7)), true, initialPos, whiteKeySize);
            initialPos = initialPos + offSet;
            keyMap[i] = newKey;
            i++;
        }
        i = 52;
        //create first a# key
        key newKey(false, blackKeySize);
        newKey.name = std::string("A");
        newKey.position = olc::vd2d(slice * .5, _KEYSIZE);
        keyMap[i] = newKey;
        //create the subsequent groups of 5 black keys
        initialPos = olc::vd2d(slice * 2.5 + 3.f, _KEYSIZE);
        for (int y = 53; y < 88; y = y + 5) {

            key k1(false, blackKeySize);
            k1.name = std::string("C");
            key k2(false, blackKeySize);
            k2.name = std::string("D");
            key k3(false, blackKeySize);
            k3.name = std::string("F");
            key k4(false, blackKeySize);
            k4.name = std::string("G");
            key k5(false, blackKeySize);
            k5.name = std::string("A");

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

            keyMap[y] = k1;
            keyMap[y + 1] = k2;
            keyMap[y + 2] = k3;
            keyMap[y + 3] = k4;
            keyMap[y + 4] = k5;

        }
    }
    ~MAPPER() {

    }
private:
    FlyingNotes* createFlyingNote(key thisKey) {
        FlyingNotes* newFlyingNote = new FlyingNotes(thisKey.isWhite);
        newFlyingNote->name = thisKey.name;
        newFlyingNote->position = thisKey.position;
        newFlyingNote->position.y = _KEYSIZE + (_KEYSIZE * .02f);
        newFlyingNote->size = thisKey.size;
        newFlyingNote->size.y = -1;
        return newFlyingNote;
    }
public:
    void flushActiveNotes() {
        std::queue<activeNotes> newqueue;
        while (!activeNotesPool.empty()) {

            activeNotes note = activeNotesPool.front();
            activeNotesPool.pop();

            if (keyMap[keyIdMap_PIANO[note.keyId]].velocity > 0) {
                newqueue.push(note);
            }
            else {
                tsf_note_off(soundFile, 0, note.keyId + 21);
            }
        }
        activeNotesPool = newqueue;
    }
    void setKeyState_PIANO(int cat, int keyId, int velocity) {
        threadLock.lock();
        if (cat >= 0x80 && cat < 0x8f) velocity = 0;
        if (cat >= 0x80 && cat < 0x8f || cat >= 0x90 && cat < 0x9f) {
            if (velocity != 0 && (cat >= 0x90 && cat < 0x9f)) {
                tsf_note_on(soundFile, 0, keyId + 21, static_cast<float>(velocity / 256.f));
                activeNotesPool.push(activeNotes(0, keyId));
                key thisKey = keyMap[keyIdMap_PIANO[keyId]];
                FlyingNotes* newFlyingNote = createFlyingNote(thisKey);
                activelyDrawing.emplace(std::make_pair(keyIdMap_PIANO[keyId], newFlyingNote));
            }
            else {
                if (activelyDrawing.count(keyIdMap_PIANO[keyId]) > 0) {
                    onScreenNoteElements.push(*(activelyDrawing.find(keyIdMap_PIANO[keyId])->second));
                    delete activelyDrawing.find(keyIdMap_PIANO[keyId])->second;
                }
                if (keyMap[keyIdMap_PIANO[keyId]].velocity > 0)
                    activelyDrawing.erase(keyIdMap_PIANO[keyId]);
                if (!pedal)
                    tsf_note_off(soundFile, 0, keyId + 21);
            }
            keyMap[keyIdMap_PIANO[keyId]].velocity = velocity;
        }
        else {
            //std::cout << "pedal switched" << std::endl;
            switch (pedal) {
                case true:
                    pedal = false;
                    flushActiveNotes();
                    //tsf_note_off_all(soundFile);
                    break;
                case false:
                    pedal = true;
                    break;
            }
        }
        threadLock.unlock();
    }
};