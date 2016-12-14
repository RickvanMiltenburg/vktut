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

////////////////////////////////////////
// Application data container

typedef struct image_s
{
	// Vulkan handles
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory deviceMemory;
} image_t;

typedef struct device_s
{
	// Vulkan handles
	VkPhysicalDevice physical;
	VkDevice device;
	VkSwapchainKHR swapchain;
	VkImage* swapchainImages;
	VkImageView* swapchainImageViews;
	VkQueue mainQueue;

	VkRenderPass renderPass;
	VkShaderModule forwardRenderVs, forwardRenderFs;
	VkShaderModule postRenderVs, postRenderFs;
	VkDescriptorSetLayout postDescriptorSetLayout;
	VkPipelineLayout forwardPipelineLayout, postPipelineLayout;
	VkPipelineCache pipelineCache;
	VkPipeline forwardPipeline, postPipeline;

	// Stored properties
	uint32_t swapchainImageCount;
	uint32_t mainQueueFamilyIndex;

	// Sub-objects
	image_t mainDepthBuffer;
} device_t;

typedef struct instance_s
{
	// Vulkan handles
	VkInstance instance;
	VkSurfaceKHR surface;
	VkDebugReportCallbackEXT debugCallback;

	// Sub-objects
	device_t device;
} instance_t;

typedef struct app_s
{
	void* userdata;
	app_window_t* window;

	instance_t instance;
} app_t;

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

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
					uint64_t srcObject, size_t location, int32_t msgCode,
					const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
	if( msgFlags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT))
	{
		// NOTE: All hardcoded codes below can change depending upon the version of the Vulkan
		// headers. There is little about this that can be done apart from trial-and-error.
		if ( strcmp ( pLayerPrefix, "Swapchain" ) == 0 )
		{
			switch ( msgCode )
			{
			case 21:		// Ignore VK_LAYER_swapchain SWAPCHAIN_PRIOR_COUNT
				return 0;	// I really don't see why this would even be a warning
							// You literally specify how many images, so why query it?
			}
		}
	}

	platform_throw_error ( msgCode, "[VULKAN] Debug callback invoked\n"
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

	// We did not "handle" the error
	return 0;
}

////////////////////////////////////////
// Vulkan demo function

static int32_t app_init_instance ( instance_t* instance, app_window_t* window );
static int32_t app_init_device ( device_t* device, VkInstance instance, VkSurfaceKHR surface );
static int32_t app_init_image (
	image_t* img, VkDevice device, VkPhysicalDeviceMemoryProperties* memoryProperties,
	VkImageCreateInfo* imageInfo, VkImageViewCreateInfo* imageViewInfo
);

////////////////////////////////////////
// 

static int32_t app_init_instance (
	instance_t* instance, app_window_t* window
)
{
	int result        = 0;
	VkResult vkResult = VK_SUCCESS;

	// Request the platform layer which layers and extensions we would like to enable
	// This again stems in no small part from Android support being a little iffy atm,
	// but also allows some slightly less-messy management of per-platform toggling of layers
	// and extensions.
	const char **requiredLayers = NULL, **requiredExtensions = NULL;
	uint32_t requiredLayerCount = 0, requiredExtensionCount  = 0;
	uint32_t debugSupported = 0;

	result = platform_vulkan_get_required_instance_layers ( &requiredLayers, &requiredLayerCount );
	if ( result != 0 )
		return result;
	result = platform_vulkan_get_required_instance_extensions (
		&requiredExtensions, &requiredExtensionCount, &debugSupported
	);
	if ( result != 0 )
		return result;

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
			&instance->instance
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
			vkGetInstanceProcAddr ( instance->instance, "vkCreateDebugReportCallbackEXT" );

	vkResult = createDebugReportFunc (
			instance->instance,
			&(VkDebugReportCallbackCreateInfoEXT){
					.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
					.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
						| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
					.pfnCallback = VulkanDebugCallback,
					.pUserData   = instance,
			},
			NULL,
			&instance->debugCallback
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkCreateDebugReportCallbackEXT failed with error %u", vkResult
		);
#endif

	// We now have the Vulkan instance, but would also like to actually output pixels to a window.
	// For this purpose, we will need to create a VkSurface. This is a cross-platform abstraction
	// of the window subsystem, and as such has a different function to create the vkSurface
	// depending upon the platform implementation.

	// Also note that this function relies upon 2 extensions: VK_KHR_surface and a
	// platform-specific variant. Not enabling either these will not allow you to create a
	// VkSurface in order to display your render result to the user, but will allow you to use
	// compute functionality as provided by Vulkan for GPGPU applications.

	result = platform_vulkan_create_surface (
		&instance->surface,
		instance->instance,
		window
	);
	if ( result != 0 )
		return result;

	// Initialize the first (and for our purposes only) device
	return app_init_device ( &instance->device, instance->instance, instance->surface );
}

static int32_t app_init_device (
	device_t* device, VkInstance instance, VkSurfaceKHR surface
)
{
	int result        = 0;
	VkResult vkResult = VK_SUCCESS;

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

	vkResult = vkEnumeratePhysicalDevices ( instance, &physicalDeviceCount, NULL );
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkEnumeratePhysicalDevices failed with error %u", vkResult
		);

	physicalDevices = alloca ( physicalDeviceCount * sizeof ( VkPhysicalDevice ) );

	vkResult = vkEnumeratePhysicalDevices (
		instance, &physicalDeviceCount, physicalDevices
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
			device->physical = physicalDevices[i];
			break;
		}
	}
	
	if ( device->physical == VK_NULL_HANDLE )
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
		device->physical, &physicalDeviceQueueFamilyCount, NULL
	);
	
	VkQueueFamilyProperties* queueFamilyProperties = NULL;
	queueFamilyProperties =
		alloca ( physicalDeviceQueueFamilyCount * sizeof ( VkQueueFamilyProperties ) );
	vkGetPhysicalDeviceQueueFamilyProperties (
		device->physical, &physicalDeviceQueueFamilyCount, queueFamilyProperties
	);

	device->mainQueueFamilyIndex = 0xFFFFFFFF;
	for ( uint32_t i = 0; i < physicalDeviceQueueFamilyCount; i++ )
	{
		VkBool32 presentSupported = VK_FALSE;
	
		vkResult = vkGetPhysicalDeviceSurfaceSupportKHR (
			device->physical, i, surface, &presentSupported
		);
		if ( vkResult != VK_SUCCESS )
			return platform_throw_error (
				-1, "vkGetPhysicalDeviceSurfaceSupportKHR failed with error %u", vkResult
			);
	
		if ( presentSupported && queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT )
		{
			device->mainQueueFamilyIndex = i;
			break;
		}
	}
	
	if ( device->mainQueueFamilyIndex == 0xFFFFFFFF )
		return platform_throw_error ( -1, "No present-capable graphics queue found" );
	
	// Now we create the device object with its extensions and queues we would like to use. This
	// object is nearly exclusively used instead of the VkPhysicalDevice from this point onward.
	
	vkResult = vkCreateDevice (
		device->physical,
		&(VkDeviceCreateInfo){
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = (VkDeviceQueueCreateInfo[1]){
				[0] = {
					.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = device->mainQueueFamilyIndex,
					.queueCount       = 1,
					.pQueuePriorities = (float[1]){ 1.0f },
				},
			},
			.enabledExtensionCount   = requiredDeviceExtensionCount,
			.ppEnabledExtensionNames = requiredDeviceExtensions,
		},
		NULL,
		&device->device
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateDevice failed with error %u", vkResult );

	// We should obtain the queue for the queue family we selected. Now that the device is created,
	// the queue can be queried from the device. We don't need it in this function just yet,
	// but we just initialize it to easily obtain it at a later time.

	vkGetDeviceQueue ( device->device, device->mainQueueFamilyIndex, 0, &device->mainQueue );
	
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

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( 
		device->physical, surface, &surfaceCapabilities
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
		device->physical, surface, &physicalDeviceSurfaceFormatCount, NULL
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetPhysicalDeviceSurfaceFormatsKHR failed with error %u", vkResult
		);
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR (
		device->physical, surface, &physicalDevicePresentModeCount, NULL
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
		device->physical, surface, &physicalDeviceSurfaceFormatCount, physicalDeviceSurfaceFormats
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetPhysicalDeviceSurfaceFormatsKHR failed with error %u", vkResult
		);
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR (
		device->physical, surface, &physicalDevicePresentModeCount, physicalDevicePresentModes
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetPhysicalDeviceSurfacePresentModesKHR failed with error %u", vkResult
		);
	
	//
	//
	//

	VkSurfaceFormatKHR* selectedSurfaceFormat = &physicalDeviceSurfaceFormats[0];
	VkPresentModeKHR    selectedPresentMode   =  physicalDevicePresentModes[0];
	VkExtent2D          surfaceSize           =	 surfaceCapabilities.currentExtent;
	
	//
	//
	//
	
	vkResult = vkCreateSwapchainKHR (
		device->device,
		&(VkSwapchainCreateInfoKHR){
			.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface          = surface,
			.minImageCount    = surfaceCapabilities.minImageCount,
			.imageFormat      = selectedSurfaceFormat->format,
			.imageColorSpace  = selectedSurfaceFormat->colorSpace,
			.imageExtent      = surfaceSize,
			.imageArrayLayers = 1,
			.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform     = surfaceCapabilities.currentTransform,
			.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode      = selectedPresentMode,
			.clipped          = VK_FALSE,
		},
		NULL,
		&device->swapchain
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateSwapchainKHR failed with error %u", vkResult );
	
	// The swapchain internally creates a number of images for use by the application. These are
	// the backbuffers which will be flipped to the window we created the surface with. As such,
	// we need to query the amount of images we requested for the swapchain

	// Note that the amount of images is not necessarily equal to the amount we requested. As such,
	// we need to re-query the amount of images of the swapchain

	vkResult = vkGetSwapchainImagesKHR (
		device->device, device->swapchain, &device->swapchainImageCount, NULL
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkGetSwapchainImagesKHR failed with error %u", vkResult
		);

	device->swapchainImages     = malloc ( device->swapchainImageCount * sizeof ( VkImage     ) );
	device->swapchainImageViews = malloc ( device->swapchainImageCount * sizeof ( VkImageView ) );
	vkResult = vkGetSwapchainImagesKHR (
			device->device, device->swapchain, &device->swapchainImageCount, device->swapchainImages
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
	
	for ( uint32_t i = 0; i < device->swapchainImageCount; i++ )
	{
		vkResult = vkCreateImageView (
			device->device,
			&(VkImageViewCreateInfo){
				.sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image      = device->swapchainImages[i],
				.viewType   = VK_IMAGE_VIEW_TYPE_2D,
				.format     = selectedSurfaceFormat->format,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.levelCount = 1, .layerCount = 1,
				},
			},
			NULL,
			&device->swapchainImageViews[i]
		);
		if ( vkResult != VK_SUCCESS )
			return platform_throw_error (
				-1, "vkCreateImageView failed with error %u", vkResult
			);
	}
	
	// For the next function, we will need to know about the available memory regions on the device
	// This function will allow us to query regions we can allocate memory for use in rendering
	// into, which will become relevant for shader resources. (textures etc)
	
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties ( device->physical, &memoryProperties );

	// We will now initialize the texture intended as depth render target. Refer to the function
	// definition for details on the function.

	result = app_init_image (
		&device->mainDepthBuffer, device->device, &memoryProperties,
		&(VkImageCreateInfo){
			.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType     = VK_IMAGE_TYPE_2D,
			.format        = VK_FORMAT_D32_SFLOAT,
			.extent        = {.width = surfaceSize.width, .height = surfaceSize.height, .depth=1},
			.mipLevels     = 1,
			.arrayLayers   = 1,
			.samples       = VK_SAMPLE_COUNT_1_BIT,
			.tiling        = VK_IMAGE_TILING_OPTIMAL,
			.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT,
			.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
		},
		&(VkImageViewCreateInfo){
			.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format   = VK_FORMAT_D32_SFLOAT,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1
			},
		}
	);

	//
	//
	//

	return result;
}

static int32_t app_init_image (
	image_t* img, VkDevice device, VkPhysicalDeviceMemoryProperties* memoryProperties,
	VkImageCreateInfo* imageInfo, VkImageViewCreateInfo* imageViewInfo
)
{
	VkResult result;

	// First we need to specify the image description itself. This will allow the internal
	// implementation to determine the necessary formats, layouts, sizes and other miscellaneous
	// properties for internal use.

	result = vkCreateImage ( device, imageInfo, NULL, &img->image );
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateImage failed (%u)", result );

	// The image handle has been created, but no memory has been allocated for the image. Using
	// the image at this stage is not possible, as without any memory, the image is unable to
	// contain actual data.

	// In order to resolve this, we will need to request how much memory they need and where
	// this memory could be obtained, then selecting our preferred place to put this memory, and
	// then allocating and binding the allocated memory.

	// For selecting the location we would prefer we have two criteria:
	// - The image should support the memory type we would like to allocate
	// - The memory should be allocated on DEVICE_LOCAL memory, if available. DEVICE_LOCAL memory
	//   is guaranteed to be the main memory of the device allowing for high throughput with low
	//   latency. The alternative (HOST) is effectively system RAM in a modern system with a
	//   dedicated GPU. Transferring data from system RAM to the GPU is to be avoided for
	//   frequently accessed data. (eg textures)
	//   Note that UMA (Uniform Memory Access) GPUs, such as integrated graphics chips, will report
	//   all memory as DEVICE_LOCAL, as they do not have (significant) dedicated graphics memory.

	// The implementation of this function is sub-optimal, as GPU memory allocations tend to be
	// fairly slow. As such, larger combined memory allocations are to be preferred over individual
	// allocations. It would be preferable to call vkAllocateMemory once to allocate the memory
	// for multiple textures and buffers at the same time, and then subdivide the memory in the
	// call to vkBindImageMemory using the "offset" property

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements ( device, img->image, &memoryRequirements );

	uint32_t memoryTypeIndex = 0xFFFFFFFF;
	for ( uint32_t i = 0; i < memoryProperties->memoryTypeCount; i++ )
	{
		if ( memoryRequirements.memoryTypeBits & (1<<i) )
		{
			memoryTypeIndex = i;

			if ( memoryProperties->memoryTypes[i].propertyFlags
				& VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT )
			{
				break;
			}
		}
	}

	result = vkAllocateMemory (
		device, 
		&(VkMemoryAllocateInfo){
			.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize  = memoryRequirements.size,
			.memoryTypeIndex = memoryTypeIndex,
		},
		NULL, &img->deviceMemory
	);
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkAllocateMemory failed (%u)", result );

	// This call is necessary in order to couple the allocated memory to the image. Note that - as
	// previously stated - the memory should preferably be subdivided from larger allocations using
	// the "offset" property.
	
	// Also note that this call takes the amount of bytes as specified by
	// vkGetImageMemoryRequirements. Which is why no size is to be specified, despite the _TYPE_ of
	// "offset" being "VkDeviceSize". This is a little confusing naming.

	result = vkBindImageMemory ( device, img->image, img->deviceMemory, 0 );
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkBindImageMemory failed (%u)", result );

	// In order to use the image in shaders, we do not need the VkImage, but the VkImageView. For
	// convenience and brevity, I decided to put both object creations in the same function.

	// Note that the image view does not need to be identical to the image itself: An image can be
	// aliased across formats, types and subresources by creating multiple views.
	
	// Since we do not know the image object before calling the function, we patch the image
	// object into the structure. Technically it would be better practice to make the argument
	// const, copy the structure over and then patching it into the copy, but for the purposes
	// of simplicity I just opted to do it this way.

	imageViewInfo->image = img->image;
	result = vkCreateImageView ( device, imageViewInfo, NULL, &img->imageView );
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateImageView failed (%u)", result );

	//
	//
	//

	return 0;
}

////////////////////////////////////////
// 

enum
{
	SUBPASS_FORWARD,
	SUBPASS_POST,

	SUBPASS_COUNT
};

int32_t app_init_renderpass ( device_t* device )
{
	// In order to continue with the objects we require, we will first need to specify our
	// rendering process. This is done in order to optimize resource transitions and rendering
	// operations internally for as much as possible.

	// This particularly benefits tiled renderers, allowing them to resolve dependencies for
	// maximum tiling efficiency. While immediate mode renderers tend to benefit less from such
	// optimizations, there are still internal implementation optimizations which can be done
	// using renderpasses.

	// Use of the renderpass system is required in Vulkan; Render passes are used when creating
	// graphics pipelines and during rendering, allowing the graphics pipelines to be optimized
	// to the furthest extent possible beforehand for during rendering operations.

	enum
	{
		ATTACHMENT_INTERMEDIATE_COLOR,
		ATTACHMENT_INTERMEDIATE_DEPTH,
		ATTACHMENT_FRAMEBUFFER,

		ATTACHMENT_COUNT
	};

	VkResult result = vkCreateRenderPass (
		device->device,
		&(VkRenderPassCreateInfo){
			.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = ATTACHMENT_COUNT,
			.pAttachments    = (VkAttachmentDescription[ATTACHMENT_COUNT]){
				[ATTACHMENT_INTERMEDIATE_COLOR] = {
					.format        = VK_FORMAT_R8G8B8A8_SRGB,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp       = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				},
				[ATTACHMENT_INTERMEDIATE_DEPTH] = {
					.format        = VK_FORMAT_D32_SFLOAT,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp       = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				},
				[ATTACHMENT_FRAMEBUFFER] = {
					.format        = VK_FORMAT_R8G8B8A8_SRGB,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.storeOp       = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				},
			},
			.subpassCount = SUBPASS_COUNT,
			.pSubpasses   = (VkSubpassDescription[SUBPASS_COUNT]){
				[SUBPASS_FORWARD] = {
					.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.colorAttachmentCount = 1,
					.pColorAttachments    = (VkAttachmentReference[1]){
						{
							.attachment = ATTACHMENT_INTERMEDIATE_COLOR,
							.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
						},
					},
					.pDepthStencilAttachment = &(VkAttachmentReference){
						.attachment = ATTACHMENT_INTERMEDIATE_DEPTH,
						.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
					},
				},
				[SUBPASS_POST] = {
					.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.inputAttachmentCount = 1,
					.pInputAttachments    = (VkAttachmentReference[1]){
						{
							.attachment = ATTACHMENT_INTERMEDIATE_COLOR,
							.layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
						},
					},
					.colorAttachmentCount = 1,
					.pColorAttachments    = (VkAttachmentReference[1]){
						{
							.attachment = ATTACHMENT_FRAMEBUFFER,
							.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
						},
					},
				},
			},
			.dependencyCount = 1,
			.pDependencies   = (VkSubpassDependency[1]){
				{
					.srcSubpass      = SUBPASS_FORWARD,
					.dstSubpass      = SUBPASS_POST,
					.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
					.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
				},
			},
		},
		NULL,
		&device->renderPass
	);
	if ( result != 0 )
		return platform_throw_error ( -1, "vkCreateRenderPass failed (%u)", result );

	//
	//
	//

	return 0;
}

int32_t app_init_graphics_pipelines ( device_t* device, app_window_t* window )
{
	VkResult vkResult = VK_SUCCESS;

	// List the shader paths and the shader module they are intended to be loaded into
	// This is just done to ensure code duplication is reduced a bit more, as shader module
	// creation tends to not be that much code, but the amount of shader modules tend to scale
	// quite quickly.

	struct
	{
		const char* path;
		VkShaderModule* outModule;
	} shaders[] = {
		{ .path = "shaders/forward_v.spv", .outModule = &device->forwardRenderVs },
		{ .path = "shaders/forward_f.spv", .outModule = &device->forwardRenderFs },
		{ .path = "shaders/post_v.spv",    .outModule = &device->postRenderVs    },
		{ .path = "shaders/post_f.spv",    .outModule = &device->postRenderFs    },
	};

	// Create the aforementioned shader modules. If any of these fail, check the documentation
	// for compiling the shaders, as you have probably skipped that step.

	for ( uint32_t i = 0; i < STATIC_ARRAY_LENGTH(shaders); i++ )
	{
		file_t file;
		if ( platform_file_load ( &file, shaders[i].path ) != 0 )
			return -1;

		vkResult = vkCreateShaderModule (
			device->device,
			&(VkShaderModuleCreateInfo){
				.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = file.sizeInBytes,
				.pCode    = file.data,
			},
			NULL,
			shaders[i].outModule
		);
		if ( vkResult != VK_SUCCESS )
			return platform_throw_error ( -1, "vkCreateShaderModule failed (%u)", vkResult );
	}

	// Create descriptor set layout
	// Note that only the post processing shader currently needs this, as the forward shaders
	// currently do not use texture mapping or other descriptors.
	
	vkResult = vkCreateDescriptorSetLayout (
		device->device,
		&(VkDescriptorSetLayoutCreateInfo){
			.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 2,
			.pBindings    = (VkDescriptorSetLayoutBinding[2]){
				{
					.binding         = 0,
					.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
					.descriptorCount = 1,
					.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
				},
				{
					.binding         = 1,
					.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
					.descriptorCount = 1,
					.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
				},
			},
		},
		NULL,
		&device->postDescriptorSetLayout
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateDescriptorSetLayout failed (%u)", vkResult );

	// In order to create the pipelines, we will first need to create the layouts depicting the
	// necessary inputs on the side of descriptors and push constants. These are used for binding
	// later on. If two pipelines use the same pipeline layouts, the bindings are guaranteed to be
	// compatible across both pipelines.

	vkResult = vkCreatePipelineLayout (
		device->device,
		&(VkPipelineLayoutCreateInfo){
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount         = 0,
			.pSetLayouts            = NULL,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges    = (VkPushConstantRange[1]){
				{
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.offset     = 0,
					.size       = 16 * sizeof ( float ),
				},
			},
		},
		NULL,
		&device->forwardPipelineLayout
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreatePipelineLayout failed (%u)", vkResult );

	vkResult = vkCreatePipelineLayout (
		device->device,
		&(VkPipelineLayoutCreateInfo){
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount         = 1,
			.pSetLayouts            = (VkDescriptorSetLayout[1]){device->postDescriptorSetLayout},
			.pushConstantRangeCount = 0,
			.pPushConstantRanges    = NULL,
		},
		NULL,
		&device->postPipelineLayout
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreatePipelineLayout failed (%u)", vkResult );

	// Get the window width and height. This is strictly speaking not "great" practice as we are
	// using a different source for our actual surface and for our graphics pipeline, but we can
	// safely assume there is no way for a window resize to occur between the two function calls.

	// (On Windows at least it is literally impossible: Only when WM_SIZE is handled in WndProc
	// is a window actually allowed to resize)

	uint32_t windowWidth, windowHeight;
	platform_window_get_size ( window, &windowWidth, &windowHeight );

	// Time to create the two pipelines. Note that the function is _absolutely massive_, but not
	// too complicated necessarily.

	// First thing to note specifically is that this function call is plural: This function call
	// can (and should!) create multiple pipelines at the same time. Properties shared between
	// multiple pipelines can be reused, and pipelines can inherit one another using the
	// "basePipeline" and "basePipelineIndex" properties, allowing creation of pipelines to be
	// accelerated.

	// Secondly, the post-processing pipeline does not use the input layout, and for this reason
	// is a fair bit smaller than the forward rendering pipeline.

	// Furthermore, note that despite the fact we do not use color blending, we need to specify
	// the blend description structure in order to configure the color output to actually output
	// color to our render targets. This is an easy property to forget to set; Please don't forget.

	// Last but not least: All properties specified in the pipeline are static, and require a new
	// pipeline in order to be changed. The only exceptions for this are the properties you can
	// (and did!) specify in pDynamicState. Any property not enabled in pDynamicState can not
	// be changed at runtime, and will require a new pipeline to be made. Since creating new
	// pipelines is expensive (and a lot of code overhead) this is to be avoided, but it is
	// ill-advised to specify more dynamic properties than needed: Dynamic properties can
	// potentially incur runtime penalties depending upon the hardware. So use dynamic properties
	// sparingly, but for properties which can change a lot (eg viewport and scissor where elements
	// of the UI are resizable) making these properties dynamic can be a good idea.n

	enum
	{
		PIPELINE_FORWARD,
		PIPELINE_POST,

		PIPELINE_COUNT,
	};

	enum
	{
		VERTEX_BINDING_POSITION,
		VERTEX_BINDING_TEXCOORD,

		VERTEX_BINDING_COUNT,
	};

	VkPipeline* pipelineAssignment[PIPELINE_COUNT] = {
		[PIPELINE_FORWARD] = &device->forwardPipeline,
		[PIPELINE_POST   ] = &device->postPipeline,
	};

	VkPipeline pipelines[PIPELINE_COUNT];
	vkResult = vkCreateGraphicsPipelines (
		device->device,
		device->pipelineCache,
		PIPELINE_COUNT,
		(VkGraphicsPipelineCreateInfo[PIPELINE_COUNT]){
			[PIPELINE_FORWARD] = {
				.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.flags      = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
				.stageCount = 2,
				.pStages    = (VkPipelineShaderStageCreateInfo[2]){
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_VERTEX_BIT,
						.module = device->forwardRenderVs,
						.pName  = "main",
					},
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
						.module = device->forwardRenderFs,
						.pName  = "main",
					},
				},
				.pVertexInputState = &(VkPipelineVertexInputStateCreateInfo){
					.sType             = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
					.vertexBindingDescriptionCount = VERTEX_BINDING_COUNT,
					.pVertexBindingDescriptions    =
						(VkVertexInputBindingDescription[VERTEX_BINDING_COUNT]){
						[VERTEX_BINDING_POSITION] = {
							.binding   = VERTEX_BINDING_POSITION,
							.stride    = 3 * sizeof ( float ),
							.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
						},
						[VERTEX_BINDING_TEXCOORD] = {
							.binding   = VERTEX_BINDING_TEXCOORD,
							.stride    = 2 * sizeof ( float ),
							.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
						},
					},
					.vertexAttributeDescriptionCount = 2,
					.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription[2]){
						{
							.location = 0,
							.binding  = VERTEX_BINDING_POSITION,
							.format   = VK_FORMAT_R32G32B32_SFLOAT,
							.offset   = 0,
						},
						{
							.location = 1,
							.binding  = VERTEX_BINDING_TEXCOORD,
							.format   = VK_FORMAT_R32G32_SFLOAT,
							.offset   = 0,
						},
					},
				},
				.pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo){
					.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
					.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				},
				.pViewportState = &(VkPipelineViewportStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
					.viewportCount = 1,
					.pViewports = (VkViewport[1]){
						{
							.width = windowWidth, .height = windowHeight,
							.minDepth = -1.0f, .maxDepth =  1.0f,
						},
					},
					.scissorCount = 1,
					.pScissors = (VkRect2D[1]){
						{ .extent.width = windowWidth, .extent.height = windowHeight },
					},
				},
				.pRasterizationState = &(VkPipelineRasterizationStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
					.depthClampEnable        = VK_FALSE,
					.rasterizerDiscardEnable = VK_TRUE,
					.polygonMode             = VK_POLYGON_MODE_FILL,
					.cullMode                = VK_CULL_MODE_NONE,
					.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
					.depthBiasEnable         = VK_FALSE,
					.lineWidth               = 1.0f,
				},
				.pMultisampleState = &(VkPipelineMultisampleStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
					.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
					.sampleShadingEnable   = VK_FALSE,
					.alphaToCoverageEnable = VK_FALSE,
					.alphaToOneEnable      = VK_FALSE,
				},
				.pDepthStencilState = &(VkPipelineDepthStencilStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
					.depthTestEnable       = VK_TRUE,
					.depthWriteEnable      = VK_TRUE,
					.depthCompareOp        = VK_COMPARE_OP_LESS,
					.depthBoundsTestEnable = VK_FALSE,
					.stencilTestEnable     = VK_FALSE,
				},
				.pColorBlendState = &(VkPipelineColorBlendStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
					.logicOpEnable = VK_FALSE,
					.attachmentCount = 1,
					.pAttachments = (VkPipelineColorBlendAttachmentState[1]){
						{
							.blendEnable = VK_FALSE,
							.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
								| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
						},
					},
				},
				.pDynamicState = &(VkPipelineDynamicStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
					.dynamicStateCount = 2,
					.pDynamicStates = (VkDynamicState[2]){
						VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
					},
				},
				.layout     = device->forwardPipelineLayout,
				.renderPass = device->renderPass,
				.subpass    = SUBPASS_FORWARD,
			},
			[PIPELINE_POST] = {
				.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.flags      = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
				.stageCount = 2,
				.pStages    = (VkPipelineShaderStageCreateInfo[2]){
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_VERTEX_BIT,
						.module = device->postRenderVs,
						.pName  = "main",
					},
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
						.module = device->postRenderFs,
						.pName  = "main",
					},
				},
				.pVertexInputState = &(VkPipelineVertexInputStateCreateInfo){
					.sType             = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
					.vertexBindingDescriptionCount   = 0,
					.pVertexBindingDescriptions      = NULL,
					.vertexAttributeDescriptionCount = 0,
					.pVertexAttributeDescriptions    = NULL,
				},
				.pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo){
					.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
					.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				},
				.pViewportState = &(VkPipelineViewportStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
					.viewportCount = 1,
					.pViewports = (VkViewport[1]){
						{
							.width = windowWidth, .height = windowHeight,
							.minDepth = -1.0f, .maxDepth =  1.0f,
						},
					},
					.scissorCount = 1,
					.pScissors = (VkRect2D[1]){
						{ .extent.width = windowWidth, .extent.height = windowHeight },
					},
				},
				.pRasterizationState = &(VkPipelineRasterizationStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
					.depthClampEnable        = VK_FALSE,
					.rasterizerDiscardEnable = VK_TRUE,
					.polygonMode             = VK_POLYGON_MODE_FILL,
					.cullMode                = VK_CULL_MODE_NONE,
					.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
					.depthBiasEnable         = VK_FALSE,
					.lineWidth               = 1.0f,
				},
				.pMultisampleState = &(VkPipelineMultisampleStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
					.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
					.sampleShadingEnable   = VK_FALSE,
					.alphaToCoverageEnable = VK_FALSE,
					.alphaToOneEnable      = VK_FALSE,
				},
				.pDepthStencilState = &(VkPipelineDepthStencilStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
					.depthTestEnable       = VK_TRUE,
					.depthWriteEnable      = VK_TRUE,
					.depthCompareOp        = VK_COMPARE_OP_LESS,
					.depthBoundsTestEnable = VK_FALSE,
					.stencilTestEnable     = VK_FALSE,
				},
				.pColorBlendState = &(VkPipelineColorBlendStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
					.logicOpEnable = VK_FALSE,
					.attachmentCount = 1,
					.pAttachments = (VkPipelineColorBlendAttachmentState[1]){
						{
							.blendEnable = VK_FALSE,
							.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
								| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
						},
					},
				},
				.pDynamicState = &(VkPipelineDynamicStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
					.dynamicStateCount = 2,
					.pDynamicStates = (VkDynamicState[2]){
						VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
					},
				},
				.layout     = device->postPipelineLayout,
				.renderPass = device->renderPass,
				.subpass    = SUBPASS_POST,
			},
		},
		NULL,
		pipelines
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateGraphicsPipelines failed (%u)", vkResult );

	// Since we initialized the pipelines in an array, it would be clearest to subsequently set the
	// pipelines to proper variables. One could use the same system as used for shaders

	for ( uint32_t i = 0; i < STATIC_ARRAY_LENGTH(pipelineAssignment); i++ )
	{
		*(pipelineAssignment[i]) = pipelines[i];
	}

	//
	//
	//

	return 0;
}

int32_t app_run ( void* userdata )
{
	int32_t result = 0;

	// Null-initialize an empty app_t structure, with the specified userdata
	app_t app = { .userdata = userdata };

	// Not all that interesting, just a platform-specific method for creating a window for our
	// Vulkan renderer to eventually render to
	result = platform_window_create ( &app.window, userdata );
	if ( result != 0 )
		return result;

	// Originally created and mostly used for Android Vulkan initialization.

	// While strictly not necessary for Vulkan, Android is currently in a bit of an iffy spot
	// when it comes to Vulkan support. As such, the functions for Vulkan cannot be statically
	// linked, necessitating for functions to be loaded dynamically.

	// If this proves no longer necessary in the future, check whether a single platform still uses
	// this, and if not, feel free to remove
	result = platform_init ( app.userdata );
	if ( result != 0 )
		return result;
	
	// Initialize the Vulkan instance
	result = app_init_instance ( &app.instance, app.window );
	if ( result != 0 )
		return result;

	// Initialize the renderpass needed for rendering and creating graphics pipelines
	result = app_init_renderpass ( &app.instance.device );
	if ( result != 0 )
		return result;

	// Create the graphics pipelines required for rendering
	result = app_init_graphics_pipelines ( &app.instance.device, app.window );
	if ( result != 0 )
		return result;

	//
	//
	//

	return 0;
}
