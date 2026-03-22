#include "clicker.h"
#include "config.h"
#include "hotkey.h"
#include "resource.h"
#include "window.h"
#include <commctrl.h>
#include <stdio.h>
#include <windows.h>

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

// Global variables
HINSTANCE g_hInstance = nullptr;
HWND g_hMainWnd = nullptr;
ClickerConfig g_config = {};
static int g_animFrame = 0;

// Animation frames (wave pattern)
static const wchar_t* g_animFrames[] = {
    L"▁▂▃▄▅▆▇█",
    L"▂▃▄▅▆▇█▇",
    L"▃▄▅▆▇█▇▆",
    L"▄▅▆▇█▇▆▅",
    L"▅▆▇█▇▆▅▄",
    L"▆▇█▇▆▅▄▃",
    L"▇█▇▆▅▄▃▂",
    L"█▇▆▅▄▃▂▁",
    L"▇▆▅▄▃▂▁▂",
    L"▆▅▄▃▂▁▂▃",
    L"▅▄▃▂▁▂▃▄",
    L"▄▃▂▁▂▃▄▅",
    L"▃▂▁▂▃▄▅▆",
    L"▂▁▂▃▄▅▆▇"
};
static const int g_animFrameCount = 14;

// Window procedure forward declaration
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Initialize default configuration
void
InitDefaultConfig()
{
    g_config.clickMode = MODE_AUTO_REPEAT;
    g_config.startKey = VK_F5;
    g_config.stopKey = VK_F6;
    g_config.leftClick = true;
    g_config.rightClick = false;
    g_config.keyFlags = 0;
    g_config.clicksPerSecond = 5;
    for (int i = 0; i < MAX_CUSTOM_KEYS; i++)
    {
        g_config.customKeys[i].enabled = false;
        g_config.customKeys[i].vkCode = 0;
    }
}

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    g_hInstance = hInstance;

    // Initialize common controls
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icc);

    // Initialize default configuration, then load saved settings
    InitDefaultConfig();
    LoadConfig(&g_config);

    // Initialize clicker module
    InitClicker();

    // Initialize modern UI font
    InitUIFont();

    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"AutoClickClass";
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));

    if (!wc.hIcon)
    {
        wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    }

    if (!RegisterClassEx(&wc))
    {
        MessageBox(nullptr, L"윈도우 클래스 등록 실패", L"오류", MB_ICONERROR);
        return 1;
    }

    // Create main window
    g_hMainWnd = CreateWindowEx(
        0, L"AutoClickClass", L"나대신클릭",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, nullptr);

    if (!g_hMainWnd)
    {
        MessageBox(nullptr, L"윈도우 생성 실패", L"오류", MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        {
            // Create UI controls
            CreateControls(hWnd, g_hInstance);

            // Apply loaded config to UI
            ApplyConfigToUI(&g_config);

            // Register hotkeys
            RegisterAppHotkeys(hWnd, g_config.startKey, g_config.stopKey);

            // Update status display with current hotkey names
            UpdateStatusDisplay(hWnd, false);
            return 0;
        }

    case WM_HOTKEY:
        {
            if (wParam == HOTKEY_ID_START)
            {
                if (!IsClickerRunning())
                {
                    ReadSettingsFromUI(&g_config);
                    StartClicking(hWnd, &g_config);
                    g_animFrame = 0;
                    SetTimer(hWnd, TIMER_ID_ANIMATION, 80, nullptr);
                    UpdateStatusDisplay(hWnd, true);

                    HWND hModeCombo = GetControlHandle(IDC_COMBO_MODE);
                    if (hModeCombo) EnableWindow(hModeCombo, FALSE);
                }
            }
            else if (wParam == HOTKEY_ID_STOP)
            {
                if (IsClickerRunning())
                {
                    KillTimer(hWnd, TIMER_ID_ANIMATION);
                    StopClicking(hWnd);
                    UpdateStatusDisplay(hWnd, false);

                    HWND hModeCombo = GetControlHandle(IDC_COMBO_MODE);
                    if (hModeCombo) EnableWindow(hModeCombo, TRUE);
                }
            }
            return 0;
        }

    case WM_TIMER:
        {
            if (wParam == TIMER_ID_ANIMATION)
            {
                HWND hStatus = GetControlHandle(IDC_STATIC_STATUS);
                if (hStatus)
                {
                    wchar_t status[64];
                    swprintf(status, 64, L"실행 중  %s", g_animFrames[g_animFrame]);
                    SetWindowText(hStatus, status);
                    g_animFrame = (g_animFrame + 1) % g_animFrameCount;
                }
            }
            return 0;
        }

    case WM_USER + 100:  // Timer expired
        {
            if (IsClickerRunning())
            {
                KillTimer(hWnd, TIMER_ID_ANIMATION);
                StopClicking(hWnd);
                UpdateStatusDisplay(hWnd, false);

                HWND hModeCombo = GetControlHandle(IDC_COMBO_MODE);
                if (hModeCombo) EnableWindow(hModeCombo, TRUE);
            }
            return 0;
        }

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            switch (wmId)
            {
            case IDC_BTN_EXIT:
                DestroyWindow(hWnd);
                break;

            case IDC_BTN_ABOUT:
                {
                    wchar_t aboutMsg[512];
                    swprintf(aboutMsg, 512,
                        L"나대신클릭 %S\n\n"
                        L"마우스 클릭과 키보드 입력을 자동으로 반복합니다.\n\n"
                        L"제작: %S\n"
                        L"커밋: %S\n\n"
                        L"https://github.com/junsik/nadaesinclick",
                        APP_VERSION, APP_AUTHOR, APP_COMMIT);
                    MessageBox(hWnd, aboutMsg, L"정보", MB_OK | MB_ICONINFORMATION);
                }
                break;

            case IDC_COMBO_START_KEY:
            case IDC_COMBO_STOP_KEY:
                if (wmEvent == CBN_SELCHANGE)
                {
                    HWND hStartCombo = GetControlHandle(IDC_COMBO_START_KEY);
                    HWND hStopCombo = GetControlHandle(IDC_COMBO_STOP_KEY);
                    int startIdx = (int)SendMessage(hStartCombo, CB_GETCURSEL, 0, 0);
                    int stopIdx = (int)SendMessage(hStopCombo, CB_GETCURSEL, 0, 0);

                    // Prevent same key for start and stop
                    if (startIdx == stopIdx)
                    {
                        if (wmId == IDC_COMBO_START_KEY)
                        {
                            // User changed start, adjust stop
                            stopIdx = (startIdx + 1) % 12;
                            SendMessage(hStopCombo, CB_SETCURSEL, stopIdx, 0);
                        }
                        else
                        {
                            // User changed stop, adjust start
                            startIdx = (stopIdx + 1) % 12;
                            SendMessage(hStartCombo, CB_SETCURSEL, startIdx, 0);
                        }
                    }

                    g_config.startKey = GetVKFromComboIndex(startIdx);
                    g_config.stopKey = GetVKFromComboIndex(stopIdx);
                    UpdateHotkeys(hWnd, g_config.startKey, g_config.stopKey);
                    UpdateStatusDisplay(hWnd, false);
                }
                break;

            case IDC_COMBO_MODE:
                if (wmEvent == CBN_SELCHANGE)
                {
                    HWND hModeCombo = GetControlHandle(IDC_COMBO_MODE);
                    int modeIdx = (int)SendMessage(hModeCombo, CB_GETCURSEL, 0, 0);

                    g_config.clickMode = (modeIdx == 1) ? MODE_HOLD : MODE_AUTO_REPEAT;
                    UpdateStatusDisplay(hWnd, false);
                }
                break;
            }
            return 0;
        }

    case WM_CLOSE:
        {
            DestroyWindow(hWnd);
            return 0;
        }

    case WM_DESTROY:
        {
            // Save current settings before exit
            ReadSettingsFromUI(&g_config);
            SaveConfig(&g_config);

            // Stop animation timer
            KillTimer(hWnd, TIMER_ID_ANIMATION);

            // Stop clicking if running
            StopClicking(hWnd);

            // Cleanup clicker resources
            CleanupClicker();

            // Cleanup UI font
            CleanupUIFont();

            // Unregister hotkeys
            UnregisterAppHotkeys(hWnd);

            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
