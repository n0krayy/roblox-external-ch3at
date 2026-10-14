#pragma once
#include <cstdint>

// ---------------------------------------------------------------------------
// Offsets for RobloxPlayerBeta.exe
// Sourced from GitHub: Offsetmanager/Roblox-Internal-External-Offsets
// Snapshot: Windows/version-f5a60436d48947d3  (External_Offsets.hpp)
// Fetched 2026-10-14. Roblox ships a new build roughly weekly; when this
// stops resolving, re-pull the latest version folder from that repo and
// replace the values below. The names are kept identical to the upstream
// file so a diff is mechanical.
// Alternate live sources:
//   - https://github.com/4anti/Roblox-Offsets-ALL        (mirrors endpoints)
//   - https://offsets.imtheo.lol/  / https://dev.imtheo.lol/Offsets/
// ---------------------------------------------------------------------------

namespace Offsets
{
    // --- module-relative base pointers -------------------------------------
    inline constexpr uintptr_t FakeDataModel_Pointer   = 0x8ca9cc8; // .text static -> FakeDataModel
    inline constexpr uintptr_t FakeDataModel_RealData = 0x1f8;     // FakeDataModel -> DataModel
    inline constexpr uintptr_t VisualEngine_Pointer    = 0x82e2128; // .data -> VisualEngine
    inline constexpr uintptr_t VisualEngine_ViewMatrix = 0x180;     // VisualEngine -> float[16]

    // --- Instance -----------------------------------------------------------
    inline constexpr uintptr_t Instance_ChildrenStart  = 0x78; // vector<uintptr_t> first
    inline constexpr uintptr_t Instance_ChildrenEnd    = 0x80; // vector<uintptr_t> last (start + 8)
    inline constexpr uintptr_t Instance_ClassDesc      = 0x18; // -> ClassDescriptor
    inline constexpr uintptr_t ClassDesc_ClassName     = 0x8;  // ClassDescriptor -> rbx string
    inline constexpr uintptr_t Instance_Name           = 0x70; // rbx string container
    inline constexpr uintptr_t RbxString_Length        = 0x10; // container + 0x10 -> length

    // --- DataModel / services ----------------------------------------------
    inline constexpr uintptr_t DataModel_Workspace     = 0x158; // -> Workspace
    inline constexpr uintptr_t Players_LocalPlayer     = 0x130; // Players service -> LocalPlayer

    // --- Player --------------------------------------------------------------
    inline constexpr uintptr_t Player_ModelInstance    = 0x298; // -> character Model
    inline constexpr uintptr_t Player_Team              = 0x2d8; // -> Team instance
    inline constexpr uintptr_t Player_DisplayName      = 0x138; // rbx string
    inline constexpr uintptr_t Player_UserId           = 0xd0;  // int

    // --- Workspace / Camera ---------------------------------------------------
    inline constexpr uintptr_t Workspace_CurrentCamera = 0x498; // -> Camera
    inline constexpr uintptr_t Camera_Position         = 0xfc;  // Vector3
    inline constexpr uintptr_t Camera_Rotation         = 0xd8;  // Vector3 euler (pitch, yaw, roll)

    // --- BasePart / Primitive --------------------------------------------------
    inline constexpr uintptr_t BasePart_Primitive      = 0x188; // -> Primitive
    inline constexpr uintptr_t Primitive_Position      = 0xec;  // Vector3
    inline constexpr uintptr_t Primitive_Rotation      = 0xc8;  // 3x3 matrix
    inline constexpr uintptr_t Primitive_Flags         = 0x1b6; // uint8 bitfield
    inline constexpr uintptr_t Primitive_LinearVel     = 0xf8;  // Vector3
    inline constexpr uint8_t   PrimitiveFlag_CanCollide = 0x8;

    // --- Humanoid ---------------------------------------------------------------
    inline constexpr uintptr_t Humanoid_HRP            = 0x478; // -> HumanoidRootPart
    inline constexpr uintptr_t Humanoid_Health         = 0x190; // float
    inline constexpr uintptr_t Humanoid_MaxHealth      = 0x1a8; // float
    inline constexpr uintptr_t Humanoid_WalkSpeed      = 0x1d0; // float
    inline constexpr uintptr_t Humanoid_JumpPower      = 0x1a4; // float
}
