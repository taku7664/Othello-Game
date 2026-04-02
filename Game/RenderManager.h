#pragma once

#include <string>
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

class RenderManager final
{
public:
    RenderManager();
    ~RenderManager();
    RenderManager(const RenderManager&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;

public:
    bool Initialize(
        HINSTANCE hInstance,
        int width,
        int height,
        const wchar_t* title);
    void Finalize();
    void Update();

public:
    void BeginDraw();
    void EndDraw();
    void Present();

public:
    HWND GetHwnd() const;
    bool IsWindowedRenderEnabled() const;
    void SetWindowTitle(const std::wstring& title);

private:
    bool InitializeWindow(HINSTANCE hInstance, int width, int height, const wchar_t* title);
    bool InitializeD3D11();
    bool InitializeImGui();

    void FinalizeWindow();
    void FinalizeD3D11();
    void FinalizeImGui();

    void HandlePendingResize();
    bool CreateRenderTarget();
    void CleanupRenderTarget();

private:
    static LRESULT WINAPI StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    HWND        m_hwnd;
    WNDCLASSEXW m_wndClass;
    std::wstring m_windowClassName;
    std::wstring m_windowTitle;

    UINT m_width;
    UINT m_height;
    UINT m_resizeWidth;
    UINT m_resizeHeight;

    bool m_useRenderWindow;

    ID3D11Device* m_pd3dDevice;
    ID3D11DeviceContext* m_pd3dDeviceContext;
    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_mainRenderTargetView;
};