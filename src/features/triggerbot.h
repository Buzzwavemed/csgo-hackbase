#pragma once

#include "../sdk/entity.h"
#include <random>
#include <chrono>
#include <algorithm>

class TriggerBot {
public:
    // Initialize the TriggerBot
    static bool Initialize();
    
    // Run TriggerBot loop - to be called each frame
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
    
    // Reaction delay - time between target detection and shooting (ms)
    static void SetReactionDelay(int minDelayMs, int maxDelayMs) { 
        m_minReactionDelay = minDelayMs; 
        m_maxReactionDelay = (m_minReactionDelay > maxDelayMs) ? m_minReactionDelay : maxDelayMs; // Manual max
        m_reactionDistribution = std::uniform_int_distribution<int>(m_minReactionDelay, m_maxReactionDelay);
    }
    static int GetMinReactionDelay() { return m_minReactionDelay; }
    static int GetMaxReactionDelay() { return m_maxReactionDelay; }
    
    // Target filtering
    static void SetTargetTeammates(bool enabled) { m_targetTeammates = enabled; }
    static bool ShouldTargetTeammates() { return m_targetTeammates; }
    
    // Target health check (only shoot at players with health > 0)
    static void SetCheckTargetHealth(bool enabled) { m_checkTargetHealth = enabled; }
    static bool ShouldCheckTargetHealth() { return m_checkTargetHealth; }
    
    // Only trigger when target visible (not through walls)
    static void SetCheckVisibility(bool enabled) { m_checkVisibility = enabled; }
    static bool ShouldCheckVisibility() { return m_checkVisibility; }
    
    // Toggle active status (when using toggle mode)
    static void ToggleActive() { m_isActive = !m_isActive; }
    static bool IsActive() { return m_isActive; }

private:
    // Internal methods
    static bool ShouldFire(const Player& target);
    static bool IsTargetValid(const Player& target, const Player& localPlayer);
    static bool IsVisible(const Player& target, const Player& localPlayer);
    static void GenerateReactionDelay();
    
    // Randomization for humanization
    static std::mt19937 m_randomGenerator;
    static std::uniform_int_distribution<int> m_reactionDistribution;
    
    // Cached variables
    static bool m_enabled;               // TriggerBot enabled/disabled
    static bool m_isActive;              // Active status (for toggle mode)
    static bool m_isWaitingForDelay;     // Currently waiting for reaction delay
    static bool m_targetTeammates;       // Target teammates or only enemies
    static bool m_checkTargetHealth;     // Check if target is alive
    static bool m_checkVisibility;       // Check if target is visible (not through walls)
    static int m_activationKey;          // Key to activate triggerbot
    static int m_activationMode;         // 0 = Hold, 1 = Toggle, 2 = Always on
    static int m_minReactionDelay;       // Minimum reaction time (ms)
    static int m_maxReactionDelay;       // Maximum reaction time (ms)
    static int m_currentDelay;           // Current reaction delay (ms)
    static std::chrono::time_point<std::chrono::high_resolution_clock> m_targetAcquiredTime; // Time when target was acquired
};
