#pragma once

#include <Windows.h>
#include "Events/IEventListener.hpp"
#include "Window.hpp"
#include "Graphics/Renderer.hpp"



class SApp : IEventListener
{
	SApp(SApp&) = delete;
	SApp(SApp&&) = delete;

public:
	SApp(SWindow* mainWindow);
	virtual ~SApp();

	void Run();
	void Update();

private:
	virtual bool OnWindowClose(IEvent* event) override;
	virtual bool OnWindowResize(IEvent* event) override;


public:
	bool m_running = true;

private:
	// The main window associated with this application
	RRenderer m_renderer;
	SWindow* m_mainWindow;
	static SApp* s_appInstance;
};
