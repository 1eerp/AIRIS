#include "CameraController.hpp"
#include "Core/Input/Input.hpp"
#include "Core/Log.hpp"
#include "Core/Events/EventSystem.hpp"

CameraController::CameraController()
{
	// REGISTER ALL COMMON EVENTS
	EVENTSYSTEM->RegisterEventListener(EventType::WindowResize, this, reinterpret_cast<EventSystem::EventCallback>(&CameraController::OnWindowResize));
}

FlightSimCC::FlightSimCC()
{
	m_camera = CreateRef<Camera>();
}

FlightSimCC::FlightSimCC(Ref<Camera>& cam)
{
	m_camera = cam;
}

void FlightSimCC::Update(float deltaTime)
{
	// If controller not in use don't update camera
	if (!m_enabled)
		return;

	// Check input for movement and create the movement vector
	glm::vec3 deltaPos = m_camera->GetRightDir() * (1.f * ((INPUT->GetKeyState(InputCode::D) > 0) - (INPUT->GetKeyState(InputCode::A) > 0)));
	deltaPos += m_camera->GetForwardDir()* (1.f * ((INPUT->GetKeyState(InputCode::W) > 0) - (INPUT->GetKeyState(InputCode::S) > 0)));

	// Using dot product instead of glm::length to check if vector is non-zero
	if (glm::dot(deltaPos, deltaPos) > 0)
	{
		deltaPos = deltaTime * m_cameraSpeed * glm::normalize(deltaPos);
		m_camera->UpdatePos(deltaPos);
	}

	// Check input for rotation
	glm::vec3 deltaRot(0.f);
	deltaRot.x = static_cast<float>(((INPUT->GetKeyState(KeyCode::O) > 0) - (INPUT->GetKeyState(KeyCode::U) > 0)));
	deltaRot.y = static_cast<float>(((INPUT->GetKeyState(KeyCode::I) > 0) - (INPUT->GetKeyState(KeyCode::K) > 0)));
	deltaRot.z = static_cast<float>(((INPUT->GetKeyState(KeyCode::J) > 0) - (INPUT->GetKeyState(KeyCode::L) > 0)));

	// Not sure if this should be normalized yet or not
	deltaRot = glm::radians(deltaTime * m_maxRotationPerSec * deltaRot);

	if(glm::length(deltaRot) > 0)
		m_camera->UpdateOrientation(deltaRot.x, deltaRot.y, deltaRot.z);

	// Update Shader Data
	m_camera->UpdateShaderData();
}


EditorCC::EditorCC()
{
	m_camera = CreateRef<Camera>();
}

EditorCC::EditorCC(Ref<Camera>& cam)
{
	m_camera = cam;
}

void EditorCC::Update(float deltaTime)
{
	// If controller not in use don't update camera
	if (!m_enabled)
		return;

	// Check input for movement and create the movement vector
	glm::vec3 deltaPos = m_camera->GetRightDir() * (1.f * ((INPUT->GetKeyState(InputCode::D) > 0) - (INPUT->GetKeyState(InputCode::A) > 0)));
	deltaPos += m_camera->GetForwardDir() * (1.f * ((INPUT->GetKeyState(InputCode::W) > 0) - (INPUT->GetKeyState(InputCode::S) > 0)));

	// Using dot product instead of glm::length to check if vector is non-zero
	if (glm::dot(deltaPos, deltaPos) > 0)
	{
		deltaPos = deltaTime * m_cameraSpeed * glm::normalize(deltaPos);
		m_camera->UpdatePos(deltaPos);
	}

	MouseCursorState cursorState = INPUT->GetMouseState();
	// Check input for rotation
	if (INPUT->GetMBState(MouseCode::MB1))
	{
		glm::vec2 deltaMouse = { m_cameraSens * (cursorState.X - m_lastMousePos.x), m_cameraSens * (cursorState.Y - m_lastMousePos.y) };
		if (deltaMouse.x || deltaMouse.y)
		{
			m_yaw += deltaMouse.x;
			m_pitch += deltaMouse.y;

			deltaMouse *= 2;
			if (glm::abs(m_yaw) > 180.f)
			{
				// YAW =		THIS				IF POSITIVE				THIS				IF NEGATIVE
				m_yaw = (deltaMouse.x - m_yaw) * (m_yaw >= 0.f) + (deltaMouse.x - m_yaw) * (m_yaw < 0.f);
			}

			if (glm::abs(m_pitch) > 180.f)
			{
				// PITCH =		THIS					IF POSITIVE				THIS					IF NEGATIVE
				m_pitch = (deltaMouse.y - m_pitch) * (m_pitch >= 0.f) + (deltaMouse.y - m_pitch) * (m_pitch < 0.f);
			}

			m_camera->SetOrientation(glm::radians(m_yaw), glm::radians(m_pitch));
		}
	}
	m_lastMousePos = { cursorState.X, cursorState.Y };


	// Update Shader Data
	m_camera->UpdateShaderData();
}

