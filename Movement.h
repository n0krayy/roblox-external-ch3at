#pragma once
#include "SDK.h"
#include "Config.h"
#include <chrono>

// Movement cheats: fly (position writes along camera basis), noclip
// (clears the CanCollide flag on every character part), walkspeed,
// jump power and infinite jump.
class Movement
{
public:
    Movement(const Memory* mem, const Roblox* game) : m_mem(mem), m_game(game) {}

    void Tick(float deltaTime);

private:
    void TickFly(float dt);
    void TickNoclip();
    void TickSpeed();
    void TickInfJump();

    Vec3 LocalHrpPos(uintptr_t& outHrpAddr) const;

    const Memory* m_mem = nullptr;
    const Roblox* m_game = nullptr;
    bool m_flyWasDown = false;
    bool m_noclipWasDown = false;
    std::chrono::steady_clock::time_point m_lastNoclip{};
};
