#include "config.h"
#include <windows.h>
#include <stdio.h>

static void
GetConfigPath(wchar_t *path, int maxLen)
{
    GetModuleFileName(nullptr, path, maxLen);
    // Replace .exe with .ini
    wchar_t *dot = wcsrchr(path, L'.');
    if (dot)
    {
        wcscpy_s(dot, 5, L".ini");
    }
    else
    {
        wcscat_s(path, maxLen, L".ini");
    }
}

static void
WriteInt(const wchar_t *path, const wchar_t *section, const wchar_t *key, int value)
{
    wchar_t buf[32];
    swprintf(buf, 32, L"%d", value);
    WritePrivateProfileString(section, key, buf, path);
}

static void
WriteStr(const wchar_t *path, const wchar_t *section, const wchar_t *key, const wchar_t *value)
{
    WritePrivateProfileString(section, key, value, path);
}

void
SaveConfig(const ClickerConfig *config)
{
    wchar_t path[MAX_PATH];
    GetConfigPath(path, MAX_PATH);

    const wchar_t *S = L"Settings";

    WriteInt(path, S, L"clickMode", config->clickMode);
    WriteInt(path, S, L"startKey", config->startKey);
    WriteInt(path, S, L"stopKey", config->stopKey);
    WriteInt(path, S, L"leftClick", config->leftClick ? 1 : 0);
    WriteInt(path, S, L"rightClick", config->rightClick ? 1 : 0);
    WriteInt(path, S, L"keyFlags", config->keyFlags);
    WriteInt(path, S, L"clicksPerSecond", config->clicksPerSecond);

    // Custom keys
    for (int i = 0; i < MAX_CUSTOM_KEYS; i++)
    {
        wchar_t keyName[32];
        swprintf(keyName, 32, L"customKey%d", i);
        if (config->customKeys[i].enabled)
        {
            wchar_t val[8];
            swprintf(val, 8, L"%d", config->customKeys[i].vkCode);
            WriteStr(path, S, keyName, val);
        }
        else
        {
            WriteStr(path, S, keyName, L"");
        }
    }

    // Options
    WriteInt(path, S, L"rotationMode", config->rotationMode ? 1 : 0);
    WriteStr(path, S, L"rotationKeys", config->rotationKeys);
    WriteInt(path, S, L"randomDelay", config->randomDelay ? 1 : 0);
    WriteInt(path, S, L"randomPercent", config->randomPercent);
    WriteInt(path, S, L"useTimer", config->useTimer ? 1 : 0);
    WriteInt(path, S, L"timerSeconds", config->timerSeconds);
}

void
LoadConfig(ClickerConfig *config)
{
    wchar_t path[MAX_PATH];
    GetConfigPath(path, MAX_PATH);

    // Check if file exists
    DWORD attr = GetFileAttributes(path);
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return; // No config file, keep defaults
    }

    const wchar_t *S = L"Settings";

    config->clickMode = (ClickMode)GetPrivateProfileInt(S, L"clickMode", MODE_AUTO_REPEAT, path);
    config->startKey = GetPrivateProfileInt(S, L"startKey", VK_F5, path);
    config->stopKey = GetPrivateProfileInt(S, L"stopKey", VK_F6, path);
    config->leftClick = GetPrivateProfileInt(S, L"leftClick", 1, path) != 0;
    config->rightClick = GetPrivateProfileInt(S, L"rightClick", 0, path) != 0;
    config->keyFlags = GetPrivateProfileInt(S, L"keyFlags", 0, path);
    config->clicksPerSecond = GetPrivateProfileInt(S, L"clicksPerSecond", 5, path);
    if (config->clicksPerSecond < 1) config->clicksPerSecond = 1;
    if (config->clicksPerSecond > 999) config->clicksPerSecond = 999;

    // Custom keys
    for (int i = 0; i < MAX_CUSTOM_KEYS; i++)
    {
        wchar_t keyName[32], val[32];
        swprintf(keyName, 32, L"customKey%d", i);
        GetPrivateProfileString(S, keyName, L"", val, 32, path);
        if (val[0] != L'\0')
        {
            config->customKeys[i].vkCode = (WORD)_wtoi(val);
            config->customKeys[i].enabled = true;
        }
        else
        {
            config->customKeys[i].vkCode = 0;
            config->customKeys[i].enabled = false;
        }
    }

    // Options
    config->rotationMode = GetPrivateProfileInt(S, L"rotationMode", 0, path) != 0;
    GetPrivateProfileString(S, L"rotationKeys", L"1234", config->rotationKeys, 16, path);
    config->randomDelay = GetPrivateProfileInt(S, L"randomDelay", 0, path) != 0;
    config->randomPercent = GetPrivateProfileInt(S, L"randomPercent", 20, path);
    config->useTimer = GetPrivateProfileInt(S, L"useTimer", 0, path) != 0;
    config->timerSeconds = GetPrivateProfileInt(S, L"timerSeconds", 60, path);
}
