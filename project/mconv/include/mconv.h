#pragma once

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

#ifdef __cplusplus
extern "C" {	// Expose functions with C linkage for cross-compat with C and C++
#endif

#include <stdint.h>

#define BOBJ_VERSION 0x100

#define MAKE_FOURCC(a,b,c,d) ((a) | (b<<8) | (c<<16) | (d<<24))
#define BOBJ_FILE_MAGIC         MAKE_FOURCC('B','O','B','J')
#define BOBJ_OBJECT_MAGIC       MAKE_FOURCC('O','B','J',' ')
#define BOBJ_TEXTURE_MAGIC      MAKE_FOURCC('T','X','T','R')
#define BOBJ_TEXTURE_DATA_MAGIC MAKE_FOURCC('T','X','D','T')
#define BOBJ_VERTEX_DATA_MAGIC  MAKE_FOURCC('V','X','D','T')
#define BOBJ_INDEX_DATA_MAGIC   MAKE_FOURCC('I','X','D','T')

typedef struct
{
	uint32_t magic;
	uint32_t version;

	uint32_t objectsStart;
	uint32_t texturesStart;
	uint32_t vertexStart;
	uint32_t indexStart;
	uint32_t texdataStart;

	uint32_t vertexCount;
	uint32_t indexCount;
	uint32_t objCount;
	uint32_t texCount;
} bobj_file_header;

typedef struct
{
	uint32_t magic;
	uint32_t indexOffset;
	uint32_t indexCount;
	uint32_t textureIndex;
	float aabbMin[3];
	float aabbMax[3];
} bobj_object_header;

typedef struct
{
	float position[3];
	float texcoord[2];
	float normal[3];
} bobj_vert;

typedef uint32_t bobj_index;

typedef struct
{
	uint32_t magic;
	uint32_t width;
	uint32_t height;
	uint32_t offset;
} bobj_texture_header;

#ifdef __cplusplus
};	// Round off the cross-compat block
#endif