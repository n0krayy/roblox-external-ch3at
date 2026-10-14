#include "Memory.h"
#include "SDK.h"
#include "Config.h"
#include "Overlay.h"
#include "Menu.h"
#include "ESP.h"
#include "Aimbot.h"
#include "Movement.h"

#include "imgui.h"
#include <thread>
#include <atomic>
#include <chrono>

static Memory g_mem;
static Roblox g_game(&g_mem);
static Aimbot g_aim(&g_mem);
static Movement* g_move = nullptr;
static std::atomic<bool> g_running{ true };
static bool g_menuWasDown = false;

// Background thread: keep the object graph + entity cache fresh.
// Render thread only reads; swap is atomic enough for an overlay.
static void CacheThread()
{
    using namespace std::chrono;
    auto lastResolve = steady_clock::now() - seconds(10);
    while (g_running)
    {
        auto now = steady_clock::now();
        if (duration_cast<milliseconds>(now - lastResolve).count() > 500)
        {
            lastResolve = now;
            if (g_game.Resolve())
                g_game.UpdateEntities();
        }
        std::this_thread::sleep_for(milliseconds(50));
    }
}

static void OnFrame(int w, int h)
{
    // menu toggle
    bool down = (GetAsyncKeyState(g_cfg.menuKey) & 0x8000) != 0;
    if (down && !g_menuWasDown)
    {
        g_cfg.showMenu = !g_cfg.showMenu;
        UpdateClickThrough(FindWindowW(L"rbx_ext_overlay", nullptr));
    }
    g_menuWasDown = down;

    if (g_mem.IsAttached())
    {
        static auto last = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        if (dt > 0.1f) dt = 0.1f;

        g_move->Tick(dt);
        g_aim.Tick(g_game, w, h);
    }

    DrawEsp(g_game, w, h);
    g_aim.DrawOverlay(w, h);
    DrawMenu();
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    // Wait for the game instead of failing outright.
    while (!g_mem.Attach(L"RobloxPlayerBeta.exe"))
    {
        if (MessageBoxW(nullptr,
                L"RobloxPlayerBeta.exe not found (or handle denied).\n"
                L"Launch Roblox, then press Retry.\n"
                L"If it keeps failing, Byfron is stripping the handle \u2014 "
                L"see README (driver section).",
                L"rbx-external", MB_RETRYCANCEL | MB_ICONWARNING) == IDCANCEL)
            return 1;
    }

    g_move = new Movement(&g_mem, &g_game);

    std::thread cache(CacheThread);
    cache.detach();

    Overlay overlay;
    if (!overlay.Create(OnFrame))
    {
        MessageBoxW(nullptr, L"Overlay init failed.", L"rbx-external", MB_ICONERROR);
        return 1;
    }
    overlay.Run();
    overlay.Destroy();

    g_running = false;
    delete g_move;
    return 0;
}
