#include "bunnyhop.h"
#include <chrono>
#include <algorithm>

// Initialize static members
std::mt19937 BunnyHop::m_randomGenerator(std::chrono::system_clock::now().time_since_epoch().count());
std::uniform_int_distribution<int> BunnyHop::m_successDistribution(1, 100);
std::uniform_int_distribution<int> BunnyHop::m_timingDistribution(-50, 50);
int BunnyHop::m_successRate = 80;
int BunnyHop::m_maxTimingOffset = 20;
bool BunnyHop::m_autoStrafe = true;
bool BunnyHop::m_enabled = false;
int BunnyHop::m_strafeDir = 1;
int BunnyHop::m_lastJumpTime = 0;

bool BunnyHop::Initialize() {
    // Set up randomization
    m_randomGenerator.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    
    // Default settings
    m_successRate = 80;       // 80% success rate by default
    m_maxTimingOffset = 20;   // 20ms max timing variation
    m_autoStrafe = true;      // Enable auto-strafe by default
    m_enabled = false;        // Disabled by default
    m_strafeDir = 1;          // Start strafing right
    
    return true;
}

void BunnyHop::Run() {
    // Skip if disabled
    if (!m_enabled)
        return;
    
    // Get local player
    Player localPlayer = Game::GetLocalPlayer();
    if (!localPlayer.IsValid() || !localPlayer.IsAlive())
        return;
    
    // Check if space key is being held
    bool isSpacePressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    if (!isSpacePressed)
        return;
    
    // Get player flags
    int flags = localPlayer.GetFlags();
    
    // Check if player is on ground
    bool isOnGround = localPlayer.IsOnGround();
    
    // Handle auto-strafe if enabled and player is in air
    if (m_autoStrafe && !isOnGround) {
        AutoStrafe();
    }
    
    // Handle jumping
    if (isOnGround) {
        // Determine if we should allow this jump (for humanization)
        if (ShouldJump()) {
            // Get current time for timing offset
            int currentTime = GetTickCount();
            
            // Apply timing offset if configured
            if (m_maxTimingOffset > 0) {
                // Calculate a random offset between -m_maxTimingOffset and +m_maxTimingOffset ms
                int offset = (m_timingDistribution(m_randomGenerator) * m_maxTimingOffset) / 50;
                
                // If not enough time has passed with our random offset, don't jump yet
                if (currentTime - m_lastJumpTime < abs(offset)) {
                    return;
                }
            }
            
            // Force jump command
            Game::ForceJump();
            
            // Store last jump time
            m_lastJumpTime = currentTime;
            
            // Switch strafe direction for next jump
            m_strafeDir = -m_strafeDir;
        }
    }
}

bool BunnyHop::ShouldJump() {
    // If success rate is 100%, always return true
    if (m_successRate >= 100)
        return true;
    
    // If success rate is 0%, never return true
    if (m_successRate <= 0)
        return false;
    
    // Generate a random number between 1 and 100
    int random = m_successDistribution(m_randomGenerator);
    
    // Return true if random number is less than or equal to success rate
    return random <= m_successRate;
}

void BunnyHop::AutoStrafe() {
    // Auto-strafe left/right while in air
    if (m_strafeDir > 0) {
        // Strafe right
        keybd_event('D', 0, 0, 0);                     // Press D
        keybd_event('A', 0, KEYEVENTF_KEYUP, 0);       // Release A
    }
    else {
        // Strafe left
        keybd_event('A', 0, 0, 0);                     // Press A
        keybd_event('D', 0, KEYEVENTF_KEYUP, 0);       // Release D
    }
    
    // Add mouse movement for turning
    // Note: This is a simplified approach. For better strafing, you would
    // need to adjust the mouse movement based on player's velocity.
    // mouse_event(MOUSEEVENTF_MOVE, m_strafeDir * 2, 0, 0, 0);
}
