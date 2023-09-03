#pragma once
#include "glm/glm.hpp"

struct RTCameraSD
{
	// Packed in 16 Byte or 4D vector size
	glm::vec3	Position;
	int			__pad0 = 0;
	glm::vec3	Direction;
	int			__pad1 = 0;
	glm::vec3	Pixel00Center;
	int			__pad2 = 0;
	glm::vec3	PixelDeltaX;
	int			__pad3 = 0;
	glm::vec3	PixelDeltaY;
};

class RTCamera
{
public:
	RTCamera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 initialWorldUp, glm::ivec2 imageDimensions, float m_aspectRatio, float verticalFOV, float focalLength);


	glm::vec3	GetPosition()		{ return m_pos; }
	glm::vec3	GetLookAt()			{ return m_lookAt; }
	glm::vec3	GetDirection()		{ return m_viewDir; }
	glm::vec3	GetRight()			{ return m_right; }
	glm::vec3	GetUp()				{ return m_up; }
	float		GetFov()			{ return m_fov; }
	bool		RequiresUpdate()	{ return m_requiresUpdate; }

	void SetPosition(glm::vec3	position);
	void SetLookAt(glm::vec3	lookAt);
	void SetViewport(glm::ivec2 dim, float aspectRatio);
	void SetFov(float fov);

	void Update();

	RTCameraSD GetShaderData();

private:
	void UpdateShaderData();

public:

private:
	bool				m_requiresUpdate = true;
	float				m_fov,
						m_focalLen,
						m_aspectRatio;
	glm::ivec2			m_viewportDims;
	glm::vec3			m_pos,
						m_lookAt,
						m_viewDir,
						m_right,
						m_up;

	RTCameraSD			m_shaderData;
};