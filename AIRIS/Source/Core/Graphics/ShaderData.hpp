#pragma once
#include "glm/glm.hpp"


struct ObjectConstants
{
    glm::mat4 Model = glm::mat4(1.f);
};

struct PassConstants
{
    glm::mat4 ViewMatrix = glm::mat4(1.f);
    glm::mat4 InvViewMatrix = glm::mat4(1.f);
    glm::mat4 ProjMatrix = glm::mat4(1.f);
    glm::mat4 InvProjMatrix = glm::mat4(1.f);
    glm::mat4 ViewProjMatrix = glm::mat4(1.f);
    glm::mat4 InvViewProjMatrix = glm::mat4(1.f);
    glm::vec3 EyePosW = glm::vec3(0.0f);
private:
    float _pad1;
public:
    glm::vec2 RenderTargetDim = { 0.0f, 0.0f };
    glm::vec2 InvRenderTargetDim = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;
};

struct Vertex
{
    glm::vec3 Pos = glm::vec3(0.f);
    glm::vec4 Color = glm::vec4(1.f);
};

struct ConstantBuffer
{
	glm::mat4 Model = glm::mat4(1.f);
	glm::mat4 View = glm::mat4(1.f);
	glm::mat4 Proj = glm::mat4(1.f);
	glm::mat4 MVP = glm::mat4(1.f);
};