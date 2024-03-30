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
        vec2 operator-(float t) const { return vec2(x - t, y - t); }
        vec2 operator*(float t) const { return vec2(x * t, y * t); }

    };

    struct vec3 {
        union { float x, r, s; };
        union { float y, g, t; };
        union { float z, b, p; };


        vec3() : x(0), y(0), z(0) {}
        vec3(float t) : x(t), y(t), z(t) {}
        vec3(float x, float y, float z) : x(x), y(y), z(z) {}
        vec3(const vec3& v) : x(v.x), y(v.y), z(v.z) {}

        // Method declarations
        float length() const;
       
        float length_squared() const;
        vec3 normalized() const;

        // Overloaded operators
        vec3 operator+(const vec3& v) const;
        vec3& operator+=(const vec3& v);
        vec3 operator-(const vec3& v) const;
        vec3& operator=(const vec3& v);
        vec3 operator*(float t) const;
        vec3 operator/(float t) const;
        vec3 operator-() const;
    };
}