#include "Core/Log.hpp"
#include "EventSystem.hpp"

#define EVENTSYSTEM_CALLBACKS_PER_EVENTTYPE 5
bool EventSystem::Init()
{
	if (m_initialized)
		return true;

	m_initialized = true;
	eventQueue.reserve(1000);
		
	// Initialize all events with empty listner lists
	for (unsigned int i = 0; i < static_cast<unsigned int>(EventType::LastEvent) + 1; i++)
	{
		eventListeners[i].reserve(EVENTSYSTEM_CALLBACKS_PER_EVENTTYPE);
	}

	return true;
}

void EventSystem::RegisterEventListener(EventType type, IEventListener* listener, EventCallback callback)
{
	eventListeners[static_cast<unsigned int>(type)].push_back(LCPair(listener,callback));
}

void EventSystem::UnregisterEventListener(EventType type, IEventListener* listener, EventCallback callback)
{
	auto index = static_cast<unsigned int>(type);

	unsigned int i = 0;
	for (LCPair& p: eventListeners[index])
	{
		p = eventListeners[index][i];
		if (p.callback == callback && p.listener == listener) {
			eventListeners[index].erase(eventListeners[index].begin() + i);
			return;
		}
		i++;
	}
}

void EventSystem::UnregisterAllEventListener()
{
	for (std::vector<LCPair>& listeners : eventListeners)
		listeners.clear();
}

void EventSystem::ProcessEvents()
{
	if (eventQueue.size() == 0)
		return;

	CORE_TRACE("Processing {} events", eventQueue.size());
	// Dispatch all recorded events
	for (EventRef& e : eventQueue)
		DispatchEvent(e);

	ClearEventQueue();
}

void EventSystem::DispatchEvent(EventRef e)
{
	// TODO: Maybe add priority to some event by processing some type/category of events before others and clearing those events
	// Dispatch the event to all the registered callback in the order the listeners were registers


	CORE_TRACE(e->ToString());
	for (LCPair l : eventListeners[static_cast<int>(e->GetEventType())])
	{
		// If event is handled break;
		if (((l.listener)->*l.callback)(e.get()) && e->Handled)
			return;
		
	}
}


void EventSystem::QueueEvent(EventRef event)
{
	eventQueue.push_back(event);
	
}


void EventSystem::ClearEventQueue()
{
	eventQueue.clear();
}


void EventSystem::ShutDown()
{
	ClearEventQueue();
	UnregisterAllEventListener();
}

EventSystem* EventSystem::GetGInstance()
{
	static EventSystem eventSystem;
	return &eventSystem;
}
