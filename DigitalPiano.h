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
struct horizontalLine {
    float y     = 880;
    float left  = 0.f;
    float right = 1920.f;
};
struct MidiTimer {
    std::chrono::high_resolution_clock::time_point  start;
    std::chrono::high_resolution_clock::time_point  finish;
    int flag                 = 0;
    int index                = 0;
    long long timeSinceStart = 0;
    std::mutex midiLock;
};

class DigitalPiano : public olc::PixelGameEngine {
public:
    DigitalPiano() {
        sAppName = "Digital Piano";
    }
    ~DigitalPiano() {
    }
public:
    std::unordered_map<std::string, vector3i> colorMap;
    MAPPER*     keyMapper = nullptr;
    MidiTimer   midiTimer;
private:
    std::queue<horizontalLine> scrollingLines;
    float timeAccumalator = 500.f;
    float targetBPM       = 1.5f;
public:
    void SeekRoutine(int direction) {
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
        keyMapper->onScreenNoteElements = reset;
        keyMapper->threadLock.unlock();
    }
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
        
        if (GetKey(olc::Key::LEFT).bPressed) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 1;
            midiTimer.start += std::chrono::milliseconds(1000);
            midiTimer.finish = std::chrono::high_resolution_clock::now();
            midiTimer.timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(midiTimer.finish - midiTimer.start).count();
            SeekRoutine(-1);
            midiTimer.flag = 0;
            midiTimer.midiLock.unlock();
        }else if (GetKey(olc::Key::RIGHT).bPressed) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 2;
            midiTimer.start -= std::chrono::milliseconds(1000);
            midiTimer.finish = std::chrono::high_resolution_clock::now();
            midiTimer.timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(midiTimer.finish - midiTimer.start).count();
            SeekRoutine(1);
            midiTimer.flag = 0;
            midiTimer.midiLock.unlock();
        }
        Clear(olc::Pixel(40, 40, 40));
        drawFrame(felaspedTime);
        SetPixelMode(olc::Pixel::NORMAL); // Draw all pixels
        return true;
    }
    bool OnUserDestroy() override{
        return true;
    }
    void connectMapper(MAPPER* newMapper) {
        keyMapper = newMapper;
    }
    //void connectMidiParser(MidiParser* newParser) {
    //    midiParser = newParser;
    //}
private:
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
            darkMask.setAll(.4);
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
        //FillCircle(olc::vi2d(40, 40), 20, olc::WHITE);
        //FillRect(olc::vd2d(10, 10), olc::vd2d(30, 100), olc::WHITE);
 /*       FillRoundedRect(olc::vd2d(10, 10), olc::vd2d(100,300), 5.0, olc::WHITE);*/
        keyMapper->threadLock.lock();
        double yOffSet = timeElasped * 100.f;
        std::queue<FlyingNotes> newOnScreenElementsQueue;
        std::queue<horizontalLine> newHorizontalLinesQueue;
        timeAccumalator += timeElasped;
        if (timeAccumalator > targetBPM) {
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