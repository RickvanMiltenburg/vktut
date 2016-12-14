/*
  Copyright (c) 2016 Rick van Miltenburg, NHTV Breda University of Applied Sciences

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
  associated documentation files (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge, publish, distribute,
  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

////////////////////////////////////////
// Input uniforms

layout(input_attachment_index = 0) uniform subpassInput gBuffer[3];
//layout(input_attachment_index = 1) uniform subpassInput gBuffer2;
//layout(input_attachment_index = 2) uniform subpassInput gBuffer3;
layout(binding = 3) uniform sampler2DArrayShadow shadowTex;

#define MAX_LIGHTS 16
struct Spotlight
{
	mat4 shadowVp;
	vec3 position;  float innerDot;
	vec3 direction; float outerDot;
	vec3 color;
	vec3 attenuation;
};

layout(std140, binding = 4) uniform LightingCB
{
	vec3 cameraPosition; uint lightCount;
	Spotlight lights[MAX_LIGHTS];
} lighting;

////////////////////////////////////////
// Output parameters (for multiple output attachments, add new output with location = N)

layout(location = 0) out vec4 fragColor;

////////////////////////////////////////
// Entry point

vec3 EvaluateSpotLight ( uint lightIndex, vec3 worldPosition, vec3 worldNormal )
{
	vec4 baseProj = lighting.lights[lightIndex].shadowVp * vec4 ( worldPosition, 1.0f );
	baseProj.xyz /= baseProj.w;
	baseProj.xy   = baseProj.xy * 0.5f + 0.5f;

	vec3 deltaPos = worldPosition - lighting.lights[lightIndex].position;
	vec3 dir      = normalize ( deltaPos );

	float angle = dot ( dir, lighting.lights[lightIndex].direction );
	float coneFactor = clamp (
		(angle - lighting.lights[lightIndex].outerDot)
		/ (lighting.lights[lightIndex].innerDot - lighting.lights[lightIndex].outerDot),
		0.0f, 1.0f
	);

	float NdotL = clamp ( dot ( -worldNormal, dir ), 0.0f, 1.0f );

	float shadowFactor = 0.0f;
	if ( baseProj.w > 0.0 )
	{
		vec4 texcoord;
		texcoord.xy  = baseProj.xy;
		texcoord.w   = baseProj.z;
		texcoord.z   = float ( lightIndex );
		shadowFactor = texture ( shadowTex, texcoord );
	}

	return (NdotL * coneFactor * shadowFactor).xxx;
}

void main()
{
	vec3 worldPosition = subpassLoad ( gBuffer[0] ).xyz;
	vec3 worldNormal   = subpassLoad ( gBuffer[1] ).xyz;
	vec3 diffuse       = subpassLoad ( gBuffer[2] ).xyz;

	vec3 accum = vec3 ( 0.0f, 0.0f, 0.0f );
	for ( uint i = 0; i < lighting.lightCount && i < MAX_LIGHTS; i++ )
		accum += EvaluateSpotLight ( i, worldPosition, worldNormal );
	
	fragColor = vec4 ( diffuse * accum, 1.0 );
}
