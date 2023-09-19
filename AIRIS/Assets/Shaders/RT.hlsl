#define FLOATMAX 3.402823466e+38f
#define UINTMAX 0xffffffff

static uint rng_state;

uint rand_pcg()
{
	uint state = rng_state;
	rng_state = rng_state * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

float GetRandomFloat()
{
	return rand_pcg() / float(UINTMAX);
}

float GetRandomSFloat()
{
	return (GetRandomFloat() - 0.5f) * 2;
}

float3 GetRandomFloat3()
{
	return float3(GetRandomFloat(), GetRandomFloat(), GetRandomFloat());
}

float3 GetRandomSFloat3()
{
	return float3(GetRandomSFloat(), GetRandomSFloat(), GetRandomSFloat());
}

float3 GetRandomUnitFloat3()
{
	for (uint i = 0; i < 50; i++)
	{
		float3 p = GetRandomSFloat3();
		if (dot(p,p) < 1)
			return p;
	}
	return normalize(GetRandomSFloat3());
}

float3 RandomNormalHemisphereFloat3(float3 normal)
{
	float3 on_unit_sphere = GetRandomUnitFloat3();
	if (dot(on_unit_sphere, normal) > 0.0)				// In the same hemisphere as the normal
		return on_unit_sphere;
	else
		return -on_unit_sphere;
	
    float3x3 a;
}

float3 RandomInUnitDisk()
{
	for (uint i = 0; i < 50; i++)
	{
		float3 p = float3(GetRandomSFloat(), GetRandomSFloat(), 0);
		if (dot(p,p) < 1)
			return p;
	}
	return float3(0, 0, 0);
}

uint3 FloatTo8BitColor(float3 color)
{
	return uint3(color.x * 255, color.y * 255, color.z * 255);
}

float3 AccumulatedColorToFloat3(uint4 color)
{
	return color.xyz / float(255);
}

class Ray
{
	float3 origin;
	float3 direction;
	float3 at(float t)
	{
		return origin + t * direction;
	}
};

struct Camera
{
	float3 origin;
	float3 lookAt;
	float focalLength;
};

class HitRecord
{
	float3 pos;
	float3 normal;
	float t;
	bool frontFace;
	uint materialIndex;

	void SetFaceNormal(const Ray r, const float3 outwardNormal)
	{
		frontFace = dot(r.direction, outwardNormal) < 0;
		normal = frontFace ? outwardNormal : -outwardNormal;
	}
};


struct RTSphere
{
	float3 position;
	float radius;
	uint matIndex;
};


struct RTMaterial
{
	float3  Albedo;
	uint    Type;
	float   Roughness;
};

class Sphere
{
	float3 position;
	float radius;

	
	bool Hit(inout Ray r, out HitRecord rec, float tMin, float tMax)
	{
		float3  oc = r.origin - position;
		float   a = 1, //Ray Direction vector is normalized during ray generation | Original: pow(length(r.direction), 2),
				half_b = dot(oc, r.direction),
				c = pow(length(oc), 2) - radius * radius,
				discriminant = half_b * half_b - a * c;

		if (discriminant < 0)
			return false;
		
		float sqrtd = sqrt(discriminant);

		// Find the nearest root that lies in the acceptable range.
		float t = (-half_b - sqrtd) / a;
		if (t <= tMin || tMax <= t)
		{
			t = (-half_b + sqrtd) / a;
			if (t <= tMin || tMax <= t)
				return false;
		}

		rec.t = t;
		rec.pos = r.at(t);
		rec.SetFaceNormal(r, (rec.pos - position) / radius);
		rec.materialIndex = 0;
		return true;
	}
};

