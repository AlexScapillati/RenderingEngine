
//--------------------------------------------------------------------------------------
// Raytracing 
//--------------------------------------------------------------------------------------


// Hit information, aka ray payload
// This sample only carries a shading color and hit distance.
// Note that the payload should be kept as small as possible,
// and that its size must be declared in the corresponding
// D3D12_RAYTRACING_SHADER_CONFIG pipeline subobjet.
struct HitInfo
{
    float4 colorAndDistance;
};

// Attributes output by the raytracing when hitting a surface,
// here the barycentric coordinates
struct Attributes
{
    float2 bary;
};

struct Vertex
{
    float3 position : position;
    float3 normal : normal;
    float3 tangent : tangent;
    float2 uv : uv;
};

struct sLight
{
    float3 position;
    float enabled;
    float3 colour;
    float intensity;
};


//cbuffer gLightsBuffer : register(b2)
//{
//    sLight gLights[64];
//}


//struct gLightsBuffer
//{
//    sLight gLights[64];
//};

StructuredBuffer<Vertex> Vertices : register(t0);
StructuredBuffer<int> indices : register(t1);

//StructuredBuffer<sLight> gLights: register(t2);

//ConstantBuffer<gLightsBuffer> gLights : register(b2);

// Retrieve hit world position.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float3 HitAttribute(float3 vertexAttribute[3], Attributes attr)
{
    return vertexAttribute[0] +
        attr.bary.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.bary.y * (vertexAttribute[2] - vertexAttribute[0]);
}

[shader("closesthit")] 
void ClosestHit(inout HitInfo payload, Attributes attrib)
{
    float3 barycentrics = float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    uint vertId = 3 * PrimitiveIndex();

    float3 outColour = float3(0.f, 0.f, 0.f);

    //float3 n =  Vertices[indices[vertId + 0]].normal.rgb * barycentrics.x +
				//Vertices[indices[vertId + 1]].normal.rgb * barycentrics.y +
				//Vertices[indices[vertId + 2]].normal.rgb * barycentrics.z;
    
    float3 n =  Vertices[vertId + 0].normal.rgb * barycentrics.x +
				Vertices[vertId + 1].normal.rgb * barycentrics.y +
				Vertices[vertId + 2].normal.rgb * barycentrics.z;



    outColour = n;

 //   for (int i = 0; i < 1; ++i)
 //   {
 //       float3 lightPos = /*gLights[i].position*/float3(0, 2, 0);
 //       float3 pixelToLight = normalize( lightPos - HitWorldPosition());

 //       // Compute attenuation
 //       float d = length(pixelToLight);
 //       float att = 1.0 / (1.0 + 1.0 * d + 1.0 * pow(d, 2));

	//	 // Diffuse contribution.
 //       float fNDotL = max(0.0f, dot(pixelToLight, n) * att);
        
 //       outColour += fNDotL;
 //   }

    
    payload.colorAndDistance = float4(outColour, RayTCurrent());
}
