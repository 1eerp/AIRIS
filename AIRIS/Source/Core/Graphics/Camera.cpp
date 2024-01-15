#include "Camera.hpp"
#include <sstream>

RTCamera::RTCamera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 initialWorldUp = glm::vec3(0,1,0), glm::ivec2 viewportDimensions = glm::ivec2(1280, 720), float m_aspectRatio = 16.f/9.f, float verticalFOV = 30.f, float focalDist = 10.f, float defocusAngle = 10.f)
	:m_fov(verticalFOV), m_focalDist(focalDist), m_aspectRatio(m_aspectRatio), m_defocusAngle(defocusAngle), m_viewportDims(viewportDimensions), m_pos(position), m_lookAt(lookAt), m_up(initialWorldUp)
{
	Update();
}

std::string RTCamera::GetInfo()
{
	std::stringstream s;
	s	<< "\nCAMERA INFO"
		<< "\n-----------"
		<< "\nFOV: " << m_fov
		<< "\nFocalDist: " << m_focalDist
		<< "\nDefocusAngle: " << m_defocusAngle
		<< "\nCameraPos: " << m_pos[0] << " " << m_pos[1] << " " << m_pos[2]
		<< "\nCameraDir: " << m_viewDir[0] << " " << m_viewDir[1] << " " << m_viewDir[2];


	return s.str();
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

void RTCamera::SetDefocusAngle(float defocusAngle)
{
	m_defocusAngle = defocusAngle;
	m_requiresUpdate = true;
}

void RTCamera::SetFocalDist(float focalDist)
{
	m_focalDist = focalDist;
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
	float viewHeight = 2 * m_focalDist * glm::tan(glm::radians(m_fov)/2);
	float viewWidth = m_aspectRatio * viewHeight;
	float defocusRadius = m_focalDist * glm::tan(glm::radians(m_defocusAngle)/2);

	glm::vec3 hDir = m_right * viewWidth,
		vDir = -m_up * viewHeight;
	
	m_shaderData.Position = m_pos;
	m_shaderData.Direction = m_viewDir;
	m_shaderData.PixelDeltaX = hDir / static_cast<float>(m_viewportDims.x);
	m_shaderData.PixelDeltaY = vDir / static_cast<float>(m_viewportDims.y);
	m_shaderData.LensDefocusX = defocusRadius * m_right;
	m_shaderData.LensDefocusY = defocusRadius * m_up;

	//									ViewPort Center				   Move to TopLeft of ViewPort			Pixel00(TLPixel) Center from TopLeft of Viewport
	m_shaderData.Pixel00Center = (m_pos + (m_viewDir * m_focalDist)) + ((hDir + vDir) * -0.5f) + ((m_shaderData.PixelDeltaX + m_shaderData.PixelDeltaY) * 0.5f);
}


RTCameraSD RTCamera::GetShaderData()
{
	// Initalize Structure
	return m_shaderData;
}



Camera::Camera(glm::vec3 position, float aspectRatio, float fov)
{
	m_transform.Position = position;
	m_camSpec.AspectRatio = aspectRatio;
	m_camSpec.FOV = fov;

	UpdateShaderData();
}

Camera::Camera(const Transform& transform, const CameraSpec& spec)
	: m_transform(transform), m_camSpec(spec)
{
	UpdateShaderData();
}


void Camera::UpdateBasisVectors()
{
	auto inv = glm::inverse(m_transform.Orientation);
	m_transform.Forward = inv * glm::vec3(0.f, 0.f, 1.f);
	m_transform.Up = inv * glm::vec3(0.f, 1.f, 0.f);
	m_transform.Right = inv * glm::vec3(1.f, 0.f, 0.f);
}

void Camera::UpdateFov(float deltaFov)
{
	m_camSpec.FOV += deltaFov;
	m_requiresUpdate = true;
}

void Camera::UpdatePos(glm::vec3 deltaPos)
{
	m_transform.Position += deltaPos;
	m_requiresUpdate = true;
}

void Camera::SetProjectionType(ProjectionType type)
{
	m_camSpec.Type = type;
	m_requiresUpdate = true;
}

void Camera::SetPosition(glm::vec3 position)
{
	m_transform.Position = position;
	m_requiresUpdate = true;
}

void Camera::SetFov(float fov)
{
	m_camSpec.FOV = fov;
	m_requiresUpdate = true;
}

void Camera::SetAspectRatio(float ar)
{
	m_camSpec.AspectRatio = ar;
	m_requiresUpdate = true;
}

void Camera::SetNearPlane(float nearPlane)
{
	m_camSpec.Near = nearPlane;
	m_requiresUpdate = true;
}

void Camera::SetFarPlane(float farPlane)
{
	m_camSpec.Far = farPlane;
	m_requiresUpdate = true;
}

void Camera::UpdateOrientation(float yaw, float pitch, float roll)
{
	UpdateOrientation({ pitch, yaw, roll });
}

void Camera::UpdateOrientation(const glm::vec3& eulerAngles)
{
	m_transform.Orientation = glm::quat(-eulerAngles) * m_transform.Orientation;
	UpdateBasisVectors();
	m_requiresUpdate = true;
}

void Camera::SetOrientation(float yaw, float pitch, float roll)
{
	glm::quat qPitch = glm::angleAxis(-pitch, glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(-yaw, glm::vec3(0, 1, 0));
	glm::quat qRoll = glm::angleAxis(-roll, glm::vec3(0, 0, 1));

	m_transform.Orientation = qRoll * qPitch * qYaw;
	UpdateBasisVectors();
	m_requiresUpdate = true;
}

void Camera::UpdateShaderData()
{
	if (!m_requiresUpdate)
		return;

	const float & ar = m_camSpec.AspectRatio;

	if (m_camSpec.Type == ProjectionType::Orothographic)
	{
		const float hs = (0.5f * m_camSpec.Size);
		m_gpuData.ProjMatrix = glm::ortho(-hs * ar, hs * ar, -hs, hs);
	}
	else
		m_gpuData.ProjMatrix = glm::perspective(m_camSpec.FOV, ar, m_camSpec.Near, m_camSpec.Far);

	m_gpuData.ViewMatrix = glm::mat4_cast(m_transform.Orientation) * glm::translate(glm::mat4(1.0f), -m_transform.Position);

	m_requiresUpdate = false;
}