#pragma once
#include "GLOBALVARIABLES.h"
#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "minisdl_audio.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "Options.h"
#include "MAPPER.h"
#include "vectors.h"
#include <iostream>
#include <chrono>
#include <signal.h>
#include <thread>
#include <map>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <set>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

class key {
public:
    char        channel = 0x00;
    olc::vd2d   position;
    olc::vd2d   size;
    std::string name;
    bool        isWhite;
    //bool active = false;
    int         velocity = 0;
public:
    explicit key() {
        this->isWhite = true;
    }
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
        this->size      = size;
        this->isWhite   = isWhite;
    };
    key(
        std::string name,
        bool        isWhite,
        olc::vd2d   position
    ) {
        this->isWhite   = isWhite;
        this->position  = position;
        this->name      = name;
    }
    key(
        std::string name,
        bool        isWhite,
        olc::vd2d   position,
        olc::vd2d   size
    ) {
        this->isWhite   = isWhite;
        this->position  = position;
        this->name      = name;
        this->size      = size;
    }
    ~key() {

    }

};

class FlyingNotes : public key {
public:
    FlyingNotes(key & Key, short channel) {
        this->name = Key.name;
        this->position = Key.position;
        this->position.y = _KEYSIZE + (_KEYSIZE * .02f);
        this->size = Key.size;
        this->size.y = -1;
        this->channel = channel;
        this->isWhite = Key.isWhite;
    }
public:
    vector3i    color;
    double      duration = 0.0f;
//public:
//    void addDuration(double dur) {
//        duration += dur;
//    }
//    void resetDuration() {
//        duration = 0.0f;
//    }
};