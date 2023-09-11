#pragma once
#include "glm/glm.hpp"

struct RTSphere
{
	glm::vec3	Posiition;
	float		Radius;
	uint32_t	MaterialIndex;
};

enum MTType : uint32_t
{
	Diffuse,
	Metal,
	Dielectric,
	HollowGlass,
};

struct RTMaterial
{
	glm::vec3	Albedo;
	MTType		Type;
	float		Roughness;
};