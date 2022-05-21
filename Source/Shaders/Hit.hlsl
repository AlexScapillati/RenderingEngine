
//--------------------------------------------------------------------------------------
// Raytracing 
//--------------------------------------------------------------------------------------


static float FLT_MAX = asfloat(0x7F7FFFFF);

// Hit information, aka ray payload
// This sample only carries a shading color and hit distance.
// Note that the payload should be kept as small as possible,
// and that its size must be declared in the corresponding
// D3D12_RAYTRACING_SHADER_CONFIG pipeline subobjet.
struct HitInfo
{
    float4 colorAndDistance;
};

struct ShadowHitInfo
{
    bool isHit;
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

cbuffer gLightsBuffer : register(b0)
{
    sLight gLights[63];
}

cbuffer TextureCoordinates : register(b1)
{
    int2 gAlbedoDims;
    int2 gNormalDims;
    int2 gDisplacementDims;
    int2 gRoughnessDims;
    int2 gMetalnessDims;
    int2 gAODims;
}

// Raytracing acceleration structure, accessed as a SRV
RaytracingAccelerationStructure SceneBVH : register(t0);

StructuredBuffer<Vertex> Vertices : register(t1);
StructuredBuffer<int> indices : register(t2);

Texture2D<float4> AlbedoMap : register(t3);
Texture2D<float4> NormalMap : register(t4);
Texture2D<float4> DisplacementMap : register(t5);
Texture2D<float4> RoughnessMap : register(t6);
Texture2D<float4> MetalnessMap : register(t7);
Texture2D<float4> AOMap : register(t8);

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


[shader("anyhit")]
void AnyHit(inout HitInfo payload, Attributes attrib)
{
    float3 barycentrics = float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    uint vertId = 3 * PrimitiveIndex();
   
    
    float3 n = Vertices[indices[vertId + 0]].normal * barycentrics.x +
				Vertices[indices[vertId + 1]].normal * barycentrics.y +
				Vertices[indices[vertId + 2]].normal * barycentrics.z;

    
    float2 uv = Vertices[indices[vertId + 0]].uv * barycentrics.x +
				Vertices[indices[vertId + 1]].uv * barycentrics.y +
				Vertices[indices[vertId + 2]].uv * barycentrics.z;
    
    int2 coord = floor(uv * gAlbedoDims);
    float alpha = AlbedoMap.Load(int3(coord, 0)).a;

    if (alpha < .5f)
        IgnoreHit();
    else
        AcceptHitAndEndSearch();
}


int2 ParallaxMapping(int2 UV, float3 v)
{
    //------------------------------
    // Common linear search for parallax occlusion mapping and relief mapping
    
    // Viewing ray descends at angle through the height map layer. Take several samples of the height map along this
    // section to find where the ray intersects the texture surface. The ray starts at the top of the layer, above the
    // surface, step it along by increments until the ray goes below the surface. This initial linear search to finds
    // the rough intersection, the last two points (one above, one below the surface) are then used to refine the search
    
    // Determine number of samples based on viewing angle. A more shallow angle needs more samples
    const float minSamples = 5.0f;
    const float maxSamples = 20.0f;
    const float numSamples = lerp(maxSamples, minSamples, abs(v.z)); // The view vector is in tangent space, so its z value indicates
                                                               // how much it is pointing directly away from the polygon
    
    // For each step along the ray direction, find the amount to move the UVs and the amount to descend in the height layer
    float rayHeight = 1.0f; // Current height of ray, 0->1 in the height map layer. Start at the top of the layer
    float heightStep = 1.0f / numSamples; // Amount the ray descends for each step
    float2 uvStep = (0.6f * v.xy / v.z) / numSamples; // Ray UV offset for each step. Can also remove the / v.z here
                                                               // to add limiting, which will reduce artefacts at glancing angles
                                                               // but will also reduce the depth at those angles
    
    // Sample height map at intial UVs (top of layer)
    float surfaceHeight = DisplacementMap.Load(int3(UV, 0)).r;
    float prevSurfaceHeight = surfaceHeight;
    
    
    // While ray is above the surface
    while (rayHeight > surfaceHeight)
    {
        // Make short step along ray - move UVs and descend ray height
        rayHeight -= heightStep;
        UV -= uvStep;

        // Sample height map again
        prevSurfaceHeight = surfaceHeight;
        surfaceHeight = NormalMap.Load(int3(UV, 0)).r;
    }


    //------------------------------
    // Parallax occulusion mapping
    
    //Final linear interpolation between last two points in linear search
    
    // Calculate how much the current step is below surface, and how much previous step was above the surface
    const float currDiff = surfaceHeight - rayHeight;
    const float prevDiff = (rayHeight + heightStep) - prevSurfaceHeight;
    
    // Use linear interpolation to estimate how far back to retrace the previous step to the instersection with the surface
    const float weight = currDiff / (currDiff + prevDiff); // 0->1 value, how much to backtrack
    
    // Final interpolation of UVs
    UV += uvStep * weight;
    
    // Also interpolate height value (only needed if doing self-shadowing)
    rayHeight += heightStep * weight;
    
    
    //-----------------
    // Relief mapping
    
    // Refine initial linear search with a binary search between the last two points (one above, one below the surface)

    const int binarySearchSamples = 5; // Number of binary steps
    for (int i = 0; i < binarySearchSamples; ++i)
    {
        // Halve search distance
        heightStep *= 0.5f;
        uvStep *= 0.5f;

        // Choose direction to move along ray based on whether we are above or below surface
        const float dir = sign(surfaceHeight - rayHeight); // -1 or 1 for above or below surface (avoid an if statement this way)
        rayHeight += dir * heightStep;
        UV += dir * uvStep;

        // Sample height map again
        surfaceHeight = DisplacementMap.Load(int3(UV, 0)).r;
    }

    return UV;
}

float ShootShadowRay(float3 lightDir)
{
    RayDesc rayDesc;
    rayDesc.Direction = lightDir;
    rayDesc.Origin = HitWorldPosition(),
    rayDesc.TMin = 0;
    rayDesc.TMax = 10000;

    ShadowHitInfo shadowPayload;
    shadowPayload.isHit = false;

    TraceRay(SceneBVH, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 1, 0, 1, rayDesc, shadowPayload);

    return shadowPayload.isHit ? 0.3f : 1.0f;
}


static const float PI = 3.14159265359f;

[shader("closesthit")]
void ClosestHit(inout HitInfo payload, Attributes attrib)
{
    float3 barycentrics = float3(1.f - attrib.bary.x - attrib.bary.y, attrib.bary.x, attrib.bary.y);

    uint vertId = 3 * PrimitiveIndex();
    
    float3 n = Vertices[indices[vertId + 0]].normal * barycentrics.x +
				Vertices[indices[vertId + 1]].normal * barycentrics.y +
				Vertices[indices[vertId + 2]].normal * barycentrics.z;

    
    float2 uv = Vertices[indices[vertId + 0]].uv * barycentrics.x +
				Vertices[indices[vertId + 1]].uv * barycentrics.y +
				Vertices[indices[vertId + 2]].uv * barycentrics.z;
    
    
    float3 worldTangent = Vertices[indices[vertId + 0]].tangent * barycentrics.x +
                          Vertices[indices[vertId + 1]].tangent * barycentrics.y +
                          Vertices[indices[vertId + 2]].tangent * barycentrics.z;

    n = normalize(mul((float3x3) ObjectToWorld3x4(), n));

    float3 v = normalize(-WorldRayDirection());

    RayDesc ray;
    ray.Origin = HitWorldPosition();
    ray.Direction = reflect(-v, n);
    ray.TMin = 0;
    ray.TMax = 10000;

    HitInfo IBLPayload;

    TraceRay(SceneBVH, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, ray, IBLPayload);

    int2 coord = floor(uv * gAlbedoDims);

	// Calculate the bitangent with the cross product of the world normal and the world tangent
    float3 worldBitangent = normalize(cross(n, normalize(worldTangent)));
    
	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate
    const float3x3 tangentMatrix = float3x3(worldTangent, worldBitangent, n);
    
	// Offset UVs due to parallax mapping, requires view vector (parallax mapping operates in tangent space, so transform vector to tangent space)
    const float3 tangentSpaceV = mul(v, transpose(tangentMatrix));

    // Enable parallax mapping only if there are both the normal map and the displacement map
    if (gDisplacementDims.x && gNormalDims.x)
        coord = ParallaxMapping(coord, tangentSpaceV);
    
    // Sample textures using the converted UV (if any)
    
    float3 albedo = AlbedoMap.Load(int3(coord, 0)).rgb;
    float roughness = RoughnessMap.Load(int3(coord, 0)).r;
    float metalness = MetalnessMap.Load(int3(coord, 0)).r;
    float ao = gMetalnessDims.x ? 1.0f : AOMap.Load(int3(coord, 0)).r;

    // Precalculate stuff
    float3 specularColour = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);
    const float nDotV = max(dot(n, v), 0.001f);
    float3 fresnel = specularColour + (1.0f - specularColour) * pow(max(1.0f - nDotV, 0.0f), 5.0f);
    float3 diffuse = ao * albedo * IBLPayload.colorAndDistance.rgb + (1 - roughness) * fresnel * IBLPayload.colorAndDistance.rgb;
    float3 specular = specularColour;

    
    // Light calculations ( BDRF )
    for (int i = 0; i < 1; ++i)
    {
        float3 lightPos = gLights[i].position;

        // Get light vector (normal towards light from pixel), attenuate light intensity at the same time
        float3 l = lightPos - HitWorldPosition();
        const float rdist = 1.0f / length(l);
        l *= rdist;

        float3 li = gLights[i].intensity * rdist * rdist;
        float3 lc = gLights[i].colour;

        // Before complex calculations, shoot ray for shadows
        float shadow = ShootShadowRay(l);
    
		// Halfway vector (normal halfway between view and light vector)
        const float3 h = normalize(l + v);

		// Various dot products used throughout
        const float nDotL = max(dot(n, l), 0.001f);
        const float nDotH = max(dot(n, h), 0.001f);
        const float vDotH = max(dot(v, h), 0.001f);

    	// Lambert diffuse
        const float3 lambert = albedo / PI;

		// Microfacet specular - fresnel term
        const float3 F = specular + (1.0f - specular) * pow(max(1.0f - vDotH, 0.0f), 5.0f);

		// Microfacet specular - normal distribution term
        const float alpha = max(roughness * roughness, 2.0e-3f); // Dividing by alpha in the dn term so don't allow it to reach 0
        const float alpha2 = alpha * alpha;
        const float nDotH2 = nDotH * nDotH;
        const float dn = nDotH2 * (alpha2 - 1.0f) + 1.0f;
        const float D = alpha2 / (PI * dn * dn);

		// Microfacet specular - geometry term
        float k = (roughness + 1.0f);
        k = k * k / 8.0f;
        const float gV = nDotV / (nDotV * (1.0f - k) + k);
        const float gL = nDotL / (nDotL * (1.0f - k) + k);
        const float G = gV * gL;

		// Full brdf, diffuse + specular
        const float3 brdf = F * lambert + F * G * D / (4.0f * nDotL * nDotV);

		// Accumulate punctual light equation for this light
        diffuse += PI * li * lc * brdf * nDotL * shadow;
    }


    payload.colorAndDistance = float4(diffuse, RayTCurrent());
}
