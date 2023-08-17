#include "Input.hpp"
#include "InputCodes.hpp"

#include <string>
#include <memory>
#include <cassert>



Input* Input::GetInstance()
{
    static Input inputInstance;
    return &inputInstance;
}

bool Input::Init()
{
    if (!m_initialized)
    {
        InitializeKeyTables();
        InitializeInputState();
        m_initialized = true;
    }
    return true;
}

void Input::UpdateKeyState(KeyCode keycode, KeyState state)
{
    // If current and last state was KeyDown, then the state should be raised to KeyRepeat
    if (state == KeyState::KeyDown && m_inputState.Keys[keycode] > KeyState::KeyUp)
        state = KeyState::KeyRepeat;

    // Ignore Warning: keycode will be valid
    m_inputState.Keys[keycode] = state;
}

void Input::UpdateMBState(MouseCode mouseCode, MouseButtonState state)
{
    if (state == MouseButtonState::MBDown && m_inputState.Keys[state] > MouseButtonState::MBUp)
        state = MouseButtonState::MBHeld;

    m_inputState.MouseButtons[mouseCode] = state;
}

void Input::UpdateMousePos(uint16_t x, uint16_t y)
{
    m_inputState.CursorState = { x, y, MouseMoved};
}

void Input::InitializeKeyTables()
{
    // Init KeyCodes and ScanCodes to -1 for unknown
    memset(m_inputCodes.KeyCodes, -1, sizeof(m_inputCodes.KeyCodes));
    memset(m_inputCodes.ScanCodes, -1, sizeof(m_inputCodes.ScanCodes));

    m_inputCodes.KeyCodes[0x00B] = InputCode::D0;
    m_inputCodes.KeyCodes[0x002] = InputCode::D1;
    m_inputCodes.KeyCodes[0x003] = InputCode::D2;
    m_inputCodes.KeyCodes[0x004] = InputCode::D3;
    m_inputCodes.KeyCodes[0x005] = InputCode::D4;
    m_inputCodes.KeyCodes[0x006] = InputCode::D5;
    m_inputCodes.KeyCodes[0x007] = InputCode::D6;
    m_inputCodes.KeyCodes[0x008] = InputCode::D7;
    m_inputCodes.KeyCodes[0x009] = InputCode::D8;
    m_inputCodes.KeyCodes[0x00A] = InputCode::D9;
    m_inputCodes.KeyCodes[0x01E] = InputCode::A;
    m_inputCodes.KeyCodes[0x030] = InputCode::B;
    m_inputCodes.KeyCodes[0x02E] = InputCode::C;
    m_inputCodes.KeyCodes[0x020] = InputCode::D;
    m_inputCodes.KeyCodes[0x012] = InputCode::E;
    m_inputCodes.KeyCodes[0x021] = InputCode::F;
    m_inputCodes.KeyCodes[0x022] = InputCode::G;
    m_inputCodes.KeyCodes[0x023] = InputCode::H;
    m_inputCodes.KeyCodes[0x017] = InputCode::I;
    m_inputCodes.KeyCodes[0x024] = InputCode::J;
    m_inputCodes.KeyCodes[0x025] = InputCode::K;
    m_inputCodes.KeyCodes[0x026] = InputCode::L;
    m_inputCodes.KeyCodes[0x032] = InputCode::M;
    m_inputCodes.KeyCodes[0x031] = InputCode::N;
    m_inputCodes.KeyCodes[0x018] = InputCode::O;
    m_inputCodes.KeyCodes[0x019] = InputCode::P;
    m_inputCodes.KeyCodes[0x010] = InputCode::Q;
    m_inputCodes.KeyCodes[0x013] = InputCode::R;
    m_inputCodes.KeyCodes[0x01F] = InputCode::S;
    m_inputCodes.KeyCodes[0x014] = InputCode::T;
    m_inputCodes.KeyCodes[0x016] = InputCode::U;
    m_inputCodes.KeyCodes[0x02F] = InputCode::V;
    m_inputCodes.KeyCodes[0x011] = InputCode::W;
    m_inputCodes.KeyCodes[0x02D] = InputCode::X;
    m_inputCodes.KeyCodes[0x015] = InputCode::Y;
    m_inputCodes.KeyCodes[0x02C] = InputCode::Z;
    m_inputCodes.KeyCodes[0x028] = InputCode::Apostrophe;
    m_inputCodes.KeyCodes[0x02B] = InputCode::Backslash;
    m_inputCodes.KeyCodes[0x033] = InputCode::Comma;
    m_inputCodes.KeyCodes[0x00D] = InputCode::Equal;
    m_inputCodes.KeyCodes[0x029] = InputCode::GraveAccent;
    m_inputCodes.KeyCodes[0x01A] = InputCode::LeftBracket;
    m_inputCodes.KeyCodes[0x00C] = InputCode::Minus;
    m_inputCodes.KeyCodes[0x034] = InputCode::Period;
    m_inputCodes.KeyCodes[0x01B] = InputCode::RightBracket;
    m_inputCodes.KeyCodes[0x027] = InputCode::Semicolon;
    m_inputCodes.KeyCodes[0x035] = InputCode::Slash;
    m_inputCodes.KeyCodes[0x056] = InputCode::World2;
    m_inputCodes.KeyCodes[0x00E] = InputCode::Backspace;
    m_inputCodes.KeyCodes[0x153] = InputCode::Delete;
    m_inputCodes.KeyCodes[0x14F] = InputCode::End;
    m_inputCodes.KeyCodes[0x01C] = InputCode::Enter;
    m_inputCodes.KeyCodes[0x001] = InputCode::Escape;
    m_inputCodes.KeyCodes[0x147] = InputCode::Home;
    m_inputCodes.KeyCodes[0x152] = InputCode::Insert;
    m_inputCodes.KeyCodes[0x15D] = InputCode::Menu;
    m_inputCodes.KeyCodes[0x151] = InputCode::PageDown;
    m_inputCodes.KeyCodes[0x149] = InputCode::PageUp;
    m_inputCodes.KeyCodes[0x045] = InputCode::Pause;
    m_inputCodes.KeyCodes[0x039] = InputCode::Space;
    m_inputCodes.KeyCodes[0x00F] = InputCode::Tab;
    m_inputCodes.KeyCodes[0x03A] = InputCode::CapsLock;
    m_inputCodes.KeyCodes[0x145] = InputCode::NumLock;
    m_inputCodes.KeyCodes[0x046] = InputCode::ScrollLock;
    m_inputCodes.KeyCodes[0x03B] = InputCode::F1;
    m_inputCodes.KeyCodes[0x03C] = InputCode::F2;
    m_inputCodes.KeyCodes[0x03D] = InputCode::F3;
    m_inputCodes.KeyCodes[0x03E] = InputCode::F4;
    m_inputCodes.KeyCodes[0x03F] = InputCode::F5;
    m_inputCodes.KeyCodes[0x040] = InputCode::F6;
    m_inputCodes.KeyCodes[0x041] = InputCode::F7;
    m_inputCodes.KeyCodes[0x042] = InputCode::F8;
    m_inputCodes.KeyCodes[0x043] = InputCode::F9;
    m_inputCodes.KeyCodes[0x044] = InputCode::F10;
    m_inputCodes.KeyCodes[0x057] = InputCode::F11;
    m_inputCodes.KeyCodes[0x058] = InputCode::F12;
    m_inputCodes.KeyCodes[0x064] = InputCode::F13;
    m_inputCodes.KeyCodes[0x065] = InputCode::F14;
    m_inputCodes.KeyCodes[0x066] = InputCode::F15;
    m_inputCodes.KeyCodes[0x067] = InputCode::F16;
    m_inputCodes.KeyCodes[0x068] = InputCode::F17;
    m_inputCodes.KeyCodes[0x069] = InputCode::F18;
    m_inputCodes.KeyCodes[0x06A] = InputCode::F19;
    m_inputCodes.KeyCodes[0x06B] = InputCode::F20;
    m_inputCodes.KeyCodes[0x06C] = InputCode::F21;
    m_inputCodes.KeyCodes[0x06D] = InputCode::F22;
    m_inputCodes.KeyCodes[0x06E] = InputCode::F23;
    m_inputCodes.KeyCodes[0x076] = InputCode::F24;
    m_inputCodes.KeyCodes[0x038] = InputCode::LeftAlt;
    m_inputCodes.KeyCodes[0x01D] = InputCode::LeftControl;
    m_inputCodes.KeyCodes[0x02A] = InputCode::LeftShift;
    m_inputCodes.KeyCodes[0x15B] = InputCode::LeftSuper;
    m_inputCodes.KeyCodes[0x137] = InputCode::PrintScreen;
    m_inputCodes.KeyCodes[0x138] = InputCode::RightAlt;
    m_inputCodes.KeyCodes[0x11D] = InputCode::RightControl;
    m_inputCodes.KeyCodes[0x036] = InputCode::RightShift;
    m_inputCodes.KeyCodes[0x15C] = InputCode::RightSuper;
    m_inputCodes.KeyCodes[0x150] = InputCode::Down;
    m_inputCodes.KeyCodes[0x14B] = InputCode::Left;
    m_inputCodes.KeyCodes[0x14D] = InputCode::Right;
    m_inputCodes.KeyCodes[0x148] = InputCode::Up;
    m_inputCodes.KeyCodes[0x052] = InputCode::KP0;
    m_inputCodes.KeyCodes[0x04F] = InputCode::KP1;
    m_inputCodes.KeyCodes[0x050] = InputCode::KP2;
    m_inputCodes.KeyCodes[0x051] = InputCode::KP3;
    m_inputCodes.KeyCodes[0x04B] = InputCode::KP4;
    m_inputCodes.KeyCodes[0x04C] = InputCode::KP5;
    m_inputCodes.KeyCodes[0x04D] = InputCode::KP6;
    m_inputCodes.KeyCodes[0x047] = InputCode::KP7;
    m_inputCodes.KeyCodes[0x048] = InputCode::KP8;
    m_inputCodes.KeyCodes[0x049] = InputCode::KP9;
    m_inputCodes.KeyCodes[0x04E] = InputCode::KPAdd;
    m_inputCodes.KeyCodes[0x053] = InputCode::KPDecimal;
    m_inputCodes.KeyCodes[0x135] = InputCode::KPDivide;
    m_inputCodes.KeyCodes[0x11C] = InputCode::KPEnter;
    m_inputCodes.KeyCodes[0x059] = InputCode::KPEqual;
    m_inputCodes.KeyCodes[0x037] = InputCode::KPMultiply;
    m_inputCodes.KeyCodes[0x04A] = InputCode::KPSubtract;


    for (int scancode = 0; scancode < 512; scancode++)
    {
        if (m_inputCodes.KeyCodes[scancode] > 0)
            m_inputCodes.ScanCodes[(int)m_inputCodes.KeyCodes[scancode]] = scancode;
    }
}

void Input::InitializeInputState()
{
    memset(m_inputState.Keys, InputState::KeyUp, sizeof(m_inputState.Keys));
    memset(m_inputState.MouseButtons, InputState::MBUp, sizeof(m_inputState.MouseButtons));
}
    
