#include "ESP.h"
#include "imgui.h"
#include <string>
#include <cstdio>

EspBox ComputeEspBox(const Entity& e, const Matrix4x4& vm, int w, int h)
{
    EspBox box;
    if (!e.Alive())
        return box;

    // fall back to an estimated head height when the Head part wasn't resolved
    Vec3 topPos = e.headPos.Length() > 0.1f ? e.headPos : e.hrpPos + Vec3{ 0.0f, 2.5f, 0.0f };

    box.head = WorldToScreen(vm, topPos, w, h);
    box.feet = WorldToScreen(vm, e.hrpPos, w, h);
    if (box.head.x < 0 || box.feet.x < 0)
        return box;

    float height = box.feet.y - box.head.y;
    if (height <= 0)
        return box;
    float width = height * 0.55f;
    box.topLeft = { box.feet.x - width * 0.5f, box.head.y };
    box.bottomRight = { box.feet.x + width * 0.5f, box.feet.y };
    box.onScreen = true;
    return box;
}

static void CornerBox(ImDrawList* d, const Vec2& tl, const Vec2& br, ImU32 col, float t = 1.5f)
{
    float lenX = (br.x - tl.x) * 0.3f;
    float lenY = (br.y - tl.y) * 0.25f;
    // top-left
    d->AddLine({ tl.x, tl.y }, { tl.x + lenX, tl.y }, col, t);
    d->AddLine({ tl.x, tl.y }, { tl.x, tl.y + lenY }, col, t);
    // top-right
    d->AddLine({ br.x, tl.y }, { br.x - lenX, tl.y }, col, t);
    d->AddLine({ br.x, tl.y }, { br.x, tl.y + lenY }, col, t);
    // bottom-left
    d->AddLine({ tl.x, br.y }, { tl.x + lenX, br.y }, col, t);
    d->AddLine({ tl.x, br.y }, { tl.x, br.y - lenY }, col, t);
    // bottom-right
    d->AddLine({ br.x, br.y }, { br.x - lenX, br.y }, col, t);
    d->AddLine({ br.x, br.y }, { br.x, br.y - lenY }, col, t);
}

void DrawEsp(const Roblox& game, int screenW, int screenH)
{
    if (!g_cfg.espEnabled || !game.HasMatrix())
        return;

    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    const Matrix4x4& vm = game.ViewMatrix();
    const Entity& local = game.Local();
    Vec3 camPos = game.CameraPosition();

    for (const Entity& e : game.Entities())
    {
        if (!e.Alive())
            continue;
        if (g_cfg.espTeamCheck && local.team != 0 && e.team == local.team)
            continue;

        float dist = camPos.Dist(e.hrpPos);
        if (dist > g_cfg.espMaxDistance)
            continue;

        EspBox box = ComputeEspBox(e, vm, screenW, screenH);
        if (!box.onScreen)
            continue;

        ImVec2 tl{ box.topLeft.x, box.topLeft.y };
        ImVec2 br{ box.bottomRight.x, box.bottomRight.y };
        ImU32 col = IM_COL32(255, 60, 60, 255);

        if (g_cfg.espSnapline)
            draw->AddLine({ screenW * 0.5f, static_cast<float>(screenH) },
                          { (tl.x + br.x) * 0.5f, br.y }, IM_COL32(255, 255, 255, 90), 1.0f);

        if (g_cfg.espBox)
        {
            draw->AddRect(tl, br, IM_COL32(0, 0, 0, 160), 0.0f, 0, 3.0f); // outline
            CornerBox(draw, box.topLeft, box.bottomRight, col);
        }

        float y = tl.y - 4.0f;
        char buf[128];

        if (g_cfg.espName)
        {
            const std::string& n = !e.displayName.empty() ? e.displayName : e.name;
            std::snprintf(buf, sizeof(buf), "%s", n.c_str());
            ImVec2 ts = ImGui::CalcTextSize(buf);
            draw->AddText({ (tl.x + br.x) * 0.5f - ts.x * 0.5f, y - ts.y }, IM_COL32(255, 255, 255, 255), buf);
            y -= ts.y;
        }
        if (g_cfg.espDistance)
        {
            std::snprintf(buf, sizeof(buf), "%.0fm", dist);
            ImVec2 ts = ImGui::CalcTextSize(buf);
            draw->AddText({ (tl.x + br.x) * 0.5f - ts.x * 0.5f, br.y + 2.0f }, IM_COL32(200, 200, 200, 255), buf);
        }
        if (g_cfg.espHealth)
        {
            float frac = e.maxHealth > 0 ? e.health / e.maxHealth : 0;
            frac = frac < 0 ? 0 : (frac > 1 ? 1 : frac);
            float barH = br.y - tl.y;
            ImVec2 b0{ tl.x - 6.0f, tl.y }, b1{ tl.x - 2.0f, br.y };
            draw->AddRectFilled(b0, b1, IM_COL32(0, 0, 0, 160));
            float fillH = barH * frac;
            ImU32 hc = IM_COL32(static_cast<int>(255 * (1 - frac)), static_cast<int>(255 * frac), 40, 255);
            draw->AddRectFilled({ b0.x, b1.y - fillH }, b1, hc);
        }
    }
}
