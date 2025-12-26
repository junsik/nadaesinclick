#ifndef HOTKEY_H
#define HOTKEY_H

#include <windows.h>

// Hotkey info structure
struct HotkeyInfo
{
    int id;
    UINT modifiers;
    UINT vkCode;
};

// Initialize hotkey module
void InitHotkeys(HWND hWnd);

// Register hotkeys with specified keys
bool RegisterAppHotkeys(HWND hWnd, UINT startKey, UINT stopKey);

// Unregister all hotkeys
void UnregisterAppHotkeys(HWND hWnd);

// Update hotkeys when combo selection changes
void UpdateHotkeys(HWND hWnd, UINT startKey, UINT stopKey);

// Get virtual key code from combo box index
UINT GetVKFromComboIndex(int index);

// Get combo box index from virtual key code
int GetComboIndexFromVK(UINT vkCode);

// Get hotkey name string
const wchar_t *GetHotkeyName(UINT vkCode);

#endif // HOTKEY_H
