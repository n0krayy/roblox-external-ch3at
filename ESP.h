#pragma once
#include "SDK.h"
#include "Config.h"

// Projects one entity to screen space and hands back the 2D box corners.
struct EspBox
{
    bool onScreen = false;
    Vec2 topLeft{}, bottomRight{};
    Vec2 head{}, feet{};
};

EspBox ComputeEspBox(const Entity& e, const Matrix4x4& vm, int w, int h);
void DrawEsp(const Roblox& game, int screenW, int screenH);
