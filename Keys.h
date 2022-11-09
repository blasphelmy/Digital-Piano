#pragma once
#include "vectors.h"

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
        this->name       = Key.name;
        this->position   = Key.position;
        this->position.y = _KEYSIZE + (_KEYSIZE * .02f);
        this->size       = Key.size;
        this->size.y     = -1;
        this->channel    = (char)channel;
        this->isWhite    = Key.isWhite;
    }
public:
    vector3i    color;
    double      duration = 0.0f;
};