#include "menu.h"
#include "../features/aimbot.h"
#include "../features/triggerbot.h"
#include "../features/bunnyhop.h"
#include "../core/memory.h"
#include <Windows.h>
#include <iostream>
#include <string>

// Global menu instance
Menu* g_pMenu = nullptr;

// Menu navigation keys
const int KEY_UP = VK_UP;
const int KEY_DOWN = VK_DOWN;
const int KEY_LEFT = VK_LEFT;
const int KEY_RIGHT = VK_RIGHT;
const int KEY_SELECT = VK_RETURN;
const int KEY_BACK = VK_ESCAPE;
const int KEY_TOGGLE_MENU = VK_INSERT;

// Menu item implementations
void ButtonMenuItem::Render(int x, int y, bool isSelected) {
    if (!m_isVisible) return;
    
    std::cout << (isSelected ? "→ " : "  ") << m_name << std::endl;
}

bool ButtonMenuItem::HandleKey(int key) {
    if (key == KEY_SELECT) {
        m_callback();
        return true;
    }
    return false;
}

void ToggleMenuItem::Render(int x, int y, bool isSelected) {
    if (!m_isVisible) return;
    
    std::cout << (isSelected ? "→ " : "  ") << m_name << ": " 
              << (*m_value ? "ON" : "OFF") << std::endl;
}

bool ToggleMenuItem::HandleKey(int key) {
    if (key == KEY_SELECT || key == KEY_RIGHT || key == KEY_LEFT) {
        Toggle();
        return true;
    }
    return false;
}

void SliderMenuItem::Render(int x, int y, bool isSelected) {
    if (!m_isVisible) return;
    
    std::string valueText;
    
    if (m_isFloat) {
        valueText = std::to_string(*m_floatValue);
        // Truncate to 2 decimal places
        valueText = valueText.substr(0, valueText.find(".") + 3);
    } else {
        valueText = std::to_string(*m_intValue);
    }
    
    std::cout << (isSelected ? "→ " : "  ") << m_name << ": " << valueText << std::endl;
}

bool SliderMenuItem::HandleKey(int key) {
    if (m_isFloat) {
        if (key == KEY_LEFT) {
            *m_floatValue -= m_stepf;
            if (*m_floatValue < m_minf) *m_floatValue = m_minf;
            return true;
        } else if (key == KEY_RIGHT) {
            *m_floatValue += m_stepf;
            if (*m_floatValue > m_maxf) *m_floatValue = m_maxf;
            return true;
        }
    } else {
        if (key == KEY_LEFT) {
            *m_intValue -= m_step;
            if (*m_intValue < m_min) *m_intValue = m_min;
            return true;
        } else if (key == KEY_RIGHT) {
            *m_intValue += m_step;
            if (*m_intValue > m_max) *m_intValue = m_max;
            return true;
        }
    }
    return false;
}

std::string KeyBindMenuItem::KeyToString(int key) {
    static const std::unordered_map<int, std::string> keyNames = {
        { VK_LBUTTON, "Left Mouse" },
        { VK_RBUTTON, "Right Mouse" },
        { VK_MBUTTON, "Middle Mouse" },
        { VK_BACK, "Backspace" },
        { VK_TAB, "Tab" },
        { VK_RETURN, "Enter" },
        { VK_SHIFT, "Shift" },
        { VK_CONTROL, "Ctrl" },
        { VK_MENU, "Alt" },
        { VK_ESCAPE, "Escape" },
        { VK_SPACE, "Space" },
        { VK_INSERT, "Insert" },
        { VK_DELETE, "Delete" },
        { VK_F1, "F1" },
        { VK_F2, "F2" },
        { VK_F3, "F3" }
    };
    
    // For letters and numbers
    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) {
        return std::string(1, (char)key);
    }
    
    // Look up in map
    auto it = keyNames.find(key);
    if (it != keyNames.end()) {
        return it->second;
    }
    
    // Default
    return "Key " + std::to_string(key);
}

void KeyBindMenuItem::Render(int x, int y, bool isSelected) {
    if (!m_isVisible) return;
    
    std::cout << (isSelected ? "→ " : "  ") << m_name << ": " 
              << (m_isWaitingForKey ? "Press Key..." : KeyToString(*m_key)) << std::endl;
}

bool KeyBindMenuItem::HandleKey(int key) {
    if (m_isWaitingForKey) {
        // Cancel with escape
        if (key == VK_ESCAPE) {
            m_isWaitingForKey = false;
            return true;
        }
        
        // Set new key binding
        *m_key = key;
        m_isWaitingForKey = false;
        return true;
    } else {
        if (key == KEY_SELECT) {
            m_isWaitingForKey = true;
            return true;
        }
    }
    
    return false;
}

int SubMenuItem::GetHeight() const {
    if (!m_isVisible) return 0;
    
    int height = MenuItem::GetHeight(); // Base height
    
    if (m_isOpen) {
        for (const auto& item : m_items) {
            if (item->IsVisible()) {
                height += item->GetHeight();
            }
        }
    }
    
    return height;
}

void SubMenuItem::Render(int x, int y, bool isSelected) {
    if (!m_isVisible) return;
    
    std::cout << (isSelected ? "→ " : "  ") << m_name 
              << (m_isOpen ? " [-]" : " [+]") << std::endl;
    
    if (m_isOpen) {
        for (size_t i = 0; i < m_items.size(); i++) {
            if (m_items[i]->IsVisible()) {
                m_items[i]->Render(x + 2, y, i == m_selectedIndex && isSelected);
            }
        }
    }
}

bool SubMenuItem::HandleKey(int key) {
    if (key == KEY_SELECT) {
        m_isOpen = !m_isOpen;
        return true;
    }
    
    if (m_isOpen) {
        if (key == KEY_UP && m_selectedIndex > 0) {
            m_selectedIndex--;
            return true;
        } else if (key == KEY_DOWN && m_selectedIndex < static_cast<int>(m_items.size()) - 1) {
            m_selectedIndex++;
            return true;
        } else if (key == KEY_LEFT || key == KEY_RIGHT) {
            if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
                return m_items[m_selectedIndex]->HandleKey(key);
            }
        } else if (key == KEY_SELECT || key == KEY_BACK) {
            if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
                return m_items[m_selectedIndex]->HandleKey(key);
            }
        }
    }
    
    return false;
}

int MenuCategory::GetHeight() const {
    int height = 1; // Category header height
    
    for (const auto& item : m_items) {
        if (item->IsVisible()) {
            height += item->GetHeight();
        }
    }
    
    return height;
}

void MenuCategory::Render(int x, int y, bool isSelected) {
    // Draw category header
    std::cout << std::string(50, '-') << std::endl;
    std::cout << (isSelected ? ">" : " ") << " " << m_name << std::endl;
    std::cout << std::string(50, '-') << std::endl;
    
    // Draw items
    for (size_t i = 0; i < m_items.size(); i++) {
        if (m_items[i]->IsVisible()) {
            m_items[i]->Render(x, y, i == m_selectedIndex && isSelected);
        }
    }
}

bool MenuCategory::HandleKey(int key) {
    if (key == KEY_UP && m_selectedIndex > 0) {
        m_selectedIndex--;
        return true;
    } else if (key == KEY_DOWN && m_selectedIndex < static_cast<int>(m_items.size()) - 1) {
        m_selectedIndex++;
        return true;
    } else if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_SELECT || key == KEY_BACK) {
        if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_items.size())) {
            return m_items[m_selectedIndex]->HandleKey(key);
        }
    }
    
    return false;
}

Menu::Menu() 
    : m_isVisible(false), m_selectedCategory(0), m_targetWindow(NULL), m_originalWndProc(NULL) {
    g_pMenu = this;
}

Menu::~Menu() {
    // Restore original window procedure
    if (m_targetWindow && m_originalWndProc) {
        SetWindowLongPtr(m_targetWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_originalWndProc));
    }
}

bool Menu::Initialize() {
    // Find CS:GO window
    m_targetWindow = FindWindow(NULL, L"Counter-Strike: Global Offensive");
    if (!m_targetWindow) {
        std::cout << "CS:GO window not found. Using simplified menu." << std::endl;
    }
    
    // Create menu structure
    CreateMenuStructure();
    
    return true;
}

LRESULT CALLBACK Menu::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // If menu is visible, handle keyboard input
    if (g_pMenu && g_pMenu->IsVisible() && msg == WM_KEYDOWN) {
        if (g_pMenu->HandleKey(static_cast<int>(wParam))) {
            return 0; // Handled
        }
    }
    
    // Toggle menu on key press
    if (msg == WM_KEYDOWN && wParam == KEY_TOGGLE_MENU) {
        if (g_pMenu) {
            g_pMenu->Toggle();
            return 0; // Handled
        }
    }
    
    // Pass to original window procedure
    return CallWindowProc(g_pMenu->m_originalWndProc, hwnd, msg, wParam, lParam);
}

bool Menu::HandleKey(int key) {
    // Handle category navigation
    if (key == KEY_LEFT) {
        if (m_selectedCategory > 0) {
            m_selectedCategory--;
            return true;
        }
    } else if (key == KEY_RIGHT) {
        if (m_selectedCategory < static_cast<int>(m_categories.size()) - 1) {
            m_selectedCategory++;
            return true;
        }
    } else if (key == KEY_BACK) {
        // Hide menu on escape
        m_isVisible = false;
        return true;
    } else {
        // Pass key to current category
        if (m_selectedCategory >= 0 && m_selectedCategory < static_cast<int>(m_categories.size())) {
            return m_categories[m_selectedCategory]->HandleKey(key);
        }
    }
    
    return false;
}

void Menu::Render() {
    if (!m_isVisible)
        return;
    
    system("cls");
    
    std::cout << "============= CS:GO Cheat Menu =============" << std::endl;
    std::cout << std::endl;
    
    // Draw categories
    for (size_t i = 0; i < m_categories.size(); i++) {
        if (i == m_selectedCategory) {
            m_categories[i]->Render(2, 0, true);
        }
    }
    
    std::cout << std::endl;
    std::cout << "Navigation: Arrow Keys, Enter, Escape" << std::endl;
    std::cout << "============================================" << std::endl;
}

void Menu::CreateMenuStructure() {
    // Create categories
    auto aimCategory = std::make_shared<MenuCategory>("Aimbot");
    auto triggerCategory = std::make_shared<MenuCategory>("Triggerbot");
    auto bhopCategory = std::make_shared<MenuCategory>("Bunnyhop");
    auto miscCategory = std::make_shared<MenuCategory>("Misc");
    
    // Test values since we can't access private members directly
    static bool aimbotEnabled = false;
    static int aimbotKey = VK_LBUTTON;
    static float aimbotFov = 5.0f;
    static float aimbotSmoothing = 5.0f;
    static bool recoilEnabled = true;
    static float recoilStrength = 1.0f;
    static float humanizationAmount = 0.3f;
    static int targetBone = 8; // HEAD
    static bool teamCheck = true;
    static bool visibilityCheck = true;
    static bool autoFire = false;
    
    // Triggerbot settings
    static bool triggerbotEnabled = false;
    static int triggerbotKey = VK_MENU; // ALT key
    static int activationMode = 0;
    static int minDelay = 50; 
    static int maxDelay = 150;
    static bool healthCheck = true;
    
    // Bunnyhop settings
    static bool bhopEnabled = false;
    static int successRate = 70;
    static int timingOffset = 20;
    static bool autoStrafe = true;
    
    // Aimbot items
    aimCategory->AddItem(std::make_shared<ToggleMenuItem>("Enable Aimbot", &aimbotEnabled));
    aimCategory->AddItem(std::make_shared<KeyBindMenuItem>("Activation Key", &aimbotKey));
    aimCategory->AddItem(std::make_shared<SliderMenuItem>("FOV", &aimbotFov, 1.0f, 30.0f, 0.5f));
    aimCategory->AddItem(std::make_shared<SliderMenuItem>("Smoothing", &aimbotSmoothing, 1.0f, 20.0f, 0.5f));
    aimCategory->AddItem(std::make_shared<ToggleMenuItem>("Recoil Control", &recoilEnabled));
    aimCategory->AddItem(std::make_shared<SliderMenuItem>("Recoil Strength", &recoilStrength, 0.0f, 2.0f, 0.1f));
    aimCategory->AddItem(std::make_shared<SliderMenuItem>("Humanization", &humanizationAmount, 0.0f, 1.0f, 0.05f));
    
    // Create target bone submenu
    auto boneSubmenu = std::make_shared<SubMenuItem>("Target Bone");
    auto setBoneHead = [&](){ targetBone = 8; }; // HEAD
    auto setBoneNeck = [&](){ targetBone = 7; }; // NECK
    auto setBoneChest = [&](){ targetBone = 5; }; // CHEST
    auto setBoneStomach = [&](){ targetBone = 3; }; // STOMACH
    boneSubmenu->AddItem(std::make_shared<ButtonMenuItem>("Head", setBoneHead));
    boneSubmenu->AddItem(std::make_shared<ButtonMenuItem>("Neck", setBoneNeck));
    boneSubmenu->AddItem(std::make_shared<ButtonMenuItem>("Chest", setBoneChest));
    boneSubmenu->AddItem(std::make_shared<ButtonMenuItem>("Stomach", setBoneStomach));
    aimCategory->AddItem(boneSubmenu);
    
    // More aimbot options
    aimCategory->AddItem(std::make_shared<ToggleMenuItem>("Team Check", &teamCheck));
    aimCategory->AddItem(std::make_shared<ToggleMenuItem>("Visibility Check", &visibilityCheck));
    aimCategory->AddItem(std::make_shared<ToggleMenuItem>("Auto Fire", &autoFire));
    
    // Triggerbot items
    triggerCategory->AddItem(std::make_shared<ToggleMenuItem>("Enable Triggerbot", &triggerbotEnabled));
    triggerCategory->AddItem(std::make_shared<KeyBindMenuItem>("Activation Key", &triggerbotKey));
    
    // Create activation mode submenu
    auto modeSubmenu = std::make_shared<SubMenuItem>("Activation Mode");
    auto setModeHold = [&](){ activationMode = 0; };
    auto setModeToggle = [&](){ activationMode = 1; };
    auto setModeAlways = [&](){ activationMode = 2; };
    modeSubmenu->AddItem(std::make_shared<ButtonMenuItem>("Hold Key", setModeHold));
    modeSubmenu->AddItem(std::make_shared<ButtonMenuItem>("Toggle", setModeToggle));
    modeSubmenu->AddItem(std::make_shared<ButtonMenuItem>("Always On", setModeAlways));
    triggerCategory->AddItem(modeSubmenu);
    
    // More triggerbot options
    triggerCategory->AddItem(std::make_shared<SliderMenuItem>("Min Delay (ms)", &minDelay, 0, 500, 10));
    triggerCategory->AddItem(std::make_shared<SliderMenuItem>("Max Delay (ms)", &maxDelay, 0, 500, 10));
    triggerCategory->AddItem(std::make_shared<ToggleMenuItem>("Team Check", &teamCheck));
    triggerCategory->AddItem(std::make_shared<ToggleMenuItem>("Health Check", &healthCheck));
    
    // Bunnyhop items
    bhopCategory->AddItem(std::make_shared<ToggleMenuItem>("Enable Bunnyhop", &bhopEnabled));
    bhopCategory->AddItem(std::make_shared<SliderMenuItem>("Success Rate (%)", &successRate, 0, 100, 5));
    bhopCategory->AddItem(std::make_shared<SliderMenuItem>("Timing Offset (ms)", &timingOffset, 0, 50, 5));
    bhopCategory->AddItem(std::make_shared<ToggleMenuItem>("Auto-Strafe", &autoStrafe));
    
    // Misc items
    auto restartOffsets = [](){
        // Refresh pattern scanning
        Memory::Initialize();
        // Alert user
        MessageBoxA(NULL, "Offsets refreshed!", "CS:GO Cheat", MB_OK | MB_ICONINFORMATION);
    };
    
    miscCategory->AddItem(std::make_shared<ButtonMenuItem>("Refresh Offsets", restartOffsets));
    
    // Add categories to menu
    AddCategory(aimCategory);
    AddCategory(triggerCategory);
    AddCategory(bhopCategory);
    AddCategory(miscCategory);
}

void Menu::ProcessInput() {
    // Check for toggle menu key
    static bool lastKeyState = false;
    bool currentKeyState = (GetAsyncKeyState(KEY_TOGGLE_MENU) & 0x8000) != 0;
    
    if (currentKeyState && !lastKeyState) {
        Toggle();
    }
    
    lastKeyState = currentKeyState;
}
