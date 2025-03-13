#include "memory.h"
#include <algorithm>
#include <cctype>
#include <Psapi.h>

// Initialize static members
HANDLE Memory::m_hProcess = nullptr;
DWORD Memory::m_processID = 0;
uintptr_t Memory::m_clientDLL = 0;
uintptr_t Memory::m_engineDLL = 0;
std::unordered_map<std::string, uintptr_t> Memory::m_patternCache;
std::unordered_map<std::wstring, Memory::ModuleInfo> Memory::m_moduleCache;

bool Memory::Initialize() {
    // Find CS:GO process
    m_processID = GetProcessID(L"csgo.exe");
    if (!m_processID) {
        return false;
    }

    // Open process with required access rights
    // Using a combination of less suspicious access rights rather than PROCESS_ALL_ACCESS
    m_hProcess = OpenProcess(
        PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION,
        FALSE,
        m_processID
    );

    if (!m_hProcess) {
        return false;
    }

    // Get module base addresses using our stealthier method
    m_clientDLL = GetModuleBaseAddress(m_processID, L"client.dll");
    m_engineDLL = GetModuleBaseAddress(m_processID, L"engine.dll");

    if (!m_clientDLL || !m_engineDLL) {
        Shutdown();
        return false;
    }

    return true;
}

void Memory::Shutdown() {
    if (m_hProcess) {
        CloseHandle(m_hProcess);
        m_hProcess = nullptr;
    }

    m_processID = 0;
    m_clientDLL = 0;
    m_engineDLL = 0;
    m_patternCache.clear();
    m_moduleCache.clear();
}

DWORD Memory::GetProcessID(const std::wstring& processName) {
    DWORD processID = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);

        if (Process32FirstW(snapshot, &entry)) {
            do {
                if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                    processID = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snapshot, &entry));
        }

        CloseHandle(snapshot);
    }

    return processID;
}

uintptr_t Memory::GetModuleBaseAddress(DWORD processID, const std::wstring& moduleName) {
    // Check cache first
    auto it = m_moduleCache.find(moduleName);
    if (it != m_moduleCache.end()) {
        return it->second.baseAddress;
    }

    uintptr_t moduleBase = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);

    if (snapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32W entry;
        entry.dwSize = sizeof(entry);

        if (Module32FirstW(snapshot, &entry)) {
            do {
                if (_wcsicmp(entry.szModule, moduleName.c_str()) == 0) {
                    moduleBase = reinterpret_cast<uintptr_t>(entry.modBaseAddr);
                    
                    // Store in cache
                    ModuleInfo info;
                    info.baseAddress = moduleBase;
                    info.size = entry.modBaseSize;
                    m_moduleCache[moduleName] = info;
                    
                    break;
                }
            } while (Module32NextW(snapshot, &entry));
        }

        CloseHandle(snapshot);
    }

    return moduleBase;
}

bool Memory::PatternToBytes(const std::string& pattern, std::vector<int>& bytes) {
    bytes.clear();
    const char* start = pattern.c_str();
    const char* end = start + pattern.length();
    
    for (const char* current = start; current < end; ++current) {
        // Skip spaces
        if (std::isspace(*current)) {
            continue;
        }
        
        // Handle wildcards
        if (*current == '?') {
            // Skip the next character if another '?'
            if (current + 1 < end && *(current + 1) == '?') {
                current++;
            }
            // Add wildcard (-1)
            bytes.push_back(-1);
            continue;
        }
        
        // Parse hex byte
        if (current + 1 < end) {
            char byte[3] = { current[0], current[1], 0 };
            bytes.push_back(strtol(byte, nullptr, 16));
            current++;
        }
        else {
            // Incomplete byte at end of pattern
            return false;
        }
    }
    
    return !bytes.empty();
}

bool Memory::CompareBytes(const uint8_t* data, const std::vector<int>& bytes) {
    for (size_t i = 0; i < bytes.size(); i++) {
        // Skip wildcards
        if (bytes[i] == -1) {
            continue;
        }
        
        // Compare exact byte
        if (data[i] != bytes[i]) {
            return false;
        }
    }
    
    return true;
}

uintptr_t Memory::FindPattern(const std::wstring& moduleName, const std::string& pattern, bool useCache) {
    // Check cache first if enabled
    if (useCache) {
        std::string cacheKey = std::string(moduleName.begin(), moduleName.end()) + ":" + pattern;
        auto it = m_patternCache.find(cacheKey);
        if (it != m_patternCache.end()) {
            return it->second;
        }
    }
    
    // Get module information
    auto moduleIt = m_moduleCache.find(moduleName);
    if (moduleIt == m_moduleCache.end()) {
        // Module not cached, try to get it
        uintptr_t baseAddr = GetModuleBaseAddress(m_processID, moduleName);
        if (baseAddr == 0) {
            return 0;
        }
        
        // We don't have the module size yet, so get it using another method
        MODULEINFO moduleInfo;
        if (!GetModuleInformation(m_hProcess, (HMODULE)baseAddr, &moduleInfo, sizeof(moduleInfo))) {
            return 0;
        }
        
        ModuleInfo info;
        info.baseAddress = baseAddr;
        info.size = moduleInfo.SizeOfImage;
        m_moduleCache[moduleName] = info;
        
        moduleIt = m_moduleCache.find(moduleName);
    }
    
    // Find pattern within module
    uintptr_t result = FindPattern(moduleIt->second.baseAddress, moduleIt->second.size, pattern);
    
    // Cache the result if enabled
    if (useCache && result != 0) {
        std::string cacheKey = std::string(moduleName.begin(), moduleName.end()) + ":" + pattern;
        m_patternCache[cacheKey] = result;
    }
    
    return result;
}

uintptr_t Memory::FindPattern(uintptr_t regionStart, uintptr_t regionSize, const std::string& pattern) {
    std::vector<int> bytes;
    if (!PatternToBytes(pattern, bytes)) {
        return 0;
    }
    
    // Randomize search pattern to avoid detection
    // Sometimes start from beginning, sometimes from end
    bool searchFromEnd = (GetTickCount() % 2) == 0;
    
    // Adjust scanning chunk size randomly
    int chunkSize = 4096 + (GetTickCount() % 4096);
    
    // Allocate buffer for reading memory
    std::vector<uint8_t> buffer(chunkSize);
    
    // Calculate scanning parameters
    size_t patternSize = bytes.size();
    uintptr_t endAddress = regionStart + regionSize - patternSize;
    
    if (searchFromEnd) {
        // Search from end to beginning
        for (uintptr_t currentChunk = endAddress - chunkSize; currentChunk >= regionStart; currentChunk -= chunkSize) {
            SIZE_T bytesRead;
            uintptr_t readAddress = currentChunk;
            uintptr_t readSize = std::min<uintptr_t>(chunkSize, currentChunk - regionStart + chunkSize);
            
            // Read chunk of memory
            if (ReadProcessMemory(m_hProcess, (LPCVOID)readAddress, buffer.data(), readSize, &bytesRead) && bytesRead >= patternSize) {
                // Search chunk for pattern
                for (size_t i = 0; i <= bytesRead - patternSize; i++) {
                    if (CompareBytes(buffer.data() + i, bytes)) {
                        return readAddress + i;
                    }
                }
            }
            
            // Check if we've reached the beginning
            if (currentChunk < chunkSize) {
                break;
            }
        }
    }
    else {
        // Search from beginning to end
        for (uintptr_t currentChunk = regionStart; currentChunk < endAddress; currentChunk += chunkSize) {
            SIZE_T bytesRead;
            uintptr_t readSize = std::min<uintptr_t>(chunkSize, endAddress - currentChunk + patternSize);
            
            // Read chunk of memory
            if (ReadProcessMemory(m_hProcess, (LPCVOID)currentChunk, buffer.data(), readSize, &bytesRead) && bytesRead >= patternSize) {
                // Search chunk for pattern
                for (size_t i = 0; i <= bytesRead - patternSize; i++) {
                    if (CompareBytes(buffer.data() + i, bytes)) {
                        return currentChunk + i;
                    }
                }
            }
        }
    }
    
    return 0;
}

uintptr_t Memory::GetAbsoluteAddress(uintptr_t instructionAddress, int offset, int size) {
    if (!instructionAddress) {
        return 0;
    }
    
    // Read the relative address from the instruction
    int32_t relativeAddress = Read<int32_t>(instructionAddress + offset);
    
    // Calculate the absolute address
    uintptr_t absoluteAddress = instructionAddress + size + relativeAddress;
    
    return absoluteAddress;
}

uintptr_t Memory::ResolveRelativeAddress(uintptr_t address, int offset) {
    if (!address) {
        return 0;
    }
    
    int32_t relativeOffset = Read<int32_t>(address + offset);
    return address + offset + 4 + relativeOffset;
}

bool Memory::IsCSGORunning() {
    // If we already have a valid process handle and ID, check if it's still valid
    if (m_hProcess != nullptr && m_processID != 0) {
        DWORD exitCode = 0;
        if (GetExitCodeProcess(m_hProcess, &exitCode) && exitCode == STILL_ACTIVE) {
            return true;
        }
    }
    
    // Try to find the CS:GO process
    DWORD csgoProcessID = GetProcessID(L"csgo.exe");
    
    // Return true if found
    return csgoProcessID != 0;
}
