#pragma once

namespace Engine
{
    struct vec2 {
        float x, y;

        vec2() : x(0), y(0) {}
        vec2(float t): x(t), y(t) {}
        vec2(float x, float y) : x(x), y(y) {}
        vec2(const vec2& v) : x(v.x), y(v.y) {}

        vec2 operator+(float t) const { return vec2(x + t, y + t); }
        vec2 operator+(const vec2& v) const { return vec2(x + v.x, y + v.y); }
        vec2 operator-(const vec2& v) const { return vec2(x - v.x, y - v.y); }
        vec2 operator-(float t) const { return vec2(x - t, y - t); }
        vec2 operator*(float t) const { return vec2(x * t, y * t); }
        vec2& operator*=(float t) { x *= t; y *= t; return *this; }
        vec2 operator/(const vec2& t) { return vec2(x / t.x, y / t.y); }
        vec2 operator*(int t) const { return vec2(x * t, y * t); }
        bool operator!=(const vec2& v) { return !((x == v.x) && (y == v.y)); }

    };

    struct vec3;

    struct vec4
    {
        float x, y, z, w;

        vec4() : x(0), y(0), z(0), w(0) {}
        vec4(float t) : x(t), y(t), z(t), w(t) {}
        vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
        vec4(const vec3& v, float w);

        float length() const;

        float length_squared() const;
        vec4 normalized() const;

        vec4 operator+(const vec4& v) const;
        vec4 operator-(const vec4& v) const;
        vec4 operator*(const vec4& v) const;
        vec4 operator*(float t) const;
        vec4 operator/(float t) const;
        vec4 operator-() const;
    };

    struct vec3 {
        union { float x, r, s; };
        union { float y, g, t; };
        union { float z, b, p; };


        vec3() : x(0), y(0), z(0) {}
        vec3(float t) : x(t), y(t), z(t) {}
        vec3(float x, float y, float z) : x(x), y(y), z(z) {}
        vec3(const vec3& v) : x(v.x), y(v.y), z(v.z) {}
        vec3(const vec4& v) : x(v.x), y(v.y), z(v.z) {}

        float length() const;
       
        float length_squared() const;
        vec3 normalized() const;
        vec3 minimum(const vec3& other);
        vec3 maximum(const vec3& other);

        const float& operator [] (size_t i) const { return (&x)[i]; }
        float& operator[](int index) { return (&x)[index]; }
        vec3& operator=(const vec4& v);
        vec3 operator+(const vec3& v) const;
        vec3 operator*(const vec3& v) const;
        vec3& operator+=(const vec3& v);
        vec3 operator-(const vec3& v) const;
        vec3& operator=(const vec3& v);
        bool operator==(const vec3& v) const;
        bool operator==(float v) const;
        bool operator!=(const vec3& v) const;
        bool operator!=(float v) const;
        vec3 operator*(float t) const;
        vec3& operator*=(float t);
        vec3 operator/(float t) const;
        vec3 operator-() const;
    };

    
}