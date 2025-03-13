#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#include "core/memory.h"
#include "core/pattern_scan.h"
#include "features/aimbot.h"
#include "features/triggerbot.h"
#include "features/bunnyhop.h"
#include "gui/menu.h"

// Function prototypes
bool SetupConsole();
void CleanupConsole();
void PrintBanner();
void MainLoop();
void CheckKeyToggles();

// Global variables
HANDLE g_hConsole = NULL;
bool g_running = true;
std::unique_ptr<Menu> g_menu;

// Main entry point
int main(int argc, char* argv[]) {
    // Initialize memory and pattern scanning
    if (!Memory::Initialize()) {
        MessageBoxA(NULL, "Failed to initialize memory access.\nMake sure CS:GO is running and try again.", "Error", MB_ICONERROR);
        return 1;
    }

    // Try to setup the console, but continue even if it fails
    bool consoleAvailable = SetupConsole();
    
    // Always show a message box with instructions in case console fails
    char infoMsg[256];
    sprintf_s(infoMsg, "CS:GO Cheat loaded successfully!\n\nCS:GO Process ID: %lu\n\nHotkeys:\nF1: Toggle Aimbot\nF2: Toggle Triggerbot\nF3: Toggle Bunnyhop\nINSERT: Toggle Menu\nEND: Exit", 
              Memory::GetProcessID());
    MessageBoxA(NULL, infoMsg, "CS:GO Cheat Activated", MB_ICONINFORMATION);
    
    if (consoleAvailable) {
        PrintBanner();
        std::cout << "[INFO] Memory initialized successfully." << std::endl;
        std::cout << "[INFO] CS:GO process ID: " << Memory::GetProcessID() << std::endl;
    }

    // Initialize features
    Aimbot::Initialize();
    TriggerBot::Initialize();
    BunnyHop::Initialize();
    
    // Initialize menu system
    g_menu = std::make_unique<Menu>();
    if (!g_menu->Initialize()) {
        MessageBoxA(NULL, "Failed to initialize menu system.\nThe cheat will run without the in-game menu.", "Warning", MB_ICONWARNING);
#ifdef _DEBUG
        std::cout << "[ERROR] Failed to initialize menu system." << std::endl;
#endif
    } else {
#ifdef _DEBUG
        std::cout << "[INFO] Menu system initialized successfully." << std::endl;
#endif
    }

    // Main loop
    MainLoop();

    // Cleanup
    Memory::Shutdown();
    
    // Keep console window open when finished
    std::cout << std::endl << "Cheat has been closed. Press Enter to exit..." << std::endl;
    
    // Make sure we flush the output
    std::cout.flush();
    
    // Ensure we wait for user input before closing
    // Try multiple approaches to handle input:
    try {
        // Get a character from stdin (blocks until input is available)
        std::cin.clear();  // Clear any error flags
        int c = std::cin.get();
        
        // Alternative method if std::cin fails
        if (std::cin.fail()) {
            // Try using Windows API directly
            HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
            if (hStdin != INVALID_HANDLE_VALUE) {
                DWORD numRead;
                char buffer[2];
                ReadConsole(hStdin, buffer, 1, &numRead, NULL);
            }
        }
    }
    catch (...) {
        // If all approaches fail, at least add a sleep to keep window open
        Sleep(5000); // Sleep for 5 seconds
    }
    
    // Final message before exit
    std::cout << "Exiting..." << std::endl;

    return 0;
}

bool SetupConsole() {
    // Check if we're already attached to a console
    HWND consoleWnd = GetConsoleWindow();
    if (consoleWnd != NULL) {
        // We already have a console, just use it
        std::cout << "Using existing console window" << std::endl;
    } else {
        // Try to create a new console
        if (!AllocConsole()) {
            // Get the error code
            DWORD error = GetLastError();
            char errorMsg[256];
            sprintf_s(errorMsg, "Failed to allocate console. Error code: %lu\nFalling back to message boxes for status updates.", error);
            MessageBoxA(NULL, errorMsg, "Console Error", MB_ICONWARNING);
            
            // If we can't allocate a console, we'll use message boxes instead
            MessageBoxA(NULL, "CS:GO Cheat is running!\n\nHotkeys:\nF1: Toggle Aimbot\nF2: Toggle Triggerbot\nF3: Toggle Bunnyhop\nINSERT: Toggle Menu\nEND: Exit", "CS:GO Cheat Active", MB_ICONINFORMATION);
            return false;
        }
    }

    // Set console mode to allow input processing
    HANDLE hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hConsoleInput != INVALID_HANDLE_VALUE) {
        DWORD mode;
        if (GetConsoleMode(hConsoleInput, &mode)) {
            // Enable line input and echo input
            mode |= ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT;
            SetConsoleMode(hConsoleInput, mode);
        }
    }

    // Try to redirect std streams
    bool redirectSuccess = true;
    FILE* fpstdin = nullptr, *fpstdout = nullptr, *fpstderr = nullptr;
    
    if (freopen_s(&fpstdin, "CONIN$", "r", stdin) != 0) {
        redirectSuccess = false;
    }
    
    if (freopen_s(&fpstdout, "CONOUT$", "w", stdout) != 0) {
        redirectSuccess = false;
    }
    
    if (freopen_s(&fpstderr, "CONOUT$", "w", stderr) != 0) {
        redirectSuccess = false;
    }

    if (!redirectSuccess) {
        MessageBoxA(NULL, "Failed to redirect standard streams.\nSome console output may not be visible.", "Console Warning", MB_ICONWARNING);
    }

    // Get handle to console output
    g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (g_hConsole == INVALID_HANDLE_VALUE || g_hConsole == NULL) {
        MessageBoxA(NULL, "Failed to get console handle", "Console Warning", MB_ICONWARNING);
    }
    
    // Set console title
    if (!SetConsoleTitleA("CS:GO Cheat - Control Panel")) {
        // Non-critical error, just continue
    }

    // Write to console using different methods to ensure visibility
    std::cout << "Console initialized successfully!" << std::endl;
    printf("CS:GO Cheat is running!\n");
    
    // Use direct Windows API as a fallback if needed
    if (g_hConsole != INVALID_HANDLE_VALUE && g_hConsole != NULL) {
        const char* msg = "Console output is working.\r\n";
        DWORD written = 0;
        WriteConsoleA(g_hConsole, msg, strlen(msg), &written, NULL);
    }

    return true;
}

void CleanupConsole() {
    // Free the console
    FreeConsole();
}

void PrintBanner() {
    std::cout << "=====================================================" << std::endl;
    std::cout << "            CS:GO Cheat - Debug Console              " << std::endl;
    std::cout << "=====================================================" << std::endl;
    std::cout << " * Pattern scanning for automatic offsets updates    " << std::endl;
    std::cout << " * Humanized aimbot with recoil control             " << std::endl;
    std::cout << " * Triggerbot with customizable delay               " << std::endl;
    std::cout << " * Bunnyhop with configurable success rate          " << std::endl;
    std::cout << "=====================================================" << std::endl;
    std::cout << std::endl;
}

void MainLoop() {
    using clock = std::chrono::high_resolution_clock;
    auto lastTime = clock::now();

    // Status reporting intervals (in milliseconds)
    const int STATUS_INTERVAL = 1000;          // Console status updates every 1 second 
    const int MSGBOX_STATUS_INTERVAL = 30000;  // Message box status updates every 30 seconds
    
    auto lastStatusTime = clock::now();
    auto lastMsgBoxTime = clock::now();

    // Track status changes for features
    bool prevAimbotStatus = false;
    bool prevTriggerbotStatus = false;
    bool prevBunnyhopStatus = false;

    // Frame timing
    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1000.0f / TARGET_FPS;

    // Let user know the cheat is running
    MessageBoxA(NULL, "CS:GO Cheat is now running and active!\n\nUse hotkeys to toggle features.", "CS:GO Cheat", MB_ICONINFORMATION | MB_SYSTEMMODAL);

    while (g_running) {
        // Current time
        auto currentTime = clock::now();
        float deltaTime = std::chrono::duration<float, std::milli>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Check for feature toggles
        CheckKeyToggles();

        // Check if any feature status changed
        bool statusChanged = (prevAimbotStatus != Aimbot::IsEnabled() || 
                             prevTriggerbotStatus != TriggerBot::IsEnabled() || 
                             prevBunnyhopStatus != BunnyHop::IsEnabled());

        // Update previous status
        prevAimbotStatus = Aimbot::IsEnabled();
        prevTriggerbotStatus = TriggerBot::IsEnabled();
        prevBunnyhopStatus = BunnyHop::IsEnabled();

        // Process menu input
        if (g_menu) {
            g_menu->ProcessInput();
        }

        // Run active features only if menu is not visible
        if (!g_menu || !g_menu->IsVisible()) {
            if (Aimbot::IsEnabled()) {
                Aimbot::Run();
            }

            if (TriggerBot::IsEnabled()) {
                TriggerBot::Run();
            }

            if (BunnyHop::IsEnabled()) {
                BunnyHop::Run();
            }
        }

        // Render menu
        if (g_menu) {
            g_menu->Render();
        }

        // Update status in console if possible
        auto statusDuration = std::chrono::duration<float, std::milli>(currentTime - lastStatusTime).count();
        if (statusDuration > STATUS_INTERVAL) {
            lastStatusTime = currentTime;
            
            // Try to print to console (will be ignored if console isn't available)
            try {
                // Clear current line
                std::cout << "\r                                                                               \r";
                
                // Print feature status
                std::cout << "Aimbot: " << (Aimbot::IsEnabled() ? "ON" : "OFF") << " | ";
                std::cout << "Triggerbot: " << (TriggerBot::IsEnabled() ? "ON" : "OFF") << " | ";
                std::cout << "Bunnyhop: " << (BunnyHop::IsEnabled() ? "ON" : "OFF") << " | ";
                std::cout << "Menu: " << (g_menu && g_menu->IsVisible() ? "VISIBLE" : "HIDDEN");
                
                std::cout.flush();
            } catch (...) {
                // Silently ignore errors if console output fails
            }
        }
        
        // Show message box on status change or periodically
        auto msgBoxDuration = std::chrono::duration<float, std::milli>(currentTime - lastMsgBoxTime).count();
        if (statusChanged || msgBoxDuration > MSGBOX_STATUS_INTERVAL) {
            lastMsgBoxTime = currentTime;
            
            // Only show message box on status change to avoid disrupting gameplay
            if (statusChanged) {
                // Prepare message with current status
                char statusMsg[256];
                sprintf_s(statusMsg, "Feature Status:\n\nAimbot: %s\nTriggerbot: %s\nBunnyhop: %s", 
                          Aimbot::IsEnabled() ? "ON" : "OFF",
                          TriggerBot::IsEnabled() ? "ON" : "OFF",
                          BunnyHop::IsEnabled() ? "ON" : "OFF");
                
                // Show non-blocking message box (MB_SERVICE_NOTIFICATION)
                // This won't interrupt gameplay as much
                MessageBoxA(NULL, statusMsg, "CS:GO Cheat Status", MB_ICONINFORMATION | MB_SYSTEMMODAL);
            }
        }

        // Frame rate limiting
        auto frameEndTime = clock::now();
        float frameTime = std::chrono::duration<float, std::milli>(frameEndTime - currentTime).count();
        if (frameTime < FRAME_TIME) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(FRAME_TIME - frameTime)));
        } else {
            // If frame took too long, still give the CPU a small break
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // We're no longer checking if CS:GO is running
        // The cheat will keep running until the user presses END key
        // This allows launching CS:GO after the cheat is already running
    }
}

void CheckKeyToggles() {
    // Check for feature toggle keys with a simple debounce
    static bool aimbotKeyPressed = false;
    static bool triggerbotKeyPressed = false;
    static bool bhopKeyPressed = false;
    static bool exitKeyPressed = false;

    // Toggle aimbot with F1
    if (GetAsyncKeyState(VK_F1) & 0x8000) {
        if (!aimbotKeyPressed) {
            Aimbot::SetEnabled(!Aimbot::IsEnabled());
            aimbotKeyPressed = true;
        }
    } else {
        aimbotKeyPressed = false;
    }

    // Toggle triggerbot with F2
    if (GetAsyncKeyState(VK_F2) & 0x8000) {
        if (!triggerbotKeyPressed) {
            TriggerBot::SetEnabled(!TriggerBot::IsEnabled());
            triggerbotKeyPressed = true;
        }
    } else {
        triggerbotKeyPressed = false;
    }

    // Toggle bunnyhop with F3
    if (GetAsyncKeyState(VK_F3) & 0x8000) {
        if (!bhopKeyPressed) {
            BunnyHop::SetEnabled(!BunnyHop::IsEnabled());
            bhopKeyPressed = true;
        }
    } else {
        bhopKeyPressed = false;
    }

    // Exit with END key
    if (GetAsyncKeyState(VK_END) & 0x8000) {
        if (!exitKeyPressed) {
            g_running = false;
            exitKeyPressed = true;
        }
    } else {
        exitKeyPressed = false;
    }
}
