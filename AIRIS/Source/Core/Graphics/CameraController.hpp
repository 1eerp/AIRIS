#pragma once
#include <iostream>
#include "glm/glm.hpp"
#include "Core/Events/IEventListener.hpp"
#include "Core/Events/ApplicationEvents.hpp"
#include "Core/Events/InputEvents.h"
#include "Camera.hpp"
#include "Util.hpp"

class CameraController : public IEventListener
{
	friend class EventSystem;
public:
	CameraController();

	// RUN AT THE START OF RENDER LOOP FOR NOW
	virtual void Update(float deltaTime) = 0;

	inline Ref<Camera>	GetCamera()			{ return m_camera; }
	inline float		GetMovementSpeed()	{ return m_cameraSpeed; }
	inline float		GetSensitivity()	{ return m_cameraSens; }
	inline bool			IsInUse()			{ return m_enabled; }

	void	SetMovementSpeed(float speed)	{ m_cameraSpeed = speed; }
	void	SetSensitivity(float sens)		{ m_cameraSens = sens; }
	bool	FlipStatus()					{ m_enabled = !m_enabled; }
	bool	SetStatus(bool status)			{ m_enabled = status; }

protected:
	virtual bool OnWindowResize(IEvent* event) override
	{
		WindowResizeEvent* resizeEvent = dynamic_cast<WindowResizeEvent*>(event);
		m_camera->SetAspectRatio(static_cast<float>(resizeEvent->GetWidth()) / resizeEvent->GetHeight());
		return false;
	}

protected:
	Ref<Camera> m_camera = nullptr;
	float m_cameraSpeed = 4.f;
	float m_cameraSens = 0.5f;
	bool m_enabled = true;
};



// FLIGHT SIMULATION CAMERA CONTROLLER
class FlightSimCC : public CameraController
{
public:
	// CONSTRUCTORS
	FlightSimCC();
	FlightSimCC(Ref<Camera>& cam);

	void Update(float deltaTime) override;

private:
	// In degrees
	float m_maxRotationPerSec = 45.f;

};



class EditorCC: public CameraController
{
public:
	// CONSTRUCTORS
	EditorCC();
	EditorCC(Ref<Camera>& cam);

	void Update(float deltaTime) override;

private:
	union
	{
		glm::vec3 m_rotation = { 0.f , 0.f, 0.f};
		struct
		{
			float m_yaw;
			float m_pitch;
			float m_roll;
		};
	};

	glm::vec2 m_lastMousePos = { 0.f, 0.f };
};