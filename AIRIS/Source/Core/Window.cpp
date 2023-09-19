#include "Window.hpp"
#include "Utils/WinMessage.hpp"
#include "Input/Input.hpp"
#include "Events/EventSystem.hpp"
#include "Events/ApplicationEvents.hpp"
#include "Events/InputEvents.h"
#include "Log.hpp"

SWindow* SWindow::s_mainWindowInstance = nullptr;
SWindow::SWindow(HINSTANCE hInstance, WindowProps windowProperties)
    :m_hInstance(hInstance), m_windowProperties(windowProperties)
{
    // Remove this assert when support for multiple windows from this process is avaliable
    assert(!s_mainWindowInstance && "Multiple windows are not currently allowed.");

    // If this is the first window it is the main window;
    if (!s_mainWindowInstance)
        s_mainWindowInstance = this;

    Init();

    // Initialize Event and Input Sysetm
    EVENTSYSTEM->Init();
    INPUT->Init();


}

bool SWindow::Show()
{
    return ShowWindow(m_hWnd, m_windowProperties.StartProps.ShowCommand); 
}

int SWindow::PumpMessages()
{
    MSG msg = { 0 };
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void SWindow::Init()
{
    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = m_hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = m_windowProperties.ClassName.c_str();
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, static_cast<LONG>(m_windowProperties.Width), static_cast<LONG>(m_windowProperties.Height) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowEx(
        0,                                              // Optional window styles.
        m_windowProperties.ClassName.c_str(),           // Window class
        m_windowProperties.Title.c_str(),               // Window text
        WS_OVERLAPPEDWINDOW,                            // Window style

        CW_USEDEFAULT, CW_USEDEFAULT,                   // Size
        windowRect.right - windowRect.left,             // Position
        windowRect.bottom - windowRect.top,
        nullptr,                                        // Parent window    
        nullptr,                                        // Menu
        m_hInstance,                                      // Instance handle
        nullptr                                         // Additional application data
    );
}

// Main message handler for the sample.
LRESULT CALLBACK SWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

#if (DEBUG) || (_DEBUG)
    static WindowsMessageMap mm;
    static long int messageNum = 0;
    messageNum++;

    // Window Event info
    //CORE_TRACE("Message #{}: {}", messageNum, mm(message, wParam, lParam));

#endif

    // Get main window pointer
    SWindow* mainWindow = GetInstance();

    switch (message)
    {
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;

    case WM_DESTROY:
    {
        ref<WindowCloseEvent> e = CreateRef<WindowCloseEvent>();
        EVENTSYSTEM->QueueEvent(e);

        PostQuitMessage(0);
        break;
    }
    case WM_QUIT:
    {
        break;
    }
    case WM_ENTERSIZEMOVE:
    {
        GetInstance()->SetResizing(true);
        break;
    }
    case WM_EXITSIZEMOVE:
    {
        SWindow::GetInstance()->SetResizing(false);
        ref<WindowResizeEvent> e = CreateRef<WindowResizeEvent>(mainWindow->GetWidth(), mainWindow->GetHeight());
        EVENTSYSTEM->QueueEvent(e);
        break;
    }
    case WM_SIZE:
    {
        // Client area dimensions (does not include border)
        int WindowWidth = LOWORD(lParam);
        int WindowHeight = HIWORD(lParam);

        // Change window properties every time, but do not propogate an resize event 
        mainWindow->Resize(WindowWidth, WindowHeight);

        bool ShouldResize = false;

        if (wParam == SIZE_MINIMIZED)
        {
            mainWindow->SetMinimized(true);
            mainWindow->SetMaximized(false);
        }
        else if (wParam == SIZE_MAXIMIZED)
        {
            mainWindow->SetMinimized(false);
            mainWindow->SetMaximized(true);
            ShouldResize = true;
        }
        else if (wParam == SIZE_RESTORED)
        {
            if (mainWindow->IsMinimized() || mainWindow->IsMaximized() || !mainWindow->IsResizing())
            {
                ShouldResize = true;
            }
            mainWindow->SetMinimized(false);
            mainWindow->SetMaximized(false);
        }

        if (ShouldResize)
        {
            ref<WindowResizeEvent> e = CreateRef<WindowResizeEvent>(WindowWidth, WindowHeight);
            EVENTSYSTEM->QueueEvent(e);
        }
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        const int32_t originalKey = static_cast<int32_t>(wParam);
        int32_t physicalKey = originalKey,
                scancode = (HIWORD(lParam) & (KF_EXTENDED | 0xff));
        bool repeat = (lParam & 0x40000000) != 0;
        const InputState::KeyState action = (HIWORD(lParam) & (KF_UP) ? InputState::KeyUp : InputState::KeyDown);

        KeyCode key = INPUT->GetKey(scancode);

        // If extended get the correct physical key
        if (scancode & (KF_EXTENDED))
            physicalKey = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);

        INPUT->UpdateKeyState(key, action);

        if (!action)
        {
            ref<KeyReleasedEvent> e = CreateRef<KeyReleasedEvent>(key);
            EVENTSYSTEM->QueueEvent(e);
        }
        else if (action && !repeat)
        {
            ref<KeyPressedEvent> e = CreateRef<KeyPressedEvent>(key, repeat);
            EVENTSYSTEM->QueueEvent(e);
        }

        break;
    }
    case WM_SYSCHAR:
    case WM_CHAR:
    {
        // TODO: Get the actual character using the scancode in the lparam
        // Need to refactor Input System
        InputCode::KeyCode keycode = InputCode::KeyCode::A;
        // TODO: Refactor KeyTypedEvent event and rename it
        ref<KeyTypedEvent> e = CreateRef<KeyTypedEvent>(keycode, static_cast<char>(wParam));
        EVENTSYSTEM->QueueEvent(e);
        break;
    }
    case WM_MOUSEMOVE:
    {
        auto [x, y] = MAKEPOINTS(lParam);
        RECT ClientRect = {};
        ::GetClientRect(hWnd, &ClientRect);
        LONG Width = ClientRect.right - ClientRect.left;
        LONG Height = ClientRect.bottom - ClientRect.top;

        if (x <= 0 || x > Width || y <= 0 || y > Height)
            break;

        INPUT->UpdateMousePos(x, y);
        ref<MouseMovedEvent> e = CreateRef<MouseMovedEvent>(x, y);
        EVENTSYSTEM->QueueEvent(e);

        break;
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        MouseCode button = MBRight;
        if (message == WM_LBUTTONDOWN)
            button = MBLeft;
        else if (message == WM_MBUTTONDOWN)
            button = MBMiddle;
        else if (message == WM_RBUTTONDOWN)
            button = MBRight;
        auto [x, y] = MAKEPOINTS(lParam);

        INPUT->UpdateMBState(button, MouseButtonState::MBDown);
        ref<MouseButtonPressedEvent> e = CreateRef<MouseButtonPressedEvent>(button, x, y);
        EVENTSYSTEM->QueueEvent(e);

        // TODO: Update Input 
        break;
    }

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
        MouseCode button = MBLast;
        if (message == WM_LBUTTONUP)
            button = MBLeft;
        else if (message == WM_MBUTTONUP)
            button = MBMiddle;
        else if (message == WM_RBUTTONUP)
            button = MBRight;
        
        auto [x, y] = MAKEPOINTS(lParam);

        INPUT->UpdateMBState(button, MouseButtonState::MBUp);
        ref<MouseButtonReleasedEvent> e = CreateRef<MouseButtonReleasedEvent>(button, x, y);
        EVENTSYSTEM->QueueEvent(e);
        break;
    }

    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    {
        MouseCode button = MBLast;
        if (message == WM_LBUTTONDBLCLK)
            button = MBLeft;
        else if (message == WM_MBUTTONDBLCLK)
            button = MBMiddle;
        else if (message == WM_RBUTTONDBLCLK)
            button = MBRight;

        auto [x, y] = MAKEPOINTS(lParam);

        ref<MouseDoubleClickEvent> e = CreateRef<MouseDoubleClickEvent>(button, x, y);
        EVENTSYSTEM->QueueEvent(e);
        break;
    }

    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);

        auto [x, y] = MAKEPOINTS(lParam);

        ref<MouseScrolledEvent> e = CreateRef<MouseScrolledEvent>(delta, x, y);
        EVENTSYSTEM->QueueEvent(e);
    }
    break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }


    return 0;

}
