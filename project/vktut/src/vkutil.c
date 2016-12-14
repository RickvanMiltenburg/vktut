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

#include "vkutil.h"
#include "../../mconv/include/mconv.h"
#include <string.h>
#include <stdlib.h>

////////////////////////////////////////
// Utilities for the utilities

// Creates a case expression for the value of an expression returning the stringified version
#define C2STR(x) case x: return #x

// Excerpts from rvm_math.h for utility functions. We don't need the entire header here.
#define RVM_MAX(x,y) (((x)>(y))?(x):(y))
#define RVM_ALIGN_UP_POW2(x,n) (((x)+((n)-1))&(~((n)-1)))

// And, of course, my favorite: alloca!
#if defined ( _WIN32 )
#define alloca _alloca
#elif defined ( __ANDROID__ )
#include <alloca.h>
#endif

////////////////////////////////////////
// The utilities themselves

// The data in the functions below is based on Vulkan header VK_HEADER_VERSION = 30

const char* vkutil_VkResult_to_string ( VkResult in )
{
	switch ( in )
	{
	C2STR(VK_SUCCESS					   );
	C2STR(VK_NOT_READY					   );
	C2STR(VK_TIMEOUT					   );
	C2STR(VK_EVENT_SET					   );
	C2STR(VK_EVENT_RESET				   );
	C2STR(VK_INCOMPLETE					   );
	C2STR(VK_ERROR_OUT_OF_HOST_MEMORY	   );
	C2STR(VK_ERROR_OUT_OF_DEVICE_MEMORY	   );
	C2STR(VK_ERROR_INITIALIZATION_FAILED   );
	C2STR(VK_ERROR_DEVICE_LOST			   );
	C2STR(VK_ERROR_MEMORY_MAP_FAILED	   );
	C2STR(VK_ERROR_LAYER_NOT_PRESENT	   );
	C2STR(VK_ERROR_EXTENSION_NOT_PRESENT   );
	C2STR(VK_ERROR_FEATURE_NOT_PRESENT	   );
	C2STR(VK_ERROR_INCOMPATIBLE_DRIVER	   );
	C2STR(VK_ERROR_TOO_MANY_OBJECTS		   );
	C2STR(VK_ERROR_FORMAT_NOT_SUPPORTED	   );
	C2STR(VK_ERROR_FRAGMENTED_POOL		   );
	C2STR(VK_ERROR_SURFACE_LOST_KHR		   );
	C2STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
	C2STR(VK_SUBOPTIMAL_KHR				   );
	C2STR(VK_ERROR_OUT_OF_DATE_KHR		   );
	C2STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
	C2STR(VK_ERROR_VALIDATION_FAILED_EXT   );
	C2STR(VK_ERROR_INVALID_SHADER_NV       );
	}
	return "<Unknown VkResult>";
}

const char* vkutil_VkDebugReportObjectTypeEXT_to_string ( VkDebugReportObjectTypeEXT in )
{
	switch ( in )
	{
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT			   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT			   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT	   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT			   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT				   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT			   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT	   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT				   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT			   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT				   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT				   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT	   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT	   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT			   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT);
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT			   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT	   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT	   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT		   );
	C2STR(VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT         );
	}
	return "<Unknown VkDebugReportObjectTypeEXT>";
}

const char* vkutil_VkDebugReportFlagBitsEXT_to_string ( VkDebugReportFlagBitsEXT in )
{
	switch ( in )
	{
	C2STR(0);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_WARNING_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_ERROR_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	C2STR(VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT);
	}
	return "<Unknown VkDebugReportFlagBitsEXT>";
}

// This function will allocate a large block of memory for all multiple resources
// It will find a memory type compatible with all resources, will determine the size required
// and return a memory block of that size with offsets indicating where the memory is to be placed.
//
// The function will also check whether or not the memory heap is large enough for the allocation.
// Note it might still fail if not enough _FREE_ space is available.
//
// outMemoryType should be a single uint32_t where the memory type index is returned
// outMemoryOffsets should be an array of memoryRequirementCount entries for memory offsets

int32_t vkutil_multi_alloc_helper (
	VkDevice device, VkPhysicalDeviceMemoryProperties* memoryProperties, uint32_t requiredProperties,
	uint32_t memoryRequirementCount, VkMemoryRequirements* memoryRequirements,
	VkDeviceMemory* outMemory, VkDeviceSize* outMemorySize, VkDeviceSize* outMemoryOffsets
)
{
	// This portion checks which memory types are supported by all memory requirement entries.
	// The memory type bits indicate which memory types are supported by the memory requirements.
	// ANDing these together will result in excluding all memory types which are not supported
	// by all memory requirements.

	uint32_t combinedMemoryTypeBits = 0xFFFFFFFF;
	for ( uint32_t i = 0; i < memoryRequirementCount; i++ )
	{
		combinedMemoryTypeBits &= memoryRequirements[i].memoryTypeBits;
	}

	// This portion determines the total size and individual offsets of the memory. Blocks may not
	// always directly follow one another, requiring to be at a specific offset within a memory
	// block. For this purpose, we round up to the offset and add this space to the total size
	// as well as the actual memory requirements.

	VkDeviceSize totalSize = 0;
	for ( uint32_t i = 0; i < memoryRequirementCount; i++ )
	{
		VkDeviceSize offset = RVM_ALIGN_UP_POW2 ( totalSize, memoryRequirements[i].alignment );
		totalSize = offset + memoryRequirements[i].size;

		if ( outMemoryOffsets )
			outMemoryOffsets[i] = offset;
	}

	// This portion will check all heaps to see whether or not we refused the tile previously,
	// does not support the required properties, or is not large enough to contain all resources
	// we would like to stuff in.

	uint32_t typeIndex = 0xFFFFFFFF;
	for ( uint32_t i = 0; i < memoryProperties->memoryTypeCount; i++ )
	{
		if ( combinedMemoryTypeBits & (1<<i) )
		{
			uint32_t propertyFlags = memoryProperties->memoryTypes[i].propertyFlags;
			uint32_t heapIndex     = memoryProperties->memoryTypes[i].heapIndex;

			if ( (propertyFlags & requiredProperties) == requiredProperties )
			{
				if ( memoryProperties->memoryHeaps[heapIndex].size >= totalSize )
				{
					VkResult result = vkAllocateMemory (
						device,
						&(VkMemoryAllocateInfo){
							.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
							.allocationSize  = totalSize,
							.memoryTypeIndex = i,
						},
						NULL,
						outMemory
					);

					if ( result == VK_SUCCESS )
					{
						if ( outMemorySize )
							*outMemorySize = totalSize;

						// All checks out, success is achieved
						return 0;
					}
				}
			}
		}
	}

	// Either we did not find a compatible memory type, the memory type was not large enough
	// or the allocations for compatible memory types all failed for miscellaneous reasons

	return -1;
}


// TODO(Rick): Error handling
int32_t vkutil_create_images_helper (
	VkDevice device, VkCommandBuffer stagingCommandBuffer, VkQueue stagingQueue,
	VkPhysicalDeviceMemoryProperties* memoryProperties,
	uint32_t imageCount, vkutil_image_desc* imageDescs, VkDeviceMemory* outMemory
)
{
	// First allocate enough memory requirement structures on the stack for all images

	VkMemoryRequirements* imageRequirements = alloca ( imageCount * sizeof ( VkMemoryRequirements ) );

	// Create all images and get their individual memory requirements. The initial layout has to
	// be _UNDEFINED on creation, though we will take the layout the user specified for use later.
	// Usage flags are set to upload data from the CPU, and - if mipmapping is to be applied -
	// to copy to itself.

	// The total size for the staging buffer is also calculated. This staging buffer will be
	// required for uploading the data to the device memory.

	VkResult result;
	uint32_t totalStagingBufferPixelSize = 0;
	for ( uint32_t i = 0; i < imageCount; i++ )
	{
		VkImageCreateInfo info = *imageDescs[i].createInfo;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		info.usage        |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if ( imageDescs[i].mipMode == VKUTIL_IMAGE_MIPMAP_GENERATE )
			info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		result = vkCreateImage ( device, &info, NULL, imageDescs[i].outImage );

		
		if ( imageDescs[i].initialData != NULL )
			totalStagingBufferPixelSize +=
				imageDescs[i].createInfo->extent.width * imageDescs[i].createInfo->extent.height * 4;
		vkGetImageMemoryRequirements ( device, *imageDescs[i].outImage, &imageRequirements[i] );
	}

	// Now we allocate the memory for use by the images and the staging buffer separately.
	// These will likely have to be in separate memory types, as dedicated GPUs have
	// separate heaps for GPU operations and for upload operations, as opposed to integrated GPUs
	// which commonly use one large heap for the same purpose.

	VkDeviceMemory imageMemory;
	VkDeviceSize* imageMemoryOffsets = alloca ( imageCount * sizeof ( VkDeviceSize ) );
	VkDeviceSize imageMemorySize, stagingMemorySize;

	if ( vkutil_multi_alloc_helper (
			device, memoryProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			imageCount, imageRequirements, &imageMemory, &imageMemorySize, imageMemoryOffsets
		) != 0 )
		return -1;

	*outMemory = imageMemory;

	// We now bind the memory we allocated to the staging buffer and the individual images.
	// Since all images are similarly allocated from the same block of memory, but will need to
	// be placed at specific offets, we use the offsets returned by the allocation helper.

	for ( uint32_t i = 0; i < imageCount; i++ )
	{
		vkBindImageMemory ( device, *imageDescs[i].outImage, imageMemory, imageMemoryOffsets[i] );
	}

	for ( uint32_t i = 0; i < imageCount; i++ )
	{
		for ( uint32_t j = 0; j < imageDescs[i].imageViewCount; j++ )
		{
			VkImageViewCreateInfo createInfo = *imageDescs[i].imageViews[j].createInfo;
			createInfo.image = *imageDescs[i].outImage;

			vkCreateImageView (
				device,
				&createInfo,
				NULL,
				imageDescs[i].imageViews[j].outImageView
			);
		}
	}

	
	if ( stagingCommandBuffer == VK_NULL_HANDLE || stagingQueue == VK_NULL_HANDLE )
		return -1;

	VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
	VkBuffer stagingBuffer = VK_NULL_HANDLE;

	// At this time, we did all we could do on the CPU side: It is time to take it over to the
	// GPU/Driver to get the data to the place we would like it to be

	vkBeginCommandBuffer (
		stagingCommandBuffer,
		&(VkCommandBufferBeginInfo) {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		}
	);
	{	
		if ( totalStagingBufferPixelSize )
		{
			// Now we create the aforementioned staging buffer and calculate its memory requirements

			result = vkCreateBuffer (
				device,
				&(VkBufferCreateInfo){
					.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
					.size  = totalStagingBufferPixelSize,
					.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				},
				NULL,
				&stagingBuffer
			);

			VkMemoryRequirements stagingBufferRequirements;
			vkGetBufferMemoryRequirements ( device, stagingBuffer, &stagingBufferRequirements );

			if ( vkutil_multi_alloc_helper (
					device, memoryProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
					1, &stagingBufferRequirements, &stagingMemory, &stagingMemorySize, NULL
				) != 0 )
				return -1;

			// Now we fill the staging buffer. We map the staging buffer memory to a virtual pointer to
			// allow us to copy the data into the memory and unbind to not linger this memory.

			// All images are located into the same block of memory of the staging buffer, right after
			// one another to minimize space requirements.

			uint32_t* pixels;
			vkMapMemory ( device, stagingMemory, 0, totalStagingBufferPixelSize, 0, &pixels );

			for ( uint32_t i = 0; i < imageCount; i++ )
			{
				if ( imageDescs[i].initialData == NULL )
					continue;

				uint32_t size = imageDescs[i].createInfo->extent.width * imageDescs[i].createInfo->extent.height;
				memcpy ( pixels, imageDescs[i].initialData, size * 4 );
				pixels += size;
			}

			vkUnmapMemory ( device, stagingMemory );

			vkBindBufferMemory ( device, stagingBuffer, stagingMemory, 0 );
		}

		// First, we need to transition images from their _UNDEFINED layout to a layout we can
		// actually make use of. Since we are to transfer the texture data to the images,
		// we pick VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, as this is the layout intended for
		// being the target of copy operations.

		// The only access requirement is writing from the transfer engine.

		VkImageMemoryBarrier* imgBarriers = alloca ( imageCount * sizeof ( VkImageMemoryBarrier ) );

		for ( uint32_t i = 0; i < imageCount; i++ )
		{
			imgBarriers[i] = (VkImageMemoryBarrier){
				.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask    = 0,
				.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
				.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.image            = *imageDescs[i].outImage,
				.subresourceRange = {
					.aspectMask = imageDescs[i].aspectMask,
					.levelCount = imageDescs[i].createInfo->mipLevels,
					.layerCount = imageDescs[i].createInfo->arrayLayers,
				},
			};
		}

		vkCmdPipelineBarrier (
			stagingCommandBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, NULL,
			0, NULL,
			imageCount, imgBarriers
		);

		// At this point, we copy over the individual portions of the staging buffer to the images.

		VkDeviceSize offset = 0;
		for ( uint32_t i = 0; i < imageCount; i++ )
		{
			if ( imageDescs[i].initialData == NULL )
				continue;

			uint32_t width  = imageDescs[i].createInfo->extent.width;
			uint32_t height = imageDescs[i].createInfo->extent.height;

			vkCmdCopyBufferToImage  (
				stagingCommandBuffer, stagingBuffer, *imageDescs[i].outImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, (VkBufferImageCopy[1]){
					{
						.bufferOffset      = offset,
						.bufferRowLength   = width,
						.bufferImageHeight = height,
						.imageSubresource  = {
							.aspectMask = imageDescs[i].aspectMask,
							.mipLevel   = 0,
							.layerCount = 1,
						},
						.imageExtent = { width, height, 1 }
					}
				} );
			offset += width * height * 4;
		}

		// If we are to generate mipmaps, we use vkCmdBlitImage to copy over scaled versions of
		// the image. We do this until we've either reached mipLevels specified in the CreateImage
		// call, or until we've reached the mip of 1x1 pixels

		for ( uint32_t i = 0; i < imageCount; i++ )
		{
			if ( imageDescs[i].initialData != NULL
				&& imageDescs[i].mipMode == VKUTIL_IMAGE_MIPMAP_GENERATE )
			{
				uint32_t width  = imageDescs[i].createInfo->extent.width;
				uint32_t height = imageDescs[i].createInfo->extent.height;

				for ( uint32_t j = 1;
					j < imageDescs[i].createInfo->mipLevels && (width > 1 || height > 1);
					j++ )
				{
					uint32_t newWidth = RVM_MAX ( 1, width / 2), newHeight = RVM_MAX ( 1, height / 2 );

					vkCmdBlitImage (
						stagingCommandBuffer,
						*imageDescs[i].outImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						*imageDescs[i].outImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1, (VkImageBlit[1]) {
							{
								.srcSubresource = {
									.aspectMask = imageDescs[i].aspectMask,
									.mipLevel   = j-1,
									.layerCount = 1,
								},
								.srcOffsets[1] = { width, height, 1 },
								.dstSubresource = {
									.aspectMask = imageDescs[i].aspectMask,
									.mipLevel   = j,
									.layerCount = 1,
								},
								.dstOffsets[1] = { newWidth, newHeight, 1 },
							}
						}, VK_FILTER_LINEAR
					);

					vkCmdPipelineBarrier (
						stagingCommandBuffer,
						VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						0,
						0, NULL,
						0, NULL,
						1, (VkImageMemoryBarrier[1]){
							{
								.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
								.srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
								.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
								.oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								.image            = *imageDescs[i].outImage,
								.subresourceRange = {
									.aspectMask = imageDescs[i].aspectMask,
									.levelCount = imageDescs[i].createInfo->mipLevels,
									.layerCount = 1,
								},
							}
						}
					);

					width = newWidth, height = newHeight;
				}
			}
		}

		// Now that we're done transferring the data to the images, we can transition the layout
		// to be optimal for what the user is going to use the data for rather than for being
		// targets of copy operations.

		for ( uint32_t i = 0; i < imageCount; i++ )
		{
			imgBarriers[i] = (VkImageMemoryBarrier){
				.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT,
				.dstAccessMask    = imageDescs[i].accessMask,
				.oldLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.newLayout        = imageDescs[i].createInfo->initialLayout,
				.image            = *imageDescs[i].outImage,
				.subresourceRange = {
					.aspectMask = imageDescs[i].aspectMask,
					.levelCount = imageDescs[i].createInfo->mipLevels,
					.layerCount = imageDescs[i].createInfo->arrayLayers,
				},
			};
		}

		vkCmdPipelineBarrier (
			stagingCommandBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, NULL,
			0, NULL,
			imageCount, imgBarriers
		);
	}

	// We now end the command buffer and immediately submit the command buffer for execution on
	// the staging queue.

	vkEndCommandBuffer ( stagingCommandBuffer );

	vkQueueSubmit (
		stagingQueue,
		1,
		&(VkSubmitInfo){
			.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.pWaitDstStageMask  = (VkPipelineStageFlags[1]){ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT },
			.commandBufferCount = 1,
			.pCommandBuffers    = (VkCommandBuffer[1]){ stagingCommandBuffer },
		},
		VK_NULL_HANDLE
	);

	// We wait for the entire staging queue up to this point to complete. This is far, far from
	// optimal performance-wise, but it allows us to deallocate the temporary staging buffer
	// we allocated earlier.

	// Ideally the staging queue would be separate from the render queue, and the staging
	// resources would be managed by the application to just be deallocated a couple of frames
	// later. This would allow the command buffer to wait on the GPU for the upload to complete
	// and the memory to be deallocated. But for the purposes of demonstration, we take pretty
	// large shortcuts here.

	vkQueueWaitIdle ( stagingQueue );

	if ( stagingBuffer != VK_NULL_HANDLE )
	{
		// Just a little cleanup, then we're done here

		vkDestroyBuffer ( device, stagingBuffer, NULL );
		vkFreeMemory ( device, stagingMemory, NULL );
	}

	

	return 0;
}

int32_t vkutil_load_bobj (
	vkutil_model_t* model,
	VkDevice device, const void* bobjData, uint64_t bobjLen,
	VkPhysicalDeviceMemoryProperties* memoryProperties,
	VkQueue stagingQueue, VkCommandBuffer stagingCommandBuffer
)
{
	// Load the file first of all

	// The BOBJ file starts with a header telling us what is in the file and where it is

	bobj_file_header* fhead = (bobj_file_header*)bobjData;

	// From this, we can now get the location of other structures

	bobj_object_header* objects   = (bobj_object_header* )((uint8_t*)bobjData + fhead->objectsStart );
	bobj_texture_header* textures = (bobj_texture_header*)((uint8_t*)bobjData + fhead->texturesStart);
	bobj_vert* vertices           = (bobj_vert*          )((uint8_t*)bobjData + fhead->vertexStart  );
	bobj_index* indices           = (bobj_index*         )((uint8_t*)bobjData + fhead->indexStart   );
	uint8_t* texdata              = (uint8_t*            )((uint8_t*)bobjData + fhead->texdataStart );

	*model = (vkutil_model_t){
		.objectCount  = fhead->objCount,
		.textureCount = fhead->texCount,
		.objects      = malloc ( fhead->objCount * sizeof ( vkutil_object_t )
			+ fhead->texCount * (sizeof ( VkImage )+sizeof ( VkImageView )) ),
	};

	model->images       = (VkImage*    )(model->objects + fhead->objCount);
	model->imageViews   = (VkImageView*)(model->images  + fhead->texCount);

	VkImageCreateInfo* imageCreateInfo =
		alloca ( fhead->texCount * sizeof ( VkImageCreateInfo ) );
	VkImageViewCreateInfo* imageViewCreateInfo =
		alloca ( fhead->texCount * sizeof ( VkImageViewCreateInfo ) );
	vkutil_image_view_desc* imageViewDescs =
		alloca ( fhead->texCount * sizeof ( vkutil_image_view_desc ) );
	vkutil_image_desc* imageDescs      = alloca ( fhead->texCount * sizeof ( vkutil_image_desc ) );
	VkImage* images                    = alloca ( fhead->texCount * sizeof ( VkImage ) );

	// Create all textures of the object

	for ( uint32_t i = 0; i < fhead->texCount; i++ )
	{
		uint32_t mipLevels = 1;
		uint32_t w = textures[i].width, h = textures[i].height;
		while ( w > 1 || h > 1 )
			mipLevels++, w /= 2, h /= 2;

		imageCreateInfo[i] = (VkImageCreateInfo){
			.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType     = VK_IMAGE_TYPE_2D,
			.format        = VK_FORMAT_R8G8B8A8_SRGB,
			.extent        = { textures[i].width, textures[i].height, 1 },
			.mipLevels     = mipLevels,
			.arrayLayers   = 1,
			.samples       = VK_SAMPLE_COUNT_1_BIT,
			.tiling        = VK_IMAGE_TILING_OPTIMAL,
			.usage         = VK_IMAGE_USAGE_SAMPLED_BIT,
			.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};
		imageViewCreateInfo[i] = (VkImageViewCreateInfo){
			.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.viewType         = VK_IMAGE_VIEW_TYPE_2D,
			.format           = VK_FORMAT_R8G8B8A8_SRGB,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.levelCount = imageCreateInfo[i].mipLevels,
				.layerCount = 1,
				.baseMipLevel = 0,
			}
		};
		imageViewDescs[i] = (vkutil_image_view_desc){
			.outImageView = &model->imageViews[i],
			.createInfo   = &imageViewCreateInfo[i],
		};
		imageDescs[i] = (vkutil_image_desc){
			.outImage   = &model->images[i],
			.initialData= texdata + textures[i].offset,
			.createInfo = &imageCreateInfo[i],
			.mipMode    = VKUTIL_IMAGE_MIPMAP_GENERATE,
			.accessMask = VK_ACCESS_SHADER_READ_BIT,
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.imageViewCount = 1, .imageViews = &imageViewDescs[i],
		};
	}
	if ( fhead->texCount > 0 )
	{
		vkutil_create_images_helper (
			device, stagingCommandBuffer, stagingQueue, memoryProperties, fhead->texCount,
			imageDescs, &model->imageMemory
		);
	}

	// Create all internal object descriptors

	VkResult result;
	
	for ( uint32_t i = 0; i < fhead->objCount; i++ )
	{
		model->objects[i] = (vkutil_object_t){
			.indexStart   = objects[i].indexOffset,
			.indexCount   = objects[i].indexCount,
			.textureIndex = objects[i].textureIndex,
		};

		for ( uint32_t j = 0; j < 3; j++ )
		{
			model->objects[i].aabbMin[j] = objects[i].aabbMin[j];
			model->objects[i].aabbMax[j] = objects[i].aabbMax[j];
		}
	}
	
	// Now we create two buffers. No matter the amount of objects contained within this object
	// model, all vertex data is shared. The other objects will simply have offset indices when
	// other vertex data is required. This allows less overhead in switching the buffers and
	// less overhead in terms of memory alignment etc
	
	uint32_t vbSize = fhead->vertexCount * sizeof ( bobj_vert ),
		ibSize = fhead->indexCount * sizeof ( bobj_index );

	result = vkCreateBuffer (
		device,
		&(VkBufferCreateInfo){
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size  = vbSize,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		},
		NULL,
		&model->vertexBuffer
	);
	
	result = vkCreateBuffer (
		device,
		&(VkBufferCreateInfo){
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size  = ibSize,
			.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		},
		NULL,
		&model->indexBuffer
	);

	// On top of the two former buffers, we will create one more buffer: While we could leave this
	// one out, we need to have a buffer we can upload our data to. This buffer needs to be in
	// HOST_VISIBLE memory, but this memory may not be optimal for GPU access. For this reason,
	// we create a "staging" buffer to send the vertex data to, and subsequently copy the data over
	// to the actual buffers

	VkBuffer stagingBuffer;
	result = vkCreateBuffer (
		device,
		&(VkBufferCreateInfo){
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size  = vbSize + ibSize,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		},
		NULL,
		&stagingBuffer
	);

	// Allocate memory large enough for the buffer object
	
	VkMemoryRequirements stagingRequirements;
	VkDeviceSize stagingMemorySize;
	VkDeviceMemory stagingMemory;

	vkGetBufferMemoryRequirements ( device, stagingBuffer, &stagingRequirements );
	vkutil_multi_alloc_helper (
		device, memoryProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		1, &stagingRequirements, &stagingMemory, &stagingMemorySize, NULL
	);
	
	// Memory acquired by vkAllocateMemory can be written to using vkMapMemory, assuming the heap
	// the memory is allocated on is HOST_VISIBLE.
	
	// Usually, the returned memory pointer is best to be kept around for as short as possible.
	// While the memory could remain mapped for the duration of the application, and simply changed
	// on-the-fly, it can be easy to forget this memory is generally not very well optimized for
	// CPU usage, and may result in unnecessary slowdowns. If the programmer properly manages
	// the memory, however, keeping the memory mapped can allow frequently or consistently
	// updated memory to be updated with as little cost as possible.
	
	// Really though, we aren't even using the appropriate memory for our purposes, so let's not
	// faff about with keeping this memory mapped. We literally have no use for doing so at this
	// point.
	
	uint8_t* data;
	result = vkMapMemory ( device, stagingMemory, 0, stagingMemorySize, 0, &data );
	
	// We will now copy our data in the returned data pointer.
	// It is worth pointing out that this memory should - for as much as possible -
	// be _EXCLUSIVELY WRITTEN TO_ in _SEQUENTIAL_ fashion. This memory may be what is called
	// "write-combine" memory, which has severe performance penalties for any read- or
	// non-sequential write operation. Which is the reason for the previous comment.
	
	// The _HOST_CACHED bit in memory properties can indicate it is safe to non-sequentially write
	// and read the memory returned by this function. This is mostly useful for data to be
	// transferred from the GPU to the CPU. For the purposes of writing, it is not beneficial
	// (for some platforms even slightly detrimental!) to use such memory for writing _TO_ the
	// GPU.
	
#if !VKUTIL_BOBJ_FLIP_TEXCOORD_V
	memcpy ( data,              vertices, fhead->vertexCount * sizeof ( bobj_vert ) );
#else
	// We could flip the V axis after copy, not requiring copies
	// But depending on the implementation this can be a catastrophic performance hit as suggested
	// above. Feel free to check, but I would really suggest leaving this as it is.
	// Timings for my machine loading sponza.bobj are included in comments

#if 1
	// 9 ms (debug), 4 ms (release)
	for ( uint32_t i = 0; i < fhead->vertexCount; i++ )
	{
		bobj_vert v = vertices[i];
		v.texcoord[1] = 1.0f - v.texcoord[1];
		((bobj_vert*)data)[i] = v;
	}
#else
	// 40 ms (debug), 26 ms (release) (>4x & >6x resp)
	memcpy ( data,              vertices, fhead->vertexCount * sizeof ( bobj_vert ) );
	for ( uint32_t i = 0; i < fhead->vertexCount; i++ )
	{
		((bobj_vert*)data)[i].texcoord[1] = 1.0f - ((bobj_vert*)data)[i].texcoord[1];
	}
#endif
#endif
	memcpy ( data + vbSize, indices,  fhead->indexCount * sizeof ( bobj_index ) );
	
	// We can now return the memory obtained to Vulkan, as we will never actually access it again.
	// Mapping operations can fail if too much memory is mapped, so do not linger mapped memory.
	// In addition, note that not all of our write operations may be immediately seen by the GPU:
	// If the memory has the _HOST_COHERENT bit set, the memory is located in the host memory and
	// only _CACHED_ on the GPU. This can result in the GPU in using outdated information.
	
	// While command buffer submissions automatically flush these caches, calls to
	// vkFlushMappedMemoryRanges may sometimes be required.
	
	vkUnmapMemory ( device, stagingMemory );

	// We would like to allocate one batch of memory in which both the vertex and index buffers
	// can be contained. To do so, we will first need to query the requirements of both buffers,
	// listing the required size, alignment and compatible memory heaps.
	
	VkMemoryRequirements requirements[2];
	VkDeviceSize offsets[2];
	vkGetBufferMemoryRequirements ( device, model->vertexBuffer, &requirements[0] );
	vkGetBufferMemoryRequirements ( device, model->indexBuffer, &requirements[1] );

	VkDeviceSize bufferMemorySize;
	vkutil_multi_alloc_helper (
		device, memoryProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		2, requirements, &model->vbIbMemory, &bufferMemorySize, offsets
	);
	
	// We will now finally bind the memory to the appropriate buffers in order to ensure they
	// can actually use the data we just passed.
	
	result = vkBindBufferMemory ( device, stagingBuffer, stagingMemory, 0 );
	result = vkBindBufferMemory ( device, model->vertexBuffer, model->vbIbMemory, offsets[0] );
	result = vkBindBufferMemory ( device, model->indexBuffer,  model->vbIbMemory, offsets[1] );

	// We will now record the instructions to pass the data from the staging buffer to the vertex-
	// and index buffer safely

	vkBeginCommandBuffer (
		stagingCommandBuffer,
		&(VkCommandBufferBeginInfo){
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		}
	);

	{
		// We want to read from the staging buffer, and write to the other two buffers
		// So we explicitly enable these features - and only these features - for the time being

		vkCmdPipelineBarrier (
			stagingCommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, NULL,
			3, (VkBufferMemoryBarrier[3]){
				{
					.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					.srcAccessMask = 0,
					.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
					.buffer        = stagingBuffer,
					.offset        = 0,
					.size          = vbSize + ibSize,
				},
				{
					.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					.srcAccessMask = 0,
					.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.buffer        = model->vertexBuffer,
					.offset        = 0,
					.size          = vbSize,
				},
				{
					.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					.srcAccessMask = 0,
					.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.buffer        = model->indexBuffer,
					.offset        = 0,
					.size          = ibSize,
				},
			},
			0, NULL
		);

		// Now we copy over the data

		vkCmdCopyBuffer (
			stagingCommandBuffer, stagingBuffer, model->vertexBuffer,
			1, (VkBufferCopy[1]){
				{
					.srcOffset = offsets[0],
					.dstOffset = 0,
					.size      = vbSize,
				},
			}
		);
		vkCmdCopyBuffer (
			stagingCommandBuffer, stagingBuffer, model->indexBuffer,
			1, (VkBufferCopy[1]){
				{
					.srcOffset = offsets[1],
					.dstOffset = 0,
					.size      = ibSize,
				},
			}
		);

		// And we transition to a state where the buffers can be used for their respective
		// purposes

		vkCmdPipelineBarrier (
			stagingCommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, NULL,
			2, (VkBufferMemoryBarrier[2]){
				{
					.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
					.buffer        = model->vertexBuffer,
					.offset        = 0,
					.size          = vbSize,
				},
				{
					.sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
					.dstAccessMask = VK_ACCESS_INDEX_READ_BIT,
					.buffer        = model->indexBuffer,
					.offset        = 0,
					.size          = ibSize,
				},
			},
			0, NULL
		);
	}

	vkEndCommandBuffer ( stagingCommandBuffer );

	// Submit the command buffer and wait until it is done

	vkQueueSubmit (
		stagingQueue,
		1, (VkSubmitInfo[1]) {
			{
				.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount   = 1,
				.pCommandBuffers      = (VkCommandBuffer[1]){ stagingCommandBuffer },
			},
		},
		VK_NULL_HANDLE
	);
	vkQueueWaitIdle ( stagingQueue );

	// The staging buffer is no longer required, and therefore deleted

	vkDestroyBuffer ( device, stagingBuffer, NULL );
	vkFreeMemory ( device, stagingMemory, NULL );

	// TODO

	return 0;
}

int32_t vkutil_destroy_bobj ( vkutil_model_t* model, VkDevice device )
{
	for ( uint32_t i = 0; i < model->textureCount; i++ )
		vkDestroyImageView ( device, model->imageViews[i], NULL );
	for ( uint32_t i = 0; i < model->textureCount; i++ )
		vkDestroyImage ( device, model->images[i], NULL );
	vkDestroyBuffer ( device, model->vertexBuffer, NULL );
	vkDestroyBuffer ( device, model->indexBuffer, NULL );
	vkFreeMemory ( device, model->vbIbMemory, NULL );
	vkFreeMemory ( device, model->imageMemory, NULL );
	free ( model->objects );
	return 0;
}