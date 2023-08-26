
struct Ray
{
    float3 origin;
    float3 direction;
};

struct Viewport
{
    // Half width and length
    float2 dimensions;
};

struct Camera
{
    float3 origin;
    float3 lookAt;
    float focalLength;
    Viewport viewport;
    
};

// 
class Sphere
{
    float3 position;
    float radius;
    
    float Hit(const Ray r)
    {
        float3 oc = r.origin - position;
        float a = pow(length(r.direction), 2);
        float half_b = dot(oc, r.direction);
        float c = pow(length(oc), 2) - radius * radius;
        float discriminant = half_b * half_b - a * c;

        if (discriminant < 0)
        {
            return -1.0;
        }
        else
        {
            return (-half_b - sqrt(discriminant)) / a;
        }
    }
};



// Function Declarations
float3 TraceRay(Ray ray);
Ray GetRay(float u, float v, Camera cam);
float hit_sphere(float3 center, float radius, const Ray r);

// GLOBAL VALUES
static uint w, h;
static float pixel_delta_u;
static float pixel_delta_v;


// CURRENT OUTPUT TEXTURE
RWTexture2D<float4> outputTex : register(u0);


[numthreads(256, 1, 1)]
void CS( uint3 dispatchThreadID : SV_DispatchThreadID, uint3 dispatchGroupID : SV_GroupThreadID)
{
    outputTex.GetDimensions(w, h);
    float aspectRatio = float(w) / h;

    
    if (dispatchThreadID.x > w - 1 || dispatchThreadID.y > h-1)
        return;
    
    // Initialize Camera
    Viewport view;
    view.dimensions = float2(aspectRatio, 1.0f);

    Camera cam;
    cam.origin = float3(0, 0, 0);
    cam.lookAt = float3(0, 0, 1);
    cam.focalLength = 1.0f;
    cam.viewport = view;
    
    pixel_delta_u = cam.viewport.dimensions.x * 2 / w;
    pixel_delta_v = cam.viewport.dimensions.y * 2 / h;
    
    // Create Inital Incident Ray
    Ray r = GetRay(dispatchThreadID.x, dispatchThreadID.y, cam);
    
    outputTex[dispatchThreadID.xy] = float4(TraceRay(r), 1.0f);
    
    

}

Ray GetRay(float u, float v, Camera cam)
{
    // The way the direction is calculated only works for viewports that are on a Z = focalLength plane
    float3 tlPixel = float3(cam.origin.x - cam.viewport.dimensions.x + pixel_delta_u, cam.origin.y + cam.viewport.dimensions.y + pixel_delta_v, cam.focalLength);
    float3 pixelLoc = tlPixel + float3(u * pixel_delta_u , -v * pixel_delta_v, 0.0f);
    
    Ray r;
    r.origin = cam.origin;
    r.direction = normalize(pixelLoc - cam.origin);
    
    return r;

}




float3 TraceRay(Ray ray)
{
    float3 color;
    
    Sphere s;
    s.position = float3(0, 0, 1);
    s.radius = 0.5;
    
    float t = s.Hit(ray);
    if (t > 0.0)
    {
        float3 N = normalize((ray.origin + (ray.direction * t)) - s.position);
        color = 0.5 * float3(N.x + 1, N.y + 1, N.z + 1);
    }
    else
    {
        float3 unit_direction = normalize(ray.direction);
        float a = 0.5 * (unit_direction.y + 1.0);
        color = (1.0 - a) * float3(1.0, 1.0, 1.0) + a * float3(0.5, 0.7, 1.0);
    }
    
    return color;
}


float hit_sphere(float3 center, float radius, const Ray r)
{
    float3 oc = r.origin - center;
    float a = pow(length(r.direction),2);
    float half_b = dot(oc, r.direction);
    float c = pow(length(oc), 2) - radius * radius;
    float discriminant = half_b*half_b - a*c;

    if (discriminant < 0) {
        return -1.0;
    } else {
        return (-half_b - sqrt(discriminant) ) / a;
    }
}
