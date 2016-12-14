:: Copyright (c) 2016 Rick van Miltenburg, NHTV Breda University of Applied Sciences
:: 
:: Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
:: associated documentation files (the "Software"), to deal in the Software without restriction,
:: including without limitation the rights to use, copy, modify, merge, publish, distribute,
:: sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
:: furnished to do so, subject to the following conditions:
:: 
:: The above copyright notice and this permission notice shall be included in all copies or
:: substantial portions of the Software.
:: 
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
:: BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
:: NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
:: DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
:: OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

:: This is the boring version of the shader compilation process. While I would personally
:: recommend shader_watchdog.py, this is still a functional alternative.

@ECHO OFF

glslangValidator -V -S vert -o "bin/shaders/shadow_v.spv" "vktut/assets/shaders/shadow_v.glsl"
glslangValidator -V -S vert -o "bin/shaders/forward_v.spv" "vktut/assets/shaders/forward_v.glsl"
glslangValidator -V -S frag -o "bin/shaders/forward_f.spv" "vktut/assets/shaders/forward_f.glsl"
glslangValidator -V -S vert -o "bin/shaders/post_v.spv" "vktut/assets/shaders/post_v.glsl"
glslangValidator -V -S frag -o "bin/shaders/post_f.spv" "vktut/assets/shaders/post_f.glsl"

"tools\mconv.exe" "vktut/assets/models/cube.obj" "bin/assets/models/cube.bobj"
"tools\mconv.exe" "vktut/assets/models/texcube.obj" "bin/assets/models/texcube.bobj"
"tools\mconv.exe" "vktut/assets/models/cylinder.obj" "bin/assets/models/cylinder.bobj"
"tools\mconv.exe" "vktut/assets/models/sponza.obj" "bin/assets/models/sponza.bobj"