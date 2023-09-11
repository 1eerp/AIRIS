#include "Camera.hpp"


RTCamera::RTCamera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 initialWorldUp = glm::vec3(0,1,0), glm::ivec2 viewportDimensions = glm::ivec2(1280, 720), float m_aspectRatio = 16.f/9.f, float verticalFOV = 45.f, float focalLength = 1.f)
	:m_fov(verticalFOV), m_focalLen(focalLength), m_aspectRatio(m_aspectRatio), m_viewportDims(viewportDimensions), m_pos(position), m_lookAt(lookAt), m_up(initialWorldUp)
{
	Update();
}

void RTCamera::SetPosition(glm::vec3 position)
{
	m_pos = position;
	m_requiresUpdate = true;
}

void RTCamera::SetLookAt(glm::vec3 lookAt)
{
	m_lookAt = lookAt;
	m_requiresUpdate = true;
}

void RTCamera::SetViewport(glm::ivec2 dim, float aspectRatio)
{
	m_viewportDims = dim;
	m_aspectRatio = aspectRatio;
	m_requiresUpdate = true;
}

void RTCamera::SetFov(float fov)
{
	m_fov = fov;
	m_requiresUpdate = true;
}

void RTCamera::Update()
{
	if (!m_requiresUpdate)
		return;
	m_viewDir = glm::normalize(m_lookAt - m_pos);
	// Since we are using a left handed coordinate system we use the left handed rule for cross products
	m_right = glm::cross(m_up, m_viewDir);
	m_up = glm::cross(m_viewDir, m_right);
	UpdateShaderData();
	m_requiresUpdate = false;
}

void RTCamera::UpdateShaderData()
{
	// Widht and Height of the camera frame, not the image dimensions
	float viewHeight = 2 * m_focalLen * glm::tan(glm::radians(m_fov)/2);
	float viewWidth = m_aspectRatio * viewHeight;

	glm::vec3 hDir = m_right * viewWidth,
		vDir = -m_up * viewHeight;
	
	m_shaderData.Position = m_pos;
	m_shaderData.Direction = m_viewDir;
	m_shaderData.PixelDeltaX = hDir / static_cast<float>(m_viewportDims.x);
	m_shaderData.PixelDeltaY = vDir / static_cast<float>(m_viewportDims.y);

	//									ViewPort Center				   Move to TopLeft of ViewPort			Pixel00(TLPixel) Center from TopLeft of Viewport
	m_shaderData.Pixel00Center = (m_pos + (m_viewDir * m_focalLen)) + (hDir * -0.5f + vDir * -0.5f) + ((m_shaderData.PixelDeltaX + m_shaderData.PixelDeltaY) * 0.5f);
}


RTCameraSD RTCamera::GetShaderData()
{
	// Initalize Structure
	return m_shaderData;
}
