#include "hotkey.h"
#include "resource.h"

// Virtual key codes for F1-F12
static const UINT g_vkCodes[] = {
    VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6,
    VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12
};

// Key names
static const wchar_t *g_keyNames[] = {
    L"F1", L"F2", L"F3", L"F4", L"F5", L"F6",
    L"F7", L"F8", L"F9", L"F10", L"F11", L"F12"
};

static bool g_hotkeysRegistered = false;

void
InitHotkeys(HWND hWnd)
{
    (void)hWnd;
    g_hotkeysRegistered = false;
}

bool
RegisterAppHotkeys(HWND hWnd, UINT startKey, UINT stopKey)
{
    if (g_hotkeysRegistered)
    {
        UnregisterAppHotkeys(hWnd);
    }

    bool success = true;

    if (!RegisterHotKey(hWnd, HOTKEY_ID_START, 0, startKey))
    {
        success = false;
    }

    if (!RegisterHotKey(hWnd, HOTKEY_ID_STOP, 0, stopKey))
    {
        success = false;
    }

    g_hotkeysRegistered = success;
    return success;
}

void
UnregisterAppHotkeys(HWND hWnd)
{
    UnregisterHotKey(hWnd, HOTKEY_ID_START);
    UnregisterHotKey(hWnd, HOTKEY_ID_STOP);
    g_hotkeysRegistered = false;
}

void
UpdateHotkeys(HWND hWnd, UINT startKey, UINT stopKey)
{
    RegisterAppHotkeys(hWnd, startKey, stopKey);
}

UINT
GetVKFromComboIndex(int index)
{
    if (index >= 0 && index < 12)
    {
        return g_vkCodes[index];
    }
    return VK_F5;
}

int
GetComboIndexFromVK(UINT vkCode)
{
    for (int i = 0; i < 12; i++)
    {
        if (g_vkCodes[i] == vkCode)
        {
            return i;
        }
    }
    return 4;
}

const wchar_t *
GetHotkeyName(UINT vkCode)
{
    for (int i = 0; i < 12; i++)
    {
        if (g_vkCodes[i] == vkCode)
        {
            return g_keyNames[i];
        }
    }
    return L"Unknown";
}
