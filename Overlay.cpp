#include "Overlay.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dwmapi.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

static ID3D11Device*           g_pd3dDevice = nullptr;
static ID3D11DeviceContext*    g_pd3dContext = nullptr;
static IDXGISwapChain*         g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_pRTV = nullptr;

static bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL fl;
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
                                     nullptr, 0, D3D11_SDK_VERSION, &sd,
                                     &g_pSwapChain, &g_pd3dDevice, &fl, &g_pd3dContext) != S_OK)
        return false;

    ID3D11Texture2D* back = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&back));
    g_pd3dDevice->CreateRenderTargetView(back, nullptr, &g_pRTV);
    back->Release();
    return true;
}

static void CleanupDeviceD3D()
{
    if (g_pRTV)      { g_pRTV->Release(); g_pRTV = nullptr; }
    if (g_pSwapChain){ g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dContext){ g_pd3dContext->Release(); g_pd3dContext = nullptr; }
    if (g_pd3dDevice){ g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

LRESULT CALLBACK Overlay::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool Overlay::Create(RenderFn onRender)
{
    m_onRender = std::move(onRender);

    m_w = GetSystemMetrics(SM_CXSCREEN);
    m_h = GetSystemMetrics(SM_CYSCREEN);

    WNDCLASSEXW wc{ sizeof(wc) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"rbx_ext_overlay";
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        wc.lpszClassName, L"", WS_POPUP,
        0, 0, m_w, m_h, nullptr, nullptr, wc.hInstance, nullptr);
    if (!m_hwnd)
        return false;

    SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // extend the frame over the whole window so DWM composites it transparently
    MARGINS m{ -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(m_hwnd, &m);

    if (!CreateDeviceD3D(m_hwnd))
        return false;

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(m_hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);
    return true;
}

void Overlay::Run()
{
    MSG msg{};
    bool done = false;
    while (!done)
    {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (m_onRender)
            m_onRender(m_w, m_h);

        ImGui::Render();
        const float clear[4] = { 0, 0, 0, 0 };
        g_pd3dContext->OMSetRenderTargets(1, &g_pRTV, nullptr);
        g_pd3dContext->ClearRenderTargetView(g_pRTV, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }
}

void Overlay::Destroy()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}
