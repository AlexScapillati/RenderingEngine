//--------------------------------------------------------------------------------------
// PBR Pixel Shader
//--------------------------------------------------------------------------------------

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory (the words map and texture are used interchangeably)

Texture2D AlbedoMap : register(t0); // Diffuse map (main colour) in rgb and specular map (shininess level) in alpha - C++ must load this into slot 0
Texture2D AOMap : register(t1); // Ambient Occlusion Map, precalculated by the artist, holds the amount of occlusion for each texel
Texture2D DisplacementMap : register(t2); // Displacement map / Height map, different from the normal map.
Texture2D NormalHeightMap : register(t3); // Normal map in rgb, holds the normals for every texel 
Texture2D RoughnessMap : register(t4); // Roughness map, holds how the material is shiny or rough, this will modify the specular light
Texture2D MetalnessMap : register(t5); // Metalness Map, tipically holds 0 or 1, rarely inbetween.
                                    
TextureCube IBLMap : register(t6);

SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic
                                             
Texture2D ShadowMaps[10] : register(t7); // An array of shadow maps
SamplerState PointClamp : register(s1); // Aampler for the shadow maps 

static const float PI = 3.14159265359f;

float3 CalculateLight(const float3 lightPos, const float lightIntensity, const float3 colour, float3 diffuse, float3 specular, float3 n, float3 v, float3 worldPos, float roughness, float3 albedo)
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

float PCF(Texture2D shadowMap, float2 shadowMapUV)
{
    //get the shadow map size
    float2 size = 0.0f;
    shadowMap.GetDimensions(size.x, size.y);
    
    //store the texel size
    const float texelSize = 1.0f / size.x;
    
    // Get the derivates
	const float2 dx = ddx(shadowMapUV);
	const float2 dy = ddy(shadowMapUV);
    
    float sum = 0.0f;
    float x, y;

    for (y = -1.5f; y <= 1.5f; y += 1.0f)
        for (x = -1.5f; x <= 1.5f; x += 1.0f)
            sum += shadowMap.SampleGrad(PointClamp, shadowMapUV + float2(x, y) * texelSize, dx, dy).r;

    return sum / 16.0f;
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
        surfaceHeight = NormalHeightMap.SampleGrad(TexSampler, UV, dx, dy).r;
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


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(NormalMappingPixelShaderInput input) : SV_Target
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
    float3 tangentSpaceN       = normalize(NormalHeightMap.Sample(TexSampler, offsetTexCoord).xyz * 2.0f - 1.0f);
    tangentSpaceN.y            = -tangentSpaceN.y; // All the normal maps have Y up, but to make a LHS with Z outwards and X rightwards, then Y should be down
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
        ao = AOMap.Sample(TexSampler, offsetTexCoord).r;
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
	const float3 diffuseIBL   = IBLMap.SampleLevel(TexSampler, r, 1.0f).rgb * 2.0f; // This approximation gives somewhat weak diffuse, so scale by 2
	const float  roughnessMip = 8 * log2(gRoughness + 1.0f);                        // Heuristic to convert roughness to mip-map. Rougher surfaces will use smaller (blurrier) mip-maps
	const float3 specularIBL  = IBLMap.SampleLevel(TexSampler, r, roughnessMip).rgb;

    // Fresnel for IBL: when surface is at more of a glancing angle reflection of the scene increases
	const float3 F_IBL = specularColour + (1.0f - specularColour) * pow(max(1.0f - nDotV, 0.0f), 5.0f);
    
    // Overall global illumination - rough approximation
    float3 resDiffuse = ao * (albedo * diffuseIBL) + (1.0f - roughness) * F_IBL * specularIBL;
    
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

            // Bias slope
            float bias = gDepthAdjust * tan(acos(dot(input.worldNormal, lightDir)));
            bias = clamp(bias, 0, 0.01);
            
			// Get depth of this pixel if it were visible from the light (another advanced projection step)
            const float depthFromLight = projection.z / projection.w - bias; //*** Adjustment so polygons don't shadow themselves
                        
			// Calcluate pcf value   
			const float PCFValue = PCF(ShadowMaps[j], shadowMapUV);
            
            // Calculate lighting based on the pcf value, 
            //if it is 0 or less there is no point to calculate it since we are in complete shadow
            if (PCFValue > 0)
            {
                resDiffuse += CalculateLight(gSpotLights[j].pos, gSpotLights[j].intensity, gSpotLights[j].colour, resDiffuse, resSpecular, textureNormal, cameraDirection, input.worldPosition, roughness, albedo) * PCFValue;
            }
        }
    }
    
    [fastopt]
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
        
		//Get depth of this pixel if it were visible from the light (another advanced projection step)
        const float depthFromLight = projection.z / projection.w - bias; //*** Adjustment so polygons don't shadow themselves
		
        // Calculate pcf value
		const float PCFValue = PCF(ShadowMaps[k], shadowMapUV);
        
        // Lighting calculations
        
        if (PCFValue > 0)
        {
			const float  li = gDirLights[k].intensity;
			const float3 l  = gDirLights[k].facing;
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
			const float alpha  = max(roughness * roughness, 2.0e-3f); // Dividing by alpha in the dn term so don't allow it to reach 0
			const float alpha2 = alpha * alpha;
			const float nDotH2 = nDotH * nDotH;
			const float dn     = nDotH2 * (alpha2 - 1) + 1;
			const float D      = alpha2 / (PI * dn * dn);

            // Microfacet specular - geometry term
            float k        = (roughness + 1);
            k              = k * k / 8;
			const float gV = nDotV / (nDotV * (1 - k) + k);
			const float gL = nDotL / (nDotL * (1 - k) + k);
			const float G  = gV * gL;

        // Full brdf, diffuse + specular
			const float3 brdf = lambert + F * G * D / (4 * nDotL * nDotV);

        // Accumulate punctual light equation for this light
			const float3 diffuse = PI * li * lc * brdf * nDotL;
        
        
        // Multiply for the pcf value 
            resDiffuse += diffuse * PCFValue;
        }
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

			const float depth = ShadowMaps[l /*+ gNumSpotLights + gNumDirLights */ + face].Sample(PointClamp, shadowMapUV).r;
            
		    // Compare pixel depth from light with depth held in shadow map of the light. If shadow map depth is less than something is nearer
		    // to the light than this pixel - so the pixel gets no effect from this light
            if (depthFromLight > 0 && depthFromLight < depth)
            {
                // Calcluate pcf value   
				const float PCFvalue = PCF(ShadowMaps[l /*+ gNumSpotLights + gNumDirLights */ + face], shadowMapUV);
                // Calculate lighting
                resDiffuse += CalculateLight(gPointLights[l].pos, gPointLights[l].intensity, gPointLights[l].colour, resDiffuse, resSpecular, textureNormal, cameraDirection, input.worldPosition, roughness, albedo) * PCFvalue;
            }
        }
    }
    
    
    return float4(resDiffuse, 1); // Always use 1.0f for alpha - no alpha blending in this lab
}

