struct VertexIn
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosS : SV_POSITION;
    float4 Color : COLOR;
};

cbuffer CBPerObject : register(b0)
{
    float4x4 Model;
    float4x4 View;
    float4x4 Proj;
    float4x4 MVP;
}

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
	
	// Transform to homogeneous clip space.
    vout.PosS = mul(MVP, float4(vin.PosL, 1.0f));
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


