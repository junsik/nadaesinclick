#include "clicker.h"
#include "resource.h"
#include <process.h>
#include <timeapi.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

// Global state
static bool g_isRunning = false;
static HANDLE g_hThread = nullptr;
static HANDLE g_hStopEvent = nullptr;
static ClickerConfig g_currentConfig = {};
static HWND g_hMainWnd = nullptr;
static int g_rotationIndex = 0;
static DWORD g_startTime = 0;

void
InitClicker()
{
    timeBeginPeriod(1);
    g_hStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

void
CleanupClicker()
{
    if (g_hStopEvent)
    {
        CloseHandle(g_hStopEvent);
        g_hStopEvent = nullptr;
    }
    timeEndPeriod(1);
}

void
ClickAtCurrentPos(bool leftClick)
{
    INPUT input = {};
    input.type = INPUT_MOUSE;

    if (leftClick)
    {
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
    }
    else
    {
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
    }

    SendInput(1, &input, sizeof(INPUT));
}

void
SendKey(WORD vkCode)
{
    INPUT inputs[2] = {};

    // Use scan code to bypass IME
    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC);

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vkCode;
    inputs[0].ki.wScan = (WORD)scanCode;
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vkCode;
    inputs[1].ki.wScan = (WORD)scanCode;
    inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

// Key mapping table for rotation mode
struct KeyMapping
{
    DWORD flag;
    WORD vkCode;
};

static const KeyMapping g_keyMappings[] = {
    {KEY_1, 0x31}, {KEY_2, 0x32}, {KEY_3, 0x33}, {KEY_4, 0x34},
    {KEY_Q, 0x51}, {KEY_W, 0x57}, {KEY_E, 0x45}, {KEY_R, 0x52}, {KEY_T, 0x54},
    {KEY_ENTER, VK_RETURN}, {KEY_SPACE, VK_SPACE}, {KEY_ESC, VK_ESCAPE},
    {KEY_UP, VK_UP}, {KEY_DOWN, VK_DOWN}, {KEY_LEFT, VK_LEFT}, {KEY_RIGHT, VK_RIGHT}
};
static const int g_numKeyMappings = sizeof(g_keyMappings) / sizeof(g_keyMappings[0]);

static void
GetActiveKeys(const ClickerConfig *config, WORD *keys, int *count)
{
    *count = 0;

    for (int i = 0; i < g_numKeyMappings; i++)
    {
        if (config->keyFlags & g_keyMappings[i].flag)
        {
            keys[(*count)++] = g_keyMappings[i].vkCode;
        }
    }

    for (int i = 0; i < MAX_CUSTOM_KEYS; i++)
    {
        if (config->customKeys[i].enabled && config->customKeys[i].vkCode != 0)
        {
            keys[(*count)++] = config->customKeys[i].vkCode;
        }
    }
}

static bool
IsRotationKey(const ClickerConfig *config, WORD vkCode)
{
    for (int i = 0; i < config->rotationKeyCount; i++)
    {
        if (config->rotationVkCodes[i] == vkCode)
        {
            return true;
        }
    }
    return false;
}

void
PerformActions(const ClickerConfig *config)
{
    if (!config)
    {
        return;
    }

    // Mouse clicks (always executed)
    if (config->leftClick)
    {
        ClickAtCurrentPos(true);
    }
    if (config->rightClick)
    {
        ClickAtCurrentPos(false);
    }

    // Get active keys
    WORD activeKeys[32];
    int keyCount = 0;
    GetActiveKeys(config, activeKeys, &keyCount);

    if (config->rotationMode && config->rotationKeyCount > 0)
    {
        // Rotation mode:
        // 1. Press one rotation key in sequence
        SendKey(config->rotationVkCodes[g_rotationIndex % config->rotationKeyCount]);
        g_rotationIndex++;

        // 2. Press all non-rotation keys (always-on keys)
        for (int i = 0; i < keyCount; i++)
        {
            if (!IsRotationKey(config, activeKeys[i]))
            {
                SendKey(activeKeys[i]);
            }
        }
    }
    else
    {
        // Normal mode: press all keys
        for (int i = 0; i < keyCount; i++)
        {
            SendKey(activeKeys[i]);
        }
    }
}

static int
CalculateInterval(const ClickerConfig *config)
{
    int baseInterval = 1000 / config->clicksPerSecond;
    if (baseInterval < 1)
    {
        baseInterval = 1;
    }

    if (config->randomDelay)
    {
        int variance = baseInterval * config->randomPercent / 100;
        int randomOffset = (rand() % (variance * 2 + 1)) - variance;
        baseInterval += randomOffset;
        if (baseInterval < 1)
        {
            baseInterval = 1;
        }
    }

    return baseInterval;
}

unsigned __stdcall
ClickerThread(void *arg)
{
    (void)arg;

    srand((unsigned int)GetTickCount());

    while (g_isRunning)
    {
        // Check timer
        if (g_currentConfig.useTimer)
        {
            DWORD elapsed = GetTickCount() - g_startTime;
            if (elapsed >= (DWORD)(g_currentConfig.timerSeconds * 1000))
            {
                // Timer expired, post message to stop
                if (g_hMainWnd)
                {
                    PostMessage(g_hMainWnd, WM_USER + 100, 0, 0);
                }
                break;
            }
        }

        int intervalMs = CalculateInterval(&g_currentConfig);

        if (WaitForSingleObject(g_hStopEvent, intervalMs) != WAIT_TIMEOUT)
        {
            break;
        }

        if (!g_isRunning)
        {
            break;
        }

        PerformActions(&g_currentConfig);
    }

    return 0;
}

void
StartClicking(HWND hWnd, const ClickerConfig *config)
{
    if (g_isRunning)
    {
        return;
    }

    ResetEvent(g_hStopEvent);

    g_hMainWnd = hWnd;
    g_currentConfig = *config;
    g_rotationIndex = 0;
    g_startTime = GetTickCount();
    g_isRunning = true;

    g_hThread = (HANDLE)_beginthreadex(nullptr, 0, ClickerThread, nullptr, 0, nullptr);
}

void
StopClicking(HWND hWnd)
{
    (void)hWnd;
    if (!g_isRunning)
    {
        return;
    }

    g_isRunning = false;

    SetEvent(g_hStopEvent);

    if (g_hThread)
    {
        WaitForSingleObject(g_hThread, 500);
        CloseHandle(g_hThread);
        g_hThread = nullptr;
    }
}

bool
IsClickerRunning()
{
    return g_isRunning;
}
