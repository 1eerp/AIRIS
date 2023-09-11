#pragma once

#include "IEvent.hpp"



// WINDOW CLOSE
class WindowCloseEvent : public IEvent
{
public:
	WindowCloseEvent() = default;

	EVENT_CLASS_TYPE(WindowClose)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

// MINIMIZE
class WindowMinimizeEvent : public IEvent
{
public:
	WindowMinimizeEvent() = default;

	EVENT_CLASS_TYPE(WindowMinimize)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

// MAXIMIZE
class WindowMaximizeEvent : public IEvent
{
public:
	WindowMaximizeEvent() = default;

	EVENT_CLASS_TYPE(WindowMaximize)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

// RESIZE
class WindowResizeEvent : public IEvent
{
public:
	WindowResizeEvent(unsigned int width, unsigned int height)
		: m_Width(width), m_Height(height) {}

	unsigned int GetWidth() const { return m_Width; }
	unsigned int GetHeight() const { return m_Height; }

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
		return ss.str();
	}

	EVENT_CLASS_TYPE(WindowResize)
	EVENT_CLASS_CATEGORY(EventCategoryApplication)
private:
	unsigned int m_Width, m_Height;
};