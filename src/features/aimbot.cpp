#include "aimbot.h"
#include <Windows.h>
#include <algorithm>

// Initialize static members
Player Aimbot::m_currentTarget = Player(0);
bool Aimbot::m_hasTarget = false;
Math::Vec3 Aimbot::m_lastAimAngle;
float Aimbot::m_aimProgress = 0.0f;

std::mt19937 Aimbot::m_randomGenerator(std::chrono::system_clock::now().time_since_epoch().count());
std::uniform_real_distribution<float> Aimbot::m_uniformDist(0.0f, 1.0f);

// Configuration
bool Aimbot::m_enabled = false;
bool Aimbot::m_isActive = false;
int Aimbot::m_activationKey = VK_LBUTTON; // Left mouse button by default
int Aimbot::m_activationMode = 0; // Hold key mode by default
float Aimbot::m_fov = 5.0f; // 5 degrees FOV by default
float Aimbot::m_smoothing = 5.0f; // Smoothing factor
int Aimbot::m_targetBone = Bones::HEAD; // Target head by default
bool Aimbot::m_visibilityCheck = true; // Check if target is visible
bool Aimbot::m_teamCheck = true; // Don't target teammates
bool Aimbot::m_autoFire = false; // Don't auto-fire by default
bool Aimbot::m_recoilControlEnabled = true; // Enable recoil control by default
float Aimbot::m_recoilControlStrength = 1.0f; // Perfect recoil control by default
float Aimbot::m_humanizationAmount = 0.3f; // Medium humanization

// Timing variables
std::chrono::time_point<std::chrono::high_resolution_clock> Aimbot::m_aimStartTime;
float Aimbot::m_aimDuration = 0.0f;

bool Aimbot::Initialize() {
    // Seed random generator
    m_randomGenerator.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    
    // Initialize with default settings
    m_enabled = false;
    m_isActive = false;
    m_hasTarget = false;
    m_activationKey = VK_LBUTTON;
    m_activationMode = 0;
    m_fov = 5.0f;
    m_smoothing = 5.0f;
    m_targetBone = Bones::HEAD;
    m_visibilityCheck = true;
    m_teamCheck = true;
    m_autoFire = false;
    m_recoilControlEnabled = true;
    m_recoilControlStrength = 1.0f;
    m_humanizationAmount = 0.3f;
    
    return true;
}

void Aimbot::Run() {
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
                m_isActive = !m_isActive;
            }
            shouldBeActive = m_isActive;
            break;
            
        case 2: // Always on
            shouldBeActive = true;
            m_isActive = true;
            break;
    }
    
    // Skip if not active
    if (!shouldBeActive) {
        // Reset targeting when deactivated
        if (m_hasTarget) {
            m_hasTarget = false;
            m_aimProgress = 0.0f;
            m_currentTarget = Player(0);
        }
        return;
    }
    
    // Get current view angles
    Math::Vec3 currentViewAngle = Game::GetViewAngles();
    
    // If we don't have a target or our current target becomes invalid, find a new one
    if (!m_hasTarget || !IsPlayerValid(m_currentTarget)) {
        m_currentTarget = FindBestTarget();
        
        if (m_currentTarget.IsValid()) {
            m_hasTarget = true;
            m_aimStartTime = std::chrono::high_resolution_clock::now();
            
            // Calculate aim duration based on distance to target and smoothing
            Math::Vec3 targetAngle = CalculateAimAngle(m_currentTarget);
            float angleDist = 0.0f;
            
            // Calculate the angular distance
            angleDist += abs(Math::AngleDifference(targetAngle.x, currentViewAngle.x));
            angleDist += abs(Math::AngleDifference(targetAngle.y, currentViewAngle.y));
            
            // Set aim duration proportional to angular distance and smoothing
            // This makes larger movements take more time
            m_aimDuration = (angleDist / 180.0f) * (m_smoothing * 0.1f);
            
            // Clamp between 50ms and 500ms
            if (m_aimDuration < 0.05f) m_aimDuration = 0.05f;
            if (m_aimDuration > 0.5f) m_aimDuration = 0.5f;
            
            // Reset aim progress
            m_aimProgress = 0.0f;
        }
    }
    
    // If we have a valid target, aim at it
    if (m_hasTarget && m_currentTarget.IsValid()) {
        // Calculate target angles
        Math::Vec3 targetAngle = CalculateAimAngle(m_currentTarget);
        
        // Update aim progress
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsedTime = std::chrono::duration<float>(currentTime - m_aimStartTime).count();
        m_aimProgress = elapsedTime / m_aimDuration;
        if (m_aimProgress > 1.0f) m_aimProgress = 1.0f; // Clamp to 1.0
        
        // Apply bezier interpolation for smoother, more natural aiming
        Math::Vec3 newAngle = ApplyBezierInterpolation(currentViewAngle, targetAngle, m_aimProgress);
        
        // Apply recoil control if enabled and needed
        if (m_recoilControlEnabled) {
            newAngle = CompensateRecoil(newAngle);
        }
        
        // Apply humanization if enabled
        if (m_humanizationAmount > 0.0f) {
            newAngle = HumanizeAimAngle(newAngle);
        }
        
        // Set the new view angles
        Game::SetViewAngles(newAngle);
        
        // Store last angle for next frame
        m_lastAimAngle = newAngle;
        
        // Handle auto-fire if enabled
        if (m_autoFire && m_aimProgress >= 0.8f) { // Only auto-fire when mostly aimed at target
            Weapon activeWeapon = localPlayer.GetActiveWeapon();
            if (activeWeapon.IsValid() && activeWeapon.CanFire() && 
                !activeWeapon.IsGrenade() && !activeWeapon.IsKnife()) {
                Game::ForceAttack();
            }
        }
    }
}

Player Aimbot::FindBestTarget() {
    Player localPlayer = Game::GetLocalPlayer();
    if (!localPlayer.IsValid())
        return Player(0);
    
    Math::Vec3 localPosition = localPlayer.GetEyePosition();
    Math::Vec3 currentViewAngle = Game::GetViewAngles();
    
    // Get all valid players
    std::vector<Player> validPlayers = Game::GetValidPlayers(m_teamCheck);
    
    // No valid targets
    if (validPlayers.empty())
        return Player(0);
    
    // Variables to track the best target
    Player bestTarget = Player(0);
    float bestScore = -1.0f;
    
    // Check each player
    for (const auto& player : validPlayers) {
        // Skip invalid players
        if (!IsPlayerValid(player))
            continue;
        
        // Calculate angles to this player
        Math::Vec3 targetPos = player.GetBonePosition(m_targetBone);
        Math::Vec3 targetAngle = Math::CalcAngle(localPosition, targetPos);
        
        // Skip if target is not in FOV
        if (!IsTargetInFOV(targetAngle, currentViewAngle))
            continue;
        
        // Skip if target is not visible (if enabled)
        if (m_visibilityCheck && !IsTargetVisible(player))
            continue;
        
        // Calculate priority score
        float score = GetTargetPriority(player, localPosition);
        
        // Update best target if this one is better
        if (score > bestScore) {
            bestScore = score;
            bestTarget = player;
        }
    }
    
    return bestTarget;
}

float Aimbot::GetTargetPriority(const Player& target, const Math::Vec3& localPosition) {
    if (!target.IsValid())
        return -1.0f;
    
    // Get target info
    Math::Vec3 targetPos = target.GetBonePosition(m_targetBone);
    float distance = localPosition.DistTo(targetPos);
    
    // Get local player view angles
    Math::Vec3 currentViewAngle = Game::GetViewAngles();
    
    // Calculate angle to target
    Math::Vec3 targetAngle = Math::CalcAngle(localPosition, targetPos);
    
    // Calculate angular distance from current view
    float yawDiff = abs(Math::AngleDifference(targetAngle.y, currentViewAngle.y));
    float pitchDiff = abs(Math::AngleDifference(targetAngle.x, currentViewAngle.x));
    float angleDist = yawDiff + pitchDiff;
    
    // Calculate health-based priority (lower health = higher priority)
    int health = target.GetHealth();
    float healthPriority = 1.0f - (health / 100.0f);
    
    // Distance priority (closer = higher priority, but not linear)
    float distancePriority = 1.0f - (distance / 3000.0f); // Assuming max distance ~3000 units
    // Clamp between 0.0 and 1.0
    if (distancePriority < 0.0f) distancePriority = 0.0f;
    if (distancePriority > 1.0f) distancePriority = 1.0f;
    
    // Angle priority (closer to crosshair = higher priority)
    float anglePriority = 1.0f - (angleDist / m_fov / 2.0f);
    // Clamp between 0.0 and 1.0
    if (anglePriority < 0.0f) anglePriority = 0.0f;
    if (anglePriority > 1.0f) anglePriority = 1.0f;
    
    // Combine priorities with different weights
    float score = anglePriority * 0.6f + distancePriority * 0.3f + healthPriority * 0.1f;
    
    return score;
}

Math::Vec3 Aimbot::CalculateAimAngle(const Player& target) {
    // Get positions
    Player localPlayer = Game::GetLocalPlayer();
    Math::Vec3 localPosition = localPlayer.GetEyePosition();
    Math::Vec3 targetPosition = target.GetBonePosition(m_targetBone);
    
    // Calculate basic angle
    Math::Vec3 aimAngle = Math::CalcAngle(localPosition, targetPosition);
    
    // Clamp to valid range
    Math::ClampAngles(aimAngle);
    
    return aimAngle;
}

Math::Vec3 Aimbot::SmoothAimAngle(const Math::Vec3& currentAngle, const Math::Vec3& targetAngle) {
    // Calculate smoothed angle
    Math::Vec3 delta = targetAngle - currentAngle;
    
    // Normalize delta angles
    if (delta.y > 180.0f) delta.y -= 360.0f;
    if (delta.y < -180.0f) delta.y += 360.0f;
    
    // Apply smoothing
    Math::Vec3 smoothed;
    smoothed.x = currentAngle.x + delta.x / m_smoothing;
    smoothed.y = currentAngle.y + delta.y / m_smoothing;
    smoothed.z = 0.0f;
    
    // Clamp to valid range
    Math::ClampAngles(smoothed);
    
    return smoothed;
}

bool Aimbot::IsTargetInFOV(const Math::Vec3& targetAngle, const Math::Vec3& currentAngle) {
    // Calculate angular distance
    float yawDiff = abs(Math::AngleDifference(targetAngle.y, currentAngle.y));
    float pitchDiff = abs(Math::AngleDifference(targetAngle.x, currentAngle.x));
    
    // Check if within FOV
    return (yawDiff <= m_fov && pitchDiff <= m_fov);
}

bool Aimbot::IsTargetVisible(const Player& target) {
    // Simple implementation that assumes target is visible
    // In a more advanced implementation, you would perform ray tracing
    return true;
}

bool Aimbot::IsPlayerValid(const Player& player) {
    // Skip invalid players
    if (!player.IsValid() || player.IsDormant())
        return false;
    
    // Skip dead players
    if (!player.IsAlive())
        return false;
    
    // Check team if enabled
    if (m_teamCheck) {
        Player localPlayer = Game::GetLocalPlayer();
        if (player.GetTeam() == localPlayer.GetTeam())
            return false;
    }
    
    return true;
}

Math::Vec3 Aimbot::HumanizeAimAngle(const Math::Vec3& angle) {
    // Add micro movement to simulate human imperfection
    Math::Vec3 humanized = angle;
    
    // Apply more randomness when moving quickly, less when close to target
    float randomFactor = m_humanizationAmount * (1.0f - m_aimProgress * 0.8f);
    
    // Add small random variation to pitch and yaw
    float randomPitch = m_uniformDist(m_randomGenerator) * 2.0f - 1.0f; // -1 to 1
    float randomYaw = m_uniformDist(m_randomGenerator) * 2.0f - 1.0f;   // -1 to 1
    
    // Scale by randomness factor
    humanized.x += randomPitch * randomFactor * 0.5f; // Less vertical variation
    humanized.y += randomYaw * randomFactor * 1.0f;   // More horizontal variation
    
    // Clamp to valid range
    Math::ClampAngles(humanized);
    
    return humanized;
}

Math::Vec3 Aimbot::AddMicroMovement(const Math::Vec3& angle) {
    // Calculate time-based micro movement
    float timeFactor = static_cast<float>(GetTickCount()) / 1000.0f;
    
    // Create very subtle sine wave motion
    float microX = sinf(timeFactor * 2.3f) * 0.08f * m_humanizationAmount;
    float microY = sinf(timeFactor * 1.9f) * 0.12f * m_humanizationAmount;
    
    // Apply to angle
    Math::Vec3 result = angle;
    result.x += microX;
    result.y += microY;
    
    // Clamp angles
    Math::ClampAngles(result);
    
    return result;
}

Math::Vec3 Aimbot::ApplyBezierInterpolation(const Math::Vec3& currentAngle, const Math::Vec3& targetAngle, float t) {
    // Calculate delta angles
    float deltaX = Math::AngleDifference(targetAngle.x, currentAngle.x);
    float deltaY = Math::AngleDifference(targetAngle.y, currentAngle.y);
    
    // Use bezier curve for smoother, more human-like motion
    float smoothFactor = 0.5f - (m_humanizationAmount * 0.3f); // More humanization = less linear curve
    float bezierT = Math::BezierSmooth(t, smoothFactor);
    
    // Calculate new angles
    Math::Vec3 newAngle;
    newAngle.x = currentAngle.x + deltaX * bezierT;
    newAngle.y = currentAngle.y + deltaY * bezierT;
    newAngle.z = 0.0f;
    
    // Clamp angles
    Math::ClampAngles(newAngle);
    
    return newAngle;
}

Math::Vec3 Aimbot::CompensateRecoil(const Math::Vec3& angle) {
    // Get local player
    Player localPlayer = Game::GetLocalPlayer();
    if (!localPlayer.IsValid() || !localPlayer.IsAlive())
        return angle;
    
    // Get recoil (aim punch angle)
    Math::Vec3 recoil = localPlayer.GetAimPunchAngle();
    
    // Only compensate recoil if shots fired > 1
    int shotsFired = localPlayer.GetShotsFired();
    if (shotsFired <= 1)
        return angle;
    
    // Compensate recoil with configured strength
    Math::Vec3 compensated = angle;
    compensated.x -= recoil.x * 2.0f * m_recoilControlStrength; // Valve scales punch by 2
    compensated.y -= recoil.y * 2.0f * m_recoilControlStrength;
    
    // Apply human imperfection to recoil control
    float imperfection = m_humanizationAmount * 0.2f; // 0-20% error based on humanization
    compensated.x += recoil.x * imperfection;
    compensated.y += recoil.y * imperfection;
    
    // Clamp angles
    Math::ClampAngles(compensated);
    
    return compensated;
}
