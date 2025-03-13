# CS:GO Cheat with Pattern Scanning

A feature-rich, advanced CS:GO cheat that includes aimbot, triggerbot, bunnyhop functionality, and automatic pattern scanning for offsets.

## Features

### Pattern Scanning
- **Auto-updating offsets**: No need to manually update after CS:GO patches
- **Resilient against game updates**: Automatically adapts to CS:GO updates
- **Signature-based detection**: Finds memory locations dynamically

### Humanized Aimbot
- **Natural aiming**: Smooth, human-like aiming movements to avoid detection
- **Customizable FOV and smoothing**: Adjust settings to your preference
- **Recoil control system**: Automatically compensates for weapon recoil
- **Multiple targeting options**: Target head, neck, chest, or stomach
- **Team and visibility checks**: Only target enemies who are visible

### Triggerbot
- **Variable reaction times**: Randomized delays to appear more human-like
- **Multiple activation modes**: Hold key, toggle, or always-on
- **Customizable delay range**: Set minimum and maximum reaction times

### Bunnyhop
- **Configurable success rate**: Adjust how often jumps are successful
- **Timing randomization**: Adds variability to avoid detection patterns
- **Auto-strafe capability**: Automatically strafes for optimal movement

### In-Game Menu
- **Real-time configuration**: Adjust all settings while playing
- **Status indicators**: Clear visual feedback on feature states
- **Key binding system**: Customize control keys

## How to Build

### Prerequisites
- Visual Studio 2022 (or later) with C++ development tools
- CMake 3.10+
- Windows SDK 10.0+

### Building from Source
1. Clone the repository
```
git clone https://github.com/yourusername/csgo-cheat.git
cd csgo-cheat
```

2. Create a build directory and run CMake
```
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

3. Find the executable in `build/bin/Release/CSGOCheat.exe`

## How to Use

1. Launch the cheat executable **first**
2. Start CS:GO (or use with CS:GO already running)
3. The cheat will automatically find necessary game addresses
4. Control features using hotkeys:
   - `F1`: Toggle Aimbot on/off
   - `F2`: Toggle Triggerbot on/off
   - `F3`: Toggle Bunnyhop on/off
   - `INSERT`: Open/close configuration menu
   - `END`: Exit the cheat

## Code Structure

- `src/core/`: Core functionality for memory reading and pattern scanning
- `src/features/`: Implementation of aimbot, triggerbot, and bunnyhop
- `src/gui/`: Menu system for in-game configuration
- `src/sdk/`: Game-related structures and interfaces
- `src/utils/`: Helper functions and utilities

## Technical Details

### Pattern Scanning
The cheat uses signature scanning to find memory addresses dynamically:
```cpp
// Example pattern: "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C"
uintptr_t address = Memory::FindPattern(L"client.dll", "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C");
```

### Aimbot Implementation
The aimbot uses smooth movement interpolation with Bezier curves and human-like micro adjustments:
```cpp
Vector CalculateAimAngle(const Vector& from, const Vector& to) {
    Vector direction = to - from;
    float distance = direction.Length();
    
    // Calculate base angles
    Vector angles;
    VectorAngles(direction, angles);
    
    // Add humanization
    if (m_humanizationAmount > 0.0f) {
        ApplyHumanization(angles, distance);
    }
    
    return angles;
}
```

### Memory Access
Memory operations are performed using a polymorphic approach to avoid detection:
```cpp
template <typename T>
static T Read(uintptr_t address) {
    // Polymorphic read method that varies its approach
    if (reinterpret_cast<uintptr_t>(&address) % 3 == 0) {
        T value;
        ReadProcessMemory(m_hProcess, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
        return value;
    } 
    else {
        // Alternative method using direct memory access
        return *reinterpret_cast<T*>(address);
    }
}
```

## Safety Recommendations

To minimize the risk of detection:
1. **Use a separate account** for testing or playing with cheats
2. **Don't use in competitive matches** where anti-cheat systems are more active
3. **Avoid using obvious settings** that make your gameplay look unnatural
4. **Don't use multiple cheats simultaneously** to avoid conflicts

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Disclaimer

This software is for **educational purposes only**. Using cheats in online games:
1. Violates terms of service
2. May result in permanent account bans
3. Negatively impacts the experience for other players

The authors are not responsible for any consequences resulting from the use of this software.
