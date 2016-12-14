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
// Global defines

// Toggles Vulkan debug layers and debug output extension when supported by the platform
// This will incur serious performance penalties, but allow for a dialog box to pop up when
// an issue has occurred.
#define VK_ENABLE_DEBUG 0
#define TEMP_TIMESTAMP_STUFF_RENDERDOC_CRASHES_IDK_MAN 0

#define STATIC_ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#if defined ( _WIN32 )
#define alloca _alloca
#elif defined ( __ANDROID__ )
#include <alloca.h>
#endif

typedef enum
{
	WINDOW_EVENT_NONE,
	WINDOW_EVENT_CLOSE,
} window_event_type_t;

////////////////////////////////////////
// Platform-specific structures

typedef struct file_s { void* data; size_t sizeInBytes; } file_t;
typedef struct log_file_s { void* platform; } log_file_t;
typedef struct window_s { void* platform; VkSurfaceKHR surface; } window_t;
typedef struct profiler_s { void* platform; } profiler_t;

typedef struct instance_s
{
	VkInstance instance;
	VkDebugReportCallbackEXT debugCallback;
} instance_t;

typedef struct device_s
{
	VkPhysicalDevice physical;
	VkDevice         device;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memoryProperties;
} device_t;

typedef struct queue_s
{
	VkQueue queue;
	uint32_t familyIndex;
} queue_t;

typedef struct swapchain_s
{
	VkSwapchainKHR     swapchain;
	VkImage*           images;
	VkImageView*       imageViews;
	uint32_t           imageCount;
	VkSurfaceFormatKHR surfaceFormat;
} swapchain_t;

////////////////////////////////////////
// 

typedef struct queue_create_info_s
{
	VkQueueFlagBits queueFlags;
	uint32_t presentWindowCount;
	uint32_t* presentWindowIndices;
} queue_create_info_t;

////////////////////////////////////////
// 

typedef struct app_s app_t;
typedef uint64_t timestamp_t;

////////////////////////////////////////
// Functions to be implemented by the demo

int32_t app_init   ( app_t** outApp, void* userdata );
int32_t app_free   ( app_t* app );
int32_t app_resize ( app_t* app, uint32_t windowWidth, uint32_t windowHeight );
int32_t app_render ( app_t* app, double dt );

////////////////////////////////////////
// 

int32_t vkbase_init_instance (
	instance_t* instance
);
int32_t vkbase_init_device (
	device_t* outDevice, instance_t* instance,
	uint32_t windowCount, window_t** windows,
	uint32_t queueCount, queue_create_info_t* queueCreateInfos,
	queue_t* outQueues
);
int32_t vkbase_init_swapchain (
	swapchain_t* outSwapchain, instance_t* instance, device_t* device, window_t* window,
	swapchain_t* oldSwapchain
);

int32_t vkbase_destroy_swapchain (
	device_t* device, swapchain_t* swapchain
);
int32_t vkbase_destroy_device (
	device_t* device
);
int32_t vkbase_destroy_instance (
	instance_t* instance
);

//int32_t vkbase_profiler_init_cpu (
//	profiler_t* outProfiler, uint32_t markerCount, const char** markerNames
//);
//int32_t vkbase_profiler_init_gpu (
//	profiler_t* outProfiler, uint32_t markerCount, const char** markerNames
//);
//int32_t vkbase_profiler_

////////////////////////////////////////
// Platform-specific utility functions

int32_t platform_log_warning           ( const char* warningFormat, ... );
int32_t platform_throw_error           ( int32_t code, const char* errorFormat, ... );
int32_t platform_get_timestamp         ( timestamp_t* outTimestamp );
int32_t platform_get_timestamp_freq    ( timestamp_t* outTimestampFreq );

////////////////////////////////////////
// Platform-specific IO functions

int32_t platform_file_load       ( file_t* outFile, const char* file );
int32_t platform_file_close      ( file_t* file );
int32_t platform_log_file_create ( log_file_t* outFile, const char* name );
int32_t platform_log_file_print  ( log_file_t* file, const char* format, ... );
int32_t platform_log_file_write  ( log_file_t* file, const void* data, uint64_t size );
int32_t platform_log_file_close  ( log_file_t* file );

////////////////////////////////////////
// Platform-specific window management functions

int32_t platform_window_create       ( window_t* outWindow, void* userdata );
int32_t platform_window_get_size     ( window_t* window, uint32_t* outWidth, uint32_t* outHeight );

////////////////////////////////////////
// Platform-specific Vulkan functions

int32_t platform_vulkan_get_required_instance_layers (
	VkLayerProperties* layers, uint32_t layerCount, const char*** outLayers, uint32_t* outCount
);
int32_t platform_vulkan_get_required_instance_extensions (
	const char*** outExtensions, uint32_t* outCount, uint32_t* outDebugSupported
);
int32_t platform_vulkan_get_required_device_extensions (
	const char*** outExtensions, uint32_t* outCount
);
int32_t platform_vulkan_create_surface (
	VkSurfaceKHR* outSurface, VkInstance instance, window_t* platform
);

#ifdef __cplusplus
};	// Round off the cross-compat block
#endif