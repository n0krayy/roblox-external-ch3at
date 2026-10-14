#pragma once
#include <Windows.h>

// Every tunable lives here so the menu is just a view over this struct.
struct Config
{
    // Aimbot
    bool aimbotEnabled = false;
    int  aimKey = VK_XBUTTON2;   // hold to aim
    int  aimBone = 0;            // 0 = Head, 1 = UpperTorso, 2 = HumanoidRootPart, 3 = Closest
    float aimFov = 120.0f;        // pixels
    float smooth = 8.0f;          // higher = slower/smoother
    bool  teamCheck = true;
    bool  visibleCheck = true;    // skip targets behind camera (true w2s), plus health>0

    // Silent aim (fires on LMB)
    bool silentAim = false;
    int  silentBone = 0;
    float silentFov = 150.0f;

    // ESP
    bool espEnabled = true;
    bool espBox = true;          // 2D corner boxes
    bool espName = true;
    bool espHealth = true;
    bool espDistance = true;
    bool espSnapline = false;
    bool espTeamCheck = true;
    float espMaxDistance = 1000.0f; // studs

    // Movement
    bool fly = false;
    float flySpeed = 60.0f;      // studs/sec
    int  flyKey = 'F';           // toggle
    bool noclip = false;
    int  noclipKey = 'N';        // toggle
    bool speedHack = false;
    float walkSpeed = 32.0f;
    bool infJump = false;

    // Overlay
    bool showMenu = true;
    int  menuKey = VK_INSERT;
    bool drawFov = true;
    bool drawCrosshair = true;
};

inline Config g_cfg;
