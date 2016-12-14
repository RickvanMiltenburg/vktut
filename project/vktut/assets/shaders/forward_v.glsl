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

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

////////////////////////////////////////
// Input uniforms

layout(push_constant)
uniform CB
{
	layout(offset = 0)  mat4 MVP;
	layout(offset = 64) mat4 MV;
} cb;

////////////////////////////////////////
// Input attributes

layout(location = 0)
in vec3 inPosition;
layout(location = 1)
in vec2 inTexcoord;
layout(location = 2)
in vec3 inNormal;

////////////////////////////////////////
// Output attributes

layout(location = 0)
out vec2 outTexcoord;
layout(location = 1)
out vec3 outWorldPosition;
layout(location = 2)
out vec3 outWorldNormal;

out gl_PerVertex
{
	vec4 gl_Position;
};

////////////////////////////////////////
// Entry point

#if 1
void main()
{
	vec4 worldPosition       =          vec4 ( inPosition, 1.0 );
	vec4 worldNormal         =          vec4 ( inNormal,   0.0 );
	vec4 transformedPosition = cb.MVP * vec4 ( inPosition, 1.0 );
	

	gl_Position      = transformedPosition;
	outWorldPosition = worldPosition.xyz;
	outWorldNormal   = worldNormal.xyz;
	outTexcoord      = inTexcoord;
}
#else
vec2 positions[3] = vec2[] (
	vec2(-1.0, -1.0),
	vec2( 1.0, -1.0),
	vec2(-1.0,  1.0)
);

vec2 texcoord[3] = vec2[] (
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(0.0, 1.0)
);

void main()
{
	gl_Position = vec4 ( positions[gl_VertexIndex%3], 0.0, 1.0 );
	outDebugPosition = positions[gl_VertexIndex%3];
	outTexcoord = texcoord[gl_VertexIndex%3];
}
#endif