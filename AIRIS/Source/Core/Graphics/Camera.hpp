#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <string>

// TODO: Get rid of pads by packing the values in matrices
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
	int			__pad4 = 0;
	glm::vec3	LensDefocusX;
	int			__pad5 = 0;
	glm::vec3	LensDefocusY;

};

class RTCamera
{
public:
	RTCamera(glm::vec3 position, glm::vec3 lookAt, glm::vec3 initialWorldUp, glm::ivec2 imageDimensions, float m_aspectRatio, float verticalFOV, float focalDist, float defocusAngle);


	glm::vec3	GetPosition()		{ return m_pos; }
	glm::vec3	GetLookAt()			{ return m_lookAt; }
	glm::vec3	GetDirection()		{ return m_viewDir; }
	glm::vec3	GetRight()			{ return m_right; }
	glm::vec3	GetUp()				{ return m_up; }
	float		GetFov()			{ return m_fov; }
	float		GetFocalDist()		{ return m_focalDist; }
	float		GetDefocusAngle()	{ return m_defocusAngle; }
	std::string GetInfo();
	bool		RequiresUpdate()	{ return m_requiresUpdate; }

	void SetPosition(glm::vec3	position);
	void SetLookAt(glm::vec3	lookAt);
	void SetViewport(glm::ivec2 dim, float aspectRatio);
	void SetFov(float fov);
	void SetDefocusAngle(float defocusAngle);
	void SetFocalDist(float focalDist);

	void Update();

	RTCameraSD GetShaderData();

private:
	void UpdateShaderData();

public:

private:
	bool				m_requiresUpdate = true;
	float				m_fov,
						m_focalDist,
						m_aspectRatio,
						m_defocusAngle;
	glm::ivec2			m_viewportDims;
	glm::vec3			m_pos,
						m_lookAt,
						m_viewDir,
						m_right,
						m_up;

	RTCameraSD			m_shaderData;
};



struct GPUCameraMatrices
{
	glm::mat4 ViewMatrix;
	glm::mat4 ProjMatrix;
	glm::mat4 ViewProjMatrix;

	// INVERSE MATRICES
	glm::mat4 InvViewMatrix;
	glm::mat4 InvProjMatrix;
	glm::mat4 InvViewProjMatrix;
};

struct Transform
{
	glm::vec3 Scale = { 1.f, 1.f, 1.f };
	glm::vec3 Position = { 0.f, 0.f, 0.f };
	glm::quat Orientation = { 1.f, 0.f, 0.f, 0.f };

	glm::vec3 Forward = { 0.f, 0.f, 1.f };
	glm::vec3 Up = { 0.f, 1.f, 0.f };
	glm::vec3 Right = { 1.f, 0.f, 0.f };
};

enum class ProjectionType
{
	Orothographic,
	Perspective
};

struct CameraSpec
{
	ProjectionType Type = ProjectionType::Perspective;
	union
	{
		float FOV = glm::radians(45.f); // Perspective
		float Size; // Orthographic
	};
	float	AspectRatio = 16.f / 9.f,
		Near = 0.01f,
		Far = 1000.f;
};



// Normal Camera
class Camera
{
public:

	Camera(glm::vec3 position = { 0.f, 5.f, -20.f }, float aspectRatio = 16.f / 9.f, float fov = glm::radians(45.f));
	Camera(const Transform& transform, const CameraSpec& spec);

	inline const glm::vec3&	GetPosition() { return m_transform.Position; }
	inline const glm::vec3&	GetForwardDir() { return m_transform.Forward; }
	inline const glm::vec3&	GetUpDir() { return m_transform.Up; }
	inline const glm::vec3&	GetRightDir() { return m_transform.Right; }
	inline float			GetFov() { return m_camSpec.FOV; }
	inline float			GetAspectRatio() { return m_camSpec.AspectRatio; }
	inline float			GetSize() { return m_camSpec.Size; }
	inline float			GetNearZ() { return m_camSpec.Near; }
	inline float			GetFarZ() { return m_camSpec.Far; }
	inline bool				RequiresUpdate() { return m_requiresUpdate; }

	inline const glm::mat4&	GetViewMatrix() { return m_gpuData.ViewMatrix; }
	inline const glm::mat4&	GetProjMatrix() { return m_gpuData.ProjMatrix; }
	inline const GPUCameraMatrices& GetGPUMatrices() { return m_gpuData; }

	void UpdatePos(glm::vec3 deltaPos);
	void UpdateFov(float deltaFov);
	void UpdateOrientation(const glm::vec3& eulerAngles); 
	void UpdateOrientation(float yaw, float pitch = 0.f, float roll = 0.f);

	void SetProjectionType(ProjectionType type);
	void SetPosition(glm::vec3	position);
	void SetFov(float fov);
	void SetAspectRatio(float ar);
	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	void SetOrientation(float yaw, float pitch = 0.f, float roll = 0.f);


	void UpdateShaderData();

private:
	void UpdateBasisVectors();


	Transform m_transform;
	CameraSpec m_camSpec;
	GPUCameraMatrices m_gpuData;
	bool m_requiresUpdate;
};