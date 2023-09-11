#pragma once
#include "Util.hpp"
#include "IEvent.hpp"
#include "IEventListener.hpp"


template<typename T, typename U>
class SPair
{
public:
	SPair() = default;
	SPair(T t, U u)
		:listener(t), callback(u)
	{}

	T listener;
	U callback;
};


class EventSystem
{
	EventSystem() {}
	~EventSystem() { ShutDown(); }

public:
	using EventCallback = bool(IEventListener::*)(IEvent*);
	using LCPair = SPair<IEventListener*, EventCallback>;
	using EventRef = Ref<IEvent>;

	bool Init();
	static EventSystem* GetGInstance();

	void RegisterEventListener(EventType type, IEventListener* listener, EventCallback callback);
	void UnregisterEventListener(EventType type, IEventListener* listener, EventCallback callback);
	void UnregisterAllEventListener();

	void QueueEvent(EventRef event);
	void ProcessEvents();


private:
	void DispatchEvent(EventRef e);
	void ClearEventQueue();

	void ShutDown();

private:
	bool m_initialized = false;
	std::vector<EventRef> eventQueue;
	std::array<std::vector<LCPair>, static_cast<int>(EventType::LastEvent) + 1> eventListeners;
};


#define EVENTSYSTEM EventSystem::GetGInstance()


