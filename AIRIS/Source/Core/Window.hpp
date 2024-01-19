#pragma once

#include <Windows.h>

#include <string>
#include <sstream>


struct WindowStartProps
{
	int ShowCommand = SW_SHOWNORMAL;
};

struct WindowProps
{
	unsigned int		Width = 1280, 
						Height = 720;

	std::wstring		ClassName = L"AIRISMainWindowClass";
	std::wstring		Title = L"AIRIS";

	WindowStartProps	StartProps;


};

class SWindow
{
public:
	SWindow(HINSTANCE hInstance, WindowProps windowProperties = {/*Default*/});
	
	inline HWND				GetWindowHandle()		{ return m_hWnd; }
	inline HINSTANCE		GetHINSTANCE()			{ return m_hInstance; }
	inline WindowProps		GetWindowProperties()	{ return m_windowProperties; };
	inline uint32_t			GetHeight()				{ return m_windowProperties.Height; }
	inline uint32_t			GetWidth()				{ return m_windowProperties.Width; }
	inline float			GetAspectRatio()		{ return (float)m_windowProperties.Width / m_windowProperties.Height; }
	inline bool				IsMinimized()			{ return m_minimized; }
	inline bool				IsMaximized()			{ return m_maximized; }
	inline bool				IsResizing()			{ return m_resizing; }

	inline void				Resize(uint32_t width, uint32_t height) 
	{ 
		m_windowProperties.Width = width, m_windowProperties.Height = height;
	}
	inline void				SetMinimized(bool s)	{ m_minimized = s; }
	inline void				SetMaximized(bool s)	{ m_maximized = s; }
	inline void				SetResizing(bool s)		{ m_resizing = s; }



	bool					Show();
	int						PumpMessages();

	static SWindow*			GetInstance()			{ return s_mainWindowInstance; }
protected:
	void					Init();
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	WindowProps		m_windowProperties;

	bool			m_minimized,
					m_maximized,
					m_resizing;


	static SWindow* s_mainWindowInstance;
};

