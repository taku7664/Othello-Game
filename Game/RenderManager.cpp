#include "pch.h"
#include "RenderManager.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

RenderManager::RenderManager()
    : m_hwnd(nullptr)
    , m_wndClass{}
    , m_windowClassName(L"RenderManagerWindowClass")
    , m_windowTitle(L"My Window")
    , m_width(0)
    , m_height(0)
    , m_resizeWidth(0)
    , m_resizeHeight(0)
    , m_useRenderWindow(false)
    , m_pd3dDevice(nullptr)
    , m_pd3dDeviceContext(nullptr)
    , m_pSwapChain(nullptr)
    , m_mainRenderTargetView(nullptr)
{
}

RenderManager::~RenderManager()
{
    Finalize();
}

bool RenderManager::Initialize(HINSTANCE hInstance, int width, int height, const wchar_t* title)
{
    m_width = (width > 0) ? static_cast<UINT>(width) : 0;
    m_height = (height > 0) ? static_cast<UINT>(height) : 0;
    m_useRenderWindow = (m_width > 0 && m_height > 0);
    m_windowTitle = (title != nullptr) ? title : L"My Window";

    if (!InitializeWindow(hInstance, width, height, m_windowTitle.c_str()))
        return false;

    if (false == InitializeD3D11())
    {
        FinalizeWindow();
        return false;
    }

    if (false == InitializeImGui())
    {
        FinalizeD3D11();
        FinalizeWindow();
        return false;
    }

    return true;
}

void RenderManager::Finalize()
{
    FinalizeImGui();
    FinalizeD3D11();
    FinalizeWindow();
}

void RenderManager::Update()
{
    HandlePendingResize();
}

void RenderManager::BeginDraw()
{
    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();
}

void RenderManager::EndDraw()
{
    ImGui::Render();

    if (m_pd3dDeviceContext && m_mainRenderTargetView)
    {
        const float clearColor[4] = { 0.10f, 0.10f, 0.10f, 1.00f };
        m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
        m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, clearColor);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

	m_pSwapChain->Present(0, 0);
}

void RenderManager::Present()
{
    if (m_pSwapChain)
    {
        m_pSwapChain->Present(1, 0);
    }
}

HWND RenderManager::GetHwnd() const
{
    return m_hwnd;
}
bool RenderManager::IsWindowedRenderEnabled() const
{
    return m_useRenderWindow;
}

void RenderManager::SetWindowTitle(const std::wstring& title)
{
    m_windowTitle = title;

    if (m_hwnd)
    {
        SetWindowTextW(m_hwnd, m_windowTitle.c_str());
    }
}

bool RenderManager::InitializeWindow(HINSTANCE hInstance, int width, int height, const wchar_t* title)
{
    m_wndClass = {};
    m_wndClass.cbSize = sizeof(WNDCLASSEXW);
    m_wndClass.style = CS_CLASSDC;
    m_wndClass.lpfnWndProc = StaticWndProc;
    m_wndClass.cbClsExtra = 0;
    m_wndClass.cbWndExtra = 0;
    m_wndClass.hInstance = hInstance;
    m_wndClass.hIcon = nullptr;
    m_wndClass.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
    m_wndClass.hbrBackground = nullptr;
    m_wndClass.lpszMenuName = nullptr;
    m_wndClass.lpszClassName = m_windowClassName.c_str();
    m_wndClass.hIconSm = nullptr;

    if (!RegisterClassExW(&m_wndClass))
        return false;

    const DWORD style = WS_OVERLAPPEDWINDOW;

	RECT rc = { 0, 0, ( width > 0 ? width : 1 ), ( height > 0 ? height : 1 ) };
	AdjustWindowRect( &rc , style , FALSE );

	int windowWidth = rc.right - rc.left;
	int windowHeight = rc.bottom - rc.top;

	int screenWidth = GetSystemMetrics( SM_CXSCREEN );
	int screenHeight = GetSystemMetrics( SM_CYSCREEN );

	int posX = ( screenWidth - windowWidth ) / 2;
	int posY = ( screenHeight - windowHeight ) / 2;

	m_hwnd = CreateWindowExW(
		0 ,
		m_wndClass.lpszClassName ,
		title ,
		style ,
		posX ,  // 여기 변경
		posY ,  // 여기 변경
		windowWidth ,
		windowHeight ,
		nullptr ,
		nullptr ,
		hInstance ,
		this );

    if (!m_hwnd)
        return false;

    // 0,0이면 ImGui 컨텍스트/플랫폼 이벤트만 쓰는 용도라고 보고
    // ShowWindow / UpdateWindow / D3D 초기화를 하지 않음.
    if (m_useRenderWindow)
    {
        ShowWindow(m_hwnd, SW_SHOWDEFAULT);
        UpdateWindow(m_hwnd);
    }

    return true;
}

bool RenderManager::InitializeD3D11()
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = m_width;
    sd.BufferDesc.Height = m_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    UINT createDeviceFlags = 0;
#if defined(_DEBUG)
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    const D3D_FEATURE_LEVEL featureLevelArray[2] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &m_pSwapChain,
        &m_pd3dDevice,
        &featureLevel,
        &m_pd3dDeviceContext);

    if (hr == DXGI_ERROR_UNSUPPORTED)
    {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            createDeviceFlags,
            featureLevelArray,
            2,
            D3D11_SDK_VERSION,
            &sd,
            &m_pSwapChain,
            &m_pd3dDevice,
            &featureLevel,
            &m_pd3dDeviceContext);
    }

    if (FAILED(hr))
        return false;

    IDXGIFactory* factory = nullptr;
    if (SUCCEEDED(m_pSwapChain->GetParent(IID_PPV_ARGS(&factory))))
    {
        factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER);
        factory->Release();
    }

    if (!CreateRenderTarget())
        return false;

    return true;
}

bool RenderManager::InitializeImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	io.IniFilename = nullptr; // ini파일 생성 X

    ImGuiStyle& style = ImGui::GetStyle();
    if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

	// 폰트 경로는 실제 폰트 파일로 바꿔주세요
	ImFontConfig fontConfig;
	fontConfig.OversampleH = 3;
	fontConfig.OversampleV = 3;
	fontConfig.PixelSnapH = true;

	// 유니코드 범위 설정 (한글 + 로마 숫자 포함)
	static const ImWchar customRanges[ ] = {
		0x0020, 0x00FF, // 기본 라틴
		0x1100, 0x11FF, // 한글 자모
		0x3130, 0x318F, // 한글 자모 (호환)
		0xAC00, 0xD7AF, // 한글 완성형
		0x2160, 0x2188, // 로마 숫자!!!
		0,              // 종료
	};
	ImFont* mainFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 15.0f, &fontConfig, customRanges);
	//io.Fonts->Build();

    ImGui_ImplWin32_EnableDpiAwareness();

    if (!ImGui_ImplWin32_Init(m_hwnd))
        return false;

    if (false == ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dDeviceContext))
    {
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    auto& colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg]			= ImVec4(0.1f, 0.105f, 0.11f, 1.0f);

    colors[ImGuiCol_Header]				= ImVec4(0.1f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_HeaderHovered]		= ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_HeaderActive]		= ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
												
    colors[ImGuiCol_Button]				= ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_ButtonHovered]		= ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_ButtonActive]		= ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
												
    colors[ImGuiCol_FrameBg]			= ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    colors[ImGuiCol_FrameBgHovered]		= ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    colors[ImGuiCol_FrameBgActive]		= ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
												
    colors[ImGuiCol_Tab]				= ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TabHovered]			= ImVec4(0.38f, 0.3805f, 0.381f, 1.0f);
    colors[ImGuiCol_TabActive]			= ImVec4(0.28f, 0.2805f, 0.281f, 1.0f);
    colors[ImGuiCol_TabUnfocused]		= ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f );
												
    colors[ImGuiCol_TitleBg]			= ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TitleBgActive]		= ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed]	= ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
												
    colors[ImGuiCol_DragDropTarget]		= ImVec4(0.2f, 0.6f, 0.4f, 1.0f );

    return true;
}

void RenderManager::FinalizeWindow()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    if (m_wndClass.lpszClassName && m_wndClass.hInstance)
    {
        UnregisterClassW(m_wndClass.lpszClassName, m_wndClass.hInstance);
    }

    m_wndClass = {};
}

void RenderManager::FinalizeD3D11()
{
    CleanupRenderTarget();

    if (m_pSwapChain)
    {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }

    if (m_pd3dDeviceContext)
    {
        m_pd3dDeviceContext->Release();
        m_pd3dDeviceContext = nullptr;
    }

    if (m_pd3dDevice)
    {
        m_pd3dDevice->Release();
        m_pd3dDevice = nullptr;
    }
}

void RenderManager::FinalizeImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void RenderManager::HandlePendingResize()
{
    if (m_resizeWidth == 0 || m_resizeHeight == 0)
        return;

	if(nullptr == m_pSwapChain || nullptr == m_mainRenderTargetView)
	{
		return;
	}

    CleanupRenderTarget();

    m_pSwapChain->ResizeBuffers(0, m_resizeWidth, m_resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
    CreateRenderTarget();

    m_width = m_resizeWidth;
    m_height = m_resizeHeight;
    m_resizeWidth = 0;
    m_resizeHeight = 0;
}

bool RenderManager::CreateRenderTarget()
{
    if (!m_pSwapChain || !m_pd3dDevice)
        return false;

    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr))
        return false;

    hr = m_pd3dDevice->CreateRenderTargetView(backBuffer, nullptr, &m_mainRenderTargetView);
    backBuffer->Release();

    return SUCCEEDED(hr);
}

void RenderManager::CleanupRenderTarget()
{
    if (m_mainRenderTargetView)
    {
        m_mainRenderTargetView->Release();
        m_mainRenderTargetView = nullptr;
    }
}

LRESULT WINAPI RenderManager::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTW* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        RenderManager* self = reinterpret_cast<RenderManager*>(createStruct->lpCreateParams);
        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }

    RenderManager* self = reinterpret_cast<RenderManager*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    if (self)
    {
        return self->WndProc(hWnd, msg, wParam, lParam);
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

LRESULT RenderManager::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return TRUE;

    switch (msg)
    {
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
            return 0;

        m_resizeWidth = static_cast<UINT>(LOWORD(lParam));
        m_resizeHeight = static_cast<UINT>(HIWORD(lParam));
        HandlePendingResize();
        return 0;
    }
    case WM_SYSCOMMAND:
    {
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    }
    case WM_DESTROY:
    {
        ::PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}