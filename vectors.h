#pragma once
#include "olcPixelGameEngineGL.h"
#include "tsf.h"
#include "minisdl_audio.h"
#include "MidiFile.h"
#include "RtMidi.h"
#include "Options.h"
#include "DigitalPiano.h"
#include "MAPPER.h"
#include "Keys.h"
#include <iostream>
#include <chrono>
#include <signal.h>
#include <thread>
#include <map>
#include <unordered_map>
#include <queue>
#include <mutex>

template <class TYPE>
struct vector2d_generic {
    TYPE x;
    TYPE y;
    vector2d_generic(TYPE x, TYPE y, TYPE z) {
        this->x = x;
        this->y = y;
    }
    vector2d_generic() {
        this->x = 0;
        this->y = 0;
    }
    ~vector2d_generic() {}
    vector2d_generic operator + (vector2d_generic const& obj) {
        vector2d_generic result;
        result.x = x + obj.x;
        result.y = y + obj.y;
        return result;
    }
    vector2d_generic operator - (vector2d_generic const& obj) {
        vector2d_generic result;
        result.x = x - obj.x;
        result.y = y - obj.y;
        return result;
    }
    vector2d_generic operator * (vector2d_generic const& obj) {
        vector2d_generic result;
        result.x = x * obj.x;
        result.y = y * obj.y;
        return result;
    }
    vector2d_generic operator / (vector2d_generic const& obj) {
        vector2d_generic result;
        result.x = x / obj.x;
        result.y = y / obj.y;
        return result;
    }
};

template <class TYPE>
struct vector3d {
    TYPE x;
    TYPE y;
    vector3d(TYPE x, TYPE y, TYPE z) {
        this->x = x;
        this->y = y;
    }
    vector3d() {
        this->x = 0;
        this->y = 0;
    }
    ~vector3d() {}
    vector3d operator + (vector3d const& obj) {
        vector3d result;
        result.x = x + obj.x;
        result.y = y + obj.y;
        return result;
    }
    vector3d operator - (vector3d const& obj) {
        vector3d result;
        result.x = x - obj.x;
        result.y = y - obj.y;
        return result;
    }
    vector3d operator * (vector3d const& obj) {
        vector3d result;
        result.x = x * obj.x;
        result.y = y * obj.y;
        return result;
    }
    vector3d operator / (vector3d const& obj) {
        vector3d result;
        result.x = x / obj.x;
        result.y = y / obj.y;
        return result;
    }
};

struct vector3f {
    float x, y, z;
    vector3f(float F) {
        this->x = F;
        this->y = F;
        this->z = F;
    }
    vector3f(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    vector3f() {
        this->x = 0;
        this->y = 0;
        this->z = 0;
    }

    ~vector3f() {}
    void setAll(float xyz) {
        this->x = xyz;
        this->y = xyz;
        this->z = xyz;
    }
};

struct vector3i {
    int x, y, z;
    vector3i(int x, int y, int z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    vector3i() {
        this->x = 0;
        this->y = 0;
        this->z = 0;
    }
    ~vector3i() {}
    vector3i operator + (vector3i const& obj) {
        vector3i result;
        result.x = x + obj.x;
        result.y = y + obj.y;
        result.z = z + obj.z;
        return result;
    }
    vector3i operator - (vector3i const& obj) {
        vector3i result;
        result.x = x - obj.x;
        result.y = y - obj.y;
        result.z = z - obj.z;
        return result;
    }
    vector3i operator * (vector3f const& obj) {
        vector3i result;
        result.x = x * (float)obj.x;
        result.y = y * (float)obj.y;
        result.z = z * (float)obj.z;
        return result;
    }
    vector3i operator * (vector3i const& obj) {
        vector3i result;
        result.x = x * obj.x;
        result.y = y * obj.y;
        result.z = z * obj.z;
        return result;
    }
    vector3i operator / (vector3i const& obj) {
        vector3i result;
        result.x = x / obj.x;
        result.y = y / obj.y;
        result.z = z / obj.z;
        return result;
    }
};

struct vector4i {
    int x, y, z, a;
    vector4i(int x, int y, int z, int a) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->a = a;
    }
    vector4i() {
        this->x = 0;
        this->y = 0;
        this->z = 0;
    }
    ~vector4i() {}
    vector4i operator + (vector4i const& obj) {
        vector4i result;
        result.x = x + obj.x;
        result.y = y + obj.y;
        result.z = z + obj.z;
        return result;
    }
    vector4i operator - (vector4i const& obj) {
        vector4i result;
        result.x = x - obj.x;
        result.y = y - obj.y;
        result.z = z - obj.z;
        return result;
    }
    vector4i operator * (vector3f const& obj) {
        vector4i result;
        result.x = x * (float)obj.x;
        result.y = y * (float)obj.y;
        result.z = z * (float)obj.z;
        return result;
    }
    vector4i operator * (vector4i const& obj) {
        vector4i result;
        result.x = x * obj.x;
        result.y = y * obj.y;
        result.z = z * obj.z;
        return result;
    }
    vector4i operator / (vector4i const& obj) {
        vector4i result;
        result.x = x / obj.x;
        result.y = y / obj.y;
        result.z = z / obj.z;
        return result;
    }
};