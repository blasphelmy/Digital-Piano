#pragma once
#include "GLOBALVARIABLES.h"
#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "MAPPER.h"
#include "Keys.h"
#include "vectors.h"
#include <iostream>
#include <chrono>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <set>
#include <math.h>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;

struct channel { 
    bool MUTED = false; bool ACTIVE = false; char channelNum; 
    int drawSelf(olc::PixelGameEngine* parent, MAPPER * keyMapper, int index) { 
        if (ACTIVE) { 
            std::string channel = "Channel : " + std::to_string(channelNum);
            olc::vi2d pos((int)_WINDOW_W - 110, 50 + (index++ * 20));
            olc::vi2d bounds(pos.x + (channel.size() * charWidth), pos.y + textHeight);
            olc::Pixel color = !MUTED ? olc::GREEN : olc::RED;
            if (parent->checkBounds(pos, bounds)) {
                if (parent->GetMouse(olc::Mouse::LEFT).bPressed) {
                    MUTED = MUTED ? false : true;
                    /*keyMapper->activelyDrawing.clear();
                    for (int j = 0; j < keyboardSize; j++) {
                        keyMapper->keyMap[j].velocity = 0;
                    }*/
                }
                color = olc::CYAN;
            }
            parent->DrawString(pos.x, pos.y, channel, color, (uint32_t)_TEXT_SCALE);
        }
        return index;
    };
};

class Channels {
public:
    Channels() {
        for (int i = 0; i < midiChannels; i++) channels[i].channelNum = i;
    };
    ~Channels() {};
public:
    std::array<channel, midiChannels> channels;
private:
    short mask = 0x0f;
    bool checkRange(short maskedChannel) { return (short)maskedChannel >= 0x00 && (short)maskedChannel <= 0x0f; }

public:
    bool checkChannel(short channel) {
        if (channels[(short)channel].ACTIVE && !channels[(short)channel].MUTED) return true;
        return false;
    }
    bool checkChannel(smf::MidiEvent& event) {
        short maskedChannel = (short)event[0] & mask;
        return checkChannel(maskedChannel);;
    }

    bool setChannels(smf::MidiFile& file) {
        for (int i = 0; i < file[0].size(); i++) {
            if (!checkChannel(file[0][i])) {
                short maskedChannel = (short)file[0][i][0] & mask;
                if (checkRange((short)maskedChannel)) channels[maskedChannel].ACTIVE = true;
            }
        }
        return true;
    }
    void resetChannels() { for (int i = 0; i < midiChannels; i++) channels[i].ACTIVE = channels[i].MUTED = false; }
};

class MidiTimer {
public:
    MidiTimer() {};
    ~MidiTimer() {};
public:
    //chrono library is so verbose.
    high_resolution_clock::time_point   start;
    high_resolution_clock::time_point   finish;
    std::set<std::string>               midiFileSet;
    Channels                            Channels;
    long double timeSinceStart          = 0.0;
    int         flag                    = 0;
    int         index                   = 0;
    float       speed                   = 1.0;
    float       qNotePerSec             = 1.5f;
    float       duration                = 0.f;
    float       targetBPM               = 1.5f;
    bool        isPlaying               = false;
    std::string fileName                = "";
    std::mutex  midiLock;
    void tick() {
        std::this_thread::sleep_for(milliseconds(1));
        if (speed > 3) speed = 3.0f;
        if (speed < 0) speed = 0.0f;
        this->finish         = high_resolution_clock::now();
        this->timeSinceStart = (this->timeSinceStart
                             + (duration_cast<milliseconds>(this->finish - this->start).count()
                             * this->speed));
        this->start          = high_resolution_clock::now();
    }
};

class PIXELGAMEENGINE_EXT : public olc::PixelGameEngine {
protected:
    void DrawStringDecal(float x, float y, std::string text, olc::Pixel color) { olc::PixelGameEngine::DrawStringDecal(olc::vd2d(x, y), text, color); }

    vector3i normalizeColorVector(vector3i colorVector, float f, float r_bias = 0.3, float g_bias = 0.6, float b_bias = 0.1) {
        float r = (float)colorVector.x;
        float g = (float)colorVector.y;
        float b = (float)colorVector.z;

        float L = r_bias * r
                + g_bias * g
                + b_bias * b;

        float new_r = r + f * (L - r);
        float new_g = g + f * (L - g);
        float new_b = b + f * (L - b);
        return vector3i((int)new_r, (int)new_g, (int)new_b);
    };

    void FillRoundedRect(const olc::vd2d & pos, olc::vd2d size, const olc::Pixel & color, float radius) {
        //size.y = size.y - 2;
        //olc::Pixel aliasedColor(color.r, color.g, color.b, 15);
        //if (size.y < radius * 2) size.y = radius * 2;
        //olc::vi2d innerRect = size - olc::vd2d(radius * 2.0, radius * 2.0);
        //if (innerRect.x % 2 == 1) innerRect.x--;
        //olc::vi2d innerRect_pos = pos + olc::vd2d(radius, radius);
        //float a = innerRect.x;
        //float b = innerRect.y;
        //float c = innerRect_pos.x;
        //float d = innerRect_pos.y;
        //float r2 = radius * radius;
        //float distance = 0;

        //for (int x = pos.x; x < pos.x + size.x; x++) {
        //    for (int y = pos.y; y < pos.y + size.y; y++) {
        //        if (x <= c && y <= d) {
        //            //top left corner
        //            distance = (x - c) * (x - c) + (y - d) * (y - d);
        //            if (distance <= r2)
        //                Draw(x, y, color);
        //            //else if (distance <= r2 + 2)
        //            //    Draw(x, y, color);
        //        }
        //        else if (x >= c + a && y <= d) {
        //            //top right corner
        //            distance = (x - (c + a)) * (x - (c + a)) + (y - d) * (y - d);
        //            if (distance <= r2)
        //                Draw(x, y, color);
        //            //else if (distance <= r2 + 2)
        //            //    Draw(x, y, color);
        //        }
        //        else if (x >= c + a && y >= d + b) {
        //            //bottom right corner
        //            distance = (x - (c + a)) * (x - (c + a)) + (y - (d + b)) * (y - (d + b));
        //            if (distance <= r2)
        //                Draw(x, y, color);
        //            //else if (distance <= r2 + 2)
        //            //    Draw(x, y, color);
        //        }
        //        else if (x <= c && y >= d + b) {
        //            //bottom left corner
        //            distance = (x - c) * (x - c) + (y - (d + b)) * (y - (d + b));
        //            if (distance <= r2)
        //                Draw(x, y, color);
        //            //else if (distance <= r2 + 2 )
        //            //    Draw(x, y, color);
        //        }
        //        else {
        //            Draw(x, y, color);
        //        }

        //    }
        //}     
        radius = 4.f;
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

        FillCircle(innerRect_pos, (int32_t)radius, color);
        FillCircle(olc::vd2d(innerRect_pos.x, innerRect_pos.y + innerRect.y - 2.f), (int32_t)radius, color);
        FillCircle(olc::vd2d(innerRect_pos.x + innerRect.x - 2.f, innerRect_pos.y + innerRect.y - 2.f), (int32_t)radius, color);
        FillCircle(olc::vd2d(innerRect_pos.x + innerRect.x - 2.f, innerRect_pos.y), (int32_t)radius, color);
    }
};

class DigitalPiano : public PIXELGAMEENGINE_EXT {
public:
    DigitalPiano() { sAppName = "Digital Piano"; }
    ~DigitalPiano() { }
private:
    bool OnUserCreate() override {
        colorMap["C"] = vector3di8t(254,   0,   0);
        colorMap["D"] = vector3di8t( 45, 122, 142);
        colorMap["E"] = vector3di8t(251, 133,  39);
        colorMap["F"] = vector3di8t(146, 209,  79);
        colorMap["G"] = vector3di8t(255, 255, 113);
        colorMap["A"] = vector3di8t(107,  60, 200);
        colorMap["B"] = vector3di8t(209,  79, 205);
        return true;
    }
private:
    MAPPER* keyMapper = nullptr;
public:
    MAPPER* returnMapper()                { return keyMapper; };
    void connectMapper(MAPPER* newMapper) { keyMapper = newMapper; }
public:
    MidiTimer midiTimer;
private:
    struct ProgressBar {
        olc::vd2d bg            = olc::vd2d(_WINDOW_W, ProgressBarHeight);
        olc::vd2d progressBar   = olc::vd2d(0, ProgressBarHeight);
        olc::vd2d pos           = olc::vd2d(0, 0);
        olc::Pixel fillColor    = olc::Pixel(33, 148, 58);

        void resetProgressBar   () { progressBar.x = progressBar.y = 0; }
        void setProgressBar     (float timeSinceStart, float duration) { progressBar.x = _WINDOW_W * (timeSinceStart / duration); }
    };
    struct horizontalLine {
        float y = (float)_KEYSIZE; float left = 0.f; float right = (float)_WINDOW_W;
        bool drawSelf(olc::PixelGameEngine* parent, float yOffSet) {
            parent->DrawLine(olc::vd2d(this->left, this->y), olc::vd2d(this->right, this->y), olc::Pixel(50, 50, 50));
            this->y -= yOffSet;
            return this->y > 0 ? true : false;
        }
    };
private:
    std::unordered_map<std::string, vector3di8t> colorMap;
    std::queue<horizontalLine>                   scrollingLines;
    ProgressBar                                  progressBar;
    float                                        timeAccumalator = 500.f;
    float                                        targetBPM = 1.5f;
    bool                                         displayNoteChannel = false;

public:
    void playSignal(smf::MidiFile& midifile) {
        midiTimer.qNotePerSec    = (float)midifile.getFileDurationInSeconds()
                                 / (float)midifile.getFileDurationInTicks()
                                 * (float)midifile.getTicksPerQuarterNote();

        midiTimer.duration       = (float)midifile.getFileDurationInSeconds();
        midiTimer.fileName       = midifile.getFilename();

        midiTimer.timeSinceStart = 0.0;
        timeAccumalator          = 0.f;
        keyMapper->pedal         = true;
        midiTimer.flag           = 0;

        if(midiTimer.qNotePerSec < .5) midiTimer.qNotePerSec = .5;

        midiTimer.index          = 0;
        midiTimer.start          = high_resolution_clock::now();
    }
    void reset() {
        tsf_note_off_all(keyMapper->soundFile);
        if (midiTimer.flag != -2) midiTimer.isPlaying = false;
        midiTimer.timeSinceStart = 0.0;
        midiTimer.Channels       .resetChannels();
        keyMapper                ->flushActiveNotes();
    }

private:
    bool OnUserUpdate(float felaspedTime) override {
        Clear           (olc::Pixel(40, 40, 40));
        keyListeners    ();
        drawFrame       (felaspedTime);
        drawData        ();
        SetPixelMode    (olc::Pixel::NORMAL); // Draw all pixels
        return true;
    }
    bool OnUserDestroy() override { midiTimer.flag = -1; return true; }

private:
    void resetActiveNotes() {
        for (int i = 0; i < keyMapper->keyMap.size(); i++) {
            keyMapper->keyMap[i].velocity = 0;
            if (keyMapper->activelyDrawing.count(i) > 0) keyMapper->activelyDrawing.erase(i);
        }
    }
    void SeekRoutine(int direction, float timeOffset) {
        midiTimer.timeSinceStart += timeOffset * direction;
        std::queue<FlyingNotes> reset;
        std::queue<horizontalLine> horizontalLinesReset;
        keyMapper->threadLock.lock();
        while (!keyMapper->onScreenNoteElements.empty()) {
            FlyingNotes onscreenKey = keyMapper->onScreenNoteElements.front();
            keyMapper->onScreenNoteElements.pop();
            onscreenKey.position.y -= timeOffset / 10.f * direction;
            if (onscreenKey.position.y < _KEYSIZE) reset.push(onscreenKey);
        }
        resetActiveNotes();
        while (!scrollingLines.empty()) {
            horizontalLine line = scrollingLines.front();
            line.y -= timeOffset / 10.f * direction;
            if (line.y <= _KEYSIZE) horizontalLinesReset.push(line);
            scrollingLines.pop();
        }
        scrollingLines = horizontalLinesReset;
        keyMapper->onScreenNoteElements = reset;
        keyMapper->threadLock.unlock();
    }

    void fillInGhost() {
        olc::vd2d mousePOS = GetMousePos();
        FillRectDecal(olc::vd2d(0.f, 0.f), olc::vi2d((int32_t)mousePOS.x, 20), olc::Pixel(45, 122, 142, 150));
        progressBar.progressBar = olc::vd2d(mousePOS.x, 20.f);
    }
    void fillInSeekRoutine() {
        olc::vd2d mousePOS = GetMousePos();
        long double newTimeSinceStart = (mousePOS.x / _WINDOW_W) * midiTimer.duration * TIMEMAGNITUDE;
        if (newTimeSinceStart < midiTimer.timeSinceStart) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 1;
            SeekRoutine(-1, (float)(midiTimer.timeSinceStart - newTimeSinceStart));
            midiTimer.midiLock.unlock();
        }
        else {
            midiTimer.midiLock.lock();
            midiTimer.flag = 2;
            SeekRoutine(1, (float)(newTimeSinceStart - midiTimer.timeSinceStart));
            midiTimer.midiLock.unlock();
        }

    }

    olc::Pixel getColor(bool isWhite, int velocity, std::string& note) {
        if (isWhite) return velocity == 0 ? olc::Pixel(210, 210, 210) : getDrawingColor(isWhite, note);
        return velocity == 0 ? olc::Pixel(25, 25, 25) : getDrawingColor(isWhite, note);
    }
    olc::Pixel getDrawingColor(bool isWhite, std::string& note) {
        vector3f darkMask(isWhite ? .8f : .6f);
        vector3f color = colorMap[note].cast_to<float>();
        color = color * darkMask;
        return olc::Pixel((uint8_t)color.x, (uint8_t)color.y, (uint8_t)color.z);
    }

    int keyListeners() {
        if (GetKey(olc::Key::LEFT).bPressed) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 1;
            SeekRoutine(-1, targetBPM * TIMEMAGNITUDE);
            midiTimer.midiLock.unlock();
        }
        else if (GetKey(olc::Key::RIGHT).bPressed) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 2;
            SeekRoutine(1, targetBPM * TIMEMAGNITUDE);
            midiTimer.midiLock.unlock();
        }
        if (GetKey(olc::SPACE).bPressed) {
            midiTimer.speed = (midiTimer.speed > 0.f ? 0.0f : 1.0f);
        }
        if (GetKey(olc::Key::UP).bPressed) {
            midiTimer.speed = midiTimer.speed + SpeedOffSets;
        }
        if (GetKey(olc::Key::DOWN).bPressed) {
            midiTimer.speed = midiTimer.speed - SpeedOffSets;
        }

        return 0;
    }

    void drawData() {
        DrawStringDecal(10, 30, "Title : " + midiTimer.fileName, olc::RED);
        DrawStringDecal(10, 50, "Speed (up/down) keys : x" + std::to_string(midiTimer.speed), olc::WHITE);
        int i = 0;
        for (std::string fileName : midiTimer.midiFileSet) {
            olc::vi2d pos(10, 70 + (i++ * 20));
            olc::vi2d bounds(pos.x + (fileName.size() * charWidth), pos.y + textHeight);
            bool inBound = checkBounds(pos, bounds);
            if (inBound && GetMouse(olc::Mouse::LEFT).bPressed) {
                resetActiveNotes();
                midiTimer.flag = -2;
                midiTimer.fileName = fileName;
                midiTimer.isPlaying = true;
            }
            DrawStringDecal((float)pos.x, (float)pos.y, fileName, inBound ? olc::CYAN : olc::YELLOW);
        }
    }
    void drawChannels() {
        olc::vi2d pos((int)_WINDOW_W - 200, 30);
        olc::vi2d bounds(pos.x + (22 * charWidth), pos.y + textHeight);
        olc::Pixel color = displayNoteChannel ? olc::GREEN : olc::RED;
        if (checkBounds(pos, bounds)) {
            if (GetMouse(olc::Mouse::LEFT).bPressed) {
                displayNoteChannel = !displayNoteChannel;
            }
            color = olc::CYAN;
        }
        DrawString(pos.x, pos.y, "Toggle Channel Display", color, (uint32_t)_TEXT_SCALE);

        int i = 0;
        for (int j = 0; j < midiChannels && displayNoteChannel; j++) {
            i = midiTimer.Channels.channels[j].drawSelf(this, keyMapper, i);
        }
    }

    void drawFlyingNote(FlyingNotes* note, bool detached) {
        olc::Pixel color = getDrawingColor(note->isWhite, note->name);
        vector3i colorVector(color.r, color.g, color.b);
        if (!midiTimer.Channels.checkChannel(note->channel)) {

            colorVector = normalizeColorVector(colorVector, .9f);
        }
        FillRoundedRect(note->position, note->size - olc::vd2d(1, 0), olc::Pixel(colorVector.x, colorVector.y, colorVector.z), 4);
        if(detached && displayNoteChannel) olc::PixelGameEngine::DrawStringDecal(note->position + olc::vd2d(3, 1), std::to_string(note->channel), olc::Pixel(0, 0, 0, 255));
    }
    void drawFlyingNote(FlyingNotes& note, bool detached) {
        drawFlyingNote(&note, detached);
    }

    void drawFrame(double timeElasped) {
        keyMapper->threadLock.lock();

        double yOffSet = timeElasped * 100.f * midiTimer.speed;
        std::queue<FlyingNotes> newOnScreenElementsQueue;
        std::queue<horizontalLine> newHorizontalLinesQueue;
        timeAccumalator += (float)timeElasped * midiTimer.speed;

        if (timeAccumalator > midiTimer.qNotePerSec) {
            timeAccumalator = 0.f;
            scrollingLines.push(horizontalLine());
        }

        while (!scrollingLines.empty()) {
            horizontalLine line = scrollingLines.front();
            scrollingLines.pop();
            if (line.drawSelf(this, (float)yOffSet)) newHorizontalLinesQueue.push(line);
        }
        scrollingLines = newHorizontalLinesQueue;

        while (!keyMapper->onScreenNoteElements.empty()) {

            FlyingNotes onscreenKey = keyMapper->onScreenNoteElements.front();
            keyMapper->onScreenNoteElements.pop();
            if (midiTimer.Channels.checkChannel(onscreenKey.channel)) drawFlyingNote(onscreenKey, true);
            onscreenKey.position.y -= yOffSet;
            if (onscreenKey.position.y + onscreenKey.size.y > 0)
                newOnScreenElementsQueue.push(onscreenKey);
        }
        keyMapper->onScreenNoteElements = newOnScreenElementsQueue;
        for (int i = 0; i < keyboardSize; i++) {
            if (i == numWhiteKeys) FillRect(olc::vd2d(0.f, _KEYSIZE), olc::vd2d(_WINDOW_W, 5.f), olc::Pixel(82, 38, 38));
            if (keyMapper->activelyDrawing.count(i) > 0) {
                FlyingNotes* drawnKey = keyMapper->activelyDrawing.find(i)->second;
                if(midiTimer.Channels.checkChannel(drawnKey->channel)) drawFlyingNote(drawnKey, false);
                drawnKey->size.y += yOffSet;
                drawnKey->position.y -= yOffSet;
            }

            key thisKey = keyMapper->keyMap[i];

            if (thisKey.isWhite) {
                FillRect(thisKey.position, thisKey.size - olc::vd2d(1, 1), getColor(thisKey.isWhite, thisKey.velocity, thisKey.name));
            }
            else {
                FillRoundedRect(thisKey.position, thisKey.size - olc::vd2d(1, 1), getColor(thisKey.isWhite, thisKey.velocity, thisKey.name), 4);
            }

        }
        drawChannels();
        keyMapper->threadLock.unlock();
        ProcessBarEvents();
        DrawStringDecal(10, 7, std::to_string(midiTimer.timeSinceStart / TIMEMAGNITUDE) + "/" + std::to_string(midiTimer.duration), olc::WHITE);
    }
    
    void ProcessBarEvents() {
        progressBar.setProgressBar((float)midiTimer.timeSinceStart / TIMEMAGNITUDE, midiTimer.duration);
        FillRectDecal(progressBar.pos, progressBar.bg, olc::Pixel(45, 45, 45));
        FillRectDecal(progressBar.pos, progressBar.progressBar, progressBar.fillColor);
        
        olc::vi2d mousePOS = GetMousePos();
        if (mousePOS.y < progressBar.progressBar.y/* && GetMouse(olc::Mouse::LEFT).bHeld*/) fillInGhost();
        if (mousePOS.y < progressBar.progressBar.y && GetMouse(olc::Mouse::LEFT).bPressed) fillInSeekRoutine();
    }
};


class DigitalPianoController {
public:
    DigitalPiano* digitalPiano = nullptr;
    MAPPER* keyMapper          = nullptr;
    tsf* soundfile             = nullptr;
public:
    DigitalPianoController(DigitalPiano * newDigitalPiano, MAPPER * newKeyMapper, tsf * soundfile) {
        this->digitalPiano = newDigitalPiano;
        this->keyMapper    = newKeyMapper;
        this->soundfile    = soundfile;
    }
    ~DigitalPianoController() {};
private: 
    smf::MidiFile getMidiFileRoutine(std::string & fileName) {
        smf::MidiFile newMidiFile("./MIDIFILES/" + fileName);
        newMidiFile              .doTimeAnalysis();
        newMidiFile              .linkNotePairs();
        newMidiFile              .joinTracks(); // we only care about 1 track right now
        digitalPiano->midiTimer  .Channels.setChannels(newMidiFile);
        return newMidiFile;
    }
    bool playMidi(std::string & fileName, MidiTimer & midiTimer) {
        bool action = false;
        smf::MidiFile midifile = getMidiFileRoutine(fileName);
        digitalPiano->playSignal(midifile);
        smf::MidiEvent event;

        while (midiTimer.index < midifile[0].size()
            && midiTimer.flag != -1 && midiTimer.flag != -2) {

            action = true;
            midiTimer.tick();
            while (midiTimer.flag == 1 && midifile[0][midiTimer.index].seconds * TIMEMAGNITUDE >= midiTimer.timeSinceStart) {
                midiTimer.index--;
                if (midiTimer.index < 0) {
                    midiTimer.index = 0;
                    midiTimer.flag = 0;
                    break;
                }
                if (midifile[0][midiTimer.index].seconds * TIMEMAGNITUDE < midiTimer.timeSinceStart) {
                    midiTimer.flag = 0;
                    break;
                }
            }

            while (midiTimer.flag == 2
                && midifile[0][midiTimer.index].seconds * TIMEMAGNITUDE <= midiTimer.timeSinceStart)
            {
                midiTimer.index++;
                if (midiTimer.index > midifile[0].size() - 1) {
                    midiTimer.index = midifile[0].size() - 1;
                    midiTimer.flag = 0;
                    break;
                }
                if (midifile[0][midiTimer.index].seconds * TIMEMAGNITUDE > midiTimer.timeSinceStart) {
                    midiTimer.flag = 0;
                    break;
                }
            }
            while (midiTimer.index < midifile[0].size()
                && midiTimer.flag == 0
                && midifile[0][midiTimer.index].seconds * TIMEMAGNITUDE <= midiTimer.timeSinceStart)
            {
                event = midifile[0][midiTimer.index];

                //ignore pedals when playing midi file.
                if (event[0] >= 0x80 && event[0] < 0x8f || event[0] >= 0x90 && event[0] < 0x9f)
                    keyMapper->setKeyState_PIANO((short)event[0], (short)event[1] - keyMapOffset, (short)event[2], digitalPiano->midiTimer.Channels.checkChannel((short) event[0] & 0x0f));

                midiTimer.index++;
            }
        }
        digitalPiano->reset();
        return action;
    }
public: 
    int ConstructGUI() {
        digitalPiano->connectMapper(keyMapper);
        if (digitalPiano->Construct((int32_t)_WINDOW_W, (int32_t)_WINDOW_H, 1, 1))
            digitalPiano->Start();
        return 0;
    }
    int SongCommandListener() {
        std::string selection;
        do {
            if (digitalPiano->midiTimer.isPlaying) {
                playMidi(digitalPiano->midiTimer.fileName, digitalPiano->midiTimer);
            }
            std::this_thread::sleep_for(milliseconds(10));
        } while (digitalPiano->midiTimer.flag != -1);
        return 0;
    }
    int MemoryManagementThread() {
        while (digitalPiano->midiTimer.flag != -1) {
            if (keyMapper->activeNotesPool.size() > 12) {
                tsf_note_off(soundfile, 0, keyMapper->activeNotesPool.front().keyId + keyMapOffset);
                keyMapper->threadLock.lock();
                keyMapper->activeNotesPool.pop();
                keyMapper->threadLock.unlock();
            }
            std::this_thread::sleep_for(milliseconds(1));
        }
        return 0;
    }
    int MidiInputThread() {
        RtMidiIn* midiin = new RtMidiIn();
        std::vector<unsigned char> message;
        size_t nBytes;
        unsigned int nPorts = midiin->getPortCount();

        if (nPorts == 0) {
            goto cleanup;
        }
        midiin->openPort(0);
        midiin->ignoreTypes(false, false, false);

        while (digitalPiano->midiTimer.flag != -1) {
            nBytes = message.size();
            if (nBytes > 1) {
                //144 keys 176 pedals
                keyMapper->setKeyState_PIANO((short)message[0], (short)message[1] - 21, (short)message[2], digitalPiano->midiTimer.Channels.checkChannel((short)message[0] & 0x0f));
            }
            std::this_thread::sleep_for(milliseconds(1));
        }
    cleanup:
        delete midiin;
        return 0;
    }
    int MidiFileListener() {
        while (digitalPiano->midiTimer.flag != -1) {
            const std::filesystem::path midiFiles{ "./MIDIFILES" };
            digitalPiano->midiTimer.midiLock.lock();
            digitalPiano->midiTimer.midiFileSet.clear();
            for (std::filesystem::directory_entry const & dir_entry : std::filesystem::directory_iterator{ midiFiles })
            {
                std::string fileName = dir_entry.path().generic_string();

                fileName = fileName.substr(fileName.find_last_of("/") + 1);
                if (fileName.substr(fileName.length() - 4) == ".mid" || fileName.substr(fileName.length() - 4) == ".MID") {
                    digitalPiano->midiTimer.midiFileSet.insert(fileName);
                }
            }
            digitalPiano->midiTimer.midiLock.unlock();
            std::this_thread::sleep_for(seconds(2));
        }
        return 0;
    }
};


//class NoteAnalyzer {
//public:
//    NoteAnalyzer(DigitalPiano* app) {
//        this->piano = app;
//    }
//    ~NoteAnalyzer() {
//
//    }
//public:
//    DigitalPiano * piano;
//
//};