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

template<class TYPE>
struct vector4d {
    TYPE x, y, z, a;
    vector4d() { this->x = 0;   this->y = 0;   this->z = 0; this->a = 0; }
    vector4d(TYPE xyz) { this->x = xyz; this->y = xyz; this->z = xyz; }
    vector4d(TYPE x, TYPE y, TYPE z, TYPE a) { this->x = x;   this->y = y;   this->z = z; this->a = a; }
    void setAll(TYPE xyz) { this->x = xyz; this->y = xyz; this->z = xyz; }

    vector4d operator + (vector4d const& obj) { vector4d result; result.x = x + obj.x; result.y = y + obj.y; result.z = z + obj.z; return result; }
    vector4d operator - (vector4d const& obj) { vector4d result; result.x = x - obj.x; result.y = y - obj.y; result.z = z - obj.z; return result; }
    vector4d operator * (vector4d const& obj) { vector4d result; result.x = x * obj.x; result.y = y * obj.y; result.z = z * obj.z; return result; }
    vector4d operator / (vector4d const& obj) { vector4d result; result.x = x / obj.x; result.y = y / obj.y; result.z = z / obj.z; return result; }

    vector4d operator *= (vector4d const& rhs) { this->x *= rhs.x; this->y *= rhs.y; this->z *= rhs.y; this->a *= rhs.a; }
    vector4d operator /= (vector4d const& rhs) { this->x /= rhs.x; this->y /= rhs.y; this->z /= rhs.y; this->a /= rhs.a; }
    vector4d operator -= (vector4d const& rhs) { this->x -= rhs.x; this->y -= rhs.y; this->z -= rhs.y; this->a -= rhs.a; }
    vector4d operator += (vector4d const& rhs) { this->x += rhs.x; this->y += rhs.y; this->z += rhs.y; this->a += rhs.a; }

    vector4d& operator += (TYPE const& rhs) { this->x += rhs; this->y += rhs; this->z += rhs; this->a += rhs; return *this; }
    vector4d& operator -= (TYPE const& rhs) { this->x -= rhs; this->y -= rhs; this->z -= rhs; this->a -= rhs; return *this; }
    vector4d& operator *= (TYPE const& rhs) { this->x *= rhs; this->y *= rhs; this->z *= rhs; this->a *= rhs; return *this; }
    vector4d& operator /= (TYPE const& rhs) { this->x /= rhs; this->y /= rhs; this->z /= rhs; this->a /= rhs; return *this; }


    bool operator == (vector4d const& rhs) { return this->x == rhs.x && this->y == rhs.y && this->y == rhs.y && this->a == rhs.a ? true : false; }
    bool operator != (vector4d const& rhs) { return this->x == rhs.x && this->y == rhs.y && this->y == rhs.y && this->a == rhs.a ? false : true; }
    bool operator <= (vector4d const& rhs) { return this->x <= rhs.x && this->y <= rhs.y && this->y <= rhs.y && this->a <= rhs.a ? true : false; }
    bool operator >= (vector4d const& rhs) { return this->x >= rhs.x && this->y >= rhs.y && this->y >= rhs.y && this->a >= rhs.a ? true : false; }
    bool operator < (vector4d const& rhs) { return this->x < rhs.x&& this->y < rhs.y&& this->y < rhs.y&& this->a < rhs.a ? true : false; }
    bool operator > (vector4d const& rhs) { return this->x > rhs.x && this->y > rhs.y && this->y > rhs.y && this->a > rhs.a ? true : false; }

    operator vector4d<int8_t>() const { return { static_cast<int8_t>(this->x), static_cast<int8_t>(this->y), static_cast<int8_t>(this->z), static_cast<int8_t>(this->a) }; }
    operator vector4d<int16_t>() const { return { static_cast<int16_t>(this->x), static_cast<int16_t>(this->y), static_cast<int16_t>(this->z), static_cast<int16_t>(this->a) }; }
    operator vector4d<int64_t>() const { return { static_cast<int64_t>(this->x), static_cast<int64_t>(this->y), static_cast<int64_t>(this->z), static_cast<int64_t>(this->a) }; }
    operator vector4d<float>() const { return { static_cast<float>(this->x), static_cast<float>(this->y), static_cast<float>(this->z), static_cast<float>(this->a) }; }
    operator vector4d<double>() const { return { static_cast<double>(this->x), static_cast<double>(this->y), static_cast<double>(this->z), static_cast<double>(this->a) }; }
    operator vector4d<long double>() const { return { static_cast<long double>(this->x), static_cast<long double>(this->y), static_cast<long double>(this->z), static_cast<long double>(this->a) }; }

    template<class TYPE>
    vector4d<TYPE> cast_to() { return vector4d<TYPE>{ static_cast<TYPE>(x), static_cast<TYPE>(y), static_cast<TYPE>(z), static_cast<TYPE>(a) }; };
};
typedef vector4d<int>         vector4i;
typedef vector4d<float>       vector4f;
typedef vector4d<double>      vector4db;
typedef vector4d<long double> vector4ld;
typedef vector4d<uint16_t>    vector4i16t;
typedef vector4d<uint8_t>     vector4di8t;