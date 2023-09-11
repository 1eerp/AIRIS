#pragma once

#include "InputCodes.hpp"
#include <Windows.h>

using namespace InputCode;
using namespace InputState;

struct InputCodes
{
	short int KeyCodes[InputCode::LastKey + 2];		// Old: 512
	short int ScanCodes[InputCode::LastKey + 1];
};

struct MouseCursorState
{
	uint16_t	X = 0, 
				Y = 0;

	// Not sure if this is needed yet so it will be always Moving for now
	MouseState State = MouseMoved;
};

struct InputStateData
{
	KeyState			Keys[InputCode::LastKey + 1];
	MouseButtonState	MouseButtons[InputCode::MBLast + 1];
	MouseCursorState	CursorState;
};

class Input
{
public:
	// INFO: Gets a pointer to the Input Singleton 
	static Input* GetInstance();
	bool Init();

	inline KeyCode GetKey(const uint16_t& scanCode)			const { return static_cast<KeyCode>(m_inputCodes.KeyCodes[scanCode]); }
	inline KeyState GetKeyState(const KeyCode& keycode)		const { return m_inputState.Keys[keycode]; }
	inline MouseButtonState GetMBState(const MouseCode& mb)	const { return m_inputState.MouseButtons[mb]; }
	inline MouseCursorState GetMouseState()					const { return m_inputState.CursorState; }

	// NOTE: Probably not implementing support for mousescroll stuff in the input system, Event system has a MouseScrolledEvent you can register to

	void UpdateKeyState(KeyCode scancode, KeyState state);
	void UpdateMBState(MouseCode mouseCode, MouseButtonState state);
	void UpdateMousePos(uint16_t x, uint16_t y);

protected:
	// NOTE: Init() should be called for intialization
	Input() = default;
	void InitializeKeyTables();
	void InitializeInputState();

private:
	bool			m_initialized;
	InputCodes		m_inputCodes;
	InputStateData	m_inputState;

};

#define INPUT Input::GetInstance()
