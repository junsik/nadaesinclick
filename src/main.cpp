#include "clicker.h"
#include "hotkey.h"
#include "resource.h"
#include "window.h"
#include <commctrl.h>
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

// Window procedure forward declaration
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Initialize default configuration
void
InitDefaultConfig()
{
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

    // Initialize default configuration
    InitDefaultConfig();

    // Initialize clicker module
    InitClicker();

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

            // Register hotkeys
            RegisterAppHotkeys(hWnd, g_config.startKey, g_config.stopKey);
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
                    UpdateStatusDisplay(hWnd, true);
                }
            }
            else if (wParam == HOTKEY_ID_STOP)
            {
                if (IsClickerRunning())
                {
                    StopClicking(hWnd);
                    UpdateStatusDisplay(hWnd, false);
                }
            }
            return 0;
        }

    case WM_USER + 100:  // Timer expired
        {
            if (IsClickerRunning())
            {
                StopClicking(hWnd);
                UpdateStatusDisplay(hWnd, false);
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
            // Stop clicking if running
            StopClicking(hWnd);

            // Cleanup clicker resources
            CleanupClicker();

            // Unregister hotkeys
            UnregisterAppHotkeys(hWnd);

            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
