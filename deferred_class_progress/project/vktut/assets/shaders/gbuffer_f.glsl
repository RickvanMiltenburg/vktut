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

layout(binding = 0) uniform texture2D tex;
layout(binding = 1) uniform sampler samp;

////////////////////////////////////////
// Input Vertex Shader parameters

layout(location = 0) in vec2 texcoord;
layout(location = 1) in vec3 worldPosition;
layout(location = 2) in vec3 worldNormal;

////////////////////////////////////////
// Output parameters (for multiple output attachments, add new output with location = N)

layout(location = 0) out vec4 gBuffer1;
layout(location = 1) out vec4 gBuffer2;
layout(location = 2) out vec4 gBuffer3;

////////////////////////////////////////
// Entry point

void main()
{
	gBuffer1 = vec4 ( worldPosition, 1.0f );
	gBuffer2 = vec4 ( worldNormal, 1.0f );
	gBuffer3 = texture ( sampler2D ( tex, samp ), texcoord );
}
