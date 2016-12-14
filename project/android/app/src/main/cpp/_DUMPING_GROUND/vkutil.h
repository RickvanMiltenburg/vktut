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

const char* vkutil_VkResult_to_string                   ( VkResult in );
const char* vkutil_VkDebugReportObjectTypeEXT_to_string ( VkDebugReportObjectTypeEXT in );
const char* vkutil_VkDebugReportFlagBitsEXT_to_string   ( VkDebugReportFlagBitsEXT in );

#ifdef __cplusplus
};	// Round off the cross-compat block
#endif