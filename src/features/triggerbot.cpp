#include "triggerbot.h"
#include "../utils/math.h"
#include <Windows.h>

// Initialize static members
std::mt19937 TriggerBot::m_randomGenerator(std::chrono::system_clock::now().time_since_epoch().count());
std::uniform_int_distribution<int> TriggerBot::m_reactionDistribution(50, 150); // Default 50-150 ms
bool TriggerBot::m_enabled = false;
bool TriggerBot::m_isActive = false;
bool TriggerBot::m_isWaitingForDelay = false;
bool TriggerBot::m_targetTeammates = false;
bool TriggerBot::m_checkTargetHealth = true;
bool TriggerBot::m_checkVisibility = true;
int TriggerBot::m_activationKey = VK_XBUTTON1; // Default to mouse side button
int TriggerBot::m_activationMode = 0; // Default to hold key mode
int TriggerBot::m_minReactionDelay = 50;
int TriggerBot::m_maxReactionDelay = 150;
int TriggerBot::m_currentDelay = 0;
std::chrono::time_point<std::chrono::high_resolution_clock> TriggerBot::m_targetAcquiredTime;

bool TriggerBot::Initialize() {
    // Set up randomization
    m_randomGenerator.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    
    // Default settings
    m_enabled = false;
    m_isActive = false;
    m_isWaitingForDelay = false;
    m_targetTeammates = false;
    m_checkTargetHealth = true;
    m_checkVisibility = true;
    m_activationKey = VK_XBUTTON1; // Mouse side button
    m_activationMode = 0; // Hold key
    m_minReactionDelay = 50;
    m_maxReactionDelay = 150;
    
    // Initialize reaction time distribution
    m_reactionDistribution = std::uniform_int_distribution<int>(m_minReactionDelay, m_maxReactionDelay);
    
    return true;
}

void TriggerBot::Run() {
    // Skip if disabled
    if (!m_enabled)
        return;
    
    // Get local player
    Player localPlayer = Game::GetLocalPlayer();
    if (!localPlayer.IsValid() || !localPlayer.IsAlive())
        return;
    
    // Handle activation based on mode
    bool shouldBeActive = false;
    
    switch (m_activationMode) {
        case 0: // Hold key
            shouldBeActive = (GetAsyncKeyState(m_activationKey) & 0x8000) != 0;
            m_isActive = shouldBeActive;
            break;
            
        case 1: // Toggle with key
            if ((GetAsyncKeyState(m_activationKey) & 0x8000) && !(GetAsyncKeyState(m_activationKey) & 0x0001)) {
                // Key just pressed (not held)
                ToggleActive();
            }
            // Keep current m_isActive value
            shouldBeActive = m_isActive;
            break;
            
        case 2: // Always on
            shouldBeActive = true;
            m_isActive = true;
            break;
    }
    
    // Skip if not active
    if (!shouldBeActive) {
        // Reset waiting state if we're not active
        m_isWaitingForDelay = false;
        return;
    }
    
    // Get crosshair target
    int crosshairID = localPlayer.GetCrosshairID();
    if (crosshairID <= 0 || crosshairID > MAX_PLAYERS)
        return;
    
    // Get target player
    Player target = Game::GetPlayerByID(crosshairID);
    if (!target.IsValid())
        return;
    
    // Check if target is valid
    if (!IsTargetValid(target, localPlayer))
        return;
    
    // If we're not already waiting for a delay, start the timer
    if (!m_isWaitingForDelay) {
        m_isWaitingForDelay = true;
        m_targetAcquiredTime = std::chrono::high_resolution_clock::now();
        GenerateReactionDelay();
    }
    
    // Check if enough time has passed (reaction delay)
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime - m_targetAcquiredTime).count();
    
    if (elapsedMs >= m_currentDelay) {
        // Fire at target
        if (ShouldFire(target)) {
            Game::ForceAttack();
            
            // Add a very small delay before releasing
            Sleep(10);
            
            // Stop attack
            Game::StopAttack();
            
            // Reset waiting state
            m_isWaitingForDelay = false;
        }
    }
}

bool TriggerBot::IsTargetValid(const Player& target, const Player& localPlayer) {
    // Skip invalid targets
    if (!target.IsValid())
        return false;
    
    // Check if target is alive
    if (m_checkTargetHealth && !target.IsAlive())
        return false;
    
    // Check if target is on our team
    if (!m_targetTeammates && target.GetTeam() == localPlayer.GetTeam())
        return false;
    
    // Check if target is visible
    if (m_checkVisibility && !IsVisible(target, localPlayer))
        return false;
    
    return true;
}

bool TriggerBot::IsVisible(const Player& target, const Player& localPlayer) {
    // Simple implementation - assumes crosshair ID already checks visibility
    // For a more accurate implementation, you would perform a ray trace
    return true;
}

bool TriggerBot::ShouldFire(const Player& target) {
    // Check if weapon can fire
    Weapon activeWeapon = Game::GetLocalPlayer().GetActiveWeapon();
    if (!activeWeapon.IsValid() || !activeWeapon.CanFire())
        return false;
    
    // Don't shoot with grenades or knife
    if (activeWeapon.IsGrenade() || activeWeapon.IsKnife())
        return false;
    
    return true;
}

void TriggerBot::GenerateReactionDelay() {
    m_currentDelay = m_reactionDistribution(m_randomGenerator);
}
