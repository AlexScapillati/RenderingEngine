
#define ROOT_SIGNATURE \
					"RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
					"CBV(b0, flags = DATA_STATIC), " \
					"CBV(b1, flags = DATA_STATIC), " \
					"CBV(b2, flags = DATA_STATIC), " \
					"CBV(b3, flags = DATA_STATIC), " \
					"CBV(b4, flags = DATA_STATIC), " \
					"CBV(b5, flags = DATA_STATIC), " \
                    "SRV(t0), " \
                    "SRV(t1), " \
                    "SRV(t2), " \
                    "SRV(t3), " \
                    "SRV(t4), " \
                    "SRV(t6), " \
                    "SRV(t7), " \
                    "StaticSampler(s1)," \
                    "StaticSampler(s2)"


//--------------------------------------------------------------------------------------
// Shader input / output
//--------------------------------------------------------------------------------------


// The structure below describes the vertex data to be sent into the vertex shader for ordinary non-skinned models
struct BasicVertex
{
    float3 position : position;
    float3 normal : normal;
    float2 uv : uv;
};

// The structure below describes the vertex data to be sent into vertex shaders that need tangents
//****| INFO | Models that contain tangents can only be sent into shaders that accept this structure ****//
struct TangentVertex
{
    float3 position : position;
    float3 normal : normal;
    float3 tangent : tangent;
    float2 uv : uv;
};


// This structure describes what data the lighting pixel shader receives from the vertex shader.
// The projected position is a required output from all vertex shaders - where the vertex is on the screen
// The world position and normal at the vertex are sent to the pixel shader for the lighting equations.
// The texture coordinates (uv) are passed from vertex shader to pixel shader unchanged to allow textures to be sampled
struct LightingPixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition; // The world position and normal of each vertex is passed to the pixel...
    float3 worldNormal : worldNormal; //...shader to calculate per-pixel lighting. These will be interpolated
                                            // automatically by the GPU (rasterizer stage) so each pixel will know
                                            // its position and normal in the world - required for lighting equations
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};

//****| INFO |*******************************************************************************************//
// Like per-pixel lighting, normal mapping expects the vertex shader to pass over the position and normal.
// However, it also expects the tangent (see lecturee). Furthermore the normal and tangent are left in
// model space, i.e. they are not transformed by the world matrix in the vertex shader - just sent as is.
// This is because the pixel shader will do the matrix transformations for normals in this case
//*******************************************************************************************************//
// The data sent from vertex to pixel shaders for normal mapping
struct NormalMappingPixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition; // Data required for lighting calculations in the pixel shader
    float3 worldNormal : worldNormal; // --"--
    float3 worldTangent : worldTangent; // --"--
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};


// This structure is similar to the one above but for the light models, which aren't themselves lit
struct SimplePixelShaderInput
{
    float4 projectedPosition : SV_Position;
    float2 uv : uv;
};

// The vertex data received by each post-process shader. Just the 2d projected position (pixel coordinate on screen), 
// and two sets of UVs - one for accessing the texture showing the scene, one refering to the area being affected (see the 2DQuad_pp.hlsl file for diagram & details)
struct PostProcessingInput
{
    float4 projectedPosition : SV_Position;
    noperspective float2 sceneUV : sceneUV; // "noperspective" is needed for polygon processing or the sampling of the scene texture doesn't work correctly (ask tutor if you are interested)
    float2 areaUV : areaUV;
};

struct sLight
{
    float3 position;
    float enabled;
    float3 colour;
    float intensity;
};

struct sSpotLight
{
    float3 colour;
    float enabled;
    float3 pos;
    float intensity;
    float3 facing; //the direction facing of the light 
    float cosHalfAngle; //pre calculate this in the c++ side, for performance reasons
    float4x4 viewMatrix; //the light view matrix (as it was a camera)
    float4x4 projMatrix; //--"--
};


struct sDirLight
{
    float3 colour;
    float enabled;
    float3 facing;
    float intensity;
    float4x4 viewMatrix; //the light view matrix (as it was a camera)
    float4x4 projMatrix; //--"--
};


struct sPointLight
{
    float3 colour;
    float enabled;
    float3 pos;
    float intensity;
    float4x4 viewMatrices[6];
    float4x4 projMatrix;
};

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

static const int MAX_LIGHTS = 64;

// These structures are "constant buffers" - a way of passing variables over from C++ to the GPU
// They are called constants but that only means they are constant for the duration of a single GPU draw call.
// These "constants" correspond to variables in C++ that we will change per-model, or per-frame etc.

// In this exercise the matrices used to position the camera are updated from C++ to GPU every frame along with lighting information
// These variables must match exactly the gPerFrameConstants structure in Scene.cpp

// If we have multiple models then we need to update the world matrix from C++ to GPU multiple times per frame because we
// only have one world matrix here. Because this data is updated more frequently it is kept in a different buffer for better performance.
// We also keep other data that changes per-model here
// These variables must match exactly the gPerModelConstants structure in Scene.cpp
cbuffer PerModelConstants : register(b0) // The b1 gives this constant buffer the number 1 - used in the C++ code
{
    float4x4 gWorldMatrix;
             
    float3 gObjectColour; // Used for tinting light models
    float gParallaxDepth; // Used in the pixel shader to control how much the polygons are bumpy
    float gHasOpacityMap;
    float gHasAoMap;
    float gHasRoughnessMap;
    float gHasAmbientMap;
    float gHasMetallnessMap;
    float gRoughness;
    float gMetalness;

    float padding[37];
}


cbuffer PerFrameConstants : register(b1) // The b0 gives this constant buffer the number 0 - used in the C++ code
{
    float4x4 gCameraMatrix; // World matrix for the camera (inverse of the ViewMatrix below) - used in particle rendering geometry shader
    float4x4 gViewMatrix;
    float4x4 gProjectionMatrix;
    float4x4 gViewProjectionMatrix; // The above two matrices multiplied together to combine their effects
    
    float3 gAmbientColour;
    float1 gSpecularPower;
    
    float gParallaxMinSample;
    float gParallaxMaxSample;
    float parallaxPad;
    
    float gDepthAdjust;
    
    float gNumLights = 0;
    float gNumDirLights = 0;
    float gNumSpotLights = 0;
    float gNumPointLights = 0;
    
    int gPcfSamples;
    float3 padding2;

    float3 gCameraPosition;
    float gFrameTime; // This app does updates on the GPU so we pass over the frame update time

    float padding3[44];
}
// Note constant buffers are not structs: we don't use the name of the constant buffer, these are really just a collection of global variables (hence the 'g')

cbuffer PerFrameLights : register(b2)
{
    sLight gLights[MAX_LIGHTS];
}

cbuffer PerFrameSpotLights : register(b3)
{
    sSpotLight gSpotLights[MAX_LIGHTS];
}

cbuffer PerFrameDirLights : register(b4)
{
    sDirLight gDirLights[MAX_LIGHTS];
}

cbuffer PerFramePointLights : register(b5)
{
    sPointLight gPointLights[MAX_LIGHTS];
}


NormalMappingPixelShaderInput VSMain(TangentVertex input)
{
    NormalMappingPixelShaderInput result;
    
     // Multiply by the world matrix passed from C++ to transform the model vertex position into world space. 
    // In a similar way use the view matrix to transform the vertex from world space into view space (camera's point of view)
    // and then use the projection matrix to transform the vertex to 2D projection space (project onto the 2D screen)
    const float4 worldPosition = mul(gWorldMatrix, float4(input.position, 1));
    const float4 viewPosition = mul(gViewMatrix, worldPosition);

    result.projectedPosition = mul(gProjectionMatrix, viewPosition);
    result.uv = input.uv;

    result.worldNormal = mul(gWorldMatrix, float4(input.normal,0)).xyz;

    result.worldPosition = worldPosition;
    result.worldTangent = input.tangent;

    return result;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.

SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic
SamplerState PointClamp : register(s1); // Sampler for the shadowMaps

Texture2D AlbedoMap         : register(t0);
Texture2D RoughnessMap      : register(t1);
Texture2D AoMap             : register(t2);
Texture2D DisplacementMap   : register(t3);
Texture2D NormalMap         : register(t4);
Texture2D MetalnessMap      : register(t5);
TextureCube IBLMap          : register(t6);
Texture2D ShadowMaps        : register(t7);

//--------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------

static const float PI = 3.14159265359f;


float3 CalculateLight(const float3 lightPos,
					  const float  lightIntensity,
					  const float3 colour,
					  float3       diffuse,
					  float3       specular,
					  float3       n,
					  float3       v,
					  float3       worldPos,
					  float        roughness,
					  float3       albedo)
{
	const float nDotV = max(dot(n, v), 0.001f);

	// Get light vector (normal towards light from pixel), attenuate light intensity at the same time
	float3      l     = lightPos - worldPos;
	const float rdist = 1.0f / length(l);
	l *= rdist;
	const float  li = lightIntensity * rdist * rdist;
	const float3 lc = colour;

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
	const float alpha  = max(roughness * roughness, 2.0e-3f); // Dividing by alpha in the dn term so don't allow it to reach 0
	const float alpha2 = alpha * alpha;
	const float nDotH2 = nDotH * nDotH;
	const float dn     = nDotH2 * (alpha2 - 1.0f) + 1.0f;
	const float D      = alpha2 / (PI * dn * dn);

	// Microfacet specular - geometry term
	float k        = (roughness + 1.0f);
	k              = k * k / 8.0f;
	const float gV = nDotV / (nDotV * (1.0f - k) + k);
	const float gL = nDotL / (nDotL * (1.0f - k) + k);
	const float G  = gV * gL;

	// Full brdf, diffuse + specular
	const float3 brdf = lambert + F * G * D / (4.0f * nDotL * nDotV);

	// Accumulate punctual light equation for this light
	diffuse += PI * li * lc * brdf * nDotL;

	return diffuse;
}


//Using helper functions for readability,pixel shader itself is below

// Normal mapping - sample normal map at given position and UV for a surface of given normal and tangent.
// Optionally uses parallax mapping, in which case UV is updated
float3 SampleNormal(float3       position,
					float3       normal,
					float3       tangent,
					inout float2 UV,
					bool         parallax = false)
{
	// Calculate inverse tangent matrix
	float3         biTangent        = cross(normal, tangent);
	const float3x3 invTangentMatrix = float3x3(tangent, biTangent, normal);

	// Parallax mapping. Comment out for plain normal mapping
	if (parallax)
	{
		// no need to pass another variable through the constant buffer
		// just get the last row of the camera matrix to get its position
		const float3 cameraPosition = gCameraMatrix._m33_m33_m33;

		// Get camera direction in model space
		const float3   cameraDir      = normalize(cameraPosition - position);
		const float3x3 invWorldMatrix = transpose((float3x3)gWorldMatrix);
		const float3   cameraModelDir = normalize(mul(cameraDir, invWorldMatrix));

		// Calculate direction to offset UVs (x and y of camera direction in tangent space)
		const float3x3 tangentMatrix    = transpose(invTangentMatrix);
		const float2   textureOffsetDir = mul(cameraModelDir, tangentMatrix).xy;

		// Offset UVs in that direction to account for depth (using height map and some geometry)
		const float texDepth = 0.06f * (DisplacementMap.Sample(TexSampler, UV).r - 0.5f);
		UV += texDepth * textureOffsetDir;
	}

	// Extract normal from map and shift to -1 to 1 range
	float3 textureNormal = 2.0f * NormalMap.Sample(TexSampler, UV).rgb - 1.0f;
	textureNormal.y      = -textureNormal.y;

	// Convert normal from tangent space to world space
	return normalize(mul(mul(textureNormal, invTangentMatrix), (float3x3)gWorldMatrix));
}



float2 ParallaxMapping(float2 UV, float3 v)
{
    //------------------------------
    // Parallax offset mapping
    
    //float depth = gParallaxDepth * (DisplacementMap.Sample(TexSampler, UV).r - 0.5f);
    //return UV + depth * v.xy / v.z; // Remove the / v.z to get parallax offset mapping with limiting
    
    //------------------------------
    // Common linear search for parallax occlusion mapping and relief mapping
    
    // Viewing ray descends at angle through the height map layer. Take several samples of the height map along this
    // section to find where the ray intersects the texture surface. The ray starts at the top of the layer, above the
    // surface, step it along by increments until the ray goes below the surface. This initial linear search to finds
    // the rough intersection, the last two points (one above, one below the surface) are then used to refine the search
    
    // Determine number of samples based on viewing angle. A more shallow angle needs more samples
    const float minSamples = gParallaxMinSample;
    const float maxSamples = gParallaxMaxSample;
    const float numSamples = lerp(maxSamples, minSamples, abs(v.z)); // The view vector is in tangent space, so its z value indicates
                                                               // how much it is pointing directly away from the polygon
    
    // For each step along the ray direction, find the amount to move the UVs and the amount to descend in the height layer
    float rayHeight = 1.0f; // Current height of ray, 0->1 in the height map layer. Start at the top of the layer
    float heightStep = 1.0f / numSamples; // Amount the ray descends for each step
    float2 uvStep = (gParallaxDepth * v.xy / v.z) / numSamples; // Ray UV offset for each step. Can also remove the / v.z here
                                                               // to add limiting, which will reduce artefacts at glancing angles
                                                               // but will also reduce the depth at those angles
    
    // Sample height map at intial UVs (top of layer)
    float surfaceHeight = DisplacementMap.Sample(TexSampler, UV).r;
    float prevSurfaceHeight = surfaceHeight;
    
    // Technical point: when sampling a texture DirectX needs the rate of change of the U and V coordinates (called the x and y
    // gradients). This is used to select a mip-map - if the UVs are changing a lot between each pixel then the texture must be
    // far away and so a small mip-map is chosen. However, you cannot use the gradient values in a loop (unless it can be unrolled).
    // So normal texture sampling often cannot be used in a loop. However we can fetch the gradient values before the loop (these
    // two lines) then use the SampleGrad function, where we can pass them as parameters.
    const float2 dx = ddx(UV);
    const float2 dy = ddy(UV);
    
    // While ray is above the surface
    while (rayHeight > surfaceHeight)
    {
        // Make short step along ray - move UVs and descend ray height
        rayHeight -= heightStep;
        UV -= uvStep;

        // Sample height map again
        prevSurfaceHeight = surfaceHeight;
        surfaceHeight = NormalMap.SampleGrad(TexSampler, UV, dx, dy).r;
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
        surfaceHeight = DisplacementMap.SampleGrad(TexSampler, UV, dx, dy).r;
    }

    return UV;
}


float4 PSMain(NormalMappingPixelShaderInput input) : SV_TARGET
{
  ////************************
	//// Normal Map Extraction
	////************************

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
    float3 worldNormal = normalize(input.worldNormal);
    float3 worldTangent = normalize(input.worldTangent);
    
    // Calculate the bitangent with the cross product of the world normal and the world tangent
    float3 worldBitangent = normalize(cross(input.worldNormal, input.worldTangent));
    
	//****| INFO |**********************************************************************************//
	// The following few lines are the parallax mapping. Converts the camera direction into model
	// space and adjusts the UVs based on that and the bump depth of the texel we are looking at
	// Although short, this code involves some intricate matrix work / space transformations
	//**********************************************************************************************//

	// Get normalised vector to camera for parallax mapping and specular equation (this vector was calculated later in previous shaders)
    const float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);
    
	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate
    const float3x3 tangentMatrix = float3x3(worldTangent, worldBitangent, worldNormal);
	
    // Offset UVs due to parallax mapping, requires view vector (parallax mapping operates in tangent space, so transform vector to tangent space)
    const float3 tangentSpaceV = mul(cameraDirection, transpose(tangentMatrix));
    
    // Calculate the offset uv coordinate for the lighting calculations
    const float2 offsetTexCoord = ParallaxMapping(input.uv, tangentSpaceV);
    
	//*******************************************

	//****| INFO |**********************************************************************************//
	// The above chunk of code is used only to calculate "offsetTexCoord", which is the offset in 
	// which part of the texture we see at this pixel due to it being bumpy. The remaining code is 
	// exactly the same as normal mapping, but uses offsetTexCoord instead of the usual input.uv
	//**********************************************************************************************//

	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
    
    // Get normal from normal map, convert from tangent space to world space
    float3 tangentSpaceN = normalize(NormalMap.Sample(TexSampler, offsetTexCoord).xyz * 2.0f - 1.0f);
    tangentSpaceN.y = -tangentSpaceN.y; // All the normal maps have Y up, but to make a LHS with Z outwards and X rightwards, then Y should be down
    const float3 textureNormal = mul(tangentSpaceN, tangentMatrix);
    
    
	///////////////////////
    // Texture Sampling
    
    //Sample the albedo
    const float3 albedo = AlbedoMap.Sample(TexSampler, offsetTexCoord).rgb;
    
    // Check for the opacity 
    const float opacity = AlbedoMap.Sample(TexSampler, offsetTexCoord).a;
    if (!opacity)
        discard;
    
    
    // Sample roughness map
    float roughness = gRoughness;
    if (gHasRoughnessMap != 0.0f)
    {
        roughness = RoughnessMap.Sample(TexSampler, offsetTexCoord).r * gRoughness;
    }
    
    // Sample ambient occlusion map
    float ao = 1.0f;
    if (gHasAoMap != 0.0f)
    {
        ao = AoMap.Sample(TexSampler, offsetTexCoord).r;
    }
    
    float metalness = gMetalness;
    if (gHasMetallnessMap != 0.0f)
    {
        metalness = MetalnessMap.Sample(TexSampler, offsetTexCoord).r * gMetalness;
    }
    
	///////////////////////
    // Global illumination
    const float nDotV = max(dot(textureNormal, cameraDirection), 0.001f);

    // Select specular color based on metalness
    const float3 specularColour = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);

    // Reflection vector for sampling the cubemap for specular reflections
    const float3 r = reflect(-cameraDirection, textureNormal);

    // Sample environment cubemap, use small mipmap for diffuse, use mipmap based on roughness for specular
    const float3 diffuseIBL = IBLMap.SampleLevel(TexSampler, r, 1.0f).rgb * 2.0f; // This approximation gives somewhat weak diffuse, so scale by 2
    const float roughnessMip = 8 * log2(gRoughness + 1.0f); // Heuristic to convert roughness to mip-map. Rougher surfaces will use smaller (blurrier) mip-maps
    const float3 specularIBL = IBLMap.SampleLevel(TexSampler, r, roughnessMip).rgb;

    // Fresnel for IBL: when surface is at more of a glancing angle reflection of the scene increases
    const float3 F_IBL = specularColour + (1.0f - specularColour) * pow(max(1.0f - nDotV, 0.0f), 5.0f);
    
    // Overall global illumination - rough approximation
    float3 resDiffuse =  ao * (albedo * diffuseIBL) + (1.0f - roughness) * F_IBL * specularIBL;
    
	///////////////////////
	// Calculate lighting
	
	//// Lights ////

    const float3 resSpecular = specularColour;
	
    for (int i = 0; i < gNumLights && gLights[i].enabled; ++i)
    {
        resDiffuse += CalculateLight(gLights[i].position, gLights[i].intensity, gLights[i].colour, resDiffuse, resSpecular, textureNormal, cameraDirection, input.worldPosition, roughness, albedo);
    }
     
    
	//for each spot light
    for (int j = 0; j < gNumSpotLights && gSpotLights[j].enabled; ++j)
    {
        const float3 lightDir = normalize(gSpotLights[j].pos - input.worldPosition);

    	//if the pixel is in the light cone
        if (dot(-gSpotLights[j].facing, lightDir) > gSpotLights[j].cosHalfAngle)
        {
    		// Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
			// pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
			// These are the same as the view / projection matrix multiplies in a vertex shader (can improve performance by putting these lines in vertex shader)
            const float4 viewPosition = mul(gSpotLights[j].viewMatrix, float4(input.worldPosition, 1.0f));
            const float4 projection = mul(gSpotLights[j].projMatrix, viewPosition);

			// Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
			// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
            float2 shadowMapUV = 0.5f * projection.xy / projection.w + float2(0.5f, 0.5f);
            shadowMapUV.y = 1.0f - shadowMapUV.y; // Check if pixel is within light cone
            
        	resDiffuse += CalculateLight(gSpotLights[j].pos, gSpotLights[j].intensity, gSpotLights[j].colour, resDiffuse, resSpecular, textureNormal, cameraDirection, input.worldPosition, roughness, albedo);
        }
    }


    for (int k = 0; k < gNumDirLights && gDirLights[k].enabled; ++k)
    {
        const float3 lightDir = normalize(gDirLights[k].facing - input.worldPosition);
        
    	//Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
		//pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
		//These are the same as the view / projection matrix multiplies in a vertex shader (can improve performance by putting these lines in vertex shader)
        const float4 viewPosition = mul(gDirLights[k].viewMatrix, float4(input.worldPosition, 1.0f));
        const float4 projection = mul(gDirLights[k].projMatrix, viewPosition);

		//Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
	    //Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
        float2 shadowMapUV = 0.5f * projection.xy / projection.w + float2(0.5f, 0.5f);
        shadowMapUV.y = 1.0f - shadowMapUV.y; // Check if pixel is within light cone
        
        // Bias slope
        float bias = gDepthAdjust * tan(acos(dot(textureNormal, lightDir)));
        bias = clamp(bias, 0, 0.01);
        
        // Lighting calculations
        
            const float li = gDirLights[k].intensity;
            const float3 l = gDirLights[k].facing;
            const float3 lc = gDirLights[k].colour;

            // Halfway vector (normal halfway between view and light vector)
            const float3 h = normalize(l + cameraDirection);

            // Various dot products used throughout
            const float nDotL = max(dot(textureNormal, l), 0.001f);
            const float nDotH = max(dot(textureNormal, h), 0.001f);
            const float vDotH = max(dot(cameraDirection, h), 0.001f);

            // Lambert diffuse
            const float3 lambert = albedo / PI;

            // Microfacet specular - fresnel term
            const float3 F = resSpecular + (1 - resSpecular) * pow(max(1.0f - vDotH, 0.0f), 5.0f);

            // Microfacet specular - normal distribution term
            const float alpha = max(roughness * roughness, 2.0e-3f); // Dividing by alpha in the dn term so don't allow it to reach 0
            const float alpha2 = alpha * alpha;
            const float nDotH2 = nDotH * nDotH;
            const float dn = nDotH2 * (alpha2 - 1) + 1;
            const float D = alpha2 / (PI * dn * dn);

            // Microfacet specular - geometry term
            float r = (roughness + 1);
            r = r * r / 8;
            const float gV = nDotV / (nDotV * (1 - r) + r);
            const float gL = nDotL / (nDotL * (1 - r) + r);
            const float G = gV * gL;

        // Full brdf, diffuse + specular
            const float3 brdf = lambert + F * G * D / (4 * nDotL * nDotV);

        // Accumulate punctual light equation for this light
            const float3 diffuse = PI * li * lc * brdf * nDotL;
        
        
        // Multiply for the pcf value 
            resDiffuse += diffuse;
        
    }
    
    //for each point light
    for (int l = 0; l < gNumPointLights && gPointLights[l].enabled; ++l)
    {
        const float3 lightDir = normalize(gPointLights[l].pos - input.worldPosition);
        
        for (int face = 0; face < 6; ++face)
        {
    	    // Using the world position of the current pixel and the matrices of the light (as a camera), find the 2D position of the
		    // pixel *as seen from the light*. Will use this to find which part of the shadow map to look at.
		    // These are the same as the view / projection matrix multiplies in a vertex shader (can improve performance by putting these lines in vertex shader)
            const float4 viewPosition = mul(gPointLights[l].viewMatrices[face], float4(input.worldPosition, 1.0f));
            const float4 projection = mul(gPointLights[l].projMatrix, viewPosition);

		    // Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
		    // Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
            float2 shadowMapUV = 0.5f * projection.xy / projection.w + float2(0.5f, 0.5f);
            shadowMapUV.y = 1.0f - shadowMapUV.y; // Check if pixel is within light cone
            
            //Bias slope
            float bias = gDepthAdjust * tan(acos(dot(textureNormal, lightDir)));
            bias = clamp(bias, 0, 0.01);
            
		    // Get depth of this pixel if it were visible from the light (another advanced projection step)
            const float depthFromLight = projection.z / projection.w - bias; //*** Adjustment so polygons don't shadow themselves

		    // Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		    // to the light than this pixel - so the pixel gets no effect from this light
            if (depthFromLight > 0)
            {
                // Calculate lighting
                resDiffuse += CalculateLight(gPointLights[l].pos, gPointLights[l].intensity, gPointLights[l].colour, resDiffuse, resSpecular, textureNormal, cameraDirection, input.worldPosition, roughness, albedo);
            }
        }
    }

    return float4(resDiffuse, 1);
}
