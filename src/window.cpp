#include "window.h"
#include "clicker.h"
#include "hotkey.h"
#include "resource.h"
#include <commctrl.h>
#include <map>
#include <stdio.h>

static std::map<int, HWND> g_controls;
static HFONT g_hFont = nullptr;

// Apply modern font to a control
static void
ApplyFont(HWND hWnd)
{
    if (g_hFont)
    {
        SendMessage(hWnd, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    }
}

// Initialize modern UI font (Segoe UI)
void
InitUIFont()
{
    if (!g_hFont)
    {
        g_hFont = CreateFont(
            -14,                    // Height (negative = character height)
            0,                      // Width
            0,                      // Escapement
            0,                      // Orientation
            FW_NORMAL,              // Weight
            FALSE,                  // Italic
            FALSE,                  // Underline
            FALSE,                  // StrikeOut
            DEFAULT_CHARSET,        // CharSet
            OUT_DEFAULT_PRECIS,     // OutPrecision
            CLIP_DEFAULT_PRECIS,    // ClipPrecision
            CLEARTYPE_QUALITY,      // Quality
            DEFAULT_PITCH | FF_SWISS, // PitchAndFamily
            L"Segoe UI"             // Font name
        );
    }
}

// Cleanup UI font
void
CleanupUIFont()
{
    if (g_hFont)
    {
        DeleteObject(g_hFont);
        g_hFont = nullptr;
    }
}

static HWND
CreateLabel(HWND hParent, HINSTANCE hInst, const wchar_t *text,
            int x, int y, int w, int h, int id = 0)
{
    HWND hwnd = CreateWindowEx(0, L"STATIC", text,
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        x, y, w, h, hParent, (HMENU)(INT_PTR)id, hInst, nullptr);
    ApplyFont(hwnd);
    if (id != 0)
    {
        g_controls[id] = hwnd;
    }
    return hwnd;
}

static HWND
CreateCheckbox(HWND hParent, HINSTANCE hInst, const wchar_t *text,
               int x, int y, int w, int h, int id, bool checked = false)
{
    HWND hChk = CreateWindowEx(0, L"BUTTON", text,
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
        x, y, w, h, hParent, (HMENU)(INT_PTR)id, hInst, nullptr);
    ApplyFont(hChk);
    if (checked)
    {
        SendMessage(hChk, BM_SETCHECK, BST_CHECKED, 0);
    }
    g_controls[id] = hChk;
    return hChk;
}

static HWND
CreateCombo(HWND hParent, HINSTANCE hInst, int x, int y, int w, int h, int id)
{
    HWND hCombo = CreateWindowEx(0, L"COMBOBOX", nullptr,
        WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
        x, y, w, h, hParent, (HMENU)(INT_PTR)id, hInst, nullptr);
    ApplyFont(hCombo);
    g_controls[id] = hCombo;
    return hCombo;
}

static HWND
CreateEdit(HWND hParent, HINSTANCE hInst, const wchar_t *text,
           int x, int y, int w, int h, int id, bool numOnly = false)
{
    DWORD style = WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT;
    if (numOnly)
    {
        style |= ES_NUMBER;
    }
    HWND hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", text, style,
        x, y, w, h, hParent, (HMENU)(INT_PTR)id, hInst, nullptr);
    ApplyFont(hEdit);
    g_controls[id] = hEdit;
    return hEdit;
}

static HWND
CreateGroupBox(HWND hParent, HINSTANCE hInst, const wchar_t *text,
               int x, int y, int w, int h)
{
    HWND hGroup = CreateWindowEx(0, L"BUTTON", text,
        WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
        x, y, w, h, hParent, nullptr, hInst, nullptr);
    ApplyFont(hGroup);
    return hGroup;
}

static void
PopulateHotkeyCombo(HWND hCombo, UINT selectedKey)
{
    const wchar_t *keys[] = {
        L"F1", L"F2", L"F3", L"F4", L"F5", L"F6",
        L"F7", L"F8", L"F9", L"F10", L"F11", L"F12"
    };

    for (int i = 0; i < 12; i++)
    {
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)keys[i]);
    }

    int selectedIdx = GetComboIndexFromVK(selectedKey);
    SendMessage(hCombo, CB_SETCURSEL, selectedIdx, 0);
}

void
CreateControls(HWND hWnd, HINSTANCE hInstance)
{
    const int M = 15;  // margin (increased)
    const int W = 370; // content width
    const int ROW_H = 28; // row height
    const int GAP = 8;    // section gap

    int y = M;

    // ===== Hotkey & Speed Section =====
    CreateGroupBox(hWnd, hInstance, L"제어", M, y, W, 60);

    CreateLabel(hWnd, hInstance, L"시작", M + 20, y + 26, 35, 20);
    HWND hStartCombo = CreateCombo(hWnd, hInstance, M + 55, y + 24, 60, 200, IDC_COMBO_START_KEY);
    PopulateHotkeyCombo(hStartCombo, VK_F5);

    CreateLabel(hWnd, hInstance, L"정지", M + 130, y + 26, 35, 20);
    HWND hStopCombo = CreateCombo(hWnd, hInstance, M + 165, y + 24, 60, 200, IDC_COMBO_STOP_KEY);
    PopulateHotkeyCombo(hStopCombo, VK_F6);

    CreateLabel(hWnd, hInstance, L"속도", M + 235, y + 26, 35, 20);
    HWND hEditCPS = CreateEdit(hWnd, hInstance, L"5", M + 270, y + 24, 60, 22, IDC_EDIT_CPS, true);

    HWND hSpin = CreateWindowEx(0, UPDOWN_CLASS, nullptr,
        WS_VISIBLE | WS_CHILD | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_NOTHOUSANDS,
        0, 0, 0, 0, hWnd, (HMENU)IDC_SPIN_CPS, hInstance, nullptr);
    SendMessage(hSpin, UDM_SETBUDDY, (WPARAM)hEditCPS, 0);
    SendMessage(hSpin, UDM_SETRANGE32, 1, 999);
    SendMessage(hSpin, UDM_SETPOS32, 0, 5);
    g_controls[IDC_SPIN_CPS] = hSpin;

    CreateLabel(hWnd, hInstance, L"회/초", M + 330, y + 26, 45, 20);

    y += 60 + GAP;

    // ===== Mouse Section =====
    CreateGroupBox(hWnd, hInstance, L"마우스", M, y, W, 55);

    CreateCheckbox(hWnd, hInstance, L"왼쪽 클릭", M + 25, y + 25, 120, 22, IDC_CHK_LCLICK, true);
    CreateCheckbox(hWnd, hInstance, L"오른쪽 클릭", M + 180, y + 25, 130, 22, IDC_CHK_RCLICK);

    y += 55 + GAP;

    // ===== Keyboard Section =====
    CreateGroupBox(hWnd, hInstance, L"키보드", M, y, W, 203);

    int kx = M + 25;
    int ky = y + 25;
    int keyWidth = 50;
    int keyGap = 55;

    // Row 1: 1 2 3 4
    CreateCheckbox(hWnd, hInstance, L"1", kx, ky, keyWidth, 22, IDC_CHK_KEY_1);
    CreateCheckbox(hWnd, hInstance, L"2", kx + keyGap, ky, keyWidth, 22, IDC_CHK_KEY_2);
    CreateCheckbox(hWnd, hInstance, L"3", kx + keyGap * 2, ky, keyWidth, 22, IDC_CHK_KEY_3);
    CreateCheckbox(hWnd, hInstance, L"4", kx + keyGap * 3, ky, keyWidth, 22, IDC_CHK_KEY_4);

    // Row 2: Q W E R T
    ky += ROW_H;
    CreateCheckbox(hWnd, hInstance, L"Q", kx, ky, keyWidth, 22, IDC_CHK_KEY_Q);
    CreateCheckbox(hWnd, hInstance, L"W", kx + keyGap, ky, keyWidth, 22, IDC_CHK_KEY_W);
    CreateCheckbox(hWnd, hInstance, L"E", kx + keyGap * 2, ky, keyWidth, 22, IDC_CHK_KEY_E);
    CreateCheckbox(hWnd, hInstance, L"R", kx + keyGap * 3, ky, keyWidth, 22, IDC_CHK_KEY_R);
    CreateCheckbox(hWnd, hInstance, L"T", kx + keyGap * 4, ky, keyWidth, 22, IDC_CHK_KEY_T);

    // Row 3: Enter Space ESC
    ky += ROW_H;
    CreateCheckbox(hWnd, hInstance, L"Enter", kx, ky, 70, 22, IDC_CHK_KEY_ENTER);
    CreateCheckbox(hWnd, hInstance, L"Space", kx + 85, ky, 75, 22, IDC_CHK_KEY_SPACE);
    CreateCheckbox(hWnd, hInstance, L"ESC", kx + 175, ky, 60, 22, IDC_CHK_KEY_ESC);

    // Row 4: Arrow keys
    ky += ROW_H;
    CreateCheckbox(hWnd, hInstance, L"↑", kx, ky, 45, 22, IDC_CHK_KEY_UP);
    CreateCheckbox(hWnd, hInstance, L"↓", kx + 55, ky, 45, 22, IDC_CHK_KEY_DOWN);
    CreateCheckbox(hWnd, hInstance, L"←", kx + 110, ky, 45, 22, IDC_CHK_KEY_LEFT);
    CreateCheckbox(hWnd, hInstance, L"→", kx + 165, ky, 45, 22, IDC_CHK_KEY_RIGHT);

    // Row 5: Custom keys (1-4)
    ky += ROW_H;
    CreateLabel(hWnd, hInstance, L"사용자 정의:", kx, ky + 2, 95, 20);
    CreateEdit(hWnd, hInstance, L"", kx + 100, ky, 32, 22, IDC_EDIT_CUSTOM_1);
    CreateEdit(hWnd, hInstance, L"", kx + 140, ky, 32, 22, IDC_EDIT_CUSTOM_2);
    CreateEdit(hWnd, hInstance, L"", kx + 180, ky, 32, 22, IDC_EDIT_CUSTOM_3);
    CreateEdit(hWnd, hInstance, L"", kx + 220, ky, 32, 22, IDC_EDIT_CUSTOM_4);

    // Row 6: Custom keys (5-8)
    ky += ROW_H;
    CreateLabel(hWnd, hInstance, L"", kx, ky + 2, 95, 20);
    CreateEdit(hWnd, hInstance, L"", kx + 100, ky, 32, 22, IDC_EDIT_CUSTOM_5);
    CreateEdit(hWnd, hInstance, L"", kx + 140, ky, 32, 22, IDC_EDIT_CUSTOM_6);
    CreateEdit(hWnd, hInstance, L"", kx + 180, ky, 32, 22, IDC_EDIT_CUSTOM_7);
    CreateEdit(hWnd, hInstance, L"", kx + 220, ky, 32, 22, IDC_EDIT_CUSTOM_8);

    int customEditIds[] = {IDC_EDIT_CUSTOM_1, IDC_EDIT_CUSTOM_2, IDC_EDIT_CUSTOM_3, IDC_EDIT_CUSTOM_4,
                           IDC_EDIT_CUSTOM_5, IDC_EDIT_CUSTOM_6, IDC_EDIT_CUSTOM_7, IDC_EDIT_CUSTOM_8};
    for (int i = 0; i < MAX_CUSTOM_KEYS; i++)
    {
        SendMessage(GetControlHandle(customEditIds[i]), EM_SETLIMITTEXT, 1, 0);
    }

    y += 203 + GAP;

    // ===== Options Section =====
    CreateGroupBox(hWnd, hInstance, L"옵션", M, y, W, 110);

    // Row 0: Mode
    CreateLabel(hWnd, hInstance, L"모드", M + 20, y + 27, 35, 20);
    HWND hModeCombo = CreateCombo(hWnd, hInstance, M + 55, y + 24, 100, 200, IDC_COMBO_MODE);
    SendMessage(hModeCombo, CB_ADDSTRING, 0, (LPARAM)L"자동 반복");
    SendMessage(hModeCombo, CB_ADDSTRING, 0, (LPARAM)L"홀드");
    SendMessage(hModeCombo, CB_SETCURSEL, 0, 0);

    // Row 1: Rotation, Random, Timer
    CreateCheckbox(hWnd, hInstance, L"순환", M + 20, y + 52, 55, 22, IDC_CHK_ROTATION);
    CreateEdit(hWnd, hInstance, L"1234", M + 75, y + 51, 65, 22, IDC_EDIT_ROTATION_KEYS, false);

    CreateCheckbox(hWnd, hInstance, L"랜덤 ±", M + 155, y + 52, 75, 22, IDC_CHK_RANDOM);
    CreateEdit(hWnd, hInstance, L"20", M + 230, y + 51, 35, 22, IDC_EDIT_RANDOM_PCT, true);
    CreateLabel(hWnd, hInstance, L"%", M + 267, y + 53, 20, 20);

    CreateCheckbox(hWnd, hInstance, L"타이머", M + 295, y + 52, 70, 22, IDC_CHK_TIMER);

    // Row 2: Timer value and hint
    CreateEdit(hWnd, hInstance, L"60", M + 295, y + 79, 40, 22, IDC_EDIT_TIMER_SEC, true);
    CreateLabel(hWnd, hInstance, L"초", M + 338, y + 81, 25, 20);

    CreateLabel(hWnd, hInstance, L"순환: 지정키만 순환, 나머지 매번 실행",
        M + 20, y + 81, 260, 20);

    y += 110 + GAP;

    // ===== Bottom: Status & Buttons =====
    HWND hStatus = CreateLabel(hWnd, hInstance, L"준비 (F5: 시작, F6: 정지)",
        M, y + 5, 200, 22, IDC_STATIC_STATUS);
    g_controls[IDC_STATIC_STATUS] = hStatus;

    HWND hAboutBtn = CreateWindowEx(0, L"BUTTON", L"정보",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        M + W - 170, y, 80, 30, hWnd, (HMENU)IDC_BTN_ABOUT, hInstance, nullptr);
    ApplyFont(hAboutBtn);

    HWND hExitBtn = CreateWindowEx(0, L"BUTTON", L"종료",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        M + W - 80, y, 80, 30, hWnd, (HMENU)IDC_BTN_EXIT, hInstance, nullptr);
    ApplyFont(hExitBtn);
}

void
ApplyConfigToUI(const ClickerConfig *config)
{
    if (!config)
    {
        return;
    }

    // Mode
    HWND hModeCombo = GetControlHandle(IDC_COMBO_MODE);
    if (hModeCombo)
    {
        SendMessage(hModeCombo, CB_SETCURSEL, config->clickMode == MODE_HOLD ? 1 : 0, 0);
    }

    // Hotkeys
    HWND hStartCombo = GetControlHandle(IDC_COMBO_START_KEY);
    HWND hStopCombo = GetControlHandle(IDC_COMBO_STOP_KEY);
    if (hStartCombo)
    {
        SendMessage(hStartCombo, CB_SETCURSEL, GetComboIndexFromVK(config->startKey), 0);
    }
    if (hStopCombo)
    {
        SendMessage(hStopCombo, CB_SETCURSEL, GetComboIndexFromVK(config->stopKey), 0);
    }

    // Speed
    HWND hSpin = GetControlHandle(IDC_SPIN_CPS);
    if (hSpin)
    {
        SendMessage(hSpin, UDM_SETPOS32, 0, config->clicksPerSecond);
    }

    // Mouse
    SendMessage(GetControlHandle(IDC_CHK_LCLICK), BM_SETCHECK,
        config->leftClick ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(GetControlHandle(IDC_CHK_RCLICK), BM_SETCHECK,
        config->rightClick ? BST_CHECKED : BST_UNCHECKED, 0);

    // Keyboard flags
    struct { int id; DWORD flag; } keyChecks[] = {
        {IDC_CHK_KEY_1, KEY_1}, {IDC_CHK_KEY_2, KEY_2},
        {IDC_CHK_KEY_3, KEY_3}, {IDC_CHK_KEY_4, KEY_4},
        {IDC_CHK_KEY_Q, KEY_Q}, {IDC_CHK_KEY_W, KEY_W},
        {IDC_CHK_KEY_E, KEY_E}, {IDC_CHK_KEY_R, KEY_R},
        {IDC_CHK_KEY_T, KEY_T}, {IDC_CHK_KEY_ENTER, KEY_ENTER},
        {IDC_CHK_KEY_SPACE, KEY_SPACE}, {IDC_CHK_KEY_ESC, KEY_ESC},
        {IDC_CHK_KEY_UP, KEY_UP}, {IDC_CHK_KEY_DOWN, KEY_DOWN},
        {IDC_CHK_KEY_LEFT, KEY_LEFT}, {IDC_CHK_KEY_RIGHT, KEY_RIGHT},
    };
    for (auto &kc : keyChecks)
    {
        HWND h = GetControlHandle(kc.id);
        if (h)
        {
            SendMessage(h, BM_SETCHECK,
                (config->keyFlags & kc.flag) ? BST_CHECKED : BST_UNCHECKED, 0);
        }
    }

    // Custom keys - convert vkCode back to character
    int customEditIds[] = {IDC_EDIT_CUSTOM_1, IDC_EDIT_CUSTOM_2, IDC_EDIT_CUSTOM_3, IDC_EDIT_CUSTOM_4,
                           IDC_EDIT_CUSTOM_5, IDC_EDIT_CUSTOM_6, IDC_EDIT_CUSTOM_7, IDC_EDIT_CUSTOM_8};
    for (int i = 0; i < MAX_CUSTOM_KEYS; i++)
    {
        HWND hEdit = GetControlHandle(customEditIds[i]);
        if (hEdit)
        {
            if (config->customKeys[i].enabled && config->customKeys[i].vkCode != 0)
            {
                UINT scanCode = MapVirtualKey(config->customKeys[i].vkCode, MAPVK_VK_TO_CHAR);
                wchar_t ch[2] = {(wchar_t)scanCode, 0};
                SetWindowText(hEdit, ch);
            }
            else
            {
                SetWindowText(hEdit, L"");
            }
        }
    }

    // Options
    SendMessage(GetControlHandle(IDC_CHK_ROTATION), BM_SETCHECK,
        config->rotationMode ? BST_CHECKED : BST_UNCHECKED, 0);
    SetWindowText(GetControlHandle(IDC_EDIT_ROTATION_KEYS), config->rotationKeys);

    SendMessage(GetControlHandle(IDC_CHK_RANDOM), BM_SETCHECK,
        config->randomDelay ? BST_CHECKED : BST_UNCHECKED, 0);
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", config->randomPercent);
    SetWindowText(GetControlHandle(IDC_EDIT_RANDOM_PCT), buf);

    SendMessage(GetControlHandle(IDC_CHK_TIMER), BM_SETCHECK,
        config->useTimer ? BST_CHECKED : BST_UNCHECKED, 0);
    swprintf(buf, 32, L"%d", config->timerSeconds);
    SetWindowText(GetControlHandle(IDC_EDIT_TIMER_SEC), buf);
}

HWND
GetControlHandle(int controlId)
{
    auto it = g_controls.find(controlId);
    if (it != g_controls.end())
    {
        return it->second;
    }
    return nullptr;
}

void
ReadSettingsFromUI(ClickerConfig *config)
{
    if (!config)
    {
        return;
    }

    wchar_t buf[32];

    // Mode
    HWND hModeCombo = GetControlHandle(IDC_COMBO_MODE);
    int modeIdx = (int)SendMessage(hModeCombo, CB_GETCURSEL, 0, 0);
    config->clickMode = (modeIdx == 1) ? MODE_HOLD : MODE_AUTO_REPEAT;

    config->leftClick = (SendMessage(GetControlHandle(IDC_CHK_LCLICK),
        BM_GETCHECK, 0, 0) == BST_CHECKED);
    config->rightClick = (SendMessage(GetControlHandle(IDC_CHK_RCLICK),
        BM_GETCHECK, 0, 0) == BST_CHECKED);

    config->keyFlags = 0;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_1), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_1;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_2), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_2;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_3), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_3;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_4), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_4;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_Q), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_Q;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_W), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_W;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_E), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_E;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_R), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_R;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_T), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_T;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_ENTER), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_ENTER;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_SPACE), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_SPACE;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_ESC), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_ESC;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_UP), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_UP;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_DOWN), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_DOWN;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_LEFT), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_LEFT;
    if (SendMessage(GetControlHandle(IDC_CHK_KEY_RIGHT), BM_GETCHECK, 0, 0) == BST_CHECKED)
        config->keyFlags |= KEY_RIGHT;

    // Custom keys: enabled if edit box has a value
    int customEditIds[] = {IDC_EDIT_CUSTOM_1, IDC_EDIT_CUSTOM_2, IDC_EDIT_CUSTOM_3, IDC_EDIT_CUSTOM_4,
                           IDC_EDIT_CUSTOM_5, IDC_EDIT_CUSTOM_6, IDC_EDIT_CUSTOM_7, IDC_EDIT_CUSTOM_8};
    for (int i = 0; i < MAX_CUSTOM_KEYS; i++)
    {
        HWND hEdit = GetControlHandle(customEditIds[i]);
        if (hEdit)
        {
            wchar_t keyChar[4] = {};
            GetWindowText(hEdit, keyChar, 4);
            if (keyChar[0] != L'\0')
            {
                SHORT vk = VkKeyScan(keyChar[0]);
                config->customKeys[i].vkCode = (WORD)(vk & 0xFF);
                config->customKeys[i].enabled = true;
            }
            else
            {
                config->customKeys[i].vkCode = 0;
                config->customKeys[i].enabled = false;
            }
        }
        else
        {
            config->customKeys[i].enabled = false;
            config->customKeys[i].vkCode = 0;
        }
    }

    GetWindowText(GetControlHandle(IDC_EDIT_CPS), buf, 32);
    config->clicksPerSecond = _wtoi(buf);
    if (config->clicksPerSecond < 1)
    {
        config->clicksPerSecond = 1;
    }
    if (config->clicksPerSecond > 999)
    {
        config->clicksPerSecond = 999;
    }

    // Options
    config->rotationMode = (SendMessage(GetControlHandle(IDC_CHK_ROTATION),
        BM_GETCHECK, 0, 0) == BST_CHECKED);

    // Parse rotation keys
    GetWindowText(GetControlHandle(IDC_EDIT_ROTATION_KEYS), config->rotationKeys, 16);
    config->rotationKeyCount = 0;
    for (int i = 0; config->rotationKeys[i] != L'\0' && i < 15; i++)
    {
        wchar_t ch = config->rotationKeys[i];
        if (ch >= L'0' && ch <= L'9')
        {
            config->rotationVkCodes[config->rotationKeyCount++] = 0x30 + (ch - L'0');
        }
        else if (ch >= L'A' && ch <= L'Z')
        {
            config->rotationVkCodes[config->rotationKeyCount++] = 0x41 + (ch - L'A');
        }
        else if (ch >= L'a' && ch <= L'z')
        {
            config->rotationVkCodes[config->rotationKeyCount++] = 0x41 + (ch - L'a');
        }
    }

    config->randomDelay = (SendMessage(GetControlHandle(IDC_CHK_RANDOM),
        BM_GETCHECK, 0, 0) == BST_CHECKED);
    GetWindowText(GetControlHandle(IDC_EDIT_RANDOM_PCT), buf, 32);
    config->randomPercent = _wtoi(buf);
    if (config->randomPercent < 1)
        config->randomPercent = 1;
    if (config->randomPercent > 50)
        config->randomPercent = 50;

    config->useTimer = (SendMessage(GetControlHandle(IDC_CHK_TIMER),
        BM_GETCHECK, 0, 0) == BST_CHECKED);
    GetWindowText(GetControlHandle(IDC_EDIT_TIMER_SEC), buf, 32);
    config->timerSeconds = _wtoi(buf);
    if (config->timerSeconds < 1)
        config->timerSeconds = 1;
    if (config->timerSeconds > 3600)
        config->timerSeconds = 3600;
}

void
UpdateStatusDisplay(HWND hWnd, bool isRunning)
{
    (void)hWnd;
    HWND hStatus = GetControlHandle(IDC_STATIC_STATUS);
    if (hStatus)
    {
        if (isRunning)
        {
            SetWindowText(hStatus, L"실행 중...");
        }
        else
        {
            // Get current hotkey selections
            HWND hStartCombo = GetControlHandle(IDC_COMBO_START_KEY);
            HWND hStopCombo = GetControlHandle(IDC_COMBO_STOP_KEY);

            int startIdx = (int)SendMessage(hStartCombo, CB_GETCURSEL, 0, 0);
            int stopIdx = (int)SendMessage(hStopCombo, CB_GETCURSEL, 0, 0);

            wchar_t startKey[8], stopKey[8];
            SendMessage(hStartCombo, CB_GETLBTEXT, startIdx, (LPARAM)startKey);
            SendMessage(hStopCombo, CB_GETLBTEXT, stopIdx, (LPARAM)stopKey);

            wchar_t status[64];
            swprintf(status, 64, L"준비 (%s: 시작, %s: 정지)", startKey, stopKey);
            SetWindowText(hStatus, status);
        }
    }
}

