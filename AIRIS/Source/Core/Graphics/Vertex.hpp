#pragma once
#include "glm/glm.hpp"


struct Vertex
{
	glm::vec3 Position;
	glm::vec4 Color;
};

struct ConstantBuffer
{
	glm::mat4 Model = glm::mat4(1.f);
	glm::mat4 View = glm::mat4(1.f);
	glm::mat4 Proj = glm::mat4(1.f);
	glm::mat4 MVP = glm::mat4(1.f);
};