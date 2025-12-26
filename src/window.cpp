#include "window.h"
#include "clicker.h"
#include "hotkey.h"
#include "resource.h"
#include <commctrl.h>
#include <map>
#include <stdio.h>

static std::map<int, HWND> g_controls;

static HWND
CreateLabel(HWND hParent, HINSTANCE hInst, const wchar_t *text,
            int x, int y, int w, int h, int id = 0)
{
    HWND hwnd = CreateWindowEx(0, L"STATIC", text,
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        x, y, w, h, hParent, (HMENU)(INT_PTR)id, hInst, nullptr);
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
    g_controls[id] = hEdit;
    return hEdit;
}

static HWND
CreateGroupBox(HWND hParent, HINSTANCE hInst, const wchar_t *text,
               int x, int y, int w, int h)
{
    return CreateWindowEx(0, L"BUTTON", text,
        WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
        x, y, w, h, hParent, nullptr, hInst, nullptr);
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
    const int M = 10;  // margin
    const int W = 380; // content width

    int y = M;

    // ===== Hotkey & Speed Section =====
    CreateGroupBox(hWnd, hInstance, L"제어", M, y, W, 55);

    CreateLabel(hWnd, hInstance, L"시작", M + 15, y + 22, 40, 20);
    HWND hStartCombo = CreateCombo(hWnd, hInstance, M + 55, y + 20, 55, 200, IDC_COMBO_START_KEY);
    PopulateHotkeyCombo(hStartCombo, VK_F5);

    CreateLabel(hWnd, hInstance, L"정지", M + 120, y + 22, 40, 20);
    HWND hStopCombo = CreateCombo(hWnd, hInstance, M + 160, y + 20, 55, 200, IDC_COMBO_STOP_KEY);
    PopulateHotkeyCombo(hStopCombo, VK_F6);

    CreateLabel(hWnd, hInstance, L"속도", M + 230, y + 22, 40, 20);
    HWND hEditCPS = CreateEdit(hWnd, hInstance, L"5", M + 270, y + 20, 40, 20, IDC_EDIT_CPS, true);

    HWND hSpin = CreateWindowEx(0, UPDOWN_CLASS, nullptr,
        WS_VISIBLE | WS_CHILD | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_NOTHOUSANDS,
        0, 0, 0, 0, hWnd, (HMENU)IDC_SPIN_CPS, hInstance, nullptr);
    SendMessage(hSpin, UDM_SETBUDDY, (WPARAM)hEditCPS, 0);
    SendMessage(hSpin, UDM_SETRANGE32, 1, 100);
    SendMessage(hSpin, UDM_SETPOS32, 0, 5);
    g_controls[IDC_SPIN_CPS] = hSpin;

    CreateLabel(hWnd, hInstance, L"회/초", M + 320, y + 22, 40, 20);

    y += 65;

    // ===== Mouse Section =====
    CreateGroupBox(hWnd, hInstance, L"마우스", M, y, W, 50);

    CreateCheckbox(hWnd, hInstance, L"왼쪽 클릭", M + 20, y + 22, 110, 20, IDC_CHK_LCLICK, true);
    CreateCheckbox(hWnd, hInstance, L"오른쪽 클릭", M + 150, y + 22, 120, 20, IDC_CHK_RCLICK);

    y += 60;

    // ===== Keyboard Section =====
    CreateGroupBox(hWnd, hInstance, L"키보드", M, y, W, 165);

    int kx = M + 20;
    int ky = y + 22;

    // Row 1: 1 2 3 4
    CreateCheckbox(hWnd, hInstance, L"1", kx, ky, 40, 20, IDC_CHK_KEY_1);
    CreateCheckbox(hWnd, hInstance, L"2", kx + 50, ky, 40, 20, IDC_CHK_KEY_2);
    CreateCheckbox(hWnd, hInstance, L"3", kx + 100, ky, 40, 20, IDC_CHK_KEY_3);
    CreateCheckbox(hWnd, hInstance, L"4", kx + 150, ky, 40, 20, IDC_CHK_KEY_4);

    // Row 2: Enter Space ESC
    ky += 25;
    CreateCheckbox(hWnd, hInstance, L"Enter", kx, ky, 70, 20, IDC_CHK_KEY_ENTER);
    CreateCheckbox(hWnd, hInstance, L"Space", kx + 80, ky, 70, 20, IDC_CHK_KEY_SPACE);
    CreateCheckbox(hWnd, hInstance, L"ESC", kx + 160, ky, 60, 20, IDC_CHK_KEY_ESC);

    // Row 3: Arrow keys
    ky += 25;
    CreateCheckbox(hWnd, hInstance, L"Up", kx, ky, 55, 20, IDC_CHK_KEY_UP);
    CreateCheckbox(hWnd, hInstance, L"Down", kx + 65, ky, 65, 20, IDC_CHK_KEY_DOWN);
    CreateCheckbox(hWnd, hInstance, L"Left", kx + 140, ky, 60, 20, IDC_CHK_KEY_LEFT);
    CreateCheckbox(hWnd, hInstance, L"Right", kx + 210, ky, 65, 20, IDC_CHK_KEY_RIGHT);

    // Row 4: Custom keys (4 slots, no checkboxes - enabled if has value)
    ky += 25;
    CreateLabel(hWnd, hInstance, L"사용자 정의:", kx, ky + 2, 100, 20);
    CreateEdit(hWnd, hInstance, L"", kx + 105, ky, 28, 20, IDC_EDIT_CUSTOM_1);
    CreateEdit(hWnd, hInstance, L"", kx + 140, ky, 28, 20, IDC_EDIT_CUSTOM_2);
    CreateEdit(hWnd, hInstance, L"", kx + 175, ky, 28, 20, IDC_EDIT_CUSTOM_3);
    CreateEdit(hWnd, hInstance, L"", kx + 210, ky, 28, 20, IDC_EDIT_CUSTOM_4);

    SendMessage(GetControlHandle(IDC_EDIT_CUSTOM_1), EM_SETLIMITTEXT, 1, 0);
    SendMessage(GetControlHandle(IDC_EDIT_CUSTOM_2), EM_SETLIMITTEXT, 1, 0);
    SendMessage(GetControlHandle(IDC_EDIT_CUSTOM_3), EM_SETLIMITTEXT, 1, 0);
    SendMessage(GetControlHandle(IDC_EDIT_CUSTOM_4), EM_SETLIMITTEXT, 1, 0);

    y += 175;

    // ===== Bottom: Status & Exit =====
    HWND hStatus = CreateLabel(hWnd, hInstance, L"준비 (F5: 시작, F6: 정지)",
        M, y + 5, 250, 20, IDC_STATIC_STATUS);
    g_controls[IDC_STATIC_STATUS] = hStatus;

    CreateWindowEx(0, L"BUTTON", L"종료",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        M + W - 70, y, 70, 28, hWnd, (HMENU)IDC_BTN_EXIT, hInstance, nullptr);
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
    int customEditIds[] = {IDC_EDIT_CUSTOM_1, IDC_EDIT_CUSTOM_2, IDC_EDIT_CUSTOM_3, IDC_EDIT_CUSTOM_4};
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
    if (config->clicksPerSecond > 100)
    {
        config->clicksPerSecond = 100;
    }
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

