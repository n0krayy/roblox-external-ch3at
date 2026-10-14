#pragma once
#include <Windows.h>
#include <functional>

// Fullscreen transparent DX11 overlay hosting ImGui. onRender is called every
// frame after ImGui::NewFrame; draw ESP/aim visuals and the menu there.
class Overlay
{
public:
    using RenderFn = std::function<void(int w, int h)>;

    bool Create(RenderFn onRender);
    void Run();      // message pump + render loop, returns on WM_QUIT
    void Destroy();

    int Width() const { return m_w; }
    int Height() const { return m_h; }

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
    int m_w = 0, m_h = 0;
    RenderFn m_onRender;
};
