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

layout(push_constant, binding = 0)
uniform CB
{
	float aspect;
} cb;

////////////////////////////////////////
// Input attachments

layout(input_attachment_index = 0) uniform subpassInput tex;

////////////////////////////////////////
// Input attributes

layout(location = 0) in vec2 texcoord;

////////////////////////////////////////
// Input attributes

layout(location = 0) out vec4 fragColor;

////////////////////////////////////////
// Entry point

void main()
{
	vec2 relDist = texcoord - vec2 ( 0.5, 0.5 );
	relDist.x   *= max ( 1,   cb.aspect );
	relDist.y   *= max ( 1, 1/cb.aspect );

	float factor = clamp ( 1.3 - length ( relDist ), 0.0, 1.0 );

#if 1
	fragColor = vec4( (factor*factor) * subpassLoad ( tex ).rgb, 1.0f );
#else
	fragColor = vec4( (factor*factor) * vec3 ( 0.5f, 0.75f, 1.0f ), 1.0);
#endif
}