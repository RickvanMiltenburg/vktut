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
#define VK_ENABLE_DEBUG 1

////////////////////////////////////////
// Platform-specific structures

typedef struct app_window_s app_window_t;
typedef struct file_s { void* data; size_t sizeInBytes; } file_t;

////////////////////////////////////////
// Function to be called by the application entry point to start the app. Not my favorite!

int32_t app_run ( void* userdata );

////////////////////////////////////////
// Platform-specific utility functions

int32_t platform_init        ( void* userdata );
int32_t platform_throw_error ( int32_t code, const char* errorFormat, ... );

////////////////////////////////////////
// Platform-specific IO functions

int32_t platform_file_load  ( file_t* outFile, const char* file );
int32_t platform_file_close ( file_t* file );

////////////////////////////////////////
// Platform-specific window management functions

int32_t platform_window_create   ( app_window_t** outWindow, void* userdata );
int32_t platform_window_get_size ( app_window_t* window, uint32_t* outWidth, uint32_t* outHeight );

////////////////////////////////////////
// Platform-specific Vulkan functions

int32_t platform_vulkan_get_required_instance_layers (
	const char*** outLayers, uint32_t* outCount
);
int32_t platform_vulkan_get_required_instance_extensions (
	const char*** outExtensions, uint32_t* outCount, uint32_t* outDebugSupported
);
int32_t platform_vulkan_get_required_device_extensions (
	const char*** outExtensions, uint32_t* outCount
);
int32_t platform_vulkan_create_surface (
	VkSurfaceKHR* outSurface, VkInstance instance, app_window_t* platform
);

#ifdef __cplusplus
};	// Round off the cross-compat block
#endif