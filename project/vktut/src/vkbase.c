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

#include "vkbase.h"
#include "vkutil.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "rvm_math.h"
#include "../../mconv/include/mconv.h"

////////////////////////////////////////
// Defines

#define STATIC_ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#if defined ( _WIN32 )
#define alloca _alloca
#elif defined ( __ANDROID__ )
#include <alloca.h>
#endif

////////////////////////////////////////
// Vulkan debug callback, called on errors and warnings

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback (
	VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location,
	int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData
)
{
	if ( msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT )
	{
		platform_throw_error ( msgCode, "[VULKAN] Debug ERROR\n"
								  "Flags: %s (%u)\n"
								  "Object type: %s (%u)\n"
								  "Source object: 0x%X\n"
								  "Location: %llu\n"
								  "Message code: %d\n"
								  "Layer prefix: %s\n"
								  "Message: %s",
						  vkutil_VkDebugReportFlagBitsEXT_to_string ( msgFlags ), msgFlags,
						  vkutil_VkDebugReportObjectTypeEXT_to_string ( objType ), objType,
						  srcObject, location, msgCode, pLayerPrefix, pMsg
		);
	}
	else
	{
		platform_log_warning ( "[VULKAN] Debug callback invoked\n"
								  "\tFlags: %s (%u)\n"
								  "\tObject type: %s (%u)\n"
								  "\tSource object: 0x%X\n"
								  "\tLocation: %llu\n"
								  "\tMessage code: %d\n"
								  "\tLayer prefix: %s\n"
								  "\tMessage: %s\n\n",
						  vkutil_VkDebugReportFlagBitsEXT_to_string ( msgFlags ), msgFlags,
						  vkutil_VkDebugReportObjectTypeEXT_to_string ( objType ), objType,
						  srcObject, location, msgCode, pLayerPrefix, pMsg
		);
	}

	// We did not "handle" the error
	return 0;
}

////////////////////////////////////////
// 

int32_t vkbase_init_instance (
	instance_t* outInstance
)
{
	int result        = 0;
	VkResult vkResult = VK_SUCCESS;

	// Now we could immediately move on to creating the Vulkan instance etc, but before doing so,
	// it is wise to ask the local Vulkan runtime whether or not the layers and extensions we 
	// requested from the platform layer are actually present. vkCreateInstance would fail if they
	// are missing, but would not immediately be able to tell you which layer/extension was missing
	//
	// To do this, we will first check how many layers and extensions are available, allocate
	// enough space to store the data for that many layer- and extension descriptions, and then
	// ask for the actual descriptions themselves.
	//
	// (Note about the "alloca" function: Memory allocated by this "function" is allocated on the
	// _STACK_, and is no longer valid when the function returns)

	VkLayerProperties* availableLayers;
	VkExtensionProperties* availableExtensions;
	uint32_t availableLayerCount = 0, availableExtensionCount = 0;
	vkResult = vkEnumerateInstanceLayerProperties ( &availableLayerCount, NULL );
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkEnumerateInstanceLayerProperties failed (%u)", vkResult
		);
	vkResult = vkEnumerateInstanceExtensionProperties ( NULL, &availableExtensionCount, NULL );
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkEnumerateInstanceExtensionProperties failed (%u)", vkResult
		);

	availableLayers     = alloca ( availableLayerCount     * sizeof ( VkLayerProperties ) );
	availableExtensions = alloca ( availableExtensionCount * sizeof ( VkExtensionProperties ) );

	vkResult = vkEnumerateInstanceLayerProperties ( &availableLayerCount, availableLayers );
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkEnumerateInstanceLayerProperties failed (%u)", vkResult
		);
	vkResult = vkEnumerateInstanceExtensionProperties (
		NULL, &availableExtensionCount, availableExtensions
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkEnumerateInstanceExtensionProperties failed (%u)", vkResult
		);

	// Request the platform layer which layers and extensions we would like to enable
	// This again stems in no small part from Android support being a little iffy atm,
	// but also allows some slightly less-messy management of per-platform toggling of layers
	// and extensions.
	const char **requiredLayers = NULL, **requiredExtensions = NULL;
	uint32_t requiredLayerCount = 0, requiredExtensionCount  = 0;
	uint32_t debugSupported = 0;

	result = platform_vulkan_get_required_instance_layers (
		availableLayers, availableLayerCount, &requiredLayers, &requiredLayerCount
	);
	if ( result != 0 )
		return result;
	result = platform_vulkan_get_required_instance_extensions (
		&requiredExtensions, &requiredExtensionCount, &debugSupported
	);
	if ( result != 0 )
		return result;

	// One more note about the above functions: These functions _CAN_ and _WILL_ fail if the
	// Vulkan Runtime fails to properly initialize.
	// This I have personally been able to reproduce on my PC by having both a Vulkan-capable
	// Intel driver and a Vulkan-capable NVidia driver installed and then not plugging in a screen
	// to the motherboard (disabling the Intel GPU, causing it to fail to initialize)
	// While I have not been able to reproduce this, it stands to reason these functions would also
	// fail when no valid Vulkan Runtime is installed or no Vulkan-capable drivers are, as these
	// are the first *REAL* Vulkan functions we are calling.

	// With that out of the way, we need to check whether or not all required layers are available
	// in the lists we just queried. This is a simplified lookup, in a serious scenario a more
	// optimized implementation with a hash table, std::map etc is recommended, as the amount of
	// layers you would like to use and especially those provided by runtimes is bound to grow.

	for ( uint32_t i = 0; i < requiredLayerCount; i++ )
	{
		uint32_t found = 0;
		for ( uint32_t j = 0; j < availableLayerCount; j++ )
		{
			if ( strcmp ( availableLayers[j].layerName, requiredLayers[i] ) == 0 )
			{
				found = 1;
				break;
			}
		}

		if ( !found )
			return platform_throw_error (
				-1, "Required layer %s not supported", requiredLayers[i]
			);
	}

#ifndef __ANDROID__
	// NOTE: The Android implementation I tested did not list VK_EXT_debug_report despite
	// supporting it
	for ( uint32_t i = 0; i < requiredExtensionCount; i++ )
	{
		uint32_t found = 0;
		for ( uint32_t j = 0; j < availableExtensionCount; j++ )
		{
			if ( strcmp ( availableExtensions[j].extensionName, requiredExtensions[i] ) == 0 )
			{
				found = 1;
				break;
			}
		}

		if ( !found )
			return platform_throw_error (
				-1, "Required extension %s not supported", requiredExtensions[i]
			);
	}
#endif

	// With compatibility code and extension/layer checking out of the way, it is now time to
	// create the Vulkan "instance". This is the central object for all of your operations with
	// the Vulkan API, and will be a requirement for pretty much any other interaction with the API

	vkResult = vkCreateInstance (
			&(VkInstanceCreateInfo){
					.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
					.pApplicationInfo = &(VkApplicationInfo) {
							.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
							.pApplicationName   = "Vulkan exanple",
							.applicationVersion = 0x10,
							.pEngineName        = "None",
							.engineVersion      = 0xFFFFFFFF,
							.apiVersion         = VK_API_VERSION_1_0
					},
					.enabledLayerCount       = requiredLayerCount,
					.ppEnabledLayerNames     = requiredLayers,
					.enabledExtensionCount   = requiredExtensionCount,
					.ppEnabledExtensionNames = requiredExtensions,
			},
			NULL,	// We are not currently interested in a custom memory allocator
			&outInstance->instance
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateInstance failed with error %u", vkResult );

#if VK_ENABLE_DEBUG
	// Vulkan, as an explicit render API, is primarily designed to maximize performance. As such,
	// features such as error reporting are extremely rudementary, to the point of being almost
	// unworkable. This is in order to relieve the driver of all kinds of error checking,
	// allowing code shipped to the player/user of your game/application to run as efficiently
	// as possible, and not putting runtime implementers with the hassle of implementing and
	// debugging error checking code.

	// However, as this is unlikely to be your first rodeo, you'll probably understand that this
	// is a terrible idea. Because you have to write your code and be able to debug it in order
	// to efficiently develop your application.

	// As such, Vulkan uses the following compromise:
	// - The runtime contains as little debug code for very few functions and conditions
	// - The runtime can optionally implement an extension allowing debug information to be passed
	//   onto the application, and allowing the application to opt-in to this functionality
	// - "Validation Layers" are implemented to intercept all API calls, inspect the calls for
	//   potential problems, and call the callback (in this case "VulkanDebugCallback")
	//   in the aforementioned extension to pass details of the error back to the application.

	// Since the implementation of this function is optional, and we might not necessarily
	// want this behaviour to be enabled when we're not using it, we will first check whether
	// or not we want the functionality, and whether or not the platform layer tells us it is
	// supported by the platform.

	// Note that because the function is an optional extension, the function cannot be statically
	// linked, and will need to be dynamically obtained using vkGetInstanceProcAddr.

	// (NOTE: Another fun Android application note: The Android implementation I had access to,
	// on a Samsung Galaxy S7, has a nasty habbit of not listing supported extensions and failing
	// to initialize when the extension is specified. But when the extensions are disabled,
	// and the extension functionality is being used, it works as though it is enabled.
	// As far as I can tell, this behaviour is simply invalid, but take it into account as a
	// possibility when developing for both platforms)

	PFN_vkCreateDebugReportCallbackEXT createDebugReportFunc = (PFN_vkCreateDebugReportCallbackEXT)
			vkGetInstanceProcAddr ( outInstance->instance, "vkCreateDebugReportCallbackEXT" );

	vkResult = createDebugReportFunc (
			outInstance->instance,
			&(VkDebugReportCallbackCreateInfoEXT){
					.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
					.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
						| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
					.pfnCallback = VulkanDebugCallback,
					.pUserData   = outInstance,
			},
			NULL,
			&outInstance->debugCallback
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkCreateDebugReportCallbackEXT failed with error %u", vkResult
		);
#endif

	// Initialize the first (and for our purposes only) device
	return 0;
}

int32_t vkbase_init_device (
	device_t* outDevice, instance_t* instance,
	uint32_t windowCount, window_t** windows,
	uint32_t queueCount, queue_create_info_t* queueCreateInfos,
	queue_t* outQueues
)
{
	int result        = 0;
	VkResult vkResult = VK_SUCCESS;

	
	// We now have the Vulkan instance, but would also like to actually output pixels to a window.
	// For this purpose, we will need to create a VkSurface. This is a cross-platform abstraction
	// of the window subsystem, and as such has a different function to create the vkSurface
	// depending upon the platform implementation.

	// Also note that this function relies upon 2 extensions: VK_KHR_surface and a
	// platform-specific variant. Not enabling either these will not allow you to create a
	// VkSurface in order to display your render result to the user, but will allow you to use
	// compute functionality as provided by Vulkan for GPGPU applications.

	for ( uint32_t i = 0; i < windowCount; i++ )
	{
		result = platform_vulkan_create_surface (
			&windows[i]->surface,
			instance->instance,
			windows[i]
		);
		if ( result != 0 )
			return result;
	}

	// Now it is time to select a device to render images for us. In order to obtain such a device,
	// we will first need to pick a _physical_ device.

	// A physical device (VkPhysicalDevice) is a Vulkan-capable hardware or software implementation
	// A device (VkDevice) is an object created using a physical device, and is the interface
	// through which you can create objects to submit commands to the physical device.

	// A system can have a number of physical devices, and every physical device can have multiple
	// VkDevice objects associated with it. In most cases, a single application will only create
	// a single VkDevice instance on a single VkPhysicalDevice.
	
	// While an application could create multiple VkDevice objects across multiple VkPhysicalDevice
	// objects, as of version 1.0 of the Vulkan API there is no method allowing for sharing
	// data between multiple device objects, not currently allowing for explicit multi-gpu
	// like DirectX 12. However, a future update of the API could fairly trivially be introduced
	// to add this functionality.

	// For the purposes of this example, we will loop through the physical devices to obtain
	// the first device to support the extensions we require. A dummy call is made to
	// vkGetPhysicalDeviceProperties so the name of the device can be seen amongst other properties
	// upon hitting a breakpoint, information from this call and similar calls could be used to
	// deduce the desired physical device if multiple are available.

	const char** requiredDeviceExtensions = NULL;
	uint32_t requiredDeviceExtensionCount = 0;
	result = platform_vulkan_get_required_device_extensions (
		&requiredDeviceExtensions, &requiredDeviceExtensionCount
	);
	if ( result != 0 )
		return result;

	uint32_t physicalDeviceCount = 0;
	VkPhysicalDevice* physicalDevices;

	vkResult = vkEnumeratePhysicalDevices ( instance->instance, &physicalDeviceCount, NULL );
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkEnumeratePhysicalDevices failed with error %u", vkResult
		);

	physicalDevices = alloca ( physicalDeviceCount * sizeof ( VkPhysicalDevice ) );

	vkResult = vkEnumeratePhysicalDevices (
		instance->instance, &physicalDeviceCount, physicalDevices
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkEnumeratePhysicalDevices failed with error %u", vkResult
		);

	for ( uint32_t i = 0; i < physicalDeviceCount; i++ )
	{
		// Note that vkGetPhysicalDeviceProperties does not return a VkResult
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties ( physicalDevices[i], &physicalDeviceProperties );
		
		uint32_t availableDeviceExtensionCount = 0;
		VkExtensionProperties* availableDeviceExtensions = NULL;
	
		vkResult = vkEnumerateDeviceExtensionProperties (
			physicalDevices[i], NULL, &availableDeviceExtensionCount, NULL
		);
		if ( vkResult != VK_SUCCESS )
			return platform_throw_error (
				-1, "vkEnumerateDeviceExtensionProperties failed with error %u", vkResult
			);
	
		// Note: Calling alloca in a loop is generally a bad idea as depending upon the
		// implementation and (compiler) optimizations the allocations done by alloca might only be
		// deallocated at the end of the function. This example assumes this is not a problem for
		// our use case, but it is important to keep in mind
		availableDeviceExtensions =
			alloca ( availableDeviceExtensionCount * sizeof ( VkExtensionProperties ) );
		vkResult = vkEnumerateDeviceExtensionProperties (
			physicalDevices[i], NULL, &availableDeviceExtensionCount, availableDeviceExtensions
		);
		if ( vkResult != VK_SUCCESS )
			return platform_throw_error (
				-1, "vkEnumerateDeviceExtensionProperties failed with error %u", vkResult
			);
	
		uint32_t suitable = 1;
		for ( uint32_t j = 0; j < requiredDeviceExtensionCount; j++ )
		{
			uint32_t found = 0;
			for ( uint32_t k = 0; k < availableDeviceExtensionCount; k++ )
			{
				if ( strcmp (
						availableDeviceExtensions[k].extensionName,
						requiredDeviceExtensions[j]
					) == 0 )
				{
					found = 1;
					break;
				}
			}
			if ( !found )
			{
				suitable = 0;
				break;
			}
		}
	
		if ( suitable )
		{
			outDevice->physical = physicalDevices[i];
			break;
		}
	}
	
	if ( outDevice->physical == VK_NULL_HANDLE )
		return platform_throw_error ( -1, "No suitable physical device found" );
	
	// While we have multiple devices, each device can be doing multiple operations at the same
	// time. Common operations such as rendering, compute and transfer operations, can depending
	// upon the GPU be executed simultaneously, where not doing so would effectively waste part
	// of the potential of the GPU.

	// The functionality of these GPUs has been abstracted into multiple "queues" and "queue
	// families".

	// A queue family is a specific type of queue group with separate properties from one another.
	// These families could be capable of doing graphics, compute operations, transfer operations
	// or a combination. These queues are not necessarily guaranteed to be completely independent.

	// For our purposes, we are just trying to get Vulkan on a level where it works and we
	// understand how the API works. As such, we might as well obtain a single queue containing
	// all functionality, and supporting to present our framebuffer to the screen. Multiple queues
	// would be preferable, as it would allow multiple threads to work more efficiently on the CPU
	// and functionality to run operations more asynchronously on the GPU. For instance, allowing
	// new data to be transferred while rendering is in progress.

	// So we enumerate the queue families, select a queue family compatible with the operations
	// we require, and remember its index. This is done to create a queue later on.
	
	uint32_t physicalDeviceQueueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties (
		outDevice->physical, &physicalDeviceQueueFamilyCount, NULL
	);
	
	VkQueueFamilyProperties* queueFamilyProperties = NULL;
	queueFamilyProperties =
		alloca ( physicalDeviceQueueFamilyCount * sizeof ( VkQueueFamilyProperties ) );
	vkGetPhysicalDeviceQueueFamilyProperties (
		outDevice->physical, &physicalDeviceQueueFamilyCount, queueFamilyProperties
	);

	VkDeviceQueueCreateInfo* deviceQueueCreateInfos =
		alloca ( queueCount * sizeof ( VkDeviceQueueCreateInfo ) );
	uint32_t* queueFamilyQueueCount = alloca(physicalDeviceQueueFamilyCount * sizeof ( uint32_t ));
	memset ( queueFamilyQueueCount, 0, physicalDeviceQueueFamilyCount * sizeof ( uint32_t ) );

	for ( uint32_t i = 0; i < queueCount; i++ )
	{
		uint32_t requiredFlags = queueCreateInfos[i].queueFlags;
		uint32_t found = 0;
		for ( uint32_t j = 0; j < physicalDeviceQueueFamilyCount; j++ )
		{
			if ( (queueFamilyProperties[j].queueFlags & requiredFlags) != requiredFlags)
				continue;

			for ( uint32_t k = 0; k < queueCreateInfos[i].presentWindowCount; k++ )
			{
				uint32_t windowIdx = queueCreateInfos[i].presentWindowIndices[k];

				VkBool32 presentSupported = VK_FALSE;
				vkResult = vkGetPhysicalDeviceSurfaceSupportKHR (
					outDevice->physical, i, windows[windowIdx]->surface, &presentSupported
				);
				if ( vkResult != VK_SUCCESS )
					return platform_throw_error (
						-1, "vkGetPhysicalDeviceSurfaceSupportKHR failed with error %u", vkResult
					);

				if ( !presentSupported )
					continue;
			}

			if ( queueFamilyQueueCount[j] >= queueFamilyProperties[j].queueCount )
				continue;

			queueFamilyQueueCount[j]++;
			outQueues[i].familyIndex = j;
			found = 1;
			deviceQueueCreateInfos[i] = (VkDeviceQueueCreateInfo){
				.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = j,
				.queueCount       = 1,
				.pQueuePriorities = (float[1]){ 1.0f },
			};
			break;
		}

		if ( found == 0 )
			return platform_throw_error (
				-2, "Could not find an appropriate queue for queue entry %u", i
			);
	}
	
	// Now we create the device object with its extensions and queues we would like to use. This
	// object is nearly exclusively used instead of the VkPhysicalDevice from this point onward.

	vkResult = vkCreateDevice (
		outDevice->physical,
		&(VkDeviceCreateInfo){
			.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount    = queueCount,
			.pQueueCreateInfos       = deviceQueueCreateInfos,
			.enabledExtensionCount   = requiredDeviceExtensionCount,
			.ppEnabledExtensionNames = requiredDeviceExtensions,
		},
		NULL,
		&outDevice->device
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateDevice failed with error %u", vkResult );

	// We should obtain the queue for the queue family we selected. Now that the device is created,
	// the queue can be queried from the device. We don't need it in this function just yet,
	// but we just initialize it to easily obtain it at a later time.

	for ( uint32_t i = 0; i < physicalDeviceQueueFamilyCount; i++ )
	{
		uint32_t cnt = 0;
		for ( uint32_t j = 0; j < queueCount && cnt < queueFamilyQueueCount[i]; j++ )
		{
			if ( outQueues[i].familyIndex == i )
			{
				vkGetDeviceQueue ( outDevice->device, i, cnt, &outQueues[j].queue );
				cnt++;
			}
		}
	}

	// For the next function, we will need to know about the available memory regions on the device
	// This function will allow us to query regions we can allocate memory for use in rendering
	// into, which will become relevant for shader resources. (textures etc)
	
	vkGetPhysicalDeviceMemoryProperties ( outDevice->physical, &outDevice->memoryProperties );

	vkGetPhysicalDeviceProperties ( outDevice->physical, &outDevice->properties );
	
	return 0;
}

int32_t vkbase_init_swapchain (
	swapchain_t* outSwapchain, instance_t* instance, device_t* device, window_t* window,
	swapchain_t* oldSwapchain
)
{

	// With the device and surface created, we will need to create the VkSwapchainKHR object
	// responsible for passing images from the device onto the surface. To do so, we will need to
	// check some things before doing so.

	// Most of these functions are issued as a warning if not called before vkCreateSwapchainKHR.
	// This is done for safety reasons, as the availability for some features are implementation-
	// and/or hardware-dependent.

	// We practically entirely skip the VkSurfaceCapabilitiesKHR properties, as most of the things
	// in there we can fairly safely assume to be well within the range of our simple demo.
	// Present modes and swapchain formats are less likely to be compatible cross-device, and as
	// such, we do queries for the compatible modes, and just use the first one.

	// Production code should check the modes available and attempt to select the correct one.
	// Refer to the Vulkan specification for the differences between all modes. For now, I will
	// move on and focus on other portions instead.

	VkResult vkResult;
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( 
		device->physical, window->surface, &surfaceCapabilities
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed with error %u", vkResult
		);
	
	uint32_t physicalDeviceSurfaceFormatCount = 0;
	uint32_t physicalDevicePresentModeCount = 0;
	VkSurfaceFormatKHR* physicalDeviceSurfaceFormats = NULL;
	VkPresentModeKHR* physicalDevicePresentModes = NULL;
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR (
		device->physical, window->surface, &physicalDeviceSurfaceFormatCount, NULL
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetPhysicalDeviceSurfaceFormatsKHR failed with error %u", vkResult
		);
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR (
		device->physical, window->surface, &physicalDevicePresentModeCount, NULL
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetPhysicalDeviceSurfacePresentModesKHR failed with error %u", vkResult
		);
	physicalDeviceSurfaceFormats = alloca ( 
		physicalDeviceSurfaceFormatCount * sizeof ( VkSurfaceFormatKHR )
	);
	physicalDevicePresentModes = alloca ( 
		physicalDevicePresentModeCount * sizeof ( VkPresentModeKHR )
	);
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR (
		device->physical, window->surface, &physicalDeviceSurfaceFormatCount, physicalDeviceSurfaceFormats
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetPhysicalDeviceSurfaceFormatsKHR failed with error %u", vkResult
		);
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR (
		device->physical, window->surface, &physicalDevicePresentModeCount, physicalDevicePresentModes
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetPhysicalDeviceSurfacePresentModesKHR failed with error %u", vkResult
		);
	
	// Select either R8G8B8A8 or B8G8R8A8 with sRGB

	uint32_t selectedSurfaceFormatIndex = 0xFFFFFFFF;
	for ( uint32_t i = 0; i < physicalDeviceSurfaceFormatCount; i++ )
	{
		if ( (physicalDeviceSurfaceFormats[i].format == VK_FORMAT_R8G8B8A8_SRGB
				|| physicalDeviceSurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
			&& physicalDeviceSurfaceFormats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR )
		{
			selectedSurfaceFormatIndex = i;
			break;
		}
	}
	if ( selectedSurfaceFormatIndex == 0xFFFFFFFF )
		return platform_throw_error (
			-1, "No suitable surface formats"
		);
	
	// While all present modes are technically fine with us, we'd prefer certain modes over others
	// Specifically: Those which can result in above-refresh rate framerates are preferable,
	// for development purposes. MAILBOX does not have tearing on top of it, whereas IMMEDIATE does,
	// FIFO_RELAXED could and FIFO is completely sequential.

	uint32_t selectedPresentModeIndex = 0xFFFFFFFF;
	VkPresentModeKHR preferredPresentModeOrder[] = {
		VK_PRESENT_MODE_MAILBOX_KHR,
		VK_PRESENT_MODE_IMMEDIATE_KHR,
		VK_PRESENT_MODE_FIFO_RELAXED_KHR,
		VK_PRESENT_MODE_FIFO_KHR,
	};
	for ( uint32_t i = 0; i < STATIC_ARRAY_LENGTH(preferredPresentModeOrder); i++ )
	{
		for ( uint32_t j = 0; j < physicalDevicePresentModeCount; j++ )
		{
			if ( physicalDevicePresentModes[j] == preferredPresentModeOrder[i] )
			{
				selectedPresentModeIndex = j;
				break;
			}
		}
		if ( selectedPresentModeIndex != 0xFFFFFFFF )
			break;
	}

	if ( selectedPresentModeIndex == 0xFFFFFFFF )
		return platform_throw_error (
			-1, "No suitable present modes"
		);

	//

	VkSurfaceFormatKHR* selectedSurfaceFormat = &physicalDeviceSurfaceFormats[selectedSurfaceFormatIndex];
	VkPresentModeKHR    selectedPresentMode   =  physicalDevicePresentModes[selectedPresentModeIndex];
	VkExtent2D          surfaceSize           =	 surfaceCapabilities.currentExtent;

	outSwapchain->surfaceFormat = *selectedSurfaceFormat;
	
	// Create the swapchain using the properties we just acquired. If we passed an old swapchain,
	// we pass it onto the function for replacement.
	
	vkResult = vkCreateSwapchainKHR (
		device->device,
		&(VkSwapchainCreateInfoKHR){
			.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface          = window->surface,
			.minImageCount    = surfaceCapabilities.minImageCount,
			.imageFormat      = selectedSurfaceFormat->format,
			.imageColorSpace  = selectedSurfaceFormat->colorSpace,
			.imageExtent      = surfaceSize,
			.imageArrayLayers = 1,
			.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform     = surfaceCapabilities.currentTransform,
			.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode      = selectedPresentMode,
			.clipped          = VK_TRUE,
			.oldSwapchain     = oldSwapchain ? oldSwapchain->swapchain : VK_NULL_HANDLE
		},
		NULL,
		&outSwapchain->swapchain
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateSwapchainKHR failed with error %u", vkResult );
	
	// The swapchain internally creates a number of images for use by the application. These are
	// the backbuffers which will be flipped to the window we created the surface with. As such,
	// we need to query the amount of images we requested for the swapchain

	// Note that the amount of images is not necessarily equal to the amount we requested. As such,
	// we need to re-query the amount of images of the swapchain, allocate space for the images and
	// then query the images themselves.

	// These arrays are allocated using malloc instead of alloca since they may go outside of the
	// scope of this function, so at a later point, we should technically free these pointers. We
	// could also of course combine these two allocations into a single allocation, but for the
	// purposes of clarity this optimization is not applied.

	vkResult = vkGetSwapchainImagesKHR (
		device->device, outSwapchain->swapchain, &outSwapchain->imageCount, NULL
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetSwapchainImagesKHR failed with error %u", vkResult
		);

	outSwapchain->images     = malloc ( outSwapchain->imageCount * sizeof ( VkImage     ) );
	outSwapchain->imageViews = malloc ( outSwapchain->imageCount * sizeof ( VkImageView ) );
	vkResult = vkGetSwapchainImagesKHR (
			device->device, outSwapchain->swapchain, &outSwapchain->imageCount, outSwapchain->images
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
				-1, "vkGetSwapchainImagesKHR failed with error %u", vkResult
		);
	
	// When using images as render targets or textures, we need to create an image view specifying
	// how these images are to be used. In this way, we can have a rendertarget with a slightly
	// different format and resource range than for instance a texture or a different rendertarget.

	// The image views are not quite ready for rendering just yet, as they need to be put into
	// framebuffers. But in order to do so, we need to put _ALL_ render targets (including the
	// depth render target) into a single framebuffer. For this reason, we skip doing so for
	// the moment.

	// Note we do not specify components: Our initializer will automatically initialize these
	// to 0, which corresponds to VK_COMPONENT_SWIZZLE_IDENTITY, taking on the standard mapping
	
	for ( uint32_t i = 0; i < outSwapchain->imageCount; i++ )
	{
		vkResult = vkCreateImageView (
			device->device,
			&(VkImageViewCreateInfo){
				.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image      = outSwapchain->images[i],
				.viewType   = VK_IMAGE_VIEW_TYPE_2D,
				.format     = selectedSurfaceFormat->format,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.levelCount = 1, .layerCount = 1,
				},
			},
			NULL,
			&outSwapchain->imageViews[i]
		);
		if ( vkResult != VK_SUCCESS )
			return platform_throw_error (
				-1, "vkCreateImageView failed with error %u", vkResult
			);
	}

	return 0;
}

int32_t vkbase_destroy_swapchain ( device_t* device, swapchain_t* swapchain )
{
	// Not sure why, but not deleting image views for the swapchain does not generate errors,
	// nor does deleting the views after all. So I think destroying the image views should be
	// _more_ valid, but I haven't been able to find good info on this in the spec

	for ( uint32_t i = 0; i < swapchain->imageCount; i++ )
	{
		vkDestroyImageView ( device->device, swapchain->imageViews[i], NULL );
		swapchain->imageViews[i] = VK_NULL_HANDLE;
	}

	vkDestroySwapchainKHR ( device->device, swapchain->swapchain, NULL );

	return 0;
}

int32_t vkbase_destroy_device ( device_t* device )
{
	vkDestroyDevice ( device->device, NULL );
	device->device = VK_NULL_HANDLE;
	return 0;
}

int32_t vkbase_destroy_instance ( instance_t* instance )
{
#if VK_ENABLE_DEBUG
	PFN_vkDestroyDebugReportCallbackEXT destroyDebugReportFunc = (PFN_vkDestroyDebugReportCallbackEXT)
			vkGetInstanceProcAddr ( instance->instance, "vkDestroyDebugReportCallbackEXT" );

	destroyDebugReportFunc (
			instance->instance, instance->debugCallback, NULL );
	instance->debugCallback = NULL;
#endif
	vkDestroyInstance ( instance->instance, NULL );
	instance->instance = NULL;
	return 0;
}