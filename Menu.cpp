#include "Menu.h"
#include "imgui.h"
#include <cstdio>

void UpdateClickThrough(HWND overlayWnd)
{
    LONG ex = GetWindowLongW(overlayWnd, GWL_EXSTYLE);
    if (g_cfg.showMenu)
        SetWindowLongW(overlayWnd, GWL_EXSTYLE, ex & ~WS_EX_TRANSPARENT);
    else
        SetWindowLongW(overlayWnd, GWL_EXSTYLE, ex | WS_EX_TRANSPARENT);
}

static void KeyPicker(const char* label, int* vk)
{
    // click the button, then press any key/mouse button to rebind
    static int* pending = nullptr;
    char btnLabel[64];
    std::snprintf(btnLabel, sizeof(btnLabel), "%s: [%d]", label, *vk);
    if (pending == vk)
        std::snprintf(btnLabel, sizeof(btnLabel), "%s: [...]", label);
    if (ImGui::Button(btnLabel))
        pending = vk;
    if (pending == vk)
    {
        for (int k = 1; k < 256; ++k)
        {
            if (k == VK_LBUTTON) continue;
            if (GetAsyncKeyState(k) & 0x8000)
            {
                *vk = k;
                pending = nullptr;
                break;
            }
        }
    }
}

void DrawMenu()
{
    if (!g_cfg.showMenu)
        return;

    ImGui::SetNextWindowSize({ 460, 520 }, ImGuiCond_FirstUseEver);
    ImGui::Begin("rbx-external", &g_cfg.showMenu, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("tabs"))
    {
        if (ImGui::BeginTabItem("Aim"))
        {
            ImGui::Checkbox("Aimbot", &g_cfg.aimbotEnabled);
            KeyPicker("Aim key", &g_cfg.aimKey);
            const char* bones[] = { "Head", "UpperTorso", "HumanoidRootPart" };
            ImGui::Combo("Bone", &g_cfg.aimBone, bones, 3);
            ImGui::SliderFloat("FOV (px)", &g_cfg.aimFov, 20.0f, 600.0f);
            ImGui::SliderFloat("Smooth", &g_cfg.smooth, 1.0f, 30.0f);
            ImGui::Checkbox("Team check", &g_cfg.teamCheck);
            ImGui::Separator();
            ImGui::Checkbox("Silent aim (on LMB)", &g_cfg.silentAim);
            ImGui::Combo("Silent bone", &g_cfg.silentBone, bones, 3);
            ImGui::SliderFloat("Silent FOV (px)", &g_cfg.silentFov, 20.0f, 600.0f);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("ESP"))
        {
            ImGui::Checkbox("Enabled", &g_cfg.espEnabled);
            ImGui::Checkbox("Corner boxes", &g_cfg.espBox);
            ImGui::Checkbox("Names", &g_cfg.espName);
            ImGui::Checkbox("Health bars", &g_cfg.espHealth);
            ImGui::Checkbox("Distance", &g_cfg.espDistance);
            ImGui::Checkbox("Snaplines", &g_cfg.espSnapline);
            ImGui::Checkbox("Team check", &g_cfg.espTeamCheck);
            ImGui::SliderFloat("Max distance (studs)", &g_cfg.espMaxDistance, 50.0f, 5000.0f);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Movement"))
        {
            ImGui::Checkbox("Fly", &g_cfg.fly);
            KeyPicker("Fly toggle", &g_cfg.flyKey);
            ImGui::SliderFloat("Fly speed", &g_cfg.flySpeed, 10.0f, 300.0f);
            ImGui::Checkbox("Noclip", &g_cfg.noclip);
            KeyPicker("Noclip toggle", &g_cfg.noclipKey);
            ImGui::Checkbox("Speed", &g_cfg.speedHack);
            ImGui::SliderFloat("WalkSpeed", &g_cfg.walkSpeed, 16.0f, 250.0f);
            ImGui::Checkbox("Infinite jump", &g_cfg.infJump);
            ImGui::TextWrapped("Fly: WASD + Space/Shift, steered by camera. "
                               "Noclip clears CanCollide on your parts every 120ms.");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Overlay"))
        {
            ImGui::Checkbox("FOV circles", &g_cfg.drawFov);
            ImGui::Checkbox("Crosshair", &g_cfg.drawCrosshair);
            KeyPicker("Menu key", &g_cfg.menuKey);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}
