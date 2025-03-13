#include "entity.h"
#include "../core/pattern_scan.h"
#include <vector>

// Entity implementation
int Entity::GetIndex() const {
    if (!IsValid())
        return -1;
    
    // In CS:GO, the entity index is typically (entityAddress - entityList) / entitySize
    // But we can't reliably calculate it that way, so we read it from memory
    // This is often stored at entity + 0x64, but we'll use our offsets
    return Memory::Read<int>(m_address + 0x64);
}

Team Entity::GetTeam() const {
    if (!IsValid())
        return Team::NONE;
    
    return static_cast<Team>(Memory::Read<int>(m_address + Offsets::m_iTeamNum));
}

Math::Vec3 Entity::GetOrigin() const {
    if (!IsValid())
        return Math::Vec3();
    
    return Memory::Read<Math::Vec3>(m_address + Offsets::m_vecOrigin);
}

bool Entity::IsDormant() const {
    if (!IsValid())
        return true;
    
    return Memory::Read<bool>(m_address + Offsets::m_bDormant);
}

uintptr_t Entity::GetBoneMatrix() const {
    if (!IsValid())
        return 0;
    
    return Memory::Read<uintptr_t>(m_address + Offsets::m_dwBoneMatrix);
}

Math::Vec3 Entity::GetBonePosition(int bone) const {
    Math::Vec3 bonePos;
    uintptr_t boneMatrix = GetBoneMatrix();
    
    if (boneMatrix == 0)
        return Math::Vec3();
    
    // Bone matrix structure: matrix3x4_t
    // Each bone has a 12-float (3x4) matrix
    // Position is at [0][3], [1][3], [2][3]
    bonePos.x = Memory::Read<float>(boneMatrix + 0x30 * bone + 0x0C);
    bonePos.y = Memory::Read<float>(boneMatrix + 0x30 * bone + 0x1C);
    bonePos.z = Memory::Read<float>(boneMatrix + 0x30 * bone + 0x2C);
    
    return bonePos;
}

void Entity::Spot() const {
    if (!IsValid())
        return;
    
    Memory::Write<bool>(m_address + Offsets::m_bSpotted, true);
}

// Player implementation
bool Player::IsAlive() const {
    if (!IsValid())
        return false;
    
    int health = GetHealth();
    if (health <= 0 || health > 100)
        return false;
    
    auto lifeState = Memory::Read<int>(m_address + Offsets::m_lifeState);
    return lifeState == static_cast<int>(LifeState::ALIVE);
}

int Player::GetHealth() const {
    if (!IsValid())
        return 0;
    
    return Memory::Read<int>(m_address + Offsets::m_iHealth);
}

Math::Vec3 Player::GetViewOffset() const {
    if (!IsValid())
        return Math::Vec3();
    
    return Memory::Read<Math::Vec3>(m_address + Offsets::m_vecViewOffset);
}

Weapon Player::GetActiveWeapon() const {
    if (!IsValid())
        return Weapon(0);
    
    // Get weapon handle
    int weaponHandle = Memory::Read<int>(m_address + Offsets::m_hActiveWeapon) & 0xFFF;
    
    // Get the actual entity based on handle
    uintptr_t entityList = Memory::Read<uintptr_t>(Memory::GetClientDLL() + Offsets::dwEntityList);
    uintptr_t weaponEntity = Memory::Read<uintptr_t>(entityList + (weaponHandle - 1) * 0x10);
    
    return Weapon(weaponEntity);
}

int Player::GetFlags() const {
    if (!IsValid())
        return 0;
    
    return Memory::Read<int>(m_address + Offsets::m_fFlags);
}

int Player::GetCrosshairID() const {
    if (!IsValid())
        return 0;
    
    return Memory::Read<int>(m_address + Offsets::m_iCrosshairId);
}

int Player::GetShotsFired() const {
    if (!IsValid())
        return 0;
    
    return Memory::Read<int>(m_address + Offsets::m_iShotsFired);
}

Math::Vec3 Player::GetAimPunchAngle() const {
    if (!IsValid())
        return Math::Vec3();
    
    return Memory::Read<Math::Vec3>(m_address + Offsets::m_aimPunchAngle);
}

Math::Vec3 Player::GetViewPunchAngle() const {
    if (!IsValid())
        return Math::Vec3();
    
    return Memory::Read<Math::Vec3>(m_address + Offsets::m_viewPunchAngle);
}

// Weapon implementation
WeaponID Weapon::GetWeaponID() const {
    if (!IsValid())
        return WeaponID::NONE;
    
    return static_cast<WeaponID>(Memory::Read<short>(m_address + Offsets::m_iItemDefinitionIndex));
}

WeaponType Weapon::GetWeaponType() const {
    WeaponID id = GetWeaponID();
    
    // Classify by weapon ID
    switch (id) {
        // Knives
        case WeaponID::KNIFE:
        case WeaponID::KNIFE_T:
            return WeaponType::KNIFE;
            
        // Pistols
        case WeaponID::DEAGLE:
        case WeaponID::ELITE:
        case WeaponID::FIVESEVEN:
        case WeaponID::GLOCK:
        case WeaponID::P2000:
        case WeaponID::P250:
        case WeaponID::TEC9:
        case WeaponID::USP_SILENCER:
        case WeaponID::CZ75A:
        case WeaponID::REVOLVER:
            return WeaponType::PISTOL;
            
        // SMGs
        case WeaponID::MAC10:
        case WeaponID::P90:
        case WeaponID::MP5:
        case WeaponID::UMP45:
        case WeaponID::BIZON:
        case WeaponID::MP7:
        case WeaponID::MP9:
            return WeaponType::SMG;
            
        // Rifles
        case WeaponID::AK47:
        case WeaponID::AUG:
        case WeaponID::FAMAS:
        case WeaponID::GALIL:
        case WeaponID::M4A1:
        case WeaponID::M4A1_SILENCER:
        case WeaponID::SG553:
            return WeaponType::RIFLE;
            
        // Snipers
        case WeaponID::AWP:
        case WeaponID::G3SG1:
        case WeaponID::SCAR20:
        case WeaponID::SSG08:
            return WeaponType::SNIPER;
            
        // Shotguns
        case WeaponID::XM1014:
        case WeaponID::MAG7:
        case WeaponID::SAWEDOFF:
        case WeaponID::NOVA:
            return WeaponType::SHOTGUN;
            
        // Machine guns
        case WeaponID::M249:
        case WeaponID::NEGEV:
            return WeaponType::MACHINE_GUN;
            
        // Grenades
        case WeaponID::FLASHBANG:
        case WeaponID::HEGRENADE:
        case WeaponID::SMOKEGRENADE:
        case WeaponID::MOLOTOV:
        case WeaponID::DECOY:
        case WeaponID::INCGRENADE:
            return WeaponType::GRENADE;
            
        // Others
        case WeaponID::TASER:
            return WeaponType::TASER;
        case WeaponID::C4:
            return WeaponType::C4;
            
        default:
            return WeaponType::UNKNOWN;
    }
}

int Weapon::GetClip1() const {
    if (!IsValid())
        return 0;
    
    return Memory::Read<int>(m_address + Offsets::m_iClip1);
}

bool Weapon::IsInReload() const {
    if (!IsValid())
        return false;
    
    return Memory::Read<bool>(m_address + Offsets::m_bInReload);
}

float Weapon::GetNextPrimaryAttack() const {
    if (!IsValid())
        return 0.0f;
    
    return Memory::Read<float>(m_address + Offsets::m_flNextPrimaryAttack);
}

bool Weapon::CanFire() const {
    if (!IsValid() || IsInReload() || GetClip1() <= 0)
        return false;
    
    float serverTime = Game::GetServerTime();
    return GetNextPrimaryAttack() <= serverTime;
}

float Weapon::GetInaccuracy() const {
    WeaponType type = GetWeaponType();
    
    // Return a reasonable inaccuracy value based on weapon type
    // These values can be adjusted based on testing
    switch (type) {
        case WeaponType::KNIFE:
            return 0.0f;
        case WeaponType::PISTOL:
            return 0.5f;
        case WeaponType::SMG:
            return 0.75f;
        case WeaponType::RIFLE:
            return 0.65f;
        case WeaponType::SHOTGUN:
            return 1.5f;
        case WeaponType::SNIPER:
            return IsWeaponID(WeaponID::AWP) ? 0.1f : 0.3f;
        case WeaponType::MACHINE_GUN:
            return 1.0f;
        default:
            return 1.0f;
    }
}

// Helper function to check weapon ID
bool Weapon::IsWeaponID(WeaponID id) const {
    return GetWeaponID() == id;
}

// Game class implementation
Player Game::GetLocalPlayer() {
    uintptr_t localPlayerPtr = Memory::Read<uintptr_t>(Memory::GetClientDLL() + Offsets::dwLocalPlayer);
    return Player(localPlayerPtr);
}

Player Game::GetPlayerByIndex(int index) {
    if (index < 0 || index >= MAX_PLAYERS)
        return Player(0);
    
    uintptr_t entityList = Memory::Read<uintptr_t>(Memory::GetClientDLL() + Offsets::dwEntityList);
    uintptr_t entity = Memory::Read<uintptr_t>(entityList + index * 0x10);
    
    return Player(entity);
}

Player Game::GetPlayerByID(int id) {
    if (id <= 0 || id > MAX_PLAYERS)
        return Player(0);
    
    // Traverse entity list to find player with matching ID
    for (int i = 1; i < MAX_PLAYERS; i++) {
        Player player = GetPlayerByIndex(i);
        if (player.IsValid() && player.GetIndex() == id)
            return player;
    }
    
    return Player(0);
}

Math::Matrix4x4 Game::GetViewMatrix() {
    return Memory::Read<Math::Matrix4x4>(Memory::GetClientDLL() + Offsets::dwViewMatrix);
}

uintptr_t Game::GetClientState() {
    return Memory::Read<uintptr_t>(Memory::GetEngineDLL() + Offsets::dwClientState);
}

std::string Game::GetCurrentMap() {
    char mapName[64] = { 0 };
    uintptr_t clientState = GetClientState();
    
    if (clientState == 0)
        return "";
    
    // The map name is typically stored at clientState + some offset
    // But this may change with updates, so we use a pattern
    // Use ReadProcessMemory directly for char arrays
    ReadProcessMemory(Memory::GetProcessHandle(), reinterpret_cast<LPCVOID>(clientState + 0x28C), mapName, sizeof(mapName), nullptr);
    
    return std::string(mapName);
}

Math::Vec3 Game::GetViewAngles() {
    uintptr_t clientState = GetClientState();
    
    if (clientState == 0)
        return Math::Vec3();
    
    return Memory::Read<Math::Vec3>(clientState + Offsets::dwClientState_ViewAngles);
}

void Game::SetViewAngles(const Math::Vec3& angles) {
    uintptr_t clientState = GetClientState();
    
    if (clientState == 0)
        return;
    
    // Create a copy and clamp it to valid angles
    Math::Vec3 clampedAngles = angles;
    Math::ClampAngles(clampedAngles);
    
    // Write the new view angles
    Memory::Write<Math::Vec3>(clientState + Offsets::dwClientState_ViewAngles, clampedAngles);
}

void Game::ForceJump() {
    Memory::Write<int>(Memory::GetClientDLL() + Offsets::dwForceJump, 6);
}

void Game::ForceAttack() {
    Memory::Write<int>(Memory::GetClientDLL() + Offsets::dwForceAttack, 6);
}

void Game::StopAttack() {
    Memory::Write<int>(Memory::GetClientDLL() + Offsets::dwForceAttack, 4);
}

std::vector<Player> Game::GetAllPlayers(bool& outSuccess) {
    // Initialize vector
    std::vector<Player> players;
    players.reserve(MAX_PLAYERS);  // Reserve space for efficiency
    
    outSuccess = true;
    
    // Get entity list base
    uintptr_t entityList = Memory::Read<uintptr_t>(Memory::GetClientDLL() + Offsets::dwEntityList);
    
    if (entityList == 0) {
        outSuccess = false;
        return players;
    }
    
    // Get all players
    for (int i = 0; i < MAX_PLAYERS; i++) {
        uintptr_t entity = Memory::Read<uintptr_t>(entityList + i * 0x10);
        players.push_back(Player(entity));  // Even if entity is 0, we'll push a default player
    }
    
    return players;
}

std::vector<Player> Game::GetValidPlayers(bool enemyOnly) {
    std::vector<Player> validPlayers;
    bool success;
    
    // Get local player
    Player localPlayer = GetLocalPlayer();
    
    if (!localPlayer.IsValid())
        return validPlayers;
    
    Team localTeam = localPlayer.GetTeam();
    
    // Get all players
    auto players = GetAllPlayers(success);
    
    if (!success)
        return validPlayers;
    
    // Filter players
    for (const auto& player : players) {
        if (!player.IsValid() || player.IsDormant() || !player.IsAlive())
            continue;
        
        // Skip self
        if (player.GetAddress() == localPlayer.GetAddress())
            continue;
        
        // Skip teammates if enemy only
        if (enemyOnly && player.GetTeam() == localTeam)
            continue;
        
        validPlayers.push_back(player);
    }
    
    return validPlayers;
}

float Game::GetTime() {
    return Memory::Read<float>(Memory::GetEngineDLL() + 0x3C);
}

float Game::GetServerTime() {
    // Get the local player pointer
    Player localPlayer = GetLocalPlayer();
    
    if (!localPlayer.IsValid())
        return 0.0f;
    
    // Server time is typically calculated from tick count
    int tickCount = Memory::Read<int>(localPlayer.GetAddress() + 0x3430);
    float tickInterval = 1.0f / 64.0f; // 64 tick servers
    
    return tickCount * tickInterval;
}
