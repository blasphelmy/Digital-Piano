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
    float y = 880;
    float left = 0.f;
    float right = 1920.f;
};
struct MidiTimer {
    long long timeSinceStart;
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point finish;
    std::unordered_map<int, vector2d<int>> tickReversal;
};

class DigitalPiano : public olc::PixelGameEngine {
public:
    DigitalPiano() {
        sAppName = "Digital Piano";
    }
    ~DigitalPiano() {
    }
public:
    MAPPER* keyMapper = nullptr;
    MidiTimer midiTimer;
    std::unordered_map<std::string, vector3i> colorMap;
private:
    std::queue<horizontalLine> scrollingLines;
    float targetBPM = 1.5f;
    float timeAccumalator = 500.f;
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
        Clear(olc::Pixel(40, 40, 40));
        drawFrame(felaspedTime);
        SetPixelMode(olc::Pixel::NORMAL); // Draw all pixels
        return true;
    }
    void connectMapper(MAPPER* newMapper) {
        keyMapper = newMapper;
    }
    //void connectMidiParser(MidiParser* newParser) {
    //    midiParser = newParser;
    //}
private:
    olc::Pixel getColor(bool isWhite, int velocity, std::string note) {
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
    olc::Pixel getDrawingColor(bool isWhite, std::string note) {
        vector3f darkMask(0.8, 0.8, 0.8);
        if (!isWhite) {
            darkMask.setAll(.6);
        }
        vector3i color = colorMap[note];
        color = color * darkMask;
        return olc::Pixel(color.x, color.y, color.z);
    }
    void drawFrame(double timeElasped) {
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
            FillRect(onscreenKey.position, onscreenKey.size - olc::vd2d(1, 1), getDrawingColor(onscreenKey.isWhite, onscreenKey.name));
            onscreenKey.position.y -= yOffSet;

            if (onscreenKey.position.y + onscreenKey.size.y > 0)
                newOnScreenElementsQueue.push(onscreenKey);
        }
        keyMapper->onScreenNoteElements = newOnScreenElementsQueue;
        for (int i = 0; i < keyMapper->keyMap.size(); i++) {

            if (keyMapper->activelyDrawing.count(i) > 0) {
                key* drawnKey = keyMapper->activelyDrawing.find(i)->second;
                FillRect(drawnKey->position, drawnKey->size - olc::vd2d(1, 1), getDrawingColor(drawnKey->isWhite, drawnKey->name));
                drawnKey->size.y += yOffSet;
                drawnKey->position.y -= yOffSet;
            }
            key thisKey = keyMapper->keyMap[i];

            FillRect(thisKey.position, thisKey.size - olc::vd2d(1, 1), getColor(thisKey.isWhite, thisKey.velocity, thisKey.name));
        }
        keyMapper->threadLock.unlock();
    }
};