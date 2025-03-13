#pragma once

#include <cmath>
#include <random>
#include <chrono>
#include <Windows.h>

#define M_PI 3.14159265358979323846f
#define DEG2RAD(x) ((x) * M_PI / 180.0f)
#define RAD2DEG(x) ((x) * 180.0f / M_PI)

namespace Math {
    // 3D Vector class
    class Vec3 {
    public:
        float x, y, z;

        // Constructors
        Vec3() : x(0), y(0), z(0) {}
        Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
        Vec3(const float* v) : x(v[0]), y(v[1]), z(v[2]) {}

        // Arithmetic operators
        Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
        Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
        Vec3 operator*(float fl) const { return Vec3(x * fl, y * fl, z * fl); }
        Vec3 operator/(float fl) const { return Vec3(x / fl, y / fl, z / fl); }
        
        // Assignment operators
        Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
        Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
        Vec3& operator*=(float fl) { x *= fl; y *= fl; z *= fl; return *this; }
        Vec3& operator/=(float fl) { x /= fl; y /= fl; z /= fl; return *this; }
        
        // Comparison
        bool operator==(const Vec3& v) const { return (x == v.x && y == v.y && z == v.z); }
        bool operator!=(const Vec3& v) const { return (x != v.x || y != v.y || z != v.z); }
        
        // Vector operations
        float Length() const { return sqrt(x * x + y * y + z * z); }
        float LengthSqr() const { return (x * x + y * y + z * z); }
        float Length2D() const { return sqrt(x * x + y * y); }
        float Dot(const Vec3& v) const { return (x * v.x + y * v.y + z * v.z); }
        float DistTo(const Vec3& v) const { return (*this - v).Length(); }
        
        Vec3 Cross(const Vec3& v) const {
            return Vec3(
                y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x
            );
        }
        
        void Normalize() {
            float length = Length();
            if (length != 0.0f) {
                x /= length;
                y /= length;
                z /= length;
            }
        }
        
        Vec3 Normalized() const {
            Vec3 vec = *this;
            vec.Normalize();
            return vec;
        }
        
        // Specific accessors for angles
        float& Pitch() { return x; }
        float& Yaw() { return y; }
        float& Roll() { return z; }
        
        const float& Pitch() const { return x; }
        const float& Yaw() const { return y; }
        const float& Roll() const { return z; }
        
        // Reset to zero
        void Reset() { x = y = z = 0.0f; }
        
        // Array access
        float& operator[](int i) { return ((float*)this)[i]; }
        float operator[](int i) const { return ((float*)this)[i]; }
    };

    // 2D Vector
    class Vec2 {
    public:
        float x, y;

        Vec2() : x(0), y(0) {}
        Vec2(float x, float y) : x(x), y(y) {}
        Vec2(const float* v) : x(v[0]), y(v[1]) {}
        
        // Arithmetic operators
        Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
        Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
        Vec2 operator*(float fl) const { return Vec2(x * fl, y * fl); }
        Vec2 operator/(float fl) const { return Vec2(x / fl, y / fl); }
        
        // Vector operations
        float Length() const { return sqrt(x * x + y * y); }
        float LengthSqr() const { return (x * x + y * y); }
        float Dot(const Vec2& v) const { return (x * v.x + y * v.y); }
        float DistTo(const Vec2& v) const { return (*this - v).Length(); }
        
        void Normalize() {
            float length = Length();
            if (length != 0.0f) {
                x /= length;
                y /= length;
            }
        }
    };

    // 4x4 Matrix for world transformations
    class Matrix4x4 {
    public:
        float m[4][4];
        
        Matrix4x4() {
            memset(m, 0, sizeof(m));
        }
        
        // Get row/column values
        float* operator[](int i) { return m[i]; }
        const float* operator[](int i) const { return m[i]; }
        
        // Transform a point by this matrix
        Vec3 Transform(const Vec3& vec) const {
            return Vec3(
                m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z + m[0][3],
                m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z + m[1][3],
                m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z + m[2][3]
            );
        }
    };

    // Convert world coordinates to screen coordinates
    inline bool WorldToScreen(const Vec3& origin, Vec2& screen, const Matrix4x4& viewMatrix, int screenWidth, int screenHeight) {
        // Get the transform coordinates
        float w = viewMatrix[3][0] * origin.x + viewMatrix[3][1] * origin.y + viewMatrix[3][2] * origin.z + viewMatrix[3][3];
        
        // Behind the camera
        if (w < 0.01f)
            return false;
        
        // Calculate screen coordinates
        float inverseW = 1.0f / w;
        screen.x = (viewMatrix[0][0] * origin.x + viewMatrix[0][1] * origin.y + viewMatrix[0][2] * origin.z + viewMatrix[0][3]) * inverseW;
        screen.y = (viewMatrix[1][0] * origin.x + viewMatrix[1][1] * origin.y + viewMatrix[1][2] * origin.z + viewMatrix[1][3]) * inverseW;
        
        // Map to screen
        float x = screenWidth / 2.0f;
        float y = screenHeight / 2.0f;
        
        screen.x = x + (screen.x * x);
        screen.y = y - (screen.y * y);
        
        return true;
    }

    // Calculate view angles from start position to target position
    inline Vec3 CalcAngle(const Vec3& src, const Vec3& dst) {
        Vec3 angles;
        Vec3 delta = src - dst;
        
        float hypotenuse = delta.Length2D();
        angles.y = RAD2DEG(atan2f(delta.y, delta.x));
        angles.x = RAD2DEG(atan2f(delta.z, hypotenuse));
        angles.z = 0.0f;
        
        return angles;
    }

    // Normalize angles to be within -180 to 180 degrees
    inline void ClampAngles(Vec3& angles) {
        if (angles.x > 89.0f) angles.x = 89.0f;
        if (angles.x < -89.0f) angles.x = -89.0f;
        
        while (angles.y > 180.0f) angles.y -= 360.0f;
        while (angles.y < -180.0f) angles.y += 360.0f;
        
        angles.z = 0.0f;
    }

    // Calculate the angle difference between two angles
    inline float AngleDifference(float angle1, float angle2) {
        float diff = angle1 - angle2;
        if (diff > 180.0f) diff -= 360.0f;
        if (diff < -180.0f) diff += 360.0f;
        return diff;
    }

    // Smoothly interpolate between two angles for humanization
    inline Vec3 SmoothAim(const Vec3& src, const Vec3& dst, float smoothAmount) {
        Vec3 smoothed;
        smoothed.x = src.x + (dst.x - src.x) / smoothAmount;
        smoothed.y = src.y + (dst.y - src.y) / smoothAmount;
        smoothed.z = 0.0f;
        return smoothed;
    }

    // Apply a bezier curve to create more natural aim movement
    inline float BezierSmooth(float t, float smoothFactor) {
        // Cubic bezier curve: slower at start, faster in middle, slower at end
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;
        
        // Bezier curve with control points adjusted by smoothFactor
        float result = uuu + 3.0f * uu * t * smoothFactor + 3.0f * u * tt * (1.0f - smoothFactor) + ttt;
        return result;
    }

    // Humanization utilities
    class Humanization {
    private:
        static std::mt19937 s_generator;
        static bool s_initialized;
        
    public:
        // Initialize randomization
        static void Initialize() {
            if (!s_initialized) {
                s_generator.seed(static_cast<unsigned int>(
                    std::chrono::high_resolution_clock::now().time_since_epoch().count()));
                s_initialized = true;
            }
        }
        
        // Get a random value between min and max
        static float GetRandomFloat(float min, float max) {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(s_generator);
        }
        
        // Add a small amount of randomness to an angle for human-like imperfection
        static Vec3 AddRandomness(const Vec3& angles, float maxRandomness) {
            Vec3 result = angles;
            result.x += GetRandomFloat(-maxRandomness, maxRandomness);
            result.y += GetRandomFloat(-maxRandomness, maxRandomness);
            return result;
        }
        
        // Simulate the human reaction delay with some variability
        static void SimulateReactionDelay(float baseDelayMs, float variabilityMs) {
            float delay = baseDelayMs + GetRandomFloat(-variabilityMs, variabilityMs);
            if (delay > 0.0f) {
                Sleep(static_cast<DWORD>(delay));
            }
        }
        
        // Determine a smooth factor based on distance for more realistic aiming
        static float DistanceBasedSmoothFactor(float distance, float minSmooth, float maxSmooth, float distanceFactor) {
            // For closer targets, have less smoothing (faster aim)
            // For farther targets, have more smoothing (slower aim)
            float smoothFactor = minSmooth + (maxSmooth - minSmooth) * (1.0f - exp(-distance / distanceFactor));
            return smoothFactor;
        }
    };
};

// Initialize static members of Humanization
inline std::mt19937 Math::Humanization::s_generator;
inline bool Math::Humanization::s_initialized = false;
