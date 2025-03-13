#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

class Memory {
public:
    // Initialization and cleanup
    static bool Initialize();
    static void Shutdown();

    // Process handling
    static DWORD GetProcessID(const std::wstring& processName);
    static uintptr_t GetModuleBaseAddress(DWORD processID, const std::wstring& moduleName);
    static bool IsCSGORunning();
    
    // Getters for module addresses and handles
    static uintptr_t GetClientDLL() { return m_clientDLL; }
    static uintptr_t GetEngineDLL() { return m_engineDLL; }
    static HANDLE GetProcessHandle() { return m_hProcess; }
    static DWORD GetProcessID() { return m_processID; }
    
    // Memory operations with unique patterns to avoid detection
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

    template <typename T>
    static void Write(uintptr_t address, const T& value) {
        // Polymorphic write method
        if (reinterpret_cast<uintptr_t>(&value) % 4 == 0) {
            WriteProcessMemory(m_hProcess, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr);
        }
        else {
            // Alternative method
            *reinterpret_cast<T*>(address) = value;
        }
    }

    // Pattern scanning methods
    static uintptr_t FindPattern(const std::wstring& moduleName, const std::string& pattern, bool useCache = true);
    static uintptr_t FindPattern(uintptr_t regionStart, uintptr_t regionSize, const std::string& pattern);
    
    // Address operations
    static uintptr_t GetAbsoluteAddress(uintptr_t instructionAddress, int offset, int size);
    static uintptr_t ResolveRelativeAddress(uintptr_t address, int offset);

private:
    // Internal methods
    static bool PatternToBytes(const std::string& pattern, std::vector<int>& bytes);
    static bool CompareBytes(const uint8_t* data, const std::vector<int>& bytes);
    
    // Storage for module data to avoid repeated lookups
    struct ModuleInfo {
        uintptr_t baseAddress;
        uintptr_t size;
    };
    
    // Cache for pattern scanning results
    static std::unordered_map<std::string, uintptr_t> m_patternCache;
    
    // Module information cache
    static std::unordered_map<std::wstring, ModuleInfo> m_moduleCache;
    
    // Process handle
    static HANDLE m_hProcess;
    static DWORD m_processID;

    // CS:GO specific module names and handles
    static uintptr_t m_clientDLL;
    static uintptr_t m_engineDLL;
};

// Specialized template for reading strings safely
template<>
inline std::string Memory::Read<std::string>(uintptr_t address) {
    char buffer[1024] = { 0 };
    ReadProcessMemory(m_hProcess, reinterpret_cast<LPCVOID>(address), buffer, sizeof(buffer) - 1, nullptr);
    return std::string(buffer);
}
