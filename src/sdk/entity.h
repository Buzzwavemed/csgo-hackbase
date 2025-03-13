#pragma once

#include "../core/memory.h"
#include "../utils/math.h"
#include <array>

// Forward declarations
class Entity;
class Player;
class Weapon;

// Max number of players in a CS:GO game
constexpr int MAX_PLAYERS = 64;

// Bones
enum class Bone : int {
    INVALID = -1,
    HEAD = 8,
    NECK = 7,
    UPPER_CHEST = 6,
    MIDDLE_CHEST = 5,
    LOWER_CHEST = 4,
    STOMACH = 3,
    PELVIS = 0
};

// Player flags
enum class PlayerFlags : int {
    ON_GROUND = (1 << 0),
    DUCKING = (1 << 1),
    WATER_JUMP = (1 << 2),
    ON_TRAIN = (1 << 3),
    IN_RAIN = (1 << 4),
    FROZEN = (1 << 5),
    AT_CONTROLS = (1 << 6),
    CLIENT = (1 << 7),
    FAKE_CLIENT = (1 << 8)
};

// Life states
enum class LifeState : int {
    ALIVE = 0,
    DYING = 1,
    DEAD = 2,
    RESPAWNABLE = 3,
    DISCARDBODY = 4
};

// Team numbers
enum class Team : int {
    NONE = 0,
    SPECTATOR = 1,
    TERRORIST = 2,
    COUNTER_TERRORIST = 3
};

// Weapon types
enum class WeaponType : int {
    KNIFE = 0,
    PISTOL = 1,
    SMG = 2,
    RIFLE = 3,
    SHOTGUN = 4,
    SNIPER = 5,
    MACHINE_GUN = 6,
    C4 = 7,
    TASER = 8,
    GRENADE = 9,
    UNKNOWN = 10
};

// Weapon IDs (only the most relevant ones)
enum class WeaponID : int {
    NONE = 0,
    DEAGLE = 1,
    ELITE = 2,
    FIVESEVEN = 3,
    GLOCK = 4,
    AK47 = 7,
    AUG = 8,
    AWP = 9,
    FAMAS = 10,
    G3SG1 = 11,
    GALIL = 13,
    M249 = 14,
    M4A1 = 16,
    MAC10 = 17,
    P90 = 19,
    MP5 = 23,
    UMP45 = 24,
    XM1014 = 25,
    BIZON = 26,
    MAG7 = 27,
    NEGEV = 28,
    SAWEDOFF = 29,
    TEC9 = 30,
    TASER = 31,
    P2000 = 32,
    MP7 = 33,
    MP9 = 34,
    NOVA = 35,
    P250 = 36,
    SCAR20 = 38,
    SG553 = 39,
    SSG08 = 40,
    KNIFE = 42,
    FLASHBANG = 43,
    HEGRENADE = 44,
    SMOKEGRENADE = 45,
    MOLOTOV = 46,
    DECOY = 47,
    INCGRENADE = 48,
    C4 = 49,
    M4A1_SILENCER = 60,
    USP_SILENCER = 61,
    CZ75A = 63,
    REVOLVER = 64,
    KNIFE_T = 59,
};

// Base Entity class
class Entity {
public:
    Entity(uintptr_t address) : m_address(address) {}
    
    // Is entity valid
    bool IsValid() const {
        return m_address != 0;
    }
    
    // Get entity index
    int GetIndex() const;
    
    // Get team number
    Team GetTeam() const;
    
    // Get origin (position)
    Math::Vec3 GetOrigin() const;
    
    // Check if entity is dormant (not active)
    bool IsDormant() const;
    
    // Get entity address
    uintptr_t GetAddress() const {
        return m_address;
    }
    
    // Get bone matrix
    uintptr_t GetBoneMatrix() const;
    
    // Get bone position
    Math::Vec3 GetBonePosition(int bone) const;
    
    // Spot the entity for ESP
    void Spot() const;
    
protected:
    uintptr_t m_address;
};

// Player class (derived from Entity)
class Player : public Entity {
public:
    Player(uintptr_t address) : Entity(address) {}
    
    // Is player alive
    bool IsAlive() const;
    
    // Get health
    int GetHealth() const;
    
    // Get view offset (eyes position relative to origin)
    Math::Vec3 GetViewOffset() const;
    
    // Get eye position (origin + view offset)
    Math::Vec3 GetEyePosition() const {
        return GetOrigin() + GetViewOffset();
    }
    
    // Get current weapon
    Weapon GetActiveWeapon() const;
    
    // Get flags
    int GetFlags() const;
    
    // Is player on ground
    bool IsOnGround() const {
        return (GetFlags() & static_cast<int>(PlayerFlags::ON_GROUND)) != 0;
    }
    
    // Get crosshair ID (entity index under crosshair)
    int GetCrosshairID() const;
    
    // Get shots fired (for recoil control)
    int GetShotsFired() const;
    
    // Get aim punch angle (recoil)
    Math::Vec3 GetAimPunchAngle() const;
    
    // Get view punch angle (visual recoil)
    Math::Vec3 GetViewPunchAngle() const;
};

// Weapon class (derived from Entity)
class Weapon : public Entity {
public:
    Weapon(uintptr_t address) : Entity(address) {}
    
    // Get weapon ID
    WeaponID GetWeaponID() const;
    
    // Get weapon type
    WeaponType GetWeaponType() const;
    
    // Get clip ammo
    int GetClip1() const;
    
    // Is weapon in reload
    bool IsInReload() const;
    
    // Get next primary attack time
    float GetNextPrimaryAttack() const;
    
    // Can weapon fire
    bool CanFire() const;
    
    // Is weapon knife
    bool IsKnife() const {
        WeaponType type = GetWeaponType();
        return type == WeaponType::KNIFE;
    }
    
    // Is weapon pistol
    bool IsPistol() const {
        WeaponType type = GetWeaponType();
        return type == WeaponType::PISTOL;
    }
    
    // Is weapon sniper
    bool IsSniper() const {
        WeaponType type = GetWeaponType();
        return type == WeaponType::SNIPER;
    }
    
    // Is weapon grenade
    bool IsGrenade() const {
        WeaponType type = GetWeaponType();
        return type == WeaponType::GRENADE;
    }
    
    // Is weapon automatic
    bool IsAutomatic() const {
        WeaponType type = GetWeaponType();
        return type == WeaponType::RIFLE || type == WeaponType::SMG || type == WeaponType::MACHINE_GUN;
    }
    
    // Get inaccuracy based on weapon type (for aimbot)
    float GetInaccuracy() const;
    
    // Check if weapon has specific ID
    bool IsWeaponID(WeaponID id) const;
};

// Game class for accessing all entities and game state
class Game {
public:
    // Get local player
    static Player GetLocalPlayer();
    
    // Get player by index
    static Player GetPlayerByIndex(int index);
    
    // Get player by ID
    static Player GetPlayerByID(int id);
    
    // Get view matrix
    static Math::Matrix4x4 GetViewMatrix();
    
    // Get client state
    static uintptr_t GetClientState();
    
    // Get current map
    static std::string GetCurrentMap();
    
    // Get view angles
    static Math::Vec3 GetViewAngles();
    
    // Set view angles
    static void SetViewAngles(const Math::Vec3& angles);
    
    // Force jump
    static void ForceJump();
    
    // Force attack
    static void ForceAttack();
    
    // Stop attack
    static void StopAttack();
    
    // Get all players
    static std::vector<Player> GetAllPlayers(bool& outSuccess);
    
    // Get valid players (alive and not dormant)
    static std::vector<Player> GetValidPlayers(bool enemyOnly = true);
    
    // Get time
    static float GetTime();
    
    // Get server time
    static float GetServerTime();
};
