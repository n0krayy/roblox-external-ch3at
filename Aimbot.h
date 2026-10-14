#pragma once
#include "SDK.h"
#include "Config.h"

// Aimbot: moves the mouse toward the best target while aimKey is held.
// SilentAim: on LMB, writes camera euler angles at the target for one frame
// and restores them, so the local view never visibly snaps.
class Aimbot
{
public:
    explicit Aimbot(const Memory* mem) : m_mem(mem) {}

    void Tick(const Roblox& game, int screenW, int screenH);
    void DrawOverlay(int screenW, int screenH);

private:
    Vec3 PickBone(const Entity& e) const;
    const Entity* FindTarget(const Roblox& game, int bone, float fovPx, int screenW, int screenH) const;
    void MoveMouseToward(Vec2 target, int screenW, int screenH) const;
    void SilentAimTick(const Roblox& game, const Entity* target);

    const Memory* m_mem = nullptr;
    bool m_silentArmed = false;
    Vec3 m_savedAngles{};
    uintptr_t m_camAddr = 0;
};
