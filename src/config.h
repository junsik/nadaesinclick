#ifndef CONFIG_H
#define CONFIG_H

#include "clicker.h"

// Save configuration to ini file (next to exe)
void SaveConfig(const ClickerConfig *config);

// Load configuration from ini file (next to exe)
void LoadConfig(ClickerConfig *config);

#endif // CONFIG_H
