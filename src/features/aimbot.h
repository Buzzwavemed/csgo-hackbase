#pragma once

#include "../sdk/entity.h"
#include "../utils/math.h"
#include <random>
#include <chrono>
#include <vector>

// Forward declaration of recoil control
class RecoilControl;

class Aimbot {
public:
    // Initialize the Aimbot
    static bool Initialize();
    
    // Run Aimbot loop - to be called each frame
    static void Run();
    
    // Configuration
    static void SetEnabled(bool enabled) { m_enabled = enabled; }
    static bool IsEnabled() { return m_enabled; }
    
    // Key to toggle or hold for activation
    static void SetActivationKey(int key) { m_activationKey = key; }
    static int GetActivationKey() { return m_activationKey; }
    
    // Mode: 0 = Hold key, 1 = Toggle with key, 2 = Always on
    static void SetActivationMode(int mode) { 
        m_activationMode = (mode < 0) ? 0 : ((mode > 2) ? 2 : mode); // Clamp between 0-2
    }
    static int GetActivationMode() { return m_activationMode; }
    
    // Field of view for target selection (degrees)
    static void SetFOV(float fov) { m_fov = fov > 0.0f ? fov : 1.0f; }
    static float GetFOV() { return m_fov; }
    
    // Smoothing factor (higher = smoother/slower)
    static void SetSmoothing(float smoothing) { m_smoothing = smoothing > 1.0f ? smoothing : 1.0f; }
    static float GetSmoothing() { return m_smoothing; }
    
    // Target bone for aiming
    static void SetTargetBone(int bone) { m_targetBone = bone; }
    static int GetTargetBone() { return m_targetBone; }
    
    // Target visibility check
    static void SetVisibilityCheck(bool enabled) { m_visibilityCheck = enabled; }
    static bool GetVisibilityCheck() { return m_visibilityCheck; }
    
    // Team check (don't aim at teammates)
    static void SetTeamCheck(bool enabled) { m_teamCheck = enabled; }
    static bool GetTeamCheck() { return m_teamCheck; }
    
    // Auto-fire when locked on target
    static void SetAutoFire(bool enabled) { m_autoFire = enabled; }
    static bool GetAutoFire() { return m_autoFire; }
    
    // Recoil control
    static void SetRecoilControl(bool enabled) { m_recoilControlEnabled = enabled; }
    static bool GetRecoilControl() { return m_recoilControlEnabled; }
    
    // Recoil control strength (0.0 - 2.0, 1.0 = perfect compensation)
    static void SetRecoilControlStrength(float strength) { 
        m_recoilControlStrength = (strength < 0.0f) ? 0.0f : ((strength > 2.0f) ? 2.0f : strength);
    }
    static float GetRecoilControlStrength() { return m_recoilControlStrength; }
    
    // Humanization settings
    static void SetHumanization(float amount) { 
        m_humanizationAmount = (amount < 0.0f) ? 0.0f : ((amount > 1.0f) ? 1.0f : amount);
    }
    static float GetHumanization() { return m_humanizationAmount; }
    
private:
    // Target selection and aiming
    static Player FindBestTarget();
    static float GetTargetPriority(const Player& target, const Math::Vec3& localPosition);
    static Math::Vec3 CalculateAimAngle(const Player& target);
    static Math::Vec3 SmoothAimAngle(const Math::Vec3& currentAngle, const Math::Vec3& targetAngle);
    static bool IsTargetInFOV(const Math::Vec3& targetAngle, const Math::Vec3& currentAngle);
    static bool IsTargetVisible(const Player& target);
    static bool IsPlayerValid(const Player& player);
    
    // Humanization methods
    static Math::Vec3 HumanizeAimAngle(const Math::Vec3& angle);
    static Math::Vec3 AddMicroMovement(const Math::Vec3& angle);
    static Math::Vec3 ApplyBezierInterpolation(const Math::Vec3& currentAngle, const Math::Vec3& targetAngle, float t);
    
    // Recoil control methods
    static Math::Vec3 CompensateRecoil(const Math::Vec3& angle);
    
    // Internal state
    static Player m_currentTarget;
    static bool m_hasTarget;
    static Math::Vec3 m_lastAimAngle;
    static float m_aimProgress; // 0.0 - 1.0, how far along the aim path we are
    
    // Random number generation for humanization
    static std::mt19937 m_randomGenerator;
    static std::uniform_real_distribution<float> m_uniformDist;
    
    // Configuration
    static bool m_enabled;
    static bool m_isActive;
    static int m_activationKey;
    static int m_activationMode;
    static float m_fov;
    static float m_smoothing;
    static int m_targetBone;
    static bool m_visibilityCheck;
    static bool m_teamCheck;
    static bool m_autoFire;
    static bool m_recoilControlEnabled;
    static float m_recoilControlStrength;
    static float m_humanizationAmount;
    
    // Timing variables for smooth aiming
    static std::chrono::time_point<std::chrono::high_resolution_clock> m_aimStartTime;
    static float m_aimDuration;
};

// Bone definitions for target selection
namespace Bones {
    constexpr int HEAD = 8;
    constexpr int NECK = 7;
    constexpr int UPPER_CHEST = 6;
    constexpr int MIDDLE_CHEST = 5;
    constexpr int LOWER_CHEST = 4;
    constexpr int STOMACH = 3;
    constexpr int PELVIS = 0;
}
