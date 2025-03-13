#include "pattern_scan.h"
#include "memory.h"
#include <algorithm>
#include <thread>

// Initialize static members
std::vector<PatternScanner::PatternInfo> PatternScanner::m_patterns;
std::unordered_map<std::string, uintptr_t> PatternScanner::m_foundAddresses;
std::mt19937 PatternScanner::m_randomGenerator(std::chrono::system_clock::now().time_since_epoch().count());
std::mutex PatternScanner::m_mutex;

// Initialize Offsets namespace variables
namespace Offsets {
    // Global CS:GO offsets
    uintptr_t dwLocalPlayer = 0;
    uintptr_t dwEntityList = 0;
    uintptr_t dwViewMatrix = 0;
    uintptr_t dwClientState = 0;
    uintptr_t dwForceJump = 0;
    uintptr_t dwForceAttack = 0;
    
    // ClientState offsets
    uintptr_t dwClientState_ViewAngles = 0;
    uintptr_t dwClientState_MaxPlayer = 0;
    uintptr_t dwClientState_State = 0;
    
    // Player offsets
    uintptr_t m_iHealth = 0;
    uintptr_t m_vecOrigin = 0;
    uintptr_t m_vecViewOffset = 0;
    uintptr_t m_dwBoneMatrix = 0;
    uintptr_t m_bDormant = 0;
    uintptr_t m_iTeamNum = 0;
    uintptr_t m_lifeState = 0;
    uintptr_t m_fFlags = 0;
    uintptr_t m_iCrosshairId = 0;
    uintptr_t m_bSpotted = 0;
    
    // Weapon offsets
    uintptr_t m_hActiveWeapon = 0;
    uintptr_t m_iItemDefinitionIndex = 0;
    uintptr_t m_flNextPrimaryAttack = 0;
    uintptr_t m_iClip1 = 0;
    uintptr_t m_bInReload = 0;
    uintptr_t m_iShotsFired = 0;
    uintptr_t m_aimPunchAngle = 0;
    uintptr_t m_viewPunchAngle = 0;
    
    // Initialize all offsets
    bool Initialize() {
        // Register patterns for important offsets

        // -- Entity related patterns --
        
        // LocalPlayer pointer
        PatternScanner::RegisterPattern(
            "dwLocalPlayer",
            L"client.dll",
            "8D 34 85 ? ? ? ? 89 15 ? ? ? ? 8B 41 08 8B 48 04 83 F9 FF",
            [](uintptr_t addr) { dwLocalPlayer = Memory::Read<uintptr_t>(addr + 3) + 4; }
        );
        
        // EntityList pointer
        PatternScanner::RegisterPattern(
            "dwEntityList",
            L"client.dll",
            "BB ? ? ? ? 83 FF 01 0F 8C ? ? ? ? 3B F8",
            [](uintptr_t addr) { dwEntityList = Memory::Read<uintptr_t>(addr + 1); }
        );
        
        // ViewMatrix
        PatternScanner::RegisterPattern(
            "dwViewMatrix",
            L"client.dll",
            "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9",
            [](uintptr_t addr) { dwViewMatrix = Memory::GetAbsoluteAddress(addr, 3, 7); }
        );
        
        // ClientState
        PatternScanner::RegisterPattern(
            "dwClientState",
            L"engine.dll",
            "A1 ? ? ? ? 33 D2 6A 00 6A 00 33 C9 89 B0",
            [](uintptr_t addr) { dwClientState = Memory::Read<uintptr_t>(addr + 1); }
        );
        
        // Force Jump
        PatternScanner::RegisterPattern(
            "dwForceJump",
            L"client.dll",
            "8B 0D ? ? ? ? 8B D6 8B C1 83 CA 02",
            [](uintptr_t addr) { dwForceJump = Memory::GetAbsoluteAddress(addr, 2, 6); }
        );
        
        // Force Attack
        PatternScanner::RegisterPattern(
            "dwForceAttack",
            L"client.dll",
            "89 0D ? ? ? ? 8B 0D ? ? ? ? 8B F2 8B C1 83 CE 04",
            [](uintptr_t addr) { dwForceAttack = Memory::GetAbsoluteAddress(addr, 2, 6); }
        );
        
        // -- ClientState offsets --
        
        // ClientState_ViewAngles
        PatternScanner::RegisterPattern(
            "dwClientState_ViewAngles",
            L"engine.dll",
            "F3 0F 11 86 ? ? ? ? F3 0F 10 44 24 ? F3 0F 11 86",
            [](uintptr_t addr) { dwClientState_ViewAngles = Memory::Read<uint32_t>(addr + 4); }
        );
        
        // ClientState_MaxPlayer
        PatternScanner::RegisterPattern(
            "dwClientState_MaxPlayer",
            L"engine.dll",
            "8B 0D ? ? ? ? 8B 87 ? ? ? ? 8B 40 ? 8B 4F 04 89 45 BC",
            [](uintptr_t addr) { dwClientState_MaxPlayer = Memory::Read<uint32_t>(addr + 11); }
        );
        
        // ClientState_State
        PatternScanner::RegisterPattern(
            "dwClientState_State",
            L"engine.dll",
            "83 B8 ? ? ? ? 06 0F 94 C0 C3",
            [](uintptr_t addr) { dwClientState_State = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // -- Player offsets --
        
        // Health
        PatternScanner::RegisterPattern(
            "m_iHealth",
            L"client.dll",
            "83 B9 ? ? ? ? 00 7F 2D 8B 01",
            [](uintptr_t addr) { m_iHealth = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // Origin
        PatternScanner::RegisterPattern(
            "m_vecOrigin",
            L"client.dll",
            "F3 0F 7E 05 ? ? ? ? 8B 40 04 C3",
            [](uintptr_t addr) { m_vecOrigin = Memory::Read<uint32_t>(addr + 4) + 0x04; }
        );
        
        // ViewOffset
        PatternScanner::RegisterPattern(
            "m_vecViewOffset",
            L"client.dll",
            "8B 83 ? ? ? ? F3 0F 10 40 ? C7 44 24",
            [](uintptr_t addr) { m_vecViewOffset = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // BoneMatrix
        PatternScanner::RegisterPattern(
            "m_dwBoneMatrix",
            L"client.dll",
            "8B 8E ? ? ? ? 89 8C 24 ? ? ? ? 8B 16",
            [](uintptr_t addr) { m_dwBoneMatrix = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // Dormant
        PatternScanner::RegisterPattern(
            "m_bDormant",
            L"client.dll",
            "8A 81 ? ? ? ? C3 32 C0",
            [](uintptr_t addr) { m_bDormant = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // Team
        PatternScanner::RegisterPattern(
            "m_iTeamNum",
            L"client.dll",
            "8B 83 ? ? ? ? 85 C0 74 08 05 ? ? ? ? EB 02 33 C0",
            [](uintptr_t addr) { m_iTeamNum = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // LifeState
        PatternScanner::RegisterPattern(
            "m_lifeState",
            L"client.dll",
            "83 B9 ? ? ? ? 00 0F 85 ? ? ? ? 8B 01",
            [](uintptr_t addr) { m_lifeState = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // Flags
        PatternScanner::RegisterPattern(
            "m_fFlags",
            L"client.dll",
            "8B 81 ? ? ? ? 8B 54 24 04 89 02 8B 81",
            [](uintptr_t addr) { m_fFlags = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // CrosshairId
        PatternScanner::RegisterPattern(
            "m_iCrosshairId",
            L"client.dll",
            "8B 89 ? ? ? ? 85 C9 74 6F",
            [](uintptr_t addr) { m_iCrosshairId = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // Spotted
        PatternScanner::RegisterPattern(
            "m_bSpotted",
            L"client.dll",
            "8A 83 ? ? ? ? 89 44 24 0C",
            [](uintptr_t addr) { m_bSpotted = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // -- Weapon offsets --
        
        // ActiveWeapon
        PatternScanner::RegisterPattern(
            "m_hActiveWeapon",
            L"client.dll",
            "8B 8E ? ? ? ? 85 C9 74 05 E8",
            [](uintptr_t addr) { m_hActiveWeapon = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // ItemDefinitionIndex
        PatternScanner::RegisterPattern(
            "m_iItemDefinitionIndex",
            L"client.dll",
            "8B 81 ? ? ? ? 85 C0 0F 84 ? ? ? ? 5B 8D 48 08",
            [](uintptr_t addr) { m_iItemDefinitionIndex = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // NextPrimaryAttack
        PatternScanner::RegisterPattern(
            "m_flNextPrimaryAttack",
            L"client.dll",
            "F3 0F 10 86 ? ? ? ? 0F 2F C6",
            [](uintptr_t addr) { m_flNextPrimaryAttack = Memory::Read<uint32_t>(addr + 4); }
        );
        
        // Clip1
        PatternScanner::RegisterPattern(
            "m_iClip1",
            L"client.dll",
            "8B 41 ? 8B 80 ? ? ? ? C3",
            [](uintptr_t addr) { m_iClip1 = Memory::Read<uint32_t>(addr + 5); }
        );
        
        // InReload
        PatternScanner::RegisterPattern(
            "m_bInReload",
            L"client.dll",
            "80 B9 ? ? ? ? 00 0F 85 ? ? ? ? 57",
            [](uintptr_t addr) { m_bInReload = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // ShotsFired
        PatternScanner::RegisterPattern(
            "m_iShotsFired",
            L"client.dll",
            "8B 83 ? ? ? ? 85 C0 75 12",
            [](uintptr_t addr) { m_iShotsFired = Memory::Read<uint32_t>(addr + 2); }
        );
        
        // AimPunchAngle
        PatternScanner::RegisterPattern(
            "m_aimPunchAngle",
            L"client.dll",
            "F3 0F 10 86 ? ? ? ? F3 0F 58 45 ? 89 45 FC",
            [](uintptr_t addr) { m_aimPunchAngle = Memory::Read<uint32_t>(addr + 4); }
        );
        
        // ViewPunchAngle
        PatternScanner::RegisterPattern(
            "m_viewPunchAngle",
            L"client.dll",
            "F3 0F 11 86 ? ? ? ? 8B 06 8B CE FF 90",
            [](uintptr_t addr) { m_viewPunchAngle = Memory::Read<uint32_t>(addr + 4); }
        );
        
        // Update all patterns
        return PatternScanner::UpdateOffsets();
    }
}

bool PatternScanner::Initialize() {
    // Seed random generator
    m_randomGenerator.seed(static_cast<unsigned int>(
        std::chrono::system_clock::now().time_since_epoch().count()));
    
    return true;
}

void PatternScanner::RegisterPattern(
    const std::string& name,
    const std::wstring& moduleName,
    const std::string& pattern,
    const std::function<void(uintptr_t)>& callback,
    int extraOffset,
    bool readRelative
) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    PatternInfo info;
    info.name = name;
    info.moduleName = moduleName;
    info.pattern = pattern;
    info.callback = callback;
    info.extraOffset = extraOffset;
    info.readRelative = readRelative;
    info.foundAddress = 0;
    
    m_patterns.push_back(info);
}

uintptr_t PatternScanner::GetOffset(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_foundAddresses.find(name);
    if (it != m_foundAddresses.end()) {
        return it->second;
    }
    
    return 0;
}

void PatternScanner::RandomizePatternOrder() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Shuffle patterns to randomize scanning order
    std::shuffle(m_patterns.begin(), m_patterns.end(), m_randomGenerator);
    
    // Add a small delay to avoid predictable timing
    AddRandomDelay();
}

void PatternScanner::AddRandomDelay() {
    // Generate a random delay between 5-20ms to avoid detection based on timing
    std::uniform_int_distribution<> dist(5, 20);
    int delay = dist(m_randomGenerator);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
}

bool PatternScanner::UpdateOffsets() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // First randomize pattern order for anti-detection
    RandomizePatternOrder();
    
    bool success = true;
    
    // Scan each pattern
    for (auto& pattern : m_patterns) {
        // Add a small random delay between scans
        AddRandomDelay();
        
        // Scan for pattern
        uintptr_t address = Memory::FindPattern(pattern.moduleName, pattern.pattern);
        
        if (address) {
            // Apply extra offset if specified
            address += pattern.extraOffset;
            
            // Read relative address if specified
            if (pattern.readRelative) {
                address = Memory::ResolveRelativeAddress(address, 0);
            }
            
            // Store the result
            pattern.foundAddress = address;
            m_foundAddresses[pattern.name] = address;
            
            // Call callback function if provided
            if (pattern.callback) {
                pattern.callback(address);
            }
        }
        else {
            success = false;
        }
    }
    
    return success;
}
