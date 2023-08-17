#pragma once

#include "IEvent.hpp"

class IEventListener
{
public:
	virtual ~IEventListener() = default;

	virtual bool OnKeyDown(IEvent*) { return false; }
	virtual bool OnKeyUp(IEvent*) { return false; }
	virtual bool OnKeyChar(IEvent*) { return false; }

	virtual bool OnMouseMove(IEvent*) { return false; }
	virtual bool OnMouseDown(IEvent*) { return false; }
	virtual bool OnMouseUp(IEvent*) { return false; }
	virtual bool OnMouseDoubleClick(IEvent*) { return false; }
	virtual bool OnMouseWheel(IEvent*) { return false; }

	virtual bool OnWindowClose(IEvent*) { return false; }
	virtual bool OnWindowResize(IEvent*) { return false; }
	virtual bool OnWindowMove(IEvent*) { return false; }

};