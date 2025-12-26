#ifndef CLICKER_H
#define CLICKER_H

#include <windows.h>

// Key flags for keyboard input
#define KEY_1       0x0001
#define KEY_2       0x0002
#define KEY_3       0x0004
#define KEY_4       0x0008
#define KEY_ENTER   0x0010
#define KEY_SPACE   0x0020
#define KEY_ESC     0x0040
#define KEY_A       0x0080
#define KEY_Q       0x0100
#define KEY_UP      0x0200
#define KEY_DOWN    0x0400
#define KEY_LEFT    0x0800
#define KEY_RIGHT   0x1000

#define MAX_CUSTOM_KEYS 4

struct CustomKey
{
    bool enabled;
    WORD vkCode;
};

struct ClickerConfig
{
    // Hotkeys
    UINT startKey;
    UINT stopKey;

    // Mouse
    bool leftClick;
    bool rightClick;

    // Keyboard (bit flags)
    DWORD keyFlags;

    // Custom Keys
    CustomKey customKeys[MAX_CUSTOM_KEYS];

    // Speed
    int clicksPerSecond;
};

// Initialize clicker module
void InitClicker();

// Click at current mouse position
void ClickAtCurrentPos(bool leftClick);

// Send keyboard input
void SendKey(WORD vkCode);

// Perform all configured actions
void PerformActions(const ClickerConfig *config);

// Start/Stop clicking
void StartClicking(HWND hWnd, const ClickerConfig *config);
void StopClicking(HWND hWnd);

// Check if clicker is running
bool IsClickerRunning();

// Clean up resources
void CleanupClicker();

#endif // CLICKER_H
