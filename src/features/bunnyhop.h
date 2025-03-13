#pragma once

#include "../sdk/entity.h"
#include <random>

class BunnyHop {
public:
    // Initialize the BunnyHop
    static bool Initialize();
    
    // Run BunnyHop loop - to be called each frame
    static void Run();
    
    // Configuration
    static void SetEnabled(bool enabled) { m_enabled = enabled; }
    static bool IsEnabled() { return m_enabled; }
    
    // Success rate randomization for humanization
    static void SetSuccessRate(int percentage) { m_successRate = std::clamp(percentage, 0, 100); }
    static int GetSuccessRate() { return m_successRate; }
    
    // Perfect jump timing offset (ms)
    static void SetTimingOffset(int maxOffsetMs) { m_maxTimingOffset = maxOffsetMs; }
    static int GetTimingOffset() { return m_maxTimingOffset; }
    
    // Configure auto-strafe (left-right movement while jumping)
    static void SetAutoStrafe(bool enabled) { m_autoStrafe = enabled; }
    static bool IsAutoStrafeEnabled() { return m_autoStrafe; }

private:
    // Internal methods
    static bool ShouldJump();
    static void AutoStrafe();
    
    // Randomization for humanization
    static std::mt19937 m_randomGenerator;
    static std::uniform_int_distribution<int> m_successDistribution;
    static std::uniform_int_distribution<int> m_timingDistribution;
    
    // Cached variables
    static int m_successRate;          // 0-100% success rate (for humanization)
    static int m_maxTimingOffset;      // Max timing offset in ms (for humanization)
    static bool m_autoStrafe;          // Auto-strafe left/right while jumping
    static bool m_enabled;             // BunnyHop enabled/disabled
    static int m_strafeDir;            // Current strafe direction (-1 left, 1 right)
    static int m_lastJumpTime;         // For timing offset randomization
};
