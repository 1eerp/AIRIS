struct VertexIn
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR;
};

cbuffer CBObjectData : register(b0)
{
    float4x4 Model;
}

cbuffer CBPassData : register(b1)
{
    float4x4 View;
    float4x4 InvView;
    float4x4 Proj;
    float4x4 InvProj;
    float4x4 ViewProj;
    float4x4 InvViewProj;
    float3 EyePosW;
    // IGNORE PAD, I don't think I need this here because this byte will get consumed (this doesn't mean it will be used) by the last varaable
    // if this is not present I believe,but will leave it there for now because I am not 100% sure and since I don't use anything past this yet
    float _ignorePad1; 
    float2 RenderTargetSize;
    float2 InvRenderTargetSize;
    float NearZ;
    float FarZ;
    float TotalTime;
    float DeltaTime;
};


VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    
    // Get World Position
    float4 posW = mul(Model, float4(vin.PosL, 1.0f));
    // Transform to homogeneous clip space.
    vout.PosH = mul(ViewProj, posW);
    
    // Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


