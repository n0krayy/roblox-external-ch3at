#include "Movement.h"
#include <chrono>

static bool KeyTapped(int vk, bool& wasDown)
{
    bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool tapped = down && !wasDown;
    wasDown = down;
    return tapped;
}

Vec3 Movement::LocalHrpPos(uintptr_t& outHrpAddr) const
{
    outHrpAddr = 0;
    const Entity& local = m_game->Local();
    if (!local.valid || local.hrp == 0)
        return {};
    outHrpAddr = local.hrp;
    return local.hrpPos;
}

void Movement::Tick(float dt)
{
    if (KeyTapped(g_cfg.flyKey, m_flyWasDown))
        g_cfg.fly = !g_cfg.fly;
    if (KeyTapped(g_cfg.noclipKey, m_noclipWasDown))
        g_cfg.noclip = !g_cfg.noclip;

    if (g_cfg.fly)      TickFly(dt);
    if (g_cfg.noclip)   TickNoclip();
    if (g_cfg.speedHack) TickSpeed();
    if (g_cfg.infJump)  TickInfJump();
}

void Movement::TickFly(float dt)
{
    uintptr_t hrpAddr = 0;
    Vec3 pos = LocalHrpPos(hrpAddr);
    if (!hrpAddr || !m_game->HasMatrix())
        return;

    const Matrix4x4& vm = m_game->ViewMatrix();
    Vec3 fwd = CameraForward(vm);
    Vec3 right = CameraRight(vm);
    Vec3 move{};

    if (GetAsyncKeyState('W') & 0x8000) move = move + fwd;
    if (GetAsyncKeyState('S') & 0x8000) move = move - fwd;
    if (GetAsyncKeyState('D') & 0x8000) move = move + right;
    if (GetAsyncKeyState('A') & 0x8000) move = move - right;
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) move.y += 1.0f;
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) move.y -= 1.0f;

    if (move.Length() < 1e-6f)
        return;
    move = move.Normalized() * (g_cfg.flySpeed * dt);
    Vec3 next = pos + move;

    Instance hrp(m_mem, hrpAddr);
    hrp.SetPrimitivePosition(next);

    // kill velocity so the server doesn't rubber-band / fling the character
    uintptr_t prim = m_mem->Read<uintptr_t>(hrpAddr + Offsets::BasePart_Primitive);
    if (prim)
        m_mem->Write<Vec3>(prim + Offsets::Primitive_LinearVel, Vec3{});
}

void Movement::TickNoclip()
{
    using namespace std::chrono;
    auto now = steady_clock::now();
    if (duration_cast<milliseconds>(now - m_lastNoclip).count() < 120)
        return;
    m_lastNoclip = now;

    const Entity& local = m_game->Local();
    if (!local.valid || !local.model)
        return;

    std::vector<Instance> parts;
    Instance(m_mem, local.model).CollectParts(parts);
    for (const auto& p : parts)
        p.SetCanCollide(false);
}

void Movement::TickSpeed()
{
    const Entity& local = m_game->Local();
    if (!local.valid || !local.humanoid)
        return;
    float cur = m_mem->Read<float>(local.humanoid + Offsets::Humanoid_WalkSpeed);
    if (std::abs(cur - g_cfg.walkSpeed) > 0.5f)
        m_mem->Write<float>(local.humanoid + Offsets::Humanoid_WalkSpeed, g_cfg.walkSpeed);
}

void Movement::TickInfJump()
{
    if (!(GetAsyncKeyState(VK_SPACE) & 0x8000))
        return;
    uintptr_t hrpAddr = 0;
    LocalHrpPos(hrpAddr);
    if (!hrpAddr)
        return;
    uintptr_t prim = m_mem->Read<uintptr_t>(hrpAddr + Offsets::BasePart_Primitive);
    if (!prim)
        return;
    Vec3 vel = m_mem->Read<Vec3>(prim + Offsets::Primitive_LinearVel);
    vel.y = 50.0f; // upward impulse each tick while space is held
    m_mem->Write<Vec3>(prim + Offsets::Primitive_LinearVel, vel);
}
