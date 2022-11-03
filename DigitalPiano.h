#pragma once
#include "GLOBALVARIABLES.h"
#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "minisdl_audio.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "Options.h"
#include "MAPPER.h"
#include "Keys.h"
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

struct channel { 
    bool MUTED = false; bool ACTIVE = false; char channelNum; 
    int drawSelf(olc::PixelGameEngine* parent, MAPPER * keyMapper, int index) { 
        if (ACTIVE) { 
            olc::vi2d pos(_WINDOW_W - 110, 30 + (index++ * 20));
            olc::vi2d bounds(110, 13);
            olc::vi2d mousePos = parent->GetMousePos();
            olc::Pixel color = !MUTED ? olc::GREEN : olc::RED;
            //parent->DrawRect(pos, bounds, olc::WHITE);
            if (mousePos.x > pos.x && mousePos.y > pos.y && mousePos.y < pos.y + 13) {
                if (parent->GetMouse(olc::Mouse::LEFT).bPressed) {
                    MUTED = MUTED ? false : true;
                    /*keyMapper->activelyDrawing.clear();
                    for (int j = 0; j < 88; j++) {
                        keyMapper->keyMap[j].velocity = 0;
                    }*/
                }
                color = olc::CYAN;
            }
            parent->DrawString(pos.x, pos.y, "Channel : " + std::to_string(channelNum), color, _TEXT_SCALE);
        }
        return index; 
    };
};

class Channels {
public:
    Channels() {
        for (int i = 0; i < 16; i++) {
            channels[i].channelNum = i;
        }
    };
public:
    std::array<channel, 16> channels;
private:
    short mask = 0x0f;
    bool checkRange(short maskedChannel) { return (short)maskedChannel >= 0x00 && (short)maskedChannel <= 0x0f; }
public:
    bool checkChannel(smf::MidiEvent & event) {
        short maskedChannel = (short)event[0] & mask;
        if (channels[(short)maskedChannel].ACTIVE && !channels[(short)maskedChannel].MUTED) return true;
        return false;
    }
    bool checkChannel(short channel) {
        if (channels[(short)channel].ACTIVE && !channels[(short)channel].MUTED) return true;
        return false;
    }
    bool setChannels(smf::MidiFile& file) {
        for (int i = 0; i < file[0].size(); i++) {
            if (!checkChannel(file[0][i])) {
                short maskedChannel = (short)file[0][i][0] & mask;
                if(checkRange((short) maskedChannel)) channels[maskedChannel].ACTIVE = true;
            }
        }
        return true;
    }
    void resetChannels() {
        for (int i = 0; i < 16; i++) {
             channels[i].ACTIVE = channels[i].MUTED = false;
        }
    }
};

class MidiTimer {
public:
    //chrono library is so verbose.
    high_resolution_clock::time_point   start;
    high_resolution_clock::time_point   finish;
    std::set<std::string>               midiFileSet;
    Channels                            Channels;
    int flag                 = 0;
    int index                = 0;
    long long timeSinceStart = 0.0;
    float speed              = 1.0;
    float qNotePerSec        = 1.5f;
    float duration           = 0.f;
    std::string fileName     = "";
    bool isPlaying           = false;
    std::mutex               midiLock;
    float targetBPM          = 1.5f;
    void tick() {
        Sleep(1);
        if (speed > 3) speed = 3.0f;
        if (speed < 0) speed = 0.0f;
        this->finish = high_resolution_clock::now();
        this->timeSinceStart = (this->timeSinceStart
            + (duration_cast<milliseconds>
                (this->finish - this->start).count() * this->speed)
            );
        this->start = high_resolution_clock::now();
    }
};

class PIXELGAMEENGINE_EXT : public olc::PixelGameEngine {
public:
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
private: 
    bool OnUserCreate() override {
        colorMap["C"] = vector3i(254, 0, 0);
        colorMap["D"] = vector3i(45, 122, 142);
        colorMap["E"] = vector3i(251, 133, 39);
        colorMap["F"] = vector3i(146, 209, 79);
        colorMap["G"] = vector3i(255, 255, 113);
        colorMap["A"] = vector3i(107, 60, 200);
        colorMap["B"] = vector3i(209, 79, 205);
        return true;
    }
private:
    MAPPER* keyMapper = nullptr;
public:
    MAPPER* returnMapper() { return keyMapper; };
    void connectMapper(MAPPER* newMapper) {
        keyMapper = newMapper;
    }
public:
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
    struct horizontalLine {
        float y = _KEYSIZE; float left = 0.f; float right = _WINDOW_W;
        bool drawSelf(olc::PixelGameEngine* parent, double yOffSet) {
            parent->DrawLine(olc::vd2d(this->left, this->y), olc::vd2d(this->right, this->y), olc::Pixel(50, 50, 50));
            this->y -= yOffSet;
            if (this->y > 0) return true;
            return false;
        }
    };
private:
    std::unordered_map<std::string, vector3i> colorMap;
    std::queue<horizontalLine>                scrollingLines;
    ProgressBar                               progressBar;
    float                                     timeAccumalator = 500.f;
    float                                     targetBPM = 1.5f;
public:
    void playSignal(smf::MidiFile & midifile) {
        midiTimer.qNotePerSec       = midifile.getFileDurationInSeconds()
                                    / midifile.getFileDurationInTicks()
                                    * midifile.getTicksPerQuarterNote();

        midiTimer.duration          = midifile.getFileDurationInSeconds();
        midiTimer.fileName          = midifile.getFilename();

        midiTimer.timeSinceStart    = 0.0;
        timeAccumalator             = 0.f;
        keyMapper->pedal            = true;
        
        if (midiTimer.qNotePerSec < .5) midiTimer.qNotePerSec = .5;

        midiTimer.index             = 0;
        midiTimer.start             = high_resolution_clock::now();
    }
    void reset() {
        tsf_note_off_all            (keyMapper->soundFile);
        midiTimer.isPlaying         = false;
        midiTimer.timeSinceStart    = 0.0;
        midiTimer.Channels          .resetChannels();
        keyMapper                   ->flushActiveNotes();
    }
private:
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
        for (int i = 0; i < keyMapper->keyMap.size(); i++) {
            keyMapper->keyMap[i].velocity = 0;
            if (keyMapper->activelyDrawing.count(i) > 0) keyMapper->activelyDrawing.erase(i);
        }
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
        FillRectDecal(olc::vd2d(0.f, 0.f), olc::vi2d(mousePOS.x, 20.f), olc::Pixel(45, 122, 142, 150));
        progressBar.progressBar = olc::vd2d(mousePOS.x, 20.f);
    }
    void fillInSeekRoutine() {
        olc::vd2d mousePOS = GetMousePos();
        long long newTimeSinceStart = (mousePOS.x / _WINDOW_W) * midiTimer.duration * 1000.f;
        if (newTimeSinceStart < midiTimer.timeSinceStart) {
            midiTimer.midiLock.lock();
            midiTimer.flag = 1;
            SeekRoutine(-1, (midiTimer.timeSinceStart - newTimeSinceStart));
            midiTimer.midiLock.unlock();
        }
        else {
            midiTimer.midiLock.lock();
            midiTimer.flag = 2;
            SeekRoutine(1, (newTimeSinceStart - midiTimer.timeSinceStart));
            midiTimer.midiLock.unlock();
        }

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


    void drawData() {
        DrawString(10, 30, "Hold shift then enter in an mid file name : " + midiTimer.fileName, GetKey(olc::SHIFT).bHeld && !midiTimer.isPlaying ? olc::CYAN : olc::RED, _TEXT_SCALE);
        DrawString(10, 50, "Speed (up/down) keys : x" + std::to_string(midiTimer.speed), olc::WHITE, _TEXT_SCALE);
        int i = 0;
        for (std::string fileName : midiTimer.midiFileSet) {
            DrawString(10, 70 + (i++ * 20), fileName, olc::YELLOW, _TEXT_SCALE);
        }
    }
    void drawChannels() {
        int i = 0;
        for (int j = 0; j < 16; j++) {
            i = midiTimer.Channels.channels[j].drawSelf(this, keyMapper, i);
        }
    }
    void drawFlyingNote(FlyingNotes* note) {
        olc::Pixel color = getDrawingColor(note->isWhite, note->name);
        vector3i colorVector(color.r, color.g, color.b);
        if (!midiTimer.Channels.checkChannel(note->channel)) {
            colorVector = colorVector * vector3f(.3);
        }
        FillRoundedRect(note->position, note->size - olc::vd2d(1, 0), olc::Pixel(colorVector.x, colorVector.y, colorVector.z));
        DrawString(note->position + olc::vd2d(1, 1), std::to_string(note->channel), olc::Pixel(255, 255, 255, 150));
    }
    void drawFlyingNote(FlyingNotes& note) {
        olc::Pixel color = getDrawingColor(note.isWhite, note.name);
        vector3i colorVector(color.r, color.g, color.b);
        if (!midiTimer.Channels.checkChannel(note.channel)) {
            colorVector = colorVector * vector3f(.3);
        }
        FillRoundedRect(note.position, note.size - olc::vd2d(1, 0), olc::Pixel(colorVector.x, colorVector.y, colorVector.z));
        DrawStringDecal(note.position + olc::vd2d(1, 1), std::to_string(note.channel), olc::Pixel(255, 255, 255, 150));
    }
    void drawFrame(double timeElasped) {
        keyMapper->threadLock.lock();

        double yOffSet = timeElasped * 100.f * midiTimer.speed;
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
            if (line.drawSelf(this, yOffSet)) newHorizontalLinesQueue.push(line);
        }
        scrollingLines = newHorizontalLinesQueue;

        while (!keyMapper->onScreenNoteElements.empty()) {

            FlyingNotes onscreenKey = keyMapper->onScreenNoteElements.front();
            keyMapper->onScreenNoteElements.pop();
            drawFlyingNote(onscreenKey);
            onscreenKey.position.y -= yOffSet;
            if (onscreenKey.position.y + onscreenKey.size.y > 0)
                newOnScreenElementsQueue.push(onscreenKey);
        }
        keyMapper->onScreenNoteElements = newOnScreenElementsQueue;
        for (int i = 0; i < keyMapper->keyMap.size(); i++) {
            if (i == 52) FillRect(olc::vd2d(0.f, _KEYSIZE), olc::vd2d(_WINDOW_W, 5.f), olc::Pixel(82, 38, 38));
            if (keyMapper->activelyDrawing.count(i) > 0) {
                FlyingNotes* drawnKey = keyMapper->activelyDrawing.find(i)->second;
                drawFlyingNote(drawnKey);
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
        drawChannels();
        keyMapper->threadLock.unlock();
        ProcessBarEvents();
        DrawStringDecal(olc::vd2d(10, 7), std::to_string(midiTimer.timeSinceStart / 1000.f) + "/" + std::to_string(midiTimer.duration), olc::WHITE);
    }

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
            midiTimer.speed = midiTimer.speed + .1f;
        }
        if (GetKey(olc::Key::DOWN).bPressed) {
            midiTimer.speed = midiTimer.speed - .1f;
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
    
    void ProcessBarEvents() {
        progressBar.setProgressBar(midiTimer.timeSinceStart / 1000.f, midiTimer.duration);
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
    smf::MidiFile getMidiFileRoutine(std::string& fileName) {
        smf::MidiFile newMidiFile("./MIDIFILES/" + fileName);
        newMidiFile.doTimeAnalysis();
        newMidiFile.linkNotePairs();
        newMidiFile.joinTracks(); // we only care about 1 track right now
        digitalPiano->midiTimer.Channels.setChannels(newMidiFile);
        return newMidiFile;
    }
    bool playMidi(std::string& fileName) {

        bool action = false;
        smf::MidiFile midifile = getMidiFileRoutine(fileName);
        MidiTimer& midiTimer = digitalPiano->midiTimer;
        digitalPiano->playSignal(midifile);
        smf::MidiEvent event;

        while (midiTimer.index < midifile[0].size()
            && midiTimer.flag != -1) {

            action = true;
            midiTimer.tick();
            while (midiTimer.flag == 1 && midifile[0][midiTimer.index].seconds * 1000.f >= midiTimer.timeSinceStart) {
                midiTimer.index--;
                if (midiTimer.index < 0) {
                    midiTimer.index = 0;
                    midiTimer.flag = 0;
                    break;
                }
                if (midifile[0][midiTimer.index].seconds * 1000.f < midiTimer.timeSinceStart) {
                    midiTimer.flag = 0;
                    break;
                }
            }

            while (midiTimer.flag == 2
                && midifile[0][midiTimer.index].seconds * 1000.f <= midiTimer.timeSinceStart)
            {
                midiTimer.index++;
                if (midiTimer.index > midifile[0].size() - 1) {
                    midiTimer.index = midifile[0].size() - 1;
                    midiTimer.flag = 0;
                    break;
                }
                if (midifile[0][midiTimer.index].seconds * 1000.f > midiTimer.timeSinceStart) {
                    midiTimer.flag = 0;
                    break;
                }
            }
            while (midiTimer.index < midifile[0].size()
                && midiTimer.flag == 0
                && midifile[0][midiTimer.index].seconds * 1000.f <= midiTimer.timeSinceStart)
            {
                event = midifile[0][midiTimer.index];

                //ignore pedals when playing midi file.
                if (event[0] >= 0x80 && event[0] < 0x8f || event[0] >= 0x90 && event[0] < 0x9f)
                    keyMapper->setKeyState_PIANO((int)event[0], (int)event[1] - 21, (int)event[2], digitalPiano->midiTimer.Channels.checkChannel((short) event[0] & 0x0f));

                midiTimer.index++;
            }
        }
        digitalPiano->reset();
        return action;
    }
public: 
    int ConstructGUI() {
        digitalPiano->connectMapper(keyMapper);
        if (digitalPiano->Construct(_WINDOW_W, _WINDOW_H, 1, 1))
            digitalPiano->Start();
        return 0;
    }
    int SongCommandListener() {
        std::string selection;
        do {
            if (digitalPiano->midiTimer.isPlaying) {
                playMidi(digitalPiano->midiTimer.fileName);
            }
            Sleep(10);
        } while (digitalPiano->midiTimer.flag != -1);
        return 0;
    }
    int MemoryManagementThread() {
        while (digitalPiano->midiTimer.flag != -1) {
            if (keyMapper->activeNotesPool.size() > 12) {
                keyMapper->threadLock.lock();
                tsf_note_off(soundfile, 0, keyMapper->activeNotesPool.front().keyId + 21);
                keyMapper->activeNotesPool.pop();
                keyMapper->threadLock.unlock();
            }
        }
        return 0;
    }
    int MidiInputThread() {
        RtMidiIn* midiin = new RtMidiIn();
        std::vector<unsigned char> message;
        int nBytes, i;
        double stamp;
        unsigned int nPorts = midiin->getPortCount();

        if (nPorts == 0) {
            goto cleanup;
        }
        midiin->openPort(0);
        midiin->ignoreTypes(false, false, false);

        while (digitalPiano->midiTimer.flag != -1) {
            stamp = midiin->getMessage(&message);
            nBytes = message.size();
            if (nBytes > 1) {
                //144 keys 176 pedals
                keyMapper->setKeyState_PIANO((int)message[0], (int)message[1] - 21, (int)message[2], digitalPiano->midiTimer.Channels.checkChannel((short)message[0] & 0x0f));
            }
            //Sleep(1);
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
            for (auto const& dir_entry : std::filesystem::directory_iterator{ midiFiles })
            {
                std::string fileName = dir_entry
                    .path()
                    .generic_string();

                fileName = fileName.substr(fileName.find_last_of("/") + 1);
                if (fileName.substr(fileName.length() - 4) == ".mid" || fileName.substr(fileName.length() - 4) == ".MID") {
                    digitalPiano->midiTimer.midiFileSet.insert(fileName);
                }
            }
            digitalPiano->midiTimer.midiLock.unlock();
            Sleep(1000);
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