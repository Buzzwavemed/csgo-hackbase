#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <random>
#include <chrono>
#include <mutex>

class PatternScanner {
public:
    // Initialize the pattern scanner
    static bool Initialize();
    
    // Update all offsets (called when needed)
    static bool UpdateOffsets();

    // Register a pattern to be scanned
    static void RegisterPattern(
        const std::string& name,
        const std::wstring& moduleName,
        const std::string& pattern,
        const std::function<void(uintptr_t)>& callback = nullptr,
        int extraOffset = 0,
        bool readRelative = false
    );
    
    // Get a found offset by name
    static uintptr_t GetOffset(const std::string& name);

private:
    // Internal structure for pattern information
    struct PatternInfo {
        std::string name;
        std::wstring moduleName;
        std::string pattern;
        std::function<void(uintptr_t)> callback;
        int extraOffset;
        bool readRelative;
        uintptr_t foundAddress;
    };

    // Randomize pattern scanning order to avoid detection
    static void RandomizePatternOrder();
    
    // Add delay between pattern scans to avoid detection
    static void AddRandomDelay();

    // List of patterns to scan
    static std::vector<PatternInfo> m_patterns;
    
    // Map of found addresses
    static std::unordered_map<std::string, uintptr_t> m_foundAddresses;
    
    // Random number generator for anti-detection
    static std::mt19937 m_randomGenerator;
    
    // Mutex for thread safety
    static std::mutex m_mutex;
};

// Namespace for game offsets
namespace Offsets {
    // Global CS:GO offsets
    extern uintptr_t dwLocalPlayer;
    extern uintptr_t dwEntityList;
    extern uintptr_t dwViewMatrix;
    extern uintptr_t dwClientState;
    extern uintptr_t dwForceJump;
    extern uintptr_t dwForceAttack;
    
    // ClientState offsets
    extern uintptr_t dwClientState_ViewAngles;
    extern uintptr_t dwClientState_MaxPlayer;
    extern uintptr_t dwClientState_State;
    
    // Player offsets
    extern uintptr_t m_iHealth;
    extern uintptr_t m_vecOrigin;
    extern uintptr_t m_vecViewOffset;
    extern uintptr_t m_dwBoneMatrix;
    extern uintptr_t m_bDormant;
    extern uintptr_t m_iTeamNum;
    extern uintptr_t m_lifeState;
    extern uintptr_t m_fFlags;
    extern uintptr_t m_iCrosshairId;
    extern uintptr_t m_bSpotted;
    
    // Weapon offsets
    extern uintptr_t m_hActiveWeapon;
    extern uintptr_t m_iItemDefinitionIndex;
    extern uintptr_t m_flNextPrimaryAttack;
    extern uintptr_t m_iClip1;
    extern uintptr_t m_bInReload;
    extern uintptr_t m_iShotsFired;
    extern uintptr_t m_aimPunchAngle;
    extern uintptr_t m_viewPunchAngle;
    
    // Initialize all offsets
    bool Initialize();
}
