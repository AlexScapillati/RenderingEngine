

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


 // Camera
cbuffer CameraParams : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewI;
    float4x4 projectionI;
}

// Raytracing output texture, accessed as a UAV
RWTexture2D<float4> gOutput : register(u0);

// Raytracing acceleration structure, accessed as a SRV
RaytracingAccelerationStructure SceneBVH : register(t0);

[shader("raygeneration")]
void RayGen()
{
	// Initialize the ray payload
    HitInfo payload;
    payload.colorAndDistance = float4(0, 0, 0, 0);
    
	// Get the location within the dispatched 2D grid of work items
	// (often maps to pixels, so this could represent a pixel coordinate).
    uint2 launchIndex = DispatchRaysIndex().xy;
    float2 dims = float2(DispatchRaysDimensions().xy);
    float2 d = (((launchIndex.xy + 0.5f) / dims.xy) * 2.f - 1.f);
	// Define a ray, consisting of origin, direction, and the min-max distance values
    float aspectRatio = dims.x / dims.y;

	// Perspective
    RayDesc ray;
    ray.Origin = mul(viewI, float4(0, 0, 0, 1)).xyz;
    float4 target = mul(projectionI, float4(d.x, -d.y, 1, 1));
    ray.Direction = mul(viewI, float4(target.xyz, 0)).xyz;
    ray.TMin = 0;
    ray.TMax = 100000;

    // Trace the ray
    TraceRay(
    SceneBVH,                            //  AccelerationStructure
    RAY_FLAG_CULL_BACK_FACING_TRIANGLES, //  RayFlags
    0xFF,                                //  InstanceInclusionMask
    0,                                   //  RayContributionToHitGroupIndex
    0,                                   //  MultiplierForGeometryContributionToHitGroupIndex
    0,                                   //  MissShaderIndex
    ray,                                 //  Ray
    payload                              //  Payload
    );

    gOutput[launchIndex] = float4(payload.colorAndDistance.rgb, 1.f);
}
