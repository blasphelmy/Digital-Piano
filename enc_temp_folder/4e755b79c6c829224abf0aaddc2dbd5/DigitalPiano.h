#pragma once
#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "minisdl_audio.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "Options.h"
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

struct MidiTimer {
    std::chrono::high_resolution_clock::time_point  start;
    std::chrono::high_resolution_clock::time_point  finish;
    int flag                 = 0;
    int index                = 0;
    long long timeSinceStart = 0.0;
    float speed              = 1.0;
    double ticks             = 0;
    int TPQ                  = 0;
    float qNotePerSec        = 1.5f;
    float duration           = 0.f;
    //std::string title        = "";
    std::string fileName     = "";
    bool isPlaying           = false;
    int numVoices            = 0;
    std::mutex midiLock;
    void tick() {
        this->finish = std::chrono::high_resolution_clock::now();
        this->timeSinceStart = (this->timeSinceStart + 
                                (std::chrono::duration_cast<std::chrono::milliseconds>(this->finish - this->start).count() * this->speed));
        this->start = std::chrono::high_resolution_clock::now();
        if (speed > 3) speed = 3;
        if (speed < 0) speed = 0;
    }
};

class DigitalPiano : public olc::PixelGameEngine {
private : 
    struct horizontalLine {
        float y = 880;
        float left = 0.f;
        float right = 1920.f;
    };
public:
    DigitalPiano() {
        sAppName = "Digital Piano";
    }
    ~DigitalPiano() {
    }
public:
    MAPPER*     keyMapper = nullptr;
    MidiTimer   midiTimer;
private:
    std::unordered_map<std::string, vector3i> colorMap;
    std::queue<horizontalLine> scrollingLines;
    float timeAccumalator = 500.f;
    float targetBPM       = 1.5f;

public:
    bool OnUserCreate() override {
        colorMap["C"] = vector3i(254, 0, 0);
        colorMap["D"] = vector3i(45, 122, 142);
        colorMap["E"] = vector3i(251, 133, 39);
        colorMap["F"] = vector3i(146, 209, 79);
        colorMap["G"] = vector3i(255, 255, 113);
        colorMap["A"] = vector3i(107, 60, 200);
        colorMap["B"] = vector3i(146, 209, 79);
        return true;
    }
    bool OnUserUpdate(float felaspedTime) override {
        keyListeners();
        Clear(olc::Pixel(40, 40, 40));
        drawFrame(felaspedTime);
        drawData();
        SetPixelMode(olc::Pixel::NORMAL); // Draw all pixels
        return true;
    }
    bool OnUserDestroy() override{
        midiTimer.flag = -1;
        return true;
    }
    void connectMapper(MAPPER* newMapper) {
        keyMapper = newMapper;
    }
    void playSignal() {
        timeAccumalator = 0.f;
        keyMapper->pedal = false;
    }
private:
    void keyListeners() {
        if (GetKey(olc::Key::LEFT).bPressed) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 1;
            SeekRoutine(-1, 15000.f);
            midiTimer.midiLock.unlock();
        }
        else if (GetKey(olc::Key::RIGHT).bPressed) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 2;
            SeekRoutine(1, 15000.f);
            midiTimer.midiLock.unlock();
        }
        if (GetKey(olc::Key::UP).bPressed) {
            midiTimer.speed = midiTimer.speed + .1;
        }
        if (GetKey(olc::Key::DOWN).bPressed) {
            midiTimer.speed = midiTimer.speed - .1;
        }
        if (!midiTimer.isPlaying && GetKey(olc::SHIFT).bHeld) {
            if (GetKey(olc::A).bPressed) {
                midiTimer.fileName += "A";
            }
            if (GetKey(olc::B).bPressed) {
                midiTimer.fileName += "B";
            }
            if (GetKey(olc::C).bPressed) {
                midiTimer.fileName += "C";
            }
            if (GetKey(olc::D).bPressed) {
                midiTimer.fileName += "D";
            }
            if (GetKey(olc::E).bPressed) {
                midiTimer.fileName += "E";
            }
            if (GetKey(olc::F).bPressed) {
                midiTimer.fileName += "F";
            }
            if (GetKey(olc::G).bPressed) {
                midiTimer.fileName += "G";
            }
            if (GetKey(olc::H).bPressed) {
                midiTimer.fileName += "H";
            }
            if (GetKey(olc::I).bPressed) {
                midiTimer.fileName += "I";
            }
            if (GetKey(olc::J).bPressed) {
                midiTimer.fileName += "J";
            }
            if (GetKey(olc::K).bPressed) {
                midiTimer.fileName += "K";
            }
            if (GetKey(olc::L).bPressed) {
                midiTimer.fileName += "L";
            }
            if (GetKey(olc::M).bPressed) {
                midiTimer.fileName += "M";
            }
            if (GetKey(olc::N).bPressed) {
                midiTimer.fileName += "N";
            }
            if (GetKey(olc::O).bPressed) {
                midiTimer.fileName += "O";
            }
            if (GetKey(olc::P).bPressed) {
                midiTimer.fileName += "P";
            }
            if (GetKey(olc::Q).bPressed) {
                midiTimer.fileName += "Q";
            }
            if (GetKey(olc::R).bPressed) {
                midiTimer.fileName += "R";
            }
            if (GetKey(olc::S).bPressed) {
                midiTimer.fileName += "S";
            }
            if (GetKey(olc::T).bPressed) {
                midiTimer.fileName += "T";
            }
            if (GetKey(olc::U).bPressed) {
                midiTimer.fileName += "U";
            }
            if (GetKey(olc::V).bPressed) {
                midiTimer.fileName += "V";
            }
            if (GetKey(olc::W).bPressed) {
                midiTimer.fileName += "W";
            }
            if (GetKey(olc::X).bPressed) {
                midiTimer.fileName += "X";
            }
            if (GetKey(olc::Y).bPressed) {
                midiTimer.fileName += "Y";
            }
            if (GetKey(olc::Z).bPressed) {
                midiTimer.fileName += "Z";
            }

            if (GetKey(olc::K0).bPressed) {
                midiTimer.fileName += "0";
            }
            if (GetKey(olc::K1).bPressed) {
                midiTimer.fileName += "1";
            }
            if (GetKey(olc::K2).bPressed) {
                midiTimer.fileName += "2";
            }
            if (GetKey(olc::K3).bPressed) {
                midiTimer.fileName += "3";
            }
            if (GetKey(olc::K4).bPressed) {
                midiTimer.fileName += "4";
            }
            if (GetKey(olc::K5).bPressed) {
                midiTimer.fileName += "5";
            }
            if (GetKey(olc::K6).bPressed) {
                midiTimer.fileName += "6";
            }
            if (GetKey(olc::K7).bPressed) {
                midiTimer.fileName += "7";
            }
            if (GetKey(olc::K8).bPressed) {
                midiTimer.fileName += "8";
            }
            if (GetKey(olc::K9).bPressed) {
                midiTimer.fileName += "9";
            }

            if (GetKey(olc::PERIOD).bPressed) {
                midiTimer.fileName += ".";
            }
            if (GetKey(olc::NP_SUB).bPressed) {
                midiTimer.fileName += "-";
            }
            if (GetKey(olc::BACK).bPressed) {
                if (midiTimer.fileName.size() > 0) midiTimer.fileName.erase(midiTimer.fileName.size() - 1);
            }
            if (GetKey(olc::ENTER).bPressed) {
                midiTimer.isPlaying = true;
            }
        }
    }
    void drawData() {
        DrawString(10, 20, "Hold shift then enter in an mid file name : " + midiTimer.fileName, GetKey(olc::SHIFT).bHeld && !midiTimer.isPlaying ? olc::CYAN : olc::RED, 2);
        DrawString(10, 45, "Options : clairedelune.mid | cianwood.mid | arab2.mid ", olc::YELLOW, 2);
        DrawString(10, 70, "blackkeys.mid | pokeCredits.mid | theEnd.mid", olc::YELLOW, 2);
        DrawString(10, 95, "Speed (up/down) keys : x" + std::to_string(midiTimer.speed), olc::WHITE, 2);
        DrawString(10, 120, "Time (forward/back) keys : " + std::to_string(midiTimer.timeSinceStart / 1000.f) + "/" + std::to_string(midiTimer.duration), olc::WHITE, 2);
        DrawString(10, 145, "Active voices : " + std::to_string(midiTimer.numVoices), olc::WHITE, 2);
        DrawString(10, 170, "Active voices : " + std::to_string(tsf_active_voice_count(keyMapper->soundFile)), olc::WHITE, 2);
    }
    void SeekRoutine(int direction, float timeOffset) {
        if (direction == -1) {
            midiTimer.timeSinceStart -= timeOffset;
        }
        else {
            midiTimer.timeSinceStart += timeOffset;
        }
        std::queue<FlyingNotes> reset;
        keyMapper->threadLock.lock();
        while (!keyMapper->onScreenNoteElements.empty()) {
            FlyingNotes onscreenKey = keyMapper->onScreenNoteElements.front();
            keyMapper->onScreenNoteElements.pop();
            onscreenKey.position.y += 100.f * -1 * direction;
            if (onscreenKey.position.y < 880.f) {
                reset.push(onscreenKey);
            }
        }
        for (int i = 0; i < keyMapper->keyMap.size(); i++) {
            keyMapper->keyMap[i].velocity = 0;
            if (keyMapper->activelyDrawing.count(i) > 0) {
                keyMapper->activelyDrawing.erase(i);
            }
        }
        keyMapper->pedal = false;
        keyMapper->onScreenNoteElements = reset;
        keyMapper->threadLock.unlock();
    }

    olc::Pixel getColor(bool isWhite, int velocity, std::string & note) {
        if (isWhite) {
            if (velocity == 0) {
                return olc::Pixel(210, 210, 210);
            }
            else {
                return getDrawingColor(isWhite, note);
            }
        }
        else {
            if (velocity == 0) {
                return olc::Pixel(25, 25, 25);
            }
            else {
                return getDrawingColor(isWhite, note);
            }
        }
        return olc::RED;
    }
    olc::Pixel getDrawingColor(bool isWhite, std::string & note) {
        vector3f darkMask(.8, .8, .8);
        if (!isWhite) {
            darkMask.setAll(.6);
        }
        vector3i color = colorMap[note];
        color = color * darkMask;
        return olc::Pixel(color.x, color.y, color.z);
    }
    void FillRoundedRect(olc::vd2d pos, olc::vd2d size, double radius, olc::Pixel color) {
        //FillRect(pos, size, olc::WHITE);
        //PATCH FOR SMALL RECTANGLES... relationship is with radius...
        if (size.y < 24) {
            size.y = 24;
        }
        olc::vd2d innerRect = size - (2 * olc::vd2d(radius, radius));
        olc::vd2d innerRect_pos = pos + olc::vd2d(radius, radius);
        FillRect(innerRect_pos, innerRect + olc::vd2d(3.0, 3.0), color);

        FillRect(olc::vd2d(innerRect_pos.x, innerRect_pos.y - radius), olc::vd2d(innerRect.x, size.y - innerRect.y - radius), color);
        FillRect(olc::vd2d(innerRect_pos.x - radius, innerRect_pos.y), olc::vd2d(size.x - innerRect.x - radius, innerRect.y), color);

        FillRect(olc::vd2d(innerRect_pos.x, innerRect_pos.y + innerRect.y + 1.0), olc::vd2d(innerRect.x, size.y - innerRect.y - radius), color);
        FillRect(olc::vd2d(innerRect_pos.x + innerRect.x + 1.0, innerRect_pos.y), olc::vd2d(size.x - innerRect.x - radius, innerRect.y), color);

        FillCircle(innerRect_pos, radius, color);
        FillCircle(olc::vd2d(innerRect_pos.x, innerRect_pos.y + innerRect.y), radius, color);
        FillCircle(olc::vd2d(innerRect_pos.x + innerRect.x, innerRect_pos.y + innerRect.y), radius, color);
        FillCircle(olc::vd2d(innerRect_pos.x + innerRect.x, innerRect_pos.y), radius, color);
    }

    void drawFrame(double timeElasped) {
        keyMapper->threadLock.lock();
        double yOffSet = timeElasped * 100.f;
        std::queue<FlyingNotes> newOnScreenElementsQueue;
        std::queue<horizontalLine> newHorizontalLinesQueue;
        timeAccumalator += timeElasped * midiTimer.speed;
        if (timeAccumalator > midiTimer.qNotePerSec) {
            timeAccumalator = 0.f;
            scrollingLines.push(horizontalLine());
        }

        while (!scrollingLines.empty()) {
            horizontalLine line = scrollingLines.front();
            scrollingLines.pop();
            DrawLine(olc::vd2d(line.left, line.y), olc::vd2d(line.right, line.y), olc::Pixel(50,50,50));
            line.y -= yOffSet;
            if (line.y > 0) newHorizontalLinesQueue.push(line);
        }
        scrollingLines = newHorizontalLinesQueue;
        while (!keyMapper->onScreenNoteElements.empty()) {

            FlyingNotes onscreenKey = keyMapper->onScreenNoteElements.front();
            keyMapper->onScreenNoteElements.pop();
            FillRoundedRect(onscreenKey.position, onscreenKey.size - olc::vd2d(1, 1), 12.0, getDrawingColor(onscreenKey.isWhite, onscreenKey.name));
            onscreenKey.position.y -= yOffSet;

            if (onscreenKey.position.y + onscreenKey.size.y + 12.0 > 0)
                newOnScreenElementsQueue.push(onscreenKey);
        }
        keyMapper->onScreenNoteElements = newOnScreenElementsQueue;
        for (int i = 0; i < keyMapper->keyMap.size(); i++) {

            if (keyMapper->activelyDrawing.count(i) > 0) {
                key* drawnKey = keyMapper->activelyDrawing.find(i)->second;
                FillRoundedRect(drawnKey->position, drawnKey->size - olc::vd2d(1, 1), 12.0, getDrawingColor(drawnKey->isWhite, drawnKey->name));
                drawnKey->size.y += yOffSet;
                drawnKey->position.y -= yOffSet;
            }
            key thisKey = keyMapper->keyMap[i];

            if (thisKey.isWhite) {
                FillRect(thisKey.position, thisKey.size - olc::vd2d(1, 1), getColor(thisKey.isWhite, thisKey.velocity, thisKey.name));
            }
            else {
                FillRoundedRect(thisKey.position, thisKey.size - olc::vd2d(1, 1), 12.0,  getColor(thisKey.isWhite, thisKey.velocity, thisKey.name));
            }

        }
        keyMapper->threadLock.unlock();
    }
};
class NoteAnalyzer {
public:
    NoteAnalyzer(DigitalPiano* app) {
        this->piano = app;
    }
    ~NoteAnalyzer() {

    }
public:
    DigitalPiano* piano;

};