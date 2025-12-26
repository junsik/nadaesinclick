#include "clicker.h"
#include "resource.h"
#include <process.h>
#include <timeapi.h>

#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif

// Global state
static bool g_isRunning = false;
static HANDLE g_hThread = nullptr;
static HANDLE g_hStopEvent = nullptr;
static ClickerConfig g_currentConfig = {};

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

void
PerformActions(const ClickerConfig *config)
{
    if (!config)
    {
        return;
    }

    if (config->leftClick)
    {
        ClickAtCurrentPos(true);
    }
    if (config->rightClick)
    {
        ClickAtCurrentPos(false);
    }

    if (config->keyFlags & KEY_1)
        SendKey(0x31);
    if (config->keyFlags & KEY_2)
        SendKey(0x32);
    if (config->keyFlags & KEY_3)
        SendKey(0x33);
    if (config->keyFlags & KEY_4)
        SendKey(0x34);
    if (config->keyFlags & KEY_ENTER)
        SendKey(VK_RETURN);
    if (config->keyFlags & KEY_SPACE)
        SendKey(VK_SPACE);
    if (config->keyFlags & KEY_ESC)
        SendKey(VK_ESCAPE);
    if (config->keyFlags & KEY_A)
        SendKey(0x41);
    if (config->keyFlags & KEY_Q)
        SendKey(0x51);
    if (config->keyFlags & KEY_UP)
        SendKey(VK_UP);
    if (config->keyFlags & KEY_DOWN)
        SendKey(VK_DOWN);
    if (config->keyFlags & KEY_LEFT)
        SendKey(VK_LEFT);
    if (config->keyFlags & KEY_RIGHT)
        SendKey(VK_RIGHT);

    for (int i = 0; i < MAX_CUSTOM_KEYS; i++)
    {
        if (config->customKeys[i].enabled && config->customKeys[i].vkCode != 0)
        {
            SendKey(config->customKeys[i].vkCode);
        }
    }
}

unsigned __stdcall
ClickerThread(void *arg)
{
    (void)arg;

    int intervalMs = 1000 / g_currentConfig.clicksPerSecond;
    if (intervalMs < 1)
    {
        intervalMs = 1;
    }

    while (WaitForSingleObject(g_hStopEvent, intervalMs) == WAIT_TIMEOUT)
    {
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
    (void)hWnd;
    if (g_isRunning)
    {
        return;
    }

    ResetEvent(g_hStopEvent);

    g_currentConfig = *config;
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
