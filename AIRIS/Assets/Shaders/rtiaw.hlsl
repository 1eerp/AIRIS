#include "RT.hlsl"

// Function Declarations
float3 TraceRay(Ray ray);
bool HitHittableList(Ray r, out HitRecord hitRec);
Ray GetRay(float u, float v, Camera cam);
float3 Scatter(inout Ray ray, HitRecord hitRec);

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
	float3  CamPosition,
			CamDirection,
			Pixel00Center,
			PixelDeltaX,
			PixelDeltaY,
			LensDefocusX,
			LensDefocusY;
};

// GEOMETRY AND MATERIAL BUFFERS
StructuredBuffer<RTSphere> spheres : register(t0);
StructuredBuffer<RTMaterial> materials : register(t1);
// OUTPUT TEXTURES
RWTexture2D<float4> OutputTex : register(u0);
RWTexture2D<float4> AccumulatedTex : register(u1);

////////////////////
//				  //
// COMPUTE SHADER //
//				  //
////////////////////

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
	cam.origin = CamPosition;
	cam.lookAt = CamDirection;
	cam.focalLength = 1.0f;
		
	// Create Inital Incident Ray
	Ray r = GetRay(dispatchThreadID.x, dispatchThreadID.y, cam);
		
    float4 rayColor = float4(TraceRay(r), 1.f);
	if(ResetSamples)
        AccumulatedTex[dispatchThreadID.xy] = rayColor;
	else if(Accumulate)
        AccumulatedTex[dispatchThreadID.xy] += rayColor;
	
    OutputTex[dispatchThreadID.xy] = sqrt(AccumulatedTex[dispatchThreadID.xy] / AccumulatedSamples);
}

Ray GetRay(float u, float v, Camera cam)
{
	//				TL Pixel Center		Move to pixel for this thread		RANDOM OFFSET WITHIN THE PIXEL/NOT INTRUDING ON SURROUNDING PIXELS
    float3 pixelLoc = Pixel00Center + (u * PixelDeltaX + v * PixelDeltaY) + (PixelDeltaX * (GetRandomFloat() - 0.5f) + PixelDeltaY * (GetRandomFloat() - 0.5f));
	
	Ray r;
    float3 randOffset = RandomInUnitDisk();
    r.origin = cam.origin + (LensDefocusX * randOffset.x) + (LensDefocusY * randOffset.y);
	r.direction = normalize(pixelLoc - r.origin);
	
	return r;
}

float3 TraceRay(Ray ray)
{
	float3 color = 1.0f;
    float3 currentAttenuation = float3(1.f, 1.f, 1.f);
	HitRecord hitRec;
	
	for (uint i = 0; i < MaxRayBounces+1; ++i)
	{   
		if (HitHittableList(ray, hitRec))
		{
			ray.origin = hitRec.pos;
            currentAttenuation *= Scatter(ray, hitRec);
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
            hitRec.materialIndex = spheres[i].matIndex;
			closestHitT = tempRec.t;
		}
	}
	
	return hitSomething;
}

float reflectance(float cosine, float ref_idx)
{
        // Use Schlick's approximation for reflectance.
    float r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

float3 Scatter(inout Ray ray, HitRecord hitRec)
{
    float3 color = materials[hitRec.materialIndex].Albedo;
    if(materials[hitRec.materialIndex].Type == 0)
    {
		ray.direction = normalize(hitRec.normal + GetRandomUnitFloat3());
    }
	else if(materials[hitRec.materialIndex].Type == 1)
    {
		float3 reflectedDir = reflect(ray.direction, hitRec.normal);
		ray.direction = normalize(reflectedDir);
    }
	else if(materials[hitRec.materialIndex].Type == 2)
    {
		// Currently all objects will have the same IOR
        float IOR = 1.33;
        float refraction_ratio = hitRec.frontFace ? (1.0 / IOR) : IOR;
		
        float cos_theta = min(dot(-ray.direction, hitRec.normal), 1.0);
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        float3 direction;
        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > GetRandomFloat())
            direction = reflect(ray.direction, hitRec.normal);
        else
            direction = refract(ray.direction, hitRec.normal, refraction_ratio);

        ray.direction = direction;
        color = float3(1.0f, 1.0f, 1.0f);

    }
    return color;
}