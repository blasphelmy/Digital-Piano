#pragma once

template<class TYPE>
struct vector3d {
    TYPE x, y, z;
    vector3d            ()                          { this->x = 0; this->y = 0; this->z = 0; }
    vector3d            (TYPE xyz)                  { this->x = xyz; this->y = xyz; this->z = xyz; }
    vector3d            (TYPE x, TYPE y, TYPE z)    { this->x = x; this->y = y; this->z = z; }

    void setAll         (TYPE xyz)                  { this->x = xyz; this->y = xyz; this->z = xyz; }

    vector3d operator + (vector3d const& obj)       { vector3d result; result.x = x + obj.x; result.y = y + obj.y; result.z = z + obj.z; return result; }
    vector3d operator - (vector3d const& obj)       { vector3d result; result.x = x - obj.x; result.y = y - obj.y; result.z = z - obj.z; return result; }
    vector3d operator * (vector3d const& obj)       { vector3d result; result.x = x * obj.x; result.y = y * obj.y; result.z = z * obj.z; return result; }
    vector3d operator / (vector3d const& obj)       { vector3d result; result.x = x / obj.x; result.y = y / obj.y; result.z = z / obj.z; return result; }

    template<class TYPE> 
    vector3d<TYPE> cast_to() { return vector3d<TYPE>{ static_cast<TYPE>(x), static_cast<TYPE>(y), static_cast<TYPE>(z) }; };
};

typedef vector3d<int>         vector3i;
typedef vector3d<float>       vector3f;
typedef vector3d<long double> vector3ld;
typedef vector3d<uint16_t>    vector3i16t;
typedef vector3d<uint8_t>     vector3di8t;