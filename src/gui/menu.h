#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <memory>

// Forward declarations
class MenuItem;
class MenuCategory;
class Menu;

// Callback function type for menu items
using MenuCallback = std::function<void()>;

// Base class for all menu items
class MenuItem {
public:
    MenuItem(const std::string& name) : m_name(name), m_isVisible(true) {}
    virtual ~MenuItem() = default;
    
    // Get the name of this menu item
    const std::string& GetName() const { return m_name; }
    
    // Set visibility
    void SetVisible(bool visible) { m_isVisible = visible; }
    bool IsVisible() const { return m_isVisible; }
    
    // Render this menu item (to be implemented by derived classes)
    virtual void Render(int x, int y, bool isSelected) = 0;
    
    // Handle key press (to be implemented by derived classes)
    virtual bool HandleKey(int key) = 0;
    
    // Get the height of this menu item
    virtual int GetHeight() const { return 20; }
    
    // Get the width of this menu item
    virtual int GetWidth() const { return 200; }

protected:
    std::string m_name;
    bool m_isVisible;
};

// A simple button menu item
class ButtonMenuItem : public MenuItem {
public:
    ButtonMenuItem(const std::string& name, const MenuCallback& callback)
        : MenuItem(name), m_callback(callback) {}
    
    void Render(int x, int y, bool isSelected) override;
    bool HandleKey(int key) override;

private:
    MenuCallback m_callback;
};

// A toggle (checkbox) menu item
class ToggleMenuItem : public MenuItem {
public:
    ToggleMenuItem(const std::string& name, bool* value)
        : MenuItem(name), m_value(value) {}
    
    void Render(int x, int y, bool isSelected) override;
    bool HandleKey(int key) override;
    
    bool GetValue() const { return *m_value; }
    void SetValue(bool value) { *m_value = value; }
    void Toggle() { *m_value = !(*m_value); }
    
private:
    bool* m_value;
};

// A slider menu item for numeric values
class SliderMenuItem : public MenuItem {
public:
    // Integer slider
    SliderMenuItem(const std::string& name, int* value, int minValue, int maxValue, int step = 1)
        : MenuItem(name), m_intValue(value), m_floatValue(nullptr), m_min(minValue), m_max(maxValue), m_step(step), m_isFloat(false) {}
    
    // Float slider
    SliderMenuItem(const std::string& name, float* value, float minValue, float maxValue, float step = 0.1f)
        : MenuItem(name), m_intValue(nullptr), m_floatValue(value), m_minf(minValue), m_maxf(maxValue), m_stepf(step), m_isFloat(true) {}
    
    void Render(int x, int y, bool isSelected) override;
    bool HandleKey(int key) override;
    
    int GetHeight() const override { return 40; }

private:
    // Integer or float value pointer (only one is valid)
    int* m_intValue;
    float* m_floatValue;
    
    // Integer range
    int m_min;
    int m_max;
    int m_step;
    
    // Float range
    float m_minf;
    float m_maxf;
    float m_stepf;
    
    // Type flag
    bool m_isFloat;
};

// A key binding menu item
class KeyBindMenuItem : public MenuItem {
public:
    KeyBindMenuItem(const std::string& name, int* key)
        : MenuItem(name), m_key(key), m_isWaitingForKey(false) {}
    
    void Render(int x, int y, bool isSelected) override;
    bool HandleKey(int key) override;
    
    int GetKey() const { return *m_key; }
    void SetKey(int key) { *m_key = key; }
    
    // Convert key code to string representation
    static std::string KeyToString(int key);

private:
    int* m_key;
    bool m_isWaitingForKey;
};

// A submenu item that contains other menu items
class SubMenuItem : public MenuItem {
public:
    SubMenuItem(const std::string& name)
        : MenuItem(name), m_isOpen(false) {}
    
    void AddItem(std::shared_ptr<MenuItem> item) { m_items.push_back(item); }
    void Render(int x, int y, bool isSelected) override;
    bool HandleKey(int key) override;
    
    bool IsOpen() const { return m_isOpen; }
    void Toggle() { m_isOpen = !m_isOpen; }
    
    int GetHeight() const override;

private:
    std::vector<std::shared_ptr<MenuItem>> m_items;
    bool m_isOpen;
    int m_selectedIndex = 0;
};

// A category of menu items (top-level container)
class MenuCategory {
public:
    MenuCategory(const std::string& name) : m_name(name) {}
    
    const std::string& GetName() const { return m_name; }
    
    void AddItem(std::shared_ptr<MenuItem> item) { 
        m_items.push_back(item); 
    }
    
    void Render(int x, int y, bool isSelected);
    bool HandleKey(int key);
    
    int GetHeight() const;

private:
    std::string m_name;
    std::vector<std::shared_ptr<MenuItem>> m_items;
    int m_selectedIndex = 0;
};

// The main menu class
class Menu {
public:
    Menu();
    ~Menu();
    
    // Initialize the menu
    bool Initialize();
    
    // Add a category to the menu
    void AddCategory(std::shared_ptr<MenuCategory> category) { 
        m_categories.push_back(category); 
    }
    
    // Toggle menu visibility
    void Toggle() { m_isVisible = !m_isVisible; }
    bool IsVisible() const { return m_isVisible; }
    
    // Render the menu
    void Render();
    
    // Process input
    void ProcessInput();
    
    // Handle key press
    bool HandleKey(int key);

private:
    std::vector<std::shared_ptr<MenuCategory>> m_categories;
    bool m_isVisible;
    int m_selectedCategory;
    HWND m_targetWindow;
    WNDPROC m_originalWndProc;
    
    // Create the menu structure with categories and items
    void CreateMenuStructure();
    
    // Windows message procedure for keyboard input
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
