//--------------------------------------------------------------------------------------
// Depth-Only Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader that only outputs the depth of a pixel - used to render the scene from the point
// of view of the lights in preparation for shadow mapping

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


Texture2D Albedo : register(t0);
SamplerState texSampler : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Shader used when rendering the shadow map depths. The pixel shader doesn't really need to calculate a pixel colour
// since we are only writing to the depth buffer.

void main(NormalMappingPixelShaderInput i)
{
    if (Albedo.Sample(texSampler, i.uv).a < 0.1f)
        discard;
}

// If the depth buffer is enabled, this will output depth but no colour (C++ side render target should be nullptr)
