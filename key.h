#pragma once
#include "GLOBALVARIABLES.h"
#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "minisdl_audio.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "Options.h"
#include "DigitalPiano.h"
#include "MAPPER.h"
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
class key {
public:
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
    using key::key;
    vector3i color;
    double duration = 0.0f;
    void addDuration(double dur) {
        duration += dur;
    }
    void resetDuration() {
        duration = 0.0f;
    }
};