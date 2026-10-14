#include "Aimbot.h"
#include "imgui.h"
#include <cmath>

static constexpr float kPi = 3.14159265f;

Vec3 Aimbot::PickBone(const Entity& e) const
{
    switch (g_cfg.aimBone)
    {
    case 1: return e.torsoPos.x != 0 ? e.torsoPos : e.hrpPos;
    case 2: return e.hrpPos;
    case 3:
        // closest of head/torso/hrp to camera
        return e.headPos;
    default: return e.headPos.x != 0 ? e.headPos : e.hrpPos;
    }
}

const Entity* Aimbot::FindTarget(const Roblox& game, int bone, float fovPx, int screenW, int screenH) const
{
    if (!game.HasMatrix())
        return nullptr;

    const Matrix4x4& vm = game.ViewMatrix();
    const Entity& local = game.Local();
    const Entity* best = nullptr;
    float bestDist = fovPx;
    Vec2 center{ screenW * 0.5f, screenH * 0.5f };

    for (const Entity& e : game.Entities())
    {
        if (!e.Alive())
            continue;
        if (g_cfg.teamCheck && local.team != 0 && e.team == local.team)
            continue;

        Vec3 bonePos = (bone == 1) ? (e.torsoPos.x != 0 ? e.torsoPos : e.hrpPos)
                     : (bone == 2) ? e.hrpPos
                     : (e.headPos.x != 0 ? e.headPos : e.hrpPos);

        Vec2 s = WorldToScreen(vm, bonePos, screenW, screenH);
        if (s.x < 0)
            continue; // behind camera

        float d = std::hypot(s.x - center.x, s.y - center.y);
        if (d < bestDist)
        {
            bestDist = d;
            best = &e;
        }
    }
    return best;
}

void Aimbot::MoveMouseToward(Vec2 target, int screenW, int screenH) const
{
    float dx = target.x - screenW * 0.5f;
    float dy = target.y - screenH * 0.5f;
    float sm = g_cfg.smooth < 1.0f ? 1.0f : g_cfg.smooth;
    int mx = static_cast<int>(dx / sm);
    int my = static_cast<int>(dy / sm);
    if (mx == 0 && my == 0 && (std::abs(dx) > 0.5f || std::abs(dy) > 0.5f))
    {
        mx = dx > 0 ? 1 : -1;
        my = dy > 0 ? 1 : -1;
    }
    if (mx != 0 || my != 0)
        mouse_event(MOUSEEVENTF_MOVE, mx, my, 0, 0);
}

void Aimbot::SilentAimTick(const Roblox& game, const Entity* target)
{
    Instance cam = game.Camera();
    bool firing = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

    // restore path: angles were overwritten last frame
    if (m_silentArmed && (!g_cfg.silentAim || !firing || !target || !cam.Valid() || cam.Addr() != m_camAddr))
    {
        m_mem->Write<Vec3>(m_camAddr + Offsets::Camera_Rotation, m_savedAngles);
        m_silentArmed = false;
        m_camAddr = 0;
        return;
    }

    if (!g_cfg.silentAim || !firing || !target || !cam.Valid() || m_silentArmed)
        return;

    Vec3 camPos = game.CameraPosition();
    Vec3 bonePos = (g_cfg.silentBone == 1) ? (target->torsoPos.x != 0 ? target->torsoPos : target->hrpPos)
                 : (g_cfg.silentBone == 2) ? target->hrpPos
                 : (target->headPos.x != 0 ? target->headPos : target->hrpPos);
    Vec3 delta = bonePos - camPos;

    // euler the camera expects: pitch around X, yaw around Y (degrees)
    float yaw   = std::atan2(-delta.x, -delta.z) * 180.0f / kPi;
    float pitch = std::atan2(delta.y, std::hypot(delta.x, delta.z)) * 180.0f / kPi;

    m_camAddr = cam.Addr();
    m_savedAngles = m_mem->Read<Vec3>(m_camAddr + Offsets::Camera_Rotation);
    m_mem->Write<Vec3>(m_camAddr + Offsets::Camera_Rotation, Vec3{ pitch, yaw, 0.0f });
    m_silentArmed = true;
    // next Tick restores; the shot raycast uses the overwritten angles.
}

void Aimbot::Tick(const Roblox& game, int screenW, int screenH)
{
    const Entity* silentTarget = nullptr;
    if (g_cfg.silentAim)
        silentTarget = FindTarget(game, g_cfg.silentBone, g_cfg.silentFov, screenW, screenH);
    SilentAimTick(game, silentTarget);

    if (!g_cfg.aimbotEnabled)
        return;
    if (!(GetAsyncKeyState(g_cfg.aimKey) & 0x8000))
        return;

    const Entity* target = FindTarget(game, g_cfg.aimBone, g_cfg.aimFov, screenW, screenH);
    if (!target)
        return;

    Vec3 bonePos = PickBone(*target);
    Vec2 s = WorldToScreen(game.ViewMatrix(), bonePos, screenW, screenH);
    if (s.x < 0)
        return;
    MoveMouseToward(s, screenW, screenH);
}

void Aimbot::DrawOverlay(int screenW, int screenH)
{
    if (g_cfg.aimbotEnabled && g_cfg.drawFov)
    {
        ImDrawList* d = ImGui::GetBackgroundDrawList();
        d->AddCircle({ screenW * 0.5f, screenH * 0.5f }, g_cfg.aimFov,
                     IM_COL32(255, 255, 255, 90), 64, 1.5f);
    }
    if (g_cfg.silentAim && g_cfg.drawFov)
    {
        ImDrawList* d = ImGui::GetBackgroundDrawList();
        d->AddCircle({ screenW * 0.5f, screenH * 0.5f }, g_cfg.silentFov,
                     IM_COL32(120, 200, 255, 60), 64, 1.0f);
    }
    if (g_cfg.drawCrosshair)
    {
        ImDrawList* d = ImGui::GetBackgroundDrawList();
        float cx = screenW * 0.5f, cy = screenH * 0.5f;
        d->AddLine({ cx - 8, cy }, { cx + 8, cy }, IM_COL32(255, 255, 255, 200), 1.5f);
        d->AddLine({ cx, cy - 8 }, { cx, cy + 8 }, IM_COL32(255, 255, 255, 200), 1.5f);
    }
}
