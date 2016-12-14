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

////////////////////////////////////////
// Vulkan include

#if !defined ( __ANDROID__ ) || __ANDROID_API__ >= 24
#include <vulkan\vulkan.h>
#else
// Android version 24 and on officially supports Vulkan, while version 23 does not
// Some vendors do ship with versions of libVulkan however, so we can use Vulkan on these
// platforms when loading the functions dynamically.
#include <vulkan_wrapper.h>
#endif
	
////////////////////////////////////////
// 

#ifndef VKUTIL_BOBJ_FLIP_TEXCOORD_V
// Most models have been created with the V texture coordinate being "up"
// In Vulkan, however, this axis is facing _down_
#define VKUTIL_BOBJ_FLIP_TEXCOORD_V 1
#endif

////////////////////////////////////////
// 

typedef enum
{
	VKUTIL_IMAGE_MIPMAP_NONE,
	VKUTIL_IMAGE_MIPMAP_GENERATE,
} vkutil_image_mipmap_mode_t;

typedef struct
{
	VkImageView* outImageView;
	VkImageViewCreateInfo* createInfo;
} vkutil_image_view_desc;

typedef struct
{
	VkImage* outImage;
	VkImageCreateInfo* createInfo;
	uint32_t imageViewCount;
	vkutil_image_view_desc* imageViews;

	void* initialData;
	vkutil_image_mipmap_mode_t mipMode;
	VkAccessFlagBits accessMask;
	VkImageAspectFlagBits aspectMask;
} vkutil_image_desc;

typedef struct object_s
{
	uint32_t indexStart, indexCount;
	uint32_t textureIndex;
	float aabbMin[3];
	float aabbMax[3];
} vkutil_object_t;

typedef struct
{
	uint32_t objectCount;
	uint32_t textureCount;

	vkutil_object_t* objects;
	VkImage* images;
	VkImageView* imageViews;

	VkBuffer vertexBuffer, indexBuffer;
	VkDeviceMemory vbIbMemory, imageMemory;

	void* userdata;
} vkutil_model_t;

////////////////////////////////////////
// 

const char* vkutil_VkResult_to_string                   ( VkResult in );
const char* vkutil_VkDebugReportObjectTypeEXT_to_string ( VkDebugReportObjectTypeEXT in );
const char* vkutil_VkDebugReportFlagBitsEXT_to_string   ( VkDebugReportFlagBitsEXT in );

////////////////////////////////////////
// 

int32_t vkutil_multi_alloc_helper (
	VkDevice device, VkPhysicalDeviceMemoryProperties* memoryProperties, uint32_t requiredProperties,
	uint32_t memoryRequirementCount, VkMemoryRequirements* memoryRequirements,
	VkDeviceMemory* outMemory, VkDeviceSize* outMemorySize, VkDeviceSize* outMemoryOffsets
);

int32_t vkutil_create_images_helper (
	VkDevice device, VkCommandBuffer stagingCommandBuffer, VkQueue stagingQueue,
	VkPhysicalDeviceMemoryProperties* memoryProperties,
	uint32_t imageCount, vkutil_image_desc* imageDescs, VkDeviceMemory* outMemory
);

int32_t vkutil_load_bobj (
	vkutil_model_t* model,
	VkDevice device, const void* bobjData, uint64_t bobjLen,
	VkPhysicalDeviceMemoryProperties* memoryProperties,
	VkQueue stagingQueue, VkCommandBuffer stagingCommandBuffer
);

int32_t vkutil_destroy_bobj (
	vkutil_model_t* model, VkDevice device
);

////////////////////////////////////////
// 

#ifdef __cplusplus
};	// Round off the cross-compat block
#endif