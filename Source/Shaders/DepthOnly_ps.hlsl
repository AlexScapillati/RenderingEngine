//--------------------------------------------------------------------------------------
// Depth-Only Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader that only outputs the depth of a pixel - used to render the scene from the point
// of view of the lights in preparation for shadow mapping

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

Texture2D Albedo : register(t0);
SamplerState texSampler : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Shader used when rendering the shadow map depths. The pixel shader doesn't really need to calculate a pixel colour
// since we are only writing to the depth buffer.

void main(LightingPixelShaderInput i)
{
    if (Albedo.Sample(texSampler, i.uv).a < 0.1f)
        discard;
}

// If the depth buffer is enabled, this will output depth but no colour (C++ side render target should be nullptr)
