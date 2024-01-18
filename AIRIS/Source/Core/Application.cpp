#include "Log.hpp"
#include "Application.hpp"
#include "Input/Input.hpp"
#include "Core/Window.hpp"
#include "Events/EventSystem.hpp"
#include "Events/InputEvents.h"
#include "Input/Input.hpp"
#include "Time.hpp"


SApp* SApp::s_appInstance;

SApp::SApp(SWindow* mainWindow)
	:m_mainWindow(mainWindow)
{
	assert(!s_appInstance && "Attempted to instantiate multiple application!");
	s_appInstance = this;

	// REGISTER EVENTS in the event system.
	EVENTSYSTEM->RegisterEventListener(EventType::WindowClose, this, reinterpret_cast<EventSystem::EventCallback>(&SApp::OnWindowClose));
	EVENTSYSTEM->RegisterEventListener(EventType::WindowResize, this, reinterpret_cast<EventSystem::EventCallback>(&SApp::OnWindowResize));
	EVENTSYSTEM->RegisterEventListener(EventType::KeyPressed, this, reinterpret_cast<EventSystem::EventCallback>(&SApp::OnKeyDown));


	// Todo: Initialize RenderAPI
	m_renderer = CreateScope<RRenderer>(mainWindow->GetWidth(), mainWindow->GetHeight());

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

bool SApp::OnKeyDown(IEvent* event)
{
	KeyCode key = dynamic_cast<KeyPressedEvent*>(event)->GetKeyCode();
	switch (key)
	{
	case KeyCode::Escape:
	{
		m_running = false;
		break;
	}
	default:
		break;
	}
	return false;
}

void SApp::Run()
{
	// Show the window and reset static variables in Time class, now that the initialization is complete
	m_mainWindow->Show();
	Time::Reset();

	// MAIN LOOP
	while (m_running)
	{
		Time::Update();
		Update();
	}
}

void SApp::Update()
{
	m_mainWindow->PumpMessages();
	EventSystem::GetGInstance()->ProcessEvents();
	m_renderer->Update();
}

