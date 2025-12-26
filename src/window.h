#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>

// Window dimensions
#define WINDOW_WIDTH    420
#define WINDOW_HEIGHT   390

// Initialize and create the main window
HWND CreateMainWindow(HINSTANCE hInstance);

// Create all UI controls
void CreateControls(HWND hWnd, HINSTANCE hInstance);

// Get control handles
HWND GetControlHandle(int controlId);

// Read settings from UI
struct ClickerConfig;
void ReadSettingsFromUI(ClickerConfig *config);

// Update status display
void UpdateStatusDisplay(HWND hWnd, bool isRunning);

#endif // WINDOW_H
