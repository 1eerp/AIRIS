#pragma once

#include "IEvent.hpp"
#include "Core/Input/InputCodes.hpp"

using namespace InputCode;

//////////////////
/// KEY EVENTS ///
//////////////////

class KeyEvent : public IEvent
{

public:
	KeyCode GetKeyCode() const { return m_KeyCode; }

	EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
protected:
	KeyEvent(const KeyCode keycode)
		: m_KeyCode(keycode) {}

	KeyCode m_KeyCode;
};

class KeyPressedEvent : public KeyEvent
{
public:
	KeyPressedEvent(const KeyCode keycode, bool isRepeat = false)
		: KeyEvent(keycode), m_IsRepeat(isRepeat) {}

	bool IsRepeat() const { return m_IsRepeat; }

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyPressedEvent: " << m_KeyCode << " (repeat = " << m_IsRepeat << ")";
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyPressed)
private:
	bool m_IsRepeat;
};

class KeyReleasedEvent : public KeyEvent
{
public:
	KeyReleasedEvent(const KeyCode keycode)
		: KeyEvent(keycode) {}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << m_KeyCode;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent : public KeyEvent
{
public:
	KeyTypedEvent(const KeyCode keycode, const char c)
		: KeyEvent(keycode), c(c){}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "KeyTypedEvent: " << m_KeyCode << ", Char: " << c;
		return ss.str();
	}

	EVENT_CLASS_TYPE(KeyTyped)
public:
	const char c;
};



//////////////////
// MOUSE EVENTS //
//////////////////

class MouseMovedEvent : public IEvent
{
public:
	MouseMovedEvent(const uint16_t x, const uint16_t y)
		: m_PosX(x), m_PosY(y) {}

	float GetX() const { return m_PosX; }
	float GetY() const { return m_PosY; }

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseMovedEvent: " << m_PosX << ", " << m_PosY;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseMoved)
	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
	uint16_t	m_PosX, 
				m_PosY;
};


class MouseScrolledEvent : public IEvent
{
public:
	MouseScrolledEvent(const int delta, const uint16_t x, const uint16_t y)
		: m_Delta(delta), m_PosX(x), m_PosY(y) {}

	inline int GetScrollDelta() const { return m_Delta; }
	inline int16_t GetX() const { return m_PosX; }
	inline int16_t GetY() const { return m_PosY; }

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseScrolledEvent: Delta = " << m_Delta << ", Pos:" << m_PosX << ", " << m_PosY;
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseScrolled)
	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
private:
	int			m_Delta;
	uint16_t	m_PosX, m_PosY;
};


class MouseButtonEvent : public IEvent
{
public:
	inline MouseCode GetMouseButton() const { return m_Button; }
	inline uint16_t GetX() const { return m_PosX; }
	inline uint16_t GetY() const { return m_PosY; }



	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)
protected:
	MouseButtonEvent(const MouseCode button, const uint16_t x, const uint16_t y)
		: m_Button(button), m_PosX(x), m_PosY(y){}

	MouseCode m_Button;
	uint16_t m_PosX, m_PosY;
};

class MouseButtonPressedEvent : public MouseButtonEvent
{
public:
	MouseButtonPressedEvent(const MouseCode button, const uint16_t x, const uint16_t y)
		: MouseButtonEvent(button, x, y){}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonPressedEvent: " << m_Button << " At location: ("<< m_PosX << ',' <<  m_PosY << ")";
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonPressed)

};

class MouseButtonReleasedEvent : public MouseButtonEvent
{
public:
	MouseButtonReleasedEvent(const MouseCode button, const uint16_t x, const uint16_t y)
		: MouseButtonEvent(button, x, y) {}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseButtonReleasedEvent: " << m_Button << ", At location: (" << m_PosX << ',' << m_PosY << ")";
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseButtonReleased)
};


class MouseDoubleClickEvent : public MouseButtonEvent
{
public:
	MouseDoubleClickEvent(const MouseCode button, const uint16_t x, const uint16_t y)
		: MouseButtonEvent(button, x, y) {}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "MouseDoubleClickEvent: " << m_Button << ", At location: (" << m_PosX << ',' << m_PosY << ")\n";
		return ss.str();
	}

	EVENT_CLASS_TYPE(MouseDoubleClick)
};