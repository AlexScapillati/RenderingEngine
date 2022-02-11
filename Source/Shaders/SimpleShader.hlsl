
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : UV;
	
};


struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPosition : worldPostion;
    float3 worldNormal : worldNormal;
    float3 worldTangent : worldTangent;
    float2 uv : UV;
};

static const int MAX_LIGHTS = 64;

struct SLight
{
    float3 position;
    float enabled;
    float3 colour;
    float intensity;
};


cbuffer PerModelConstants : register(b0) // The b1 gives this constant buffer the number 1 - used in the C++ code
{
    float4x4 gWorldMatrix;
    float4x4 padding[3];
}

cbuffer PerFrameConstants : register(b1) // The b0 gives this constant buffer the number 0 - used in the C++ code
{
    float4x4 gCameraMatrix; // World matrix for the camera (inverse of the ViewMatrix below) - used in particle rendering geometry shader
    float4x4 gViewMatrix;
    float4x4 gProjectionMatrix;
    float4x4 gViewProjectionMatrix; // The above two matrices multiplied together to combine their effects
    float3   gAmbient;
    float    gSpecularPower;
	SLight   gLights[MAX_LIGHTS];
    
    float   gRoughness;
    float   gMetalness;
    bool gUseCustomValues;
    
    float padding1[58];
}

PSInput VSMain(VSInput input)
{
    PSInput result;
    
     // Multiply by the world matrix passed from C++ to transform the model vertex position into world space. 
    // In a similar way use the view matrix to transform the vertex from world space into view space (camera's point of view)
    // and then use the projection matrix to transform the vertex to 2D projection space (project onto the 2D screen)
    const float4 worldPosition = mul(gWorldMatrix, float4(input.position, 1));
    const float4 viewPosition = mul(gViewMatrix, worldPosition);

    result.position = mul(gProjectionMatrix, viewPosition);
    result.uv = input.uv;

    result.worldNormal = mul(gWorldMatrix, float4(input.normal,0)).xyz;

    result.worldPosition = worldPosition;
    result.worldTangent = input.tangent;

    return result;
}

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.

SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic


Texture2D AlbedoMap         : register(t0);
Texture2D RoughnessMap      : register(t1);
Texture2D AoMap             : register(t2);
Texture2D DisplacementMap   : register(t3);
Texture2D NormalMap         : register(t4);
Texture2D MetalnessMap      : register(t5);


//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------

static const float PI = 3.14159265359f;


//Using helper functions for readability,pixel shader itself is below

// Normal mapping - sample normal map at given position and UV for a surface of given normal and tangent.
// Optionally uses parallax mapping, in which case UV is updated
float3 SampleNormal(float3 position, float3 normal, float3 tangent, inout float2 UV, bool parallax = false)
{

	// Calculate inverse tangent matrix
    float3 biTangent = cross(normal, tangent);
    float3x3 invTangentMatrix = float3x3(tangent, biTangent, normal);

	// Parallax mapping. Comment out for plain normal mapping
    if (parallax)
    {

		// no need to pass another variable through the constant buffer
		// just get the last row of the camera matrix to get its position
        float3 cameraPosition = gCameraMatrix._m33_m33_m33;

		// Get camera direction in model space
        float3 cameraDir = normalize(cameraPosition - position);
        float3x3 invWorldMatrix = transpose((float3x3) gWorldMatrix);
        float3 cameraModelDir = normalize(mul(cameraDir, invWorldMatrix));

		// Calculate direction to offset UVs (x and y of camera direction in tangent space)
        float3x3 tangentMatrix = transpose(invTangentMatrix);
        float2 textureOffsetDir = mul(cameraModelDir, tangentMatrix).xy;

		// Offset UVs in that direction to account for depth (using height map and some geometry)
        float texDepth = 0.06f * (DisplacementMap.Sample(TexSampler, UV).r - 0.5f);
        UV += texDepth * textureOffsetDir;
    }

	// Extract normal from map and shift to -1 to 1 range
    float3 textureNormal = 2.0f * NormalMap.Sample(TexSampler, UV).rgb - 1.0f;
    textureNormal.y = -textureNormal.y;

	// Convert normal from tangent space to world space
    return normalize(mul(mul(textureNormal, invTangentMatrix), (float3x3) gWorldMatrix));
}




float4 PSMain(PSInput input) : SV_TARGET
{
    input.worldNormal = normalize(input.worldNormal);
    input.worldTangent = normalize(input.worldTangent);

	float3 v = SampleNormal(input.worldPosition,input.worldNormal,input.worldTangent,input.uv,true);

    //-----------------------------
    // Sample Textures
    //-----------------------------
    
    // Get the texture colour
    const float3 albedo = AlbedoMap.Sample(TexSampler, input.uv).rgb;

    const float alpha1 = AlbedoMap.Sample(TexSampler, input.uv).a;
    const float roughness = gUseCustomValues ? gRoughness : RoughnessMap.Sample(TexSampler, input.uv).r;
    const float ao = AoMap.Sample(TexSampler, input.uv).r;
    const float metalness = gUseCustomValues ? gMetalness : MetalnessMap.Sample(TexSampler, input.uv).r;
    

    if (AlbedoMap.Sample(TexSampler, input.uv).a == 0.0f)
        discard;

    float3 specular = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);

    //-----------------------------
    // Calculate lighting
    //-----------------------------

    const float3 n = input.worldNormal;
    
	const float3 lightPos       = gLights[0].position;
	const float  lightIntensity = gLights[0].intensity;
    
    const float nDotV = max(dot(n, v), 0.001f);

    float3 colour = gAmbient * ao;

    // Get light vector (normal towards light from pixel), attenuate light intensity at the same time
    float3 l = lightPos - input.worldPosition;
    const float rdist = 1.0f / length(l);
    l *= rdist;
    const float li = lightIntensity * rdist * rdist;
    const float3 lc = gLights[0].colour;

    // Halfway vector (normal halfway between view and light vector)
    const float3 h = normalize(l + v);
    
    // Various dot products used throughout
    float nDotL = max(dot(n, l), 0.001f);
    float nDotH = max(dot(n, h), 0.001f);
    float vDotH = max(dot(v, h), 0.001f);

    // Lambert diffuse
    float3 lambert = albedo / PI;

    // Microfacet specular - fresnel term
    float3 F = specular + (1 - specular) * pow(max(1.0f - vDotH, 0.0f), 5.0f);

    // Microfacet specular - normal distribution term
    float alpha = max(roughness * roughness, 2.0e-3f); // Dividing by alpha in the dn term so don't allow it to reach 0
    float alpha2 = alpha * alpha;
    float nDotH2 = nDotH * nDotH;
    float dn = nDotH2 * (alpha2 - 1) + 1;
    float D = alpha2 / (PI * dn * dn);

    // Microfacet specular - geometry term
    float k = (roughness + 1);
    k = k * k / 8;
    float gV = nDotV / (nDotV * (1 - k) + k);
    float gL = nDotL / (nDotL * (1 - k) + k);
    float G = gV * gL;

    // Full brdf, diffuse + specular
    float3 brdf = lambert + F * G * D / (4 * nDotL * nDotV);

    // Accumulate punctual light equation for this light
    colour += PI * li * lc * brdf * nDotL;

    return float4(colour, alpha1);
}
