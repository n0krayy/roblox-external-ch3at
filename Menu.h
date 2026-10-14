#pragma once
#include "Config.h"

// ImGui menu: one window, tabs per feature set. Rendered on the overlay;
// click-through is toggled so the menu is usable when visible.
void DrawMenu();
void UpdateClickThrough(HWND overlayWnd);
