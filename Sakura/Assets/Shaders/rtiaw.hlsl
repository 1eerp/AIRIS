#include "RT.hlsl"

// Function Declarations
float3 TraceRay(Ray ray);
bool HitHittableList(Ray r, out HitRecord hitRec);
Ray GetRay(float u, float v, Camera cam);

cbuffer RTConstants : register(b0)
{
	bool Accumulate;
	bool ResetSamples;
    uint AccumulatedSamples,
		 MaxRayBounces,
		 InitalRandomSeed;
	
}

// CAMERA FRAME CONSTANTS
cbuffer RTCameraSD : register(b1)
{
	float3  Position,
			Direction,
			Pixel00Center,
			PixelDeltaX,
			PixelDeltaY;
};

// SPHERE BUFFER
StructuredBuffer<SphereShape> spheres : register(t0);

// CURRENT OUTPUT TEXTURE
RWTexture2D<float4> OutputTex : register(u0);
RWTexture2D<float4> AccumulatedTex : register(u1);


[numthreads(256, 1, 1)]
void CS( uint3 dispatchThreadID : SV_DispatchThreadID, uint3 dispatchGroupID : SV_GroupThreadID)
{
	uint w, h;
	OutputTex.GetDimensions(w, h);
	if (dispatchThreadID.x > w - 1 || dispatchThreadID.y > h-1)
		return;
	
	// Initilize RNG by setting the inital seed and calling a RN function once
	rng_state = InitalRandomSeed * (dispatchThreadID.x * w + dispatchThreadID.y);
	
	Camera cam;
	cam.origin = Position;
	cam.lookAt = Direction;
	cam.focalLength = 1.0f;
		
	// Create Inital Incident Ray
	Ray r = GetRay(dispatchThreadID.x, dispatchThreadID.y, cam);
		
    float4 rayColor = float4(TraceRay(r), 1.f);
	if(ResetSamples)
        AccumulatedTex[dispatchThreadID.xy] = rayColor;
	else if(Accumulate)
        AccumulatedTex[dispatchThreadID.xy] += rayColor;
	
    OutputTex[dispatchThreadID.xy] = AccumulatedTex[dispatchThreadID.xy] / AccumulatedSamples;
}

Ray GetRay(float u, float v, Camera cam)
{
	float3 pixelLoc = Pixel00Center + (u * PixelDeltaX + v * PixelDeltaY);
	
	Ray r;
	r.origin = cam.origin;
	r.direction = normalize(pixelLoc - cam.origin);
	
	return r;
}

float3 TraceRay(Ray ray)
{
	float3 color = 1.0f;
	float currentAttenuation = 1.0f;
	HitRecord hitRec;
	
	for (uint i = 0; i < MaxRayBounces+1; ++i)
	{   
		if (HitHittableList(ray, hitRec))
		{
			ray.origin = hitRec.pos;
			ray.direction = RandomNormalHemisphereFloat3(hitRec.normal);
			currentAttenuation *= 0.5f;
			continue;
		}
		float a = 0.5 * (ray.direction.y + 1.0);
		return currentAttenuation * ((1.0 - a) * float3(1.0, 1.0, 1.0) + a * float3(0.5, 0.7, 1.0));
	}
	
	return float3(0, 0, 0);
}


bool HitHittableList(Ray r, out HitRecord hitRec)
{
	uint numOfSpheres, sphereStride;
	spheres.GetDimensions(numOfSpheres, sphereStride);
	
	HitRecord tempRec;
	float closestHitT = FLOATMAX;
	bool hitSomething = false;
	Sphere tempSphere;
	for (uint i = 0; i < numOfSpheres; ++i)
	{
		tempSphere.position = spheres[i].position;
		tempSphere.radius = spheres[i].radius;
		if (tempSphere.Hit(r, tempRec, 0.01, FLOATMAX) && tempRec.t < closestHitT)
		{
			hitSomething = true;
			hitRec = tempRec;
			closestHitT = tempRec.t;
		}
	}
	
	return hitSomething;
}