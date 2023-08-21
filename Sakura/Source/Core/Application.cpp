#include "Log.hpp"
#include "Application.hpp"
#include "Input/Input.hpp"
#include "Core/Window.hpp"
#include "Events/EventSystem.hpp"
#include "Input/Input.hpp"


SApp* SApp::s_appInstance;

SApp::SApp(SWindow* mainWindow)
	:m_mainWindow(mainWindow), m_renderer(mainWindow->GetWidth(), mainWindow->GetHeight())
{
	assert(!s_appInstance && "Attempted to instantiate multiple application!");
	s_appInstance = this;

	// REGISTER EVENTS in the event system.
	EVENTSYSTEM->RegisterEventListener(EventType::WindowClose, this, reinterpret_cast<EventSystem::EventCallback>(&SApp::OnWindowClose));
	EVENTSYSTEM->RegisterEventListener(EventType::WindowResize, this, reinterpret_cast<EventSystem::EventCallback>(&SApp::OnWindowResize));


	// Todo: Initialize RenderAPI

}

SApp::~SApp()
{

}

bool SApp::OnWindowClose(IEvent* event) 
{
	m_running = false;

	return false;
}

bool SApp::OnWindowResize(IEvent* event)
{
	return false;
}

void SApp::Run()
{
	// Show the window now that the initialization is complete
	m_mainWindow->Show();

	// MAIN LOOP
	while (m_running)
	{
		Update();
	}
}

void SApp::Update()
{
	m_mainWindow->PumpMessages();
	EventSystem::GetGInstance()->ProcessEvents();
	m_renderer.Draw();
}

