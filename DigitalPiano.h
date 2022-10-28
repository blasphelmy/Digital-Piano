#pragma once
#include "GLOBALVARIABLES.h"
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
// #include <sqltypes.h>
// #include <sql.h>
// #include <sqlext.h>
#include <signal.h>
#include <thread>
#include <map>
#include <unordered_map>
#include <queue>
#include <mutex>
struct MidiTimer {
    //chrono library is so verbose.
    std::chrono::high_resolution_clock::time_point  start;
    std::chrono::high_resolution_clock::time_point  finish;
    int flag                 = 0;
    int index                = 0;
    long long timeSinceStart = 0.0;
    float speed              = 1.0;
    float qNotePerSec        = 1.5f;
    float duration           = 0.f;
    std::string fileName     = "";
    bool isPlaying           = false;
    std::mutex midiLock;
    void tick() {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (speed > 3) speed = 3;
        if (speed < 0) speed = 0;
        this->finish         = std::chrono::high_resolution_clock::now();
        this->timeSinceStart = (this->timeSinceStart 
                             + (std::chrono::duration_cast<std::chrono::milliseconds>(this->finish - this->start).count() * this->speed));
        this->start          = std::chrono::high_resolution_clock::now();
    }
};

class PIXELGAMEENGINE_EXT : public olc::PixelGameEngine {
protected:
    struct horizontalLine { float y = _KEYSIZE; float left = 0.f; float right = _WINDOW_W; };
protected :
    void FillRoundedRect(olc::vd2d pos, olc::vd2d size, olc::Pixel color) {
        float radius = 4.f;
        if (size.y < 15) {
            size.y = 15;
        }
        olc::vd2d innerRect = size - (2 * olc::vd2d(radius, radius));
        olc::vd2d innerRect_pos = pos + olc::vd2d(radius, radius);
        FillRect(innerRect_pos, innerRect, color);

        FillRect(olc::vd2d(innerRect_pos.x, innerRect_pos.y - radius), olc::vd2d(innerRect.x, size.y - innerRect.y - radius), color);
        FillRect(olc::vd2d(innerRect_pos.x - radius, innerRect_pos.y), olc::vd2d(size.x - innerRect.x - radius, innerRect.y), color);

        FillRect(olc::vd2d(innerRect_pos.x, innerRect_pos.y + innerRect.y - 1.f), olc::vd2d(innerRect.x, size.y - innerRect.y - radius), color);
        FillRect(olc::vd2d(innerRect_pos.x + innerRect.x - 1.f, innerRect_pos.y), olc::vd2d(size.x - innerRect.x - radius, innerRect.y), color);

        FillCircle(innerRect_pos, radius, color);
        FillCircle(olc::vd2d(innerRect_pos.x, innerRect_pos.y + innerRect.y - 2.f), radius, color);
        FillCircle(olc::vd2d(innerRect_pos.x + innerRect.x - 2.f, innerRect_pos.y + innerRect.y - 2.f), radius, color);
        FillCircle(olc::vd2d(innerRect_pos.x + innerRect.x - 2.f, innerRect_pos.y), radius, color);
    }
};

class DigitalPiano : public PIXELGAMEENGINE_EXT {
public:
    DigitalPiano() {
        sAppName = "Digital Piano";
    }
    ~DigitalPiano() {
    }
public:
    MAPPER* keyMapper = nullptr;
    MidiTimer midiTimer;
private:
    struct ProgressBar {
        olc::vd2d bg            = olc::vd2d(_WINDOW_W, 20.f);
        olc::vd2d progressBar   = olc::vd2d(0, 20.f);
        olc::vd2d pos           = olc::vd2d(0, 0);
        olc::Pixel fillColor    = olc::Pixel(33, 148, 58);
        void resetProgressBar   () { progressBar.x = progressBar.y = 0; }
        void setProgressBar     (float timeSinceStart, float duration) { progressBar.x = _WINDOW_W * (timeSinceStart / duration); }
    };
private:
    std::unordered_map<std::string, vector3i> colorMap;
    std::queue<horizontalLine>                scrollingLines;
    ProgressBar                               progressBar;
    float                                     timeAccumalator = 500.f;
    float                                     targetBPM = 1.5f;

public:
    void connectMapper(MAPPER* newMapper) {
        keyMapper = newMapper;
    }
    void playSignal() {
        midiTimer.index             = 0;
        midiTimer.timeSinceStart    = 0.0;
        timeAccumalator             = 0.f;
        keyMapper->pedal            = true;
    }
    void reset() {
        tsf_note_off_all            (keyMapper->soundFile);
        midiTimer.isPlaying         = false;
        midiTimer.timeSinceStart    = 0.0;
        keyMapper                   ->flushActiveNotes();
    }
private:
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
    bool OnUserDestroy() override {
        midiTimer.flag = -1;
        return true;
    }

private:

    void keyListeners() {
        if (GetKey(olc::Key::LEFT).bPressed) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 1;
            SeekRoutine(-1, targetBPM * 1000.f * 4);
            midiTimer.midiLock.unlock();
        }
        else if (GetKey(olc::Key::RIGHT).bPressed) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 2;
            SeekRoutine(1, targetBPM * 1000.f * 4);
            midiTimer.midiLock.unlock();
        }
        if (GetKey(olc::SPACE).bPressed) {
            midiTimer.speed = (midiTimer.speed > 0 ? 0.0 : 1.0);
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
        int i = 0;
        DrawString(10, 30, "Hold shift then enter in an mid file name : " + midiTimer.fileName, GetKey(olc::SHIFT).bHeld && !midiTimer.isPlaying ? olc::CYAN : olc::RED, _TEXT_SCALE);
        DrawString(10, 50, "Options : clairedelune.mid | mozartk545.mid | arab2.mid ", olc::YELLOW, _TEXT_SCALE);
        DrawString(10, 70, "blackkeys.mid | pokeCredits.mid | theEnd.mid | cianwood.mid", olc::YELLOW, _TEXT_SCALE);
        DrawString(10, 90, "Speed (up/down) keys : x" + std::to_string(midiTimer.speed), olc::WHITE, _TEXT_SCALE);
        //DrawString(10, 120, "Time (forward/back) keys : " + std::to_string(midiTimer.timeSinceStart / 1000.f) + "/" + std::to_string(midiTimer.duration), olc::WHITE, _TEXT_SCALE);
        //DrawString(10, 145, "Active voices : " + std::to_string(tsf_active_voice_count(keyMapper->soundFile)), olc::WHITE, _TEXT_SCALE);
        //DrawString(10, 170, "ActiveNotes Size : " + std::to_string(keyMapper->activeNotesPool.size()), olc::WHITE, _TEXT_SCALE);
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
            onscreenKey.position.y += timeOffset * 100.f * -1 * direction;
            if (onscreenKey.position.y < _KEYSIZE)  reset.push(onscreenKey);
        }
        for (int i = 0; i < keyMapper->keyMap.size(); i++) {
            keyMapper->keyMap[i].velocity = 0;
            if (keyMapper->activelyDrawing.count(i) > 0) keyMapper->activelyDrawing.erase(i);
        }
        keyMapper->onScreenNoteElements = reset;
        keyMapper->threadLock.unlock();
    }

    olc::Pixel getColor(bool isWhite, int velocity, std::string& note) {
        if (isWhite) return velocity == 0 ? olc::Pixel(210, 210, 210) : getDrawingColor(isWhite, note);
        return velocity == 0 ? olc::Pixel(25, 25, 25) : getDrawingColor(isWhite, note);
    }
    olc::Pixel getDrawingColor(bool isWhite, std::string& note) {
        vector3f darkMask(isWhite ? .8 : .6);
        vector3i color = colorMap[note];
        color = color * darkMask;
        return olc::Pixel(color.x, color.y, color.z);
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
            DrawLine(olc::vd2d(line.left, line.y), olc::vd2d(line.right, line.y), olc::Pixel(50, 50, 50));
            line.y -= yOffSet;
            if (line.y > 0) newHorizontalLinesQueue.push(line);
        }
        scrollingLines = newHorizontalLinesQueue;

        while (!keyMapper->onScreenNoteElements.empty()) {

            FlyingNotes onscreenKey = keyMapper->onScreenNoteElements.front();
            keyMapper->onScreenNoteElements.pop();

            FillRoundedRect(onscreenKey.position, onscreenKey.size - olc::vd2d(1, 1), getDrawingColor(onscreenKey.isWhite, onscreenKey.name));
            onscreenKey.position.y -= yOffSet;

            if (onscreenKey.position.y + onscreenKey.size.y + 12.0 > 0)
                newOnScreenElementsQueue.push(onscreenKey);
        }
        keyMapper->onScreenNoteElements = newOnScreenElementsQueue;
        for (int i = 0; i < keyMapper->keyMap.size(); i++) {
            if (i == 52) FillRect(olc::vd2d(0.f, _KEYSIZE), olc::vd2d(_WINDOW_W, 5.f), olc::Pixel(82, 38, 38));
            if (keyMapper->activelyDrawing.count(i) > 0) {
                key* drawnKey = keyMapper->activelyDrawing.find(i)->second;
                FillRoundedRect(drawnKey->position, drawnKey->size - olc::vd2d(1, 1), getDrawingColor(drawnKey->isWhite, drawnKey->name));
                drawnKey->size.y += yOffSet;
                drawnKey->position.y -= yOffSet;
            }

            key thisKey = keyMapper->keyMap[i];

            if (thisKey.isWhite) {
                FillRect(thisKey.position, thisKey.size - olc::vd2d(1, 1), getColor(thisKey.isWhite, thisKey.velocity, thisKey.name));
            }
            else {
                FillRoundedRect(thisKey.position, thisKey.size - olc::vd2d(1, 1), getColor(thisKey.isWhite, thisKey.velocity, thisKey.name));
            }

        }
        keyMapper->threadLock.unlock();
        progressBar.setProgressBar(midiTimer.timeSinceStart / 1000.f, midiTimer.duration);
        FillRect(progressBar.pos, progressBar.bg, olc::DARK_GREY);
        FillRect(progressBar.pos, progressBar.progressBar, progressBar.fillColor);
        DrawString(10, 7, std::to_string(midiTimer.timeSinceStart / 1000.f) + "/" + std::to_string(midiTimer.duration), olc::WHITE, 1);
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