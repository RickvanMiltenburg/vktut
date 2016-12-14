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

#define RVM_MATH_IMPLEMENTATION
#include "../vkbase.h"
#include "../vkutil.h"
#include "../../../mconv/include/mconv.h"
#include "../rvm_math.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

////////////////////////////////////////
// Function index. Both for the compiler and for the user. F12-ahoy!

int32_t app_init_command_infrastructure ( app_t* app );
int32_t app_destroy_command_infrastructure ( app_t* app );

int32_t app_init_swapchain_image_layouts ( app_t* app );

int32_t app_init_descriptor_sets ( app_t* app );
int32_t app_destroy_descriptor_sets ( app_t* app );

int32_t app_init_renderpass ( app_t* device );
int32_t app_destroy_renderpass ( app_t* app );

int32_t app_init_static_resources ( app_t* app );
int32_t app_destroy_static_resources ( app_t* app );

int32_t app_init_graphics_pipeline_prerequisites ( app_t* app );
int32_t app_destroy_graphics_pipeline_prerequisites ( app_t* app );

int32_t app_init_graphics_pipelines ( app_t* app, uint32_t windowWidth, uint32_t windowHeight );
int32_t app_destroy_graphics_pipelines ( app_t* app );

int32_t app_init_renderpass_framebuffers ( app_t* app, uint32_t windowWidth, uint32_t windowHeight );
int32_t app_destroy_renderpass_framebuffers ( app_t* app );

int32_t app_init_render_attachments ( app_t* app, uint32_t windowWidth, uint32_t windowHeight );
int32_t app_destroy_render_attachments ( app_t* app );

////////////////////////////////////////
// Settings-ish

#define RENDER_COMMAND_BUFFER_COUNT 3
#define MAX_LIGHTS                  16
#define SHADOW_MAP_WIDTH            1024
#define SHADOW_MAP_HEIGHT           1024

typedef struct light_s
{
	rvm_aos_vec3 pos;
	rvm_aos_vec3 initialRotation;
	rvm_aos_vec3 rotSpeed;
	rvm_aos_vec3 attenuation;	// Constant, Linear, Quadratic
	float fovOuter, fovInner;
} light_t;

light_t LIGHTS[] = {
	{
		.pos             = { 0.0f, 100.0f, 0.0f },
		.initialRotation = { 0.0f, 0.0f, 0.0f },
		.rotSpeed        = { 0.0f, 1.0f, 0.0f },
		.attenuation     = { 1.0f, 0.5f, 0.02f },
		.fovOuter        = RVM_PI/2, .fovInner = RVM_PI/8,
	},
	{
		.pos             = { 0.0f, 100.0f, 0.0f },
		.initialRotation = { 0.0f, RVM_PI/2, 0.0f },
		.rotSpeed        = { 0.0f, 1.0f, 0.0f },
		.attenuation     = { 1.0f, 0.5f, 0.02f },
		.fovOuter        = RVM_PI/2, .fovInner = RVM_PI/8,
	},
	{
		.pos             = { 0.0f, 100.0f, 0.0f },
		.initialRotation = { 0.0f, RVM_PI, 0.0f },
		.rotSpeed        = { 0.0f, 1.0f, 0.0f },
		.attenuation     = { 1.0f, 0.5f, 0.02f },
		.fovOuter        = RVM_PI/2, .fovInner = RVM_PI/8,
	},
	{
		.pos             = { 0.0f, 100.0f, 0.0f },
		.initialRotation = { 0.0f, RVM_PI + RVM_PI/2, 0.0f },
		.rotSpeed        = { 0.0f, 1.0f, 0.0f },
		.attenuation     = { 1.0f, 0.5f, 0.02f },
		.fovOuter        = RVM_PI/2, .fovInner = RVM_PI/8,
	},
};
#define LIGHT_COUNT STATIC_ARRAY_LENGTH(LIGHTS)

////////////////////////////////////////
// Enumerations

enum
{
	QUEUE_MAIN,

	QUEUE_COUNT,
};

enum
{
	SUBPASS_DEFERRED,
	SUBPASS_DEFERRED_LIGHTING,
	SUBPASS_POST,

	SUBPASS_COUNT
};

enum
{
	PIPELINE_DEFERRED,
	PIPELINE_DEFERRED_LIGHTING,
	PIPELINE_POST,

	PIPELINE_COUNT,
};

enum
{
	MODEL_TEXCUBE,

	MODEL_COUNT,
};

enum
{
	ATTACHMENT_INTERMEDIATE_GBUFFER1,
	ATTACHMENT_INTERMEDIATE_GBUFFER2,
	ATTACHMENT_INTERMEDIATE_GBUFFER3,
	
	ATTACHMENT_INTERMEDIATE_COLOR,
	ATTACHMENT_INTERMEDIATE_DEPTH,
	ATTACHMENT_FRAMEBUFFER,

	ATTACHMENT_COUNT
};

enum
{
	VERTEX_ATTRIBUTE_POSITION,
	VERTEX_ATTRIBUTE_TEXCOORD,
	VERTEX_ATTRIBUTE_NORMAL,

	VERTEX_ATTRIBUTE_COUNT,
};

enum
{
	STATIC_TEXTURE_DIFFUSE_DEFAULT,
	STATIC_TEXTURE_SHADOWMAP,

	STATIC_TEXTURE_COUNT,
};

enum
{
	TRANSIENT_ATTACHMENT_GBUFFER1,
	TRANSIENT_ATTACHMENT_GBUFFER2,
	TRANSIENT_ATTACHMENT_GBUFFER3,

	TRANSIENT_ATTACHMENT_COLOR,
	TRANSIENT_ATTACHMENT_DEPTH,

	TRANSIENT_ATTACHMENT_COUNT,
};

enum
{
	MARKER_CPU_RENDER_FENCE_WAIT,
	MARKER_CPU_RENDER_IMAGE_ACQUIRE,
	MARKER_CPU_RENDER_CB_INIT,
	MARKER_CPU_RENDER_RP_START,
	MARKER_CPU_RENDER_RP_FWD,
	MARKER_CPU_RENDER_RP_POST,
	MARKER_CPU_RENDER_CB_SUBMIT,
	MARKER_CPU_RENDER_FB_PRESENT,

	MARKER_CPU_COUNT,
};

static const char* MarkerCPUNames[MARKER_CPU_COUNT] = {
	[MARKER_CPU_RENDER_FENCE_WAIT   ] = "Waiting for fence",
	[MARKER_CPU_RENDER_IMAGE_ACQUIRE] = "Image acquire",
	[MARKER_CPU_RENDER_CB_INIT      ] = "Command buffer init",
	[MARKER_CPU_RENDER_RP_START     ] = "Renderpass start",
	[MARKER_CPU_RENDER_RP_FWD       ] = "Renderpass forward",
	[MARKER_CPU_RENDER_RP_POST      ] = "Renderpass post",
	[MARKER_CPU_RENDER_CB_SUBMIT    ] = "Command buffer submit",
	[MARKER_CPU_RENDER_FB_PRESENT   ] = "Backbuffer present",
};

enum
{
	MARKER_GPU_FORWARD,
	MARKER_GPU_POST,

	MARKER_GPU_COUNT
};

static const char* MarkerGPUNames[MARKER_GPU_COUNT] = {
	[MARKER_GPU_FORWARD] = "Forward subpass",
	[MARKER_GPU_POST   ] = "Post subpass",
};

////////////////////////////////////////
//

typedef struct render_cmd_buffer_s
{
	VkCommandBuffer commandBuffer;
	VkFence         fenceComplete;
	VkSemaphore     semaphoreBackbufferWritable;
	VkSemaphore     semaphoreComplete;
} render_cmd_buffer_t;

typedef struct forward_fs_cb_s
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!! WARNING !!!
	// This structure is to be laid out with the "std140" alignment rules in mind
	// Refer to 14.5.4 (Offset and Stride Assignment) in the specification
	// All variables below try to align to 4-word boundaries
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	rvm_aos_vec3 cameraPosition; uint32_t lightCount;
	struct
	{
		rvm_aos_mat4 shadowVp;
		rvm_aos_vec3 position;		float innerDot;
		rvm_aos_vec3 direction;		float outerDot;
		rvm_aos_vec3 color;			float _dummy2;
		rvm_aos_vec3 attenuation;	float _dummy3;
	} lights[MAX_LIGHTS];
} forward_vs_cb_t;

struct app_s
{
	// Standard vkbase objects
	window_t    window;
	instance_t  instance;
	device_t    device;
	queue_t     queues[QUEUE_COUNT];
	swapchain_t swapchain;

	// Command buffer resources for rendering
	VkCommandPool       commandPool;
	VkCommandBuffer     commandBufferStaging;
	render_cmd_buffer_t commandBufferRender[RENDER_COMMAND_BUFFER_COUNT];
	uint32_t            commandBufferRenderIndex;
	uint32_t            backbufferIndex;

	// Renderpass objects
	struct
	{
		VkRenderPass   renderPass;
		VkFramebuffer* framebuffers;
		VkPipeline     pipeline[PIPELINE_COUNT];
	} renderpass;

	struct
	{
		VkRenderPass renderPass;
		VkFramebuffer framebuffers[LIGHT_COUNT];
		VkPipeline pipeline;
	} shadowRenderpass;

	// Static resources
	struct
	{
		//
		VkBuffer lightBuffer;

		// Samplers
		VkSampler samplerAnisotropic;
		VkSampler samplerNearest;
		VkSampler samplerShadow;

		// Images
		VkImage imageDummyDiffuse;
		VkImage imageShadowArray;
		
		// Image views
		VkImageView imageViewDummyDiffuse;
		VkImageView imageViewShadowArray;
		VkImageView* imageViewShadowDepthAttachment;

		// Memory
		VkDeviceMemory imageMemory, lightBufferMemory;
	} staticResources;

	struct
	{
		VkImage imageIntermediateColor;
		VkImage imageIntermediateDepth;
		VkImage imageGBuffer[3];

		VkImageView imageViewIntermediateColor;
		VkImageView imageViewIntermediateDepth;
		VkImageView imageViewGBuffer[3];

		VkDeviceMemory memory;
	} attachments;

	// Descriptor management
	VkDescriptorPool      descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout[PIPELINE_COUNT];
	VkDescriptorSet       descriptorSet      [PIPELINE_COUNT];

	// Pipeline management
	VkPipelineCache pipelineCache;
	VkPipelineLayout pipelineLayout[PIPELINE_COUNT];
	VkPipelineLayout pipelineLayoutShadow;
	
	// Model(s)
	vkutil_model_t model[MODEL_COUNT];
	VkDescriptorSet* modelDescriptorSets;
};

////////////////////////////////////////
// Callback functions from the platform layer

int32_t app_init ( app_t** outApp, void* userdata )
{
	// Allocate an app_t object, initialize to NULL data (calloc) and return it in outApp
	app_t* app = calloc ( 1, sizeof ( app_t ) );
	*outApp = app;

	// First thing we do is create a single window

	int32_t ret = platform_window_create ( &app->window, userdata );
	if ( ret != 0 )
		return ret;

	// We then create an instance, which will also create the surface for use with the window
	// we just obtained

	ret = vkbase_init_instance ( &app->instance );
	if ( ret != 0 )
		return ret;

	// We create a single device with a single queue, where we are only interested in graphics
	// and transfer functionality

	ret = vkbase_init_device (
		&app->device, &app->instance,
		1, (window_t*[1]) { &app->window },
		QUEUE_COUNT, (queue_create_info_t[1]){
			[QUEUE_MAIN] = {
				.queueFlags            = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT,
				.presentWindowCount    = 1,
				.presentWindowIndices  = (uint32_t[1]) { 0 },
			},
		}, app->queues
	);
	if ( ret != 0 )
		return ret;

	// The swapchain - responsible for the communication between the device and the window - is then
	// created.

	ret = vkbase_init_swapchain (
		&app->swapchain, &app->instance, &app->device, &app->window, NULL
	);
	if ( ret != 0 )
		return ret;

	// We now create the objects required for passing commands onto the driver/GPU.

	ret = app_init_command_infrastructure ( app );
	if ( ret != 0 )
		return ret;

	// Images created by the swapchain need to be transitioned to the proper layout before use.
	// I have got to be honest and say I do not understand why the images are not implicitly
	// transferred to _PRESENT layout at creation, as they are supposed to be in this layout
	// when not in use by the application anyway. But... Eh, it is how it is I suppose

	ret = app_init_swapchain_image_layouts ( app );
	if ( ret != 0 )
		return ret;

	// This is just a simple process to load a binary object file into objects.
	// vkutil_load_bobj contains the functionality required to create the model object we will use
	// in the application. The platform_file_load is fundamentally pretty uninteresting.

	file_t bobjFile;
	ret = platform_file_load ( &bobjFile, "models/sponza.bobj" );
	if ( ret != 0 )
		return ret;

	ret = vkutil_load_bobj (
		&app->model[MODEL_TEXCUBE], app->device.device, bobjFile.data, bobjFile.sizeInBytes,
		&app->device.memoryProperties, app->queues[QUEUE_MAIN].queue, app->commandBufferStaging
	);
	if ( ret != 0 )
		return ret;

	ret = platform_file_close ( &bobjFile );
	if ( ret != 0 )
		return ret;

	// We now ask the platform what size window we have. While it would be nice to specify the size
	// manually in the application code, that is clearly not really how platforms like Android
	// function

	uint32_t windowWidth, windowHeight;
	ret = platform_window_get_size ( &app->window, &windowWidth, &windowHeight );
	if ( ret != 0 )
		return ret;

	// We will now initialize static resources. This being for example the default texture for
	// untextured objects. It also includes any potential attachments (render targets) we might
	// need.
	
	ret = app_init_static_resources ( app );
	if ( ret != 0 )
		return ret;

	// We now want to initialize the descriptor sets. As these are intended for texture binding,
	// we first needed to know how many textures we would have. Since this information is sneakily
	// hidden in the bobj file, it is now available, and we can start initializing the descriptor
	// sets.

	ret = app_init_descriptor_sets ( app );
	if ( ret != 0 )
		return ret;

	// TODO

	ret = app_init_render_attachments ( app, windowWidth, windowHeight );
	if ( ret != 0 )
		return ret;

	// Time to initialize the renderpass

	ret = app_init_renderpass ( app );
	if ( ret != 0 )
		return ret;

	// Since framebuffers are tied to renderpasses, only now we can actually create the
	// framebuffers we intend to use.

	ret = app_init_renderpass_framebuffers ( app, windowWidth, windowHeight );
	if ( ret != 0 )
		return ret;

	ret = app_init_graphics_pipeline_prerequisites ( app );
	if ( ret != 0 )
		return ret;

	// Create the graphics pipelines describing the actual rendering process.

	ret = app_init_graphics_pipelines ( app, windowWidth, windowHeight );
	if ( ret != 0 )
		return ret;

	return 0;
}

int32_t app_free ( app_t* app )
{
	// TODO
	for ( uint32_t i = 0; i < QUEUE_COUNT; i++ )
		vkQueueWaitIdle ( app->queues[i].queue );
	app_destroy_command_infrastructure ( app );
	app_destroy_descriptor_sets ( app );
	app_destroy_renderpass ( app );
	
	app_destroy_static_resources ( app );
	app_destroy_render_attachments ( app );
	app_destroy_graphics_pipelines ( app );
	app_destroy_graphics_pipeline_prerequisites ( app );
	app_destroy_renderpass_framebuffers ( app );

	vkutil_destroy_bobj ( &app->model[MODEL_TEXCUBE], app->device.device );
	
	vkbase_destroy_swapchain ( &app->device, &app->swapchain );
	vkbase_destroy_device ( &app->device );
	vkDestroySurfaceKHR ( app->instance.instance, app->window.surface, NULL );
	vkbase_destroy_instance ( &app->instance );

	return 0;
}

int32_t app_resize ( app_t* app, uint32_t windowWidth, uint32_t windowHeight )
{
	// Whenever the window is resized, it will be our responsibility to size all of our rendering
	// resources accordingly.

	// Resources we will need to reinitialize:
	//	- Swapchain
	//	- Backbuffer views (tied to swapchain)
	//	- Intermediate attachments (/render targets)
	//  - Graphics pipelines (*)

	// * = This can be circumvented by specifying the viewport and scissor as _dynamic_ state
	//     when creating the graphics pipeline. This is probably a good idea for your own
	//     applications, even if there might potentially be a minor performance penalty.


	// First, we will create a new swapchain. This call will also take the swapchain we previously
	// had. This indicates to the Vulkan implementation we are intending to replace that swapchain
	// with the one we are now creating.

	swapchain_t newSwapchain;
	swapchain_t oldSwapchain = app->swapchain;

	int32_t ret = vkbase_init_swapchain (
		&newSwapchain, &app->instance, &app->device, &app->window, &app->swapchain
	);

	// Before we go on: The GPU might still be executing commands we submitted during the app_render
	// function execution. So before we go on to remove elements still potentially in use by the
	// command buffer, we ensure the queue we submit all our command buffers to becomes idle for
	// the moment.

	// We could potentially skip this step if we wanted to. We could defer the destruction of old
	// objects when we know the command buffer would be done under normal circumstances (eg in
	// app_render itself) and in the meantime submit new command buffers only with the new
	// resources.
	
	// For your initial implementation, just waiting for a queue idle is a more simple, reliable
	// and clean method to ensure safe deletion of resources. It is not as though we execute this
	// function a lot anyway, only if the user resizes.

	vkQueueWaitIdle ( app->queues[QUEUE_MAIN].queue );

	// TODO 
	app_destroy_renderpass_framebuffers ( app );
	app_destroy_graphics_pipelines ( app );
	app_destroy_render_attachments ( app );
	
	// TODO(Rick): Do we need to delete image views? Cube demo seems to "forget" to do so
	vkbase_destroy_swapchain ( &app->device, &oldSwapchain );

	// With all resources related to the old swapchain deleted, and the old swapchain itself also
	// having been destroyed, we can now safely store the new swapchain as the one, true, only
	// canonical swapchain

	app->swapchain = newSwapchain;
	
	// Now we reign in our former destructive tendencies by rebuilding the infrastructure we so
	// cruelly and violently, but justifyably, destroyed.

	app_init_render_attachments ( app, windowWidth, windowHeight );
	app_init_renderpass_framebuffers ( app, windowWidth, windowHeight );
	app_init_graphics_pipelines ( app, windowWidth, windowHeight );
	app_init_swapchain_image_layouts ( app );
	
	// Now we just render a single frame and wait for the frame to complete before returning. This
	// gives the platform an opportunity to repaint the window while it is still being resized
	
	app_render ( app, 0.0 );
	vkQueueWaitIdle ( app->queues[QUEUE_MAIN].queue );

	return 0;
}

static int32_t app_util_object_visibility_check ( vkutil_object_t* obj, rvm_aos_mat4* mvp )
{
	rvm_aos_vec4 corners[8] = {
		{ obj->aabbMin[0], obj->aabbMin[1], obj->aabbMin[2], 1.0f },
		{ obj->aabbMax[0], obj->aabbMin[1], obj->aabbMin[2], 1.0f },
		{ obj->aabbMin[0], obj->aabbMax[1], obj->aabbMin[2], 1.0f },
		{ obj->aabbMax[0], obj->aabbMax[1], obj->aabbMin[2], 1.0f },

		{ obj->aabbMax[0], obj->aabbMin[1], obj->aabbMax[2], 1.0f },
		{ obj->aabbMin[0], obj->aabbMin[1], obj->aabbMax[2], 1.0f },
		{ obj->aabbMax[0], obj->aabbMax[1], obj->aabbMax[2], 1.0f },
		{ obj->aabbMin[0], obj->aabbMax[1], obj->aabbMax[2], 1.0f },
	};

	for ( uint32_t i = 0; i < 8; i++ )
		corners[i] = rvm_aos_mat4_mul_aos_vec4 ( mvp, &corners[i] );

	uint32_t cornerValidBits = 0;
	for ( uint32_t i = 0; i < 8; i++ )
	{
		if ( corners[i].w > 0.0f )
			cornerValidBits |= (1<<i);
	}

	if ( cornerValidBits == 0 )
		return 0;	// All objects definitely

	for ( uint32_t i = 0; i < 8; i++ )
	{
		corners[i].x /= corners[i].w, corners[i].y /= corners[i].w, corners[i].z /= corners[i].w;
	}

	uint8_t outcodeAND = 0xFF;
	uint8_t outcodeOR  = 0x0;
	for ( uint32_t i = 0; i < 8; i++ )
	{
		uint8_t out = 0;
		if ( corners[i].x < -1.0f )
			out |= 1;
		else if ( corners[i].x > 1.0f )
			out |= 2;
		if ( corners[i].y < -1.0f )
			out |= 4;
		else if ( corners[i].y > 1.0f )
			out |= 8;
		if ( corners[i].z < 0.0f )
			out |= 16;
		else if ( corners[i].z > 1.0f )
			out |= 32;
		outcodeAND &= out;
		outcodeOR  |= out;
	}

	if ( outcodeAND )
		return 0;	// All corners out on at least one of the sides

	// Potentially visible
	return 1;
}

static void app_util_create_rotating_vp (
	float T, float rotX, float rotY, float rotZ, float rotXSpeed, float rotYSpeed, float rotZSpeed,
	float posX, float posY, float posZ,
	float aspect, float minDepth, float maxDepth, float fov,
	rvm_aos_mat4* outV, rvm_aos_mat4* outP
)
{
	float x = rotX + T * rotXSpeed, y = rotY + T * rotYSpeed, z = rotZ + T * rotZSpeed;

	rvm_aos_mat4 rx = rvm_aos_mat4_rotate_x ( x ),
		ry = rvm_aos_mat4_rotate_y ( y ),
		rz = rvm_aos_mat4_rotate_z ( z );

	rvm_aos_mat4 _r = rvm_aos_mat4_mul_aos_mat4 ( &rx, &ry );
	rvm_aos_mat4 r = rvm_aos_mat4_mul_aos_mat4 ( &_r, &rz );

	rvm_aos_mat4 t = rvm_aos_mat4_translate ( posX, posY, posZ );

	rvm_aos_mat4 m = rvm_aos_mat4_mul_aos_mat4 ( &t, &r );
	rvm_aos_mat4 v = rvm_aos_mat4_inverse ( &m );

	rvm_aos_mat4 p = rvm_aos_mat4_perspective ( fov, aspect, minDepth, maxDepth );
	rvm_aos_mat4 pyi = rvm_aos_mat4_scale ( 1.0f, -1.0f, 1.0f );
	p = rvm_aos_mat4_mul_aos_mat4 ( &p, &pyi );

	*outV = v;
	*outP = p;
}

static void app_util_create_orbiting_vp (
	float T, float rotXSpeed, float rotYSpeed, float rotZSpeed, float offX, float offY, float offZ,
	float aspect, float minDepth, float maxDepth,
	rvm_aos_mat4* outV, rvm_aos_mat4* outP
)
{
	float x = T * rotXSpeed, y = T * rotYSpeed, z = T * rotZSpeed;

	rvm_aos_mat4 rx = rvm_aos_mat4_rotate_x ( x ),
		ry = rvm_aos_mat4_rotate_y ( y ),
		rz = rvm_aos_mat4_rotate_z ( z );

	rvm_aos_mat4 _r = rvm_aos_mat4_mul_aos_mat4 ( &rx, &ry );
	rvm_aos_mat4 r = rvm_aos_mat4_mul_aos_mat4 ( &_r, &rz );

	rvm_aos_mat4 t = rvm_aos_mat4_translate ( offX, offY, offZ );

	rvm_aos_mat4 m = rvm_aos_mat4_mul_aos_mat4 ( &r, &t );
	rvm_aos_mat4 v = rvm_aos_mat4_inverse ( &m );

	rvm_aos_mat4 p = rvm_aos_mat4_perspective ( 1.0f, aspect, minDepth, maxDepth );

	rvm_aos_mat4 pyi = rvm_aos_mat4_scale ( 1.0f, -1.0f, 1.0f );
	p = rvm_aos_mat4_mul_aos_mat4 ( &p, &pyi );

	*outV = v;
	*outP = p;
}

static void app_render_model (
	vkutil_model_t* model, rvm_aos_mat4* p, rvm_aos_mat4* v, VkCommandBuffer commandBuffer,
	VkPipelineLayout pipelineLayout, VkDescriptorSet* modelDescriptorSet,
	VkDescriptorSet dummyDescriptorSet, uint32_t dynamicOffsetCount, uint32_t* dynamicOffsets
)
{
	rvm_aos_mat4 vp = rvm_aos_mat4_mul_aos_mat4 ( p, v );

	vkCmdPushConstants (
		commandBuffer,
		pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		16 * sizeof ( float ),
		vp.cells
	);

	vkCmdPushConstants (
		commandBuffer,
		pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT,
		64,
		16 * sizeof ( float ),
		v->cells
	);

	vkCmdBindVertexBuffers (
		commandBuffer,
		0, 1,
		(VkBuffer[1]){ model->vertexBuffer },
		(VkDeviceSize[1]){ 0 }
	);

	vkCmdBindIndexBuffer (
		commandBuffer,
		model->indexBuffer,
		0,
		VK_INDEX_TYPE_UINT32
	);

	for ( uint32_t j = 0; j < model->objectCount; j++ )
	{
		vkutil_object_t* obj = &model->objects[j];

		if ( !app_util_object_visibility_check ( obj, &vp ) )
			continue;

		VkDescriptorSet descriptorSet;

		if ( obj->textureIndex == 0xFFFFFFFF || modelDescriptorSet == NULL )
			descriptorSet = dummyDescriptorSet;
		else
			descriptorSet = modelDescriptorSet[obj->textureIndex];

		if ( descriptorSet != VK_NULL_HANDLE )
		{
			vkCmdBindDescriptorSets (
				commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0, 1, (VkDescriptorSet[1]){ descriptorSet },
				dynamicOffsetCount, dynamicOffsets
			);
		}

		vkCmdDrawIndexed (
			commandBuffer,
			obj->indexCount, 1, obj->indexStart,
			0, 0
		);
	}
}

int32_t app_render ( app_t* app, double dt )
{
	platform_log_warning ( "dt: %8.02f, FPS: %8.02f\n", dt, 1.0 / dt );

	VkResult result = VK_SUCCESS;

	app->commandBufferRenderIndex = (app->commandBufferRenderIndex + 1) % RENDER_COMMAND_BUFFER_COUNT;
	render_cmd_buffer_t* renderCommandBuffer =
		&app->commandBufferRender[app->commandBufferRenderIndex];

	// TODO

	uint32_t windowWidth, windowHeight;
	platform_window_get_size ( &app->window, &windowWidth, &windowHeight );
	float aspect = windowWidth / (float)windowHeight;

	// TODO

	timestamp_t timestamp, freq;
	platform_get_timestamp ( &timestamp );
	platform_get_timestamp_freq ( &freq );
	float T = (float)(timestamp / (double)freq);

	// TODO

	rvm_aos_mat4 v, p;
#if 0
	app_util_create_rotating_vp (
		T, 1.0f, 0.5f, 0.25f, 0.0f, 0.0f, 5.0f, aspect, 0.1f, 10.0f, &v, &p
	);
#else
	//app_util_create_rotating_vp (
	//	T, 0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 0.0f, 1000.0f, 100.0f, 0.0f, aspect, 10.0f, 2500.0f, &v, &p
	//);

	app_util_create_rotating_vp (
		T, 0.0f, -RVM_PI/2, 0.0f, 0.0f, 0.0f, 0.0f, -250.0f, 100.0f, 0.0f, aspect, 10.0f, 2500.0f, 1.0f, &v, &p
	);
#endif

	//vp = rvm_aos_mat4_mul_aos_mat4 ( &p, &v );

	// Before we do any actual rendering, we will need to wait for a fence in order to ensure we
	// can do the operations we would like to do. Specifically, we would like to wait for the
	// command list we submitted the previous frame before we attempt to put more data into
	// the same command list. Doing so while the command list is still being executed is illegal,
	// as allowing it to happen would effectively mean you could change GPU instructions while it
	// is still executing its instructions.

	result = vkWaitForFences (
		app->device.device,
		1,
		(VkFence[1]){ renderCommandBuffer->fenceComplete },
		VK_TRUE,
		UINT64_MAX
	);
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkWaitForFences failed (%u)", result );

	// At a later stage we would need to wait for this fence again. In order to do so, we do need
	// to reset the fence despite already having waited for that fence.

	result = vkResetFences (
		app->device.device,
		1,
		(VkFence[1]){ renderCommandBuffer->fenceComplete }
	);
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkResetFences failed (%u)", result );

	// Now we request an unused image from the swapchain, we will need this in order to know
	// which target we are to be rendering to.

	// TODO

	result = vkAcquireNextImageKHR (
		app->device.device,
		app->swapchain.swapchain,
		UINT64_MAX,
		renderCommandBuffer->semaphoreBackbufferWritable,
		VK_NULL_HANDLE,
		&app->backbufferIndex
	);
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkAcquireNextImageKHR failed (%u)", result );

	// TODO

	uint32_t lightBufferOffset = app->commandBufferRenderIndex * RVM_ALIGN_UP_POW2 (
			sizeof ( forward_vs_cb_t ),
			app->device.properties.limits.minUniformBufferOffsetAlignment
		);

	forward_vs_cb_t* lightData = NULL;
	vkMapMemory (
		app->device.device, app->staticResources.lightBufferMemory, lightBufferOffset,
		sizeof ( forward_vs_cb_t ), 0, &lightData
	);

	lightData->lightCount     = LIGHT_COUNT;
	lightData->cameraPosition = (rvm_aos_vec3){ 1000.0f, 100.0f, 0.0f };
	for ( uint32_t i = 0; i < LIGHT_COUNT; i++ )
	{
		rvm_aos_mat4 shadowV, shadowP;
		app_util_create_rotating_vp (
			T,
			LIGHTS[i].initialRotation.x, LIGHTS[i].initialRotation.y, LIGHTS[i].initialRotation.z,
			LIGHTS[i].rotSpeed.x, LIGHTS[i].rotSpeed.y, LIGHTS[i].rotSpeed.z,
			LIGHTS[i].pos.x, LIGHTS[i].pos.y, LIGHTS[i].pos.z,
			1.0f, 1.0f, 2500.0f, LIGHTS[i].fovOuter,
			&shadowV, &shadowP
		);

		lightData->lights[i].shadowVp    = rvm_aos_mat4_mul_aos_mat4 ( &shadowP, &shadowV );
		lightData->lights[i].position    = LIGHTS[i].pos;
		lightData->lights[i].direction   = rvm_aos_mat4_mul_aos_vec3w0 ( 
			&lightData->lights[i].shadowVp, &(rvm_aos_vec3){ 0.0f, 0.0f, 1.0f }
		);
		lightData->lights[i].color       = (rvm_aos_vec3){ 1.0f, 1.0f, 1.0f };
		lightData->lights[i].attenuation = LIGHTS[i].attenuation;
		lightData->lights[i].outerDot    = cosf ( LIGHTS[i].fovOuter / 2.0f );
		lightData->lights[i].innerDot    = cosf ( LIGHTS[i].fovInner / 2.0f );
	}

	vkUnmapMemory ( app->device.device, app->staticResources.lightBufferMemory );

	// Begin the command buffer
	result = vkBeginCommandBuffer (
		renderCommandBuffer->commandBuffer,
		&(VkCommandBufferBeginInfo){
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		}
	);
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkWaitForFences failed (%u)", result );

	{
		for ( uint32_t i = 0; i < LIGHT_COUNT; i++ )
		{
			vkCmdBeginRenderPass (
				renderCommandBuffer->commandBuffer,
				&(VkRenderPassBeginInfo){
					.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.pNext           = NULL,
					.renderPass      = app->shadowRenderpass.renderPass,
					.framebuffer     = app->shadowRenderpass.framebuffers[i],
					.renderArea      = { .extent = { SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT } },
					.clearValueCount = 1,
					.pClearValues    = (VkClearValue[1]){
						[0] = { .depthStencil  = { 1.0f, 0 } },
					},
				},
				VK_SUBPASS_CONTENTS_INLINE
			);

			{
				rvm_aos_mat4 shadowV, shadowP;
				app_util_create_rotating_vp (
					T,
					LIGHTS[i].initialRotation.x, LIGHTS[i].initialRotation.y, LIGHTS[i].initialRotation.z,
					LIGHTS[i].rotSpeed.x, LIGHTS[i].rotSpeed.y, LIGHTS[i].rotSpeed.z,
					LIGHTS[i].pos.x, LIGHTS[i].pos.y, LIGHTS[i].pos.z,
					1.0f, 1.0f, 2500.0f, LIGHTS[i].fovOuter,
					&shadowV, &shadowP
				);
	
				vkCmdBindPipeline (
					renderCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					app->shadowRenderpass.pipeline
				);
			
				{
					for ( uint32_t i = 0; i < MODEL_COUNT; i++ )
					{
						app_render_model (
							&app->model[i], &shadowP, &shadowV, renderCommandBuffer->commandBuffer,
							app->pipelineLayoutShadow, NULL,
							VK_NULL_HANDLE, 0, NULL
						);
					}
				}
			}
	
			vkCmdEndRenderPass ( renderCommandBuffer->commandBuffer );
		}

		// At this point, we can begin with our renderpass. The renderpass will begin from subpass 0
		// and only advance to the next subpass upon calling vkCmdNextSubpass. In our current scenario
		// we have a forward render pass in the first subpass, and some post processing in the second
		// subpass.

		// We specify a "render area" of the entire screen. Note that the render area different from
		// both the viewport and the scissor. The render area specifies the area in which we will
		// actually be rendering for the entire renderpass. It has no actual effect on any render
		// operations, but can affect load/store operations of the renderpass when rendering outside
		// of this area depending upon the implementation.

		// The "clear values" apply to all attachments with a load op of "VK_ATTACHMENT_LOAD_OP_CLEAR".
		// In our case, both the color buffer and the depth buffer have this setting, and as such
		// we specify 2 clear values. They specify implicit clearing opportunity without explicitly
		// calling a clear function, allowing the implementation to deduce the cheapest way to ensure
		// previous data is cleared.

		// For the subpasses we specify "inline" contents, meaning we are going to specify the subpass
		// in the same command buffer. With the alternative "external command buffer" option, we can
		// create the commands for the other subpass at a different time or in a different thread, and
		// tell this command buffer to merely call the command buffer on that subpass.

		static float x = 0.0f, y = 0.524f, z = 1.57f;
		x += (float)dt * 3.14f, y += (float)dt * 3.14f, z += (float)dt * 3.14f;

		vkCmdBeginRenderPass (
			renderCommandBuffer->commandBuffer,
			&(VkRenderPassBeginInfo){
				.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.pNext           = NULL,
				.renderPass      = app->renderpass.renderPass,
				.framebuffer     = app->renderpass.framebuffers[app->backbufferIndex],
				.renderArea      = { .offset = { 0, 0 }, .extent = { windowWidth, windowHeight } },
				.clearValueCount = 5,
				.pClearValues    = (VkClearValue[5]){
					[ATTACHMENT_INTERMEDIATE_GBUFFER1] = { .color.float32 = { 0, 0, 0, 0 } },
					[ATTACHMENT_INTERMEDIATE_GBUFFER2] = { .color.float32 = { 0, 0, 0, 0 } },
					[ATTACHMENT_INTERMEDIATE_GBUFFER3] = { .color.float32 = { 0, 0, 0, 0 } },
					[ATTACHMENT_INTERMEDIATE_COLOR]    = { .color.float32 = { 0, 0, 0, 0 } },
					[ATTACHMENT_INTERMEDIATE_DEPTH]    = { .depthStencil  = { 1.0f, 0 } },
				},
			},
			VK_SUBPASS_CONTENTS_INLINE
		);

		{
			// When we want to render in the subpass, we do need to specify the graphics pipeline.
			// Since the pipeline is dependent upon the renderpass and subpass, this can only be
			// set in the when the subpass is active.
	
			vkCmdBindPipeline (
				renderCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				app->renderpass.pipeline[PIPELINE_DEFERRED]
			);
			
			{
				for ( uint32_t i = 0; i < MODEL_COUNT; i++ )
				{
					app_render_model (
						&app->model[i], &p, &v, renderCommandBuffer->commandBuffer,
						app->pipelineLayout[PIPELINE_DEFERRED], app->modelDescriptorSets,
						app->descriptorSet[PIPELINE_DEFERRED], 0, NULL
					);
				}
			}
		}
	
		// Now that we have completed the forward subpass, we will move onto the post-processing
		// subpass.
	
		vkCmdNextSubpass ( renderCommandBuffer->commandBuffer, VK_SUBPASS_CONTENTS_INLINE );
	
		{
			vkCmdBindPipeline (
				renderCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				app->renderpass.pipeline[PIPELINE_DEFERRED_LIGHTING]
			);
	
			vkCmdPushConstants (
				renderCommandBuffer->commandBuffer,
				app->pipelineLayout[PIPELINE_POST],
				VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				1 * sizeof ( float ),
				(float[1]){ windowWidth / (float)windowHeight }
			);

			vkCmdBindDescriptorSets (
				renderCommandBuffer->commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				app->pipelineLayout[PIPELINE_DEFERRED_LIGHTING],
				0, 1, (VkDescriptorSet[1]){ app->descriptorSet[PIPELINE_DEFERRED_LIGHTING] },
				1, (uint32_t[1]) { lightBufferOffset }
			);

			vkCmdDraw ( renderCommandBuffer->commandBuffer, 3, 1, 0, 0 );
		}
	
		// Now that we have completed the forward subpass, we will move onto the post-processing
		// subpass.
	
		vkCmdNextSubpass ( renderCommandBuffer->commandBuffer, VK_SUBPASS_CONTENTS_INLINE );
	
		{
			vkCmdBindPipeline (
				renderCommandBuffer->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				app->renderpass.pipeline[PIPELINE_POST]
			);
	
			vkCmdPushConstants (
				renderCommandBuffer->commandBuffer,
				app->pipelineLayout[PIPELINE_POST],
				VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				1 * sizeof ( float ),
				(float[1]){ windowWidth / (float)windowHeight }
			);

			vkCmdBindDescriptorSets (
				renderCommandBuffer->commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				app->pipelineLayout[PIPELINE_POST],
				0, 1, (VkDescriptorSet[1]){ app->descriptorSet[PIPELINE_POST] },
				0, NULL
			);

			vkCmdDraw ( renderCommandBuffer->commandBuffer, 3, 1, 0, 0 );
		}
	
		vkCmdEndRenderPass ( renderCommandBuffer->commandBuffer );
	}

	result = vkEndCommandBuffer ( renderCommandBuffer->commandBuffer );
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkWaitForFences failed (%u)", result );
	
	// TODO

	result = vkQueueSubmit (
		app->queues[QUEUE_MAIN].queue,
		1, (VkSubmitInfo[1]){
			[0] = {
				.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.waitSemaphoreCount   = 1,
				.pWaitSemaphores      = (VkSemaphore[1]){ renderCommandBuffer->semaphoreBackbufferWritable },
				.pWaitDstStageMask    = (VkPipelineStageFlags[1]) { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
				.commandBufferCount   = 1,
				.pCommandBuffers      = (VkCommandBuffer[1]){ renderCommandBuffer->commandBuffer },
				.signalSemaphoreCount = 1,
				.pSignalSemaphores    = (VkSemaphore[1]){
					renderCommandBuffer->semaphoreComplete
				},
			},
		},
		renderCommandBuffer->fenceComplete
	);
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkQueueSubmit failed (%u)", result );

	//vkFreeCommandBuffers ( device->device, device->commandPool[device->currentCommandBuffer], 1, &commandBuffer );

	// TODO

	result = vkQueuePresentKHR (
		app->queues[QUEUE_MAIN].queue,
		&(VkPresentInfoKHR){
			.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores    = (VkSemaphore[1]) { renderCommandBuffer->semaphoreComplete },
			.swapchainCount     = 1,
			.pSwapchains        = (VkSwapchainKHR[1]) { app->swapchain.swapchain },
			.pImageIndices      = (uint32_t[1]) { app->backbufferIndex },
		}
	);
	if ( result != VK_SUCCESS )
		return platform_throw_error ( -1, "vkQueuePresentKHR failed (%u)", result );

	return 0;
}

////////////////////////////////////////
//

int32_t app_init_command_infrastructure ( app_t* app )
{
	vkCreateCommandPool (
		app->device.device,
		&(VkCommandPoolCreateInfo){
			.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = app->queues[QUEUE_MAIN].familyIndex,
		},
		NULL,
		&app->commandPool
	);


	VkCommandBuffer commandBuffers[1 + STATIC_ARRAY_LENGTH(app->commandBufferRender)];
	vkAllocateCommandBuffers (
		app->device.device,
		&(VkCommandBufferAllocateInfo){
			.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool        = app->commandPool,
			.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1 + STATIC_ARRAY_LENGTH(app->commandBufferRender),
		},
		commandBuffers
	);

	for ( uint32_t i = 0; i < STATIC_ARRAY_LENGTH(app->commandBufferRender); i++ )
	{
		render_cmd_buffer_t* cmdBuffer = &app->commandBufferRender[i];

		cmdBuffer->commandBuffer = commandBuffers[i];

		vkCreateFence (
			app->device.device,
			&(VkFenceCreateInfo){
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT,
			},
			NULL,
			&cmdBuffer->fenceComplete
		);

		vkCreateSemaphore (
			app->device.device,
			&(VkSemaphoreCreateInfo){
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			},
			NULL,
			&cmdBuffer->semaphoreComplete
		);

		vkCreateSemaphore (
			app->device.device,
			&(VkSemaphoreCreateInfo){
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			},
			NULL,
			&cmdBuffer->semaphoreBackbufferWritable
		);
	}

	app->commandBufferStaging = commandBuffers[STATIC_ARRAY_LENGTH(app->commandBufferRender)];

	return 0;
}

int32_t app_destroy_command_infrastructure ( app_t* app )
{
	for ( uint32_t i = 0; i < RENDER_COMMAND_BUFFER_COUNT; i++ )
	{
		render_cmd_buffer_t* cmdBuffer = &app->commandBufferRender[i];

		vkDestroyFence ( app->device.device, cmdBuffer->fenceComplete, NULL );
		vkDestroySemaphore ( app->device.device, cmdBuffer->semaphoreComplete, NULL );
		vkDestroySemaphore ( app->device.device, cmdBuffer->semaphoreBackbufferWritable, NULL );
	}

	vkDestroyCommandPool ( app->device.device, app->commandPool, NULL );
	return 0;
}

////////////////////////////////////////
//

int32_t app_init_swapchain_image_layouts ( app_t* app )
{
	VkImageMemoryBarrier* memoryBarriers =
		alloca ( app->swapchain.imageCount * sizeof ( VkImageMemoryBarrier ) );

	for ( uint32_t i = 0; i < app->swapchain.imageCount; i++ )
	{
		memoryBarriers[i] = (VkImageMemoryBarrier){
			.sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask    = 0,
			.dstAccessMask    = VK_ACCESS_MEMORY_READ_BIT,
			.oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.image            = app->swapchain.images[i],
			.subresourceRange = {
				.levelCount = 1, .layerCount = 1, .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			},
		};
	}

	vkBeginCommandBuffer (
		app->commandBufferStaging, 
		&(VkCommandBufferBeginInfo){
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		}
	);

	vkCmdPipelineBarrier (
		app->commandBufferStaging,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, NULL,
		0, NULL,
		app->swapchain.imageCount, memoryBarriers
	);

	vkEndCommandBuffer ( app->commandBufferStaging );

	vkQueueSubmit (
		app->queues[QUEUE_MAIN].queue,
		1, (VkSubmitInfo[1]){
			[0] = {
				.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pWaitDstStageMask    = (VkPipelineStageFlags[1]) { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT },
				.commandBufferCount   = 1,
				.pCommandBuffers      = (VkCommandBuffer[1]){
					app->commandBufferStaging,
				},
			},
		},
		VK_NULL_HANDLE
	);

	vkQueueWaitIdle ( app->queues[QUEUE_MAIN].queue );
	return 0;
}

////////////////////////////////////////
//

int32_t app_init_descriptor_sets ( app_t* app )
{
	VkResult vkResult = VK_SUCCESS;

	// Given the rendering we are to do, we need the following:
	//
	// The forward pipeline will need to be able to bind a number of textures to show diversely
	// colored shapes and sizes. We have a number of these textures, and since every single object
	// will only use one of these textures, we will only need to bind a single one at a time.
	//
	// On top of that, we will need to have a sampler, defining for every given position on a
	// texture, we define how the hardware is to pull color data from that texture. For instance,
	// if we ask for a pixel at any boundary between two pixels, what are we asking of the hardwre?
	// Do we want the closest one, like would be preferable for a game with pixelart, or do we
	// want a combination of the surrounding pixels to give smooth transitions between the pixels.
	//
	// The post processing stage will need the results of the initial forward rendering to generate
	// its results. For this, we could bind the color buffer as a texture, but another option
	// Vulkan provides us optimized for this purpose is the Input Attachment. We will use this
	// option.
	//
	// This leads us to this function. In this function, we will:
	//
	// 1. Determine the amount of descriptors of specific types we will need, and specify the
	//    layout of descriptor sets used in rendering so we can allocate those descriptor sets
	// 2. Allocate a pool to contain all of these descriptors
	// 3. Create a number of descriptor sets containing descriptors from this pool for use
	//    in the rendering pipeline.
	// 4. Fill the descriptors in these descriptor sets with meaningful data. In our case, our
	//    textures, samplers and input attachment.
	//
	// Steps 1 and 2 (combined) follow

	uint32_t textureCount = app->model->textureCount;

	vkResult = vkCreateDescriptorPool (
		app->device.device,
		&(VkDescriptorPoolCreateInfo){
			.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags         = 0,
			.maxSets       = textureCount + 3,
			.poolSizeCount = 5,
			.pPoolSizes    = (VkDescriptorPoolSize[5]){
				{ .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          .descriptorCount = textureCount+1 },
				{ .type = VK_DESCRIPTOR_TYPE_SAMPLER,                .descriptorCount = textureCount+1 },
				{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 },
				{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, .descriptorCount = 1 },
				{ .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       .descriptorCount = 4 },
			},
		},
		NULL,
		&app->descriptorPool
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateDescriptorPool failed (%u)", vkResult );

	// Our forward stage wants a single sampled image and a single sampler to sample this image.
	
	vkResult = vkCreateDescriptorSetLayout (
		app->device.device,
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
		&app->descriptorSetLayout[PIPELINE_DEFERRED]
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateDescriptorSetLayout failed (%u)", vkResult );

	
	vkResult = vkCreateDescriptorSetLayout (
		app->device.device,
		&(VkDescriptorSetLayoutCreateInfo){
			.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 3,
			.pBindings    = (VkDescriptorSetLayoutBinding[3]){
				{
					.binding            = 0,
					.descriptorType     = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
					.descriptorCount    = 3,
					.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
				},
				{
					.binding            = 3,
					.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount    = 1,
					.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
					.pImmutableSamplers = (VkSampler[1]) { app->staticResources.samplerShadow },
				},
				{
					.binding            = 4,
					.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
					.descriptorCount    = 1,
					.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
				}
			}
		},
		NULL,
		&app->descriptorSetLayout[PIPELINE_DEFERRED_LIGHTING]
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateDescriptorSetLayout failed (%u)", vkResult );

	// Our post processing stage only wants the color attachment we previously used in the forward
	// rendering stage to use as input

	vkResult = vkCreateDescriptorSetLayout (
		app->device.device,
		&(VkDescriptorSetLayoutCreateInfo){
			.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = 1,
			.pBindings    = (VkDescriptorSetLayoutBinding[1]){
				{
					.binding         = 0,
					.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
					.descriptorCount = 1,
					.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
				},
			},
		},
		NULL,
		&app->descriptorSetLayout[PIPELINE_POST]
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateDescriptorSetLayout failed (%u)", vkResult );

	// Note we set "maxSets" to "textureCount + 2". With this, I intend to say "We want a descriptor
	// set for every single texture, plus two descriptor sets unrelated to textures."
	//
	// We also want "textureCount + 1" sampled image objects. (images to be sampled in the shader)
	// The +1 in this case is to account for the dummy texture in the case the object has no
	// texture bound.
	//
	// Since every sampled image will be accompanied by a sampler to sample the image, we ask for
	// the same amount of samplers. The amount of input attachments is 1, as the post processing
	// stage will not require textures, just the result from the first stage.
	//
	// With the pool created, we can start allocating descriptor sets from the pool. We start off
	// by allocating the default descriptor sets for the dummy texture and the post processing
	// pipeline respectively, because we know exactly how much of those we are going to need
	// before the application starts. These do not need to be in a separate batch, but this presents
	// what the calls below, made to account for the dynamic amount of textures provided by the
	// model, eventually end up with approximately.

	vkResult = vkAllocateDescriptorSets (
		app->device.device,
		&(VkDescriptorSetAllocateInfo){
			.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool     = app->descriptorPool,
			.descriptorSetCount = 3,
			.pSetLayouts        = (VkDescriptorSetLayout[3]){
				app->descriptorSetLayout[PIPELINE_DEFERRED],
				app->descriptorSetLayout[PIPELINE_DEFERRED_LIGHTING],
				app->descriptorSetLayout[PIPELINE_POST],
			},
		},
		app->descriptorSet
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkAllocateDescriptorSets failed (%u)", vkResult );

	// If we have textures, we can allocate the texture descriptor sets as well. If we do not wish
	// any descriptor sets with textures aside from our dummy texture, we could skip this.
	
	if ( app->model->textureCount > 0 )
	{
		VkDescriptorSetLayout* dsLayouts =
			alloca ( app->model->textureCount * sizeof ( VkDescriptorSetLayout ) );
		for ( uint32_t i = 0; i < app->model->textureCount; i++ )
			dsLayouts[i] = app->descriptorSetLayout[PIPELINE_DEFERRED];

		app->modelDescriptorSets = malloc ( app->model->textureCount * sizeof ( VkDescriptorSet ) );
		vkResult = vkAllocateDescriptorSets (
			app->device.device,
			&(VkDescriptorSetAllocateInfo){
				.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.descriptorPool     = app->descriptorPool,
				.descriptorSetCount = app->model->textureCount,
				.pSetLayouts        = dsLayouts,
			},
			app->modelDescriptorSets
		);
		if ( vkResult != VK_SUCCESS )
			return platform_throw_error ( -1, "vkAllocateDescriptorSets failed (%u)", vkResult );
	}

	// Now we are to write data to the descriptors. The data in question is a reference to the
	// resources we are to use when using these descriptor sets.
	//
	// As above, we first deal with the static resources for demonstration purposes. In reality
	// land, aside from a level of potential clarity, there is no purpose to do two separate
	// calls to vkUpdateDescriptorSets. A single call would likely be faster, if only slightly.

	vkUpdateDescriptorSets (
		app->device.device,
		4, (VkWriteDescriptorSet[4]){
			{
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = app->descriptorSet[PIPELINE_DEFERRED],
				.dstBinding      = 0,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.pImageInfo      = &(VkDescriptorImageInfo){
					.imageView   = app->staticResources.imageViewDummyDiffuse,
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
			},
			{
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = app->descriptorSet[PIPELINE_DEFERRED],
				.dstBinding      = 1,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
				.pImageInfo      = &(VkDescriptorImageInfo){
					.sampler     = app->staticResources.samplerNearest,
				},
			},
			{
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = app->descriptorSet[PIPELINE_DEFERRED_LIGHTING],
				.dstBinding      = 3,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.pImageInfo      = &(VkDescriptorImageInfo){
					.imageView   = app->staticResources.imageViewShadowArray,
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
			},
			{
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = app->descriptorSet[PIPELINE_DEFERRED_LIGHTING],
				.dstBinding      = 4,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				.pBufferInfo     = &(VkDescriptorBufferInfo){
					.buffer = app->staticResources.lightBuffer,
					.offset = 0,
					.range  = VK_WHOLE_SIZE,
				},
			},
		},
		0, NULL
	);

	// Here we deal with the dynamic case of N textures again. There is no real notable difference
	// from the previous segment aside from this portion having to deal with an unknown amount of
	// textures at compile time. (Or even textures at all)

	if ( app->model->textureCount > 0 )
	{
		VkDescriptorImageInfo shadowMap = (VkDescriptorImageInfo){
			.imageView   = app->staticResources.imageViewShadowArray,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};
		VkDescriptorBufferInfo lightBuffer = (VkDescriptorBufferInfo){
			.buffer = app->staticResources.lightBuffer,
			.offset = 0,
			.range  = VK_WHOLE_SIZE,
		};

		VkWriteDescriptorSet* descriptorWriteOps =
			alloca ( (app->model->textureCount*2) * sizeof ( VkWriteDescriptorSet ) );
		VkDescriptorImageInfo* descriptorWriteImageInfo =
			alloca ( (app->model->textureCount*2) * sizeof ( VkDescriptorImageInfo ) );

		uint32_t writeOpIdx = 0;

		for ( uint32_t i = 0; i < app->model->textureCount; i++ )
		{
			VkDescriptorImageInfo* imageInfo = descriptorWriteImageInfo++;
			*imageInfo = (VkDescriptorImageInfo){
				.imageView   = app->model->imageViews[i],
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			};
			descriptorWriteOps[writeOpIdx++] = (VkWriteDescriptorSet){
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = app->modelDescriptorSets[i],
				.dstBinding      = 0,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.pImageInfo      = imageInfo,
			};
			imageInfo = descriptorWriteImageInfo++;
			*imageInfo = (VkDescriptorImageInfo){
				.sampler = app->staticResources.samplerAnisotropic,
			};
			descriptorWriteOps[writeOpIdx++] = (VkWriteDescriptorSet){
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = app->modelDescriptorSets[i],
				.dstBinding      = 1,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
				.pImageInfo      = imageInfo,
			};
		}

		vkUpdateDescriptorSets (
			app->device.device,
			(app->model->textureCount*2), descriptorWriteOps,
			0, NULL
		);
	}

	// Note the above two sections of descriptor updates have missed one descriptor update:
	// The post processing descriptor set does not have its INPUT_ATTACHMENT descriptor set to
	// the proper value. This will be done when we create the input attachment.

	return 0;
}

int32_t app_destroy_descriptor_sets ( app_t* app )
{
	vkDestroyDescriptorPool ( app->device.device, app->descriptorPool, NULL );
	vkDestroyDescriptorSetLayout ( app->device.device, app->descriptorSetLayout[PIPELINE_DEFERRED], NULL );
	vkDestroyDescriptorSetLayout ( app->device.device, app->descriptorSetLayout[PIPELINE_DEFERRED_LIGHTING], NULL );
	vkDestroyDescriptorSetLayout ( app->device.device, app->descriptorSetLayout[PIPELINE_POST], NULL );
	return 0;
}

////////////////////////////////////////
//

int32_t app_init_renderpass ( app_t* app )
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

	VkResult result = vkCreateRenderPass (
		app->device.device,
		&(VkRenderPassCreateInfo){
			.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = ATTACHMENT_COUNT,
			.pAttachments    = (VkAttachmentDescription[ATTACHMENT_COUNT]){
				[ATTACHMENT_INTERMEDIATE_GBUFFER1] = {
					.format        = VK_FORMAT_R32G32B32A32_SFLOAT,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				},
				[ATTACHMENT_INTERMEDIATE_GBUFFER2] = {
					.format        = VK_FORMAT_R32G32B32A32_SFLOAT,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				},
				[ATTACHMENT_INTERMEDIATE_GBUFFER3] = {
					.format        = VK_FORMAT_R32G32B32A32_SFLOAT,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				},
				[ATTACHMENT_INTERMEDIATE_COLOR] = {
					.format        = VK_FORMAT_R8G8B8A8_SRGB,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.finalLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				},
				[ATTACHMENT_INTERMEDIATE_DEPTH] = {
					.format        = VK_FORMAT_D32_SFLOAT,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp       = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
					.finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				},
				[ATTACHMENT_FRAMEBUFFER]        = {
					.format        = app->swapchain.surfaceFormat.format,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.storeOp       = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				},
			},
			.subpassCount = SUBPASS_COUNT,
			.pSubpasses   = (VkSubpassDescription[SUBPASS_COUNT]){
				[SUBPASS_DEFERRED] = {
					.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.colorAttachmentCount = 3,
					.pColorAttachments    = (VkAttachmentReference[3]){
						{
							.attachment = ATTACHMENT_INTERMEDIATE_GBUFFER1,
							.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
						},
						{
							.attachment = ATTACHMENT_INTERMEDIATE_GBUFFER2,
							.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
						},
						{
							.attachment = ATTACHMENT_INTERMEDIATE_GBUFFER3,
							.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
						},
					},
					.pDepthStencilAttachment = &(VkAttachmentReference){
						.attachment = ATTACHMENT_INTERMEDIATE_DEPTH,
						.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
					},
				},
				[SUBPASS_DEFERRED_LIGHTING] = {
					.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.inputAttachmentCount = 3,
					.pInputAttachments    = (VkAttachmentReference[3]){
						{
							.attachment = ATTACHMENT_INTERMEDIATE_GBUFFER1,
							.layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						},
						{
							.attachment = ATTACHMENT_INTERMEDIATE_GBUFFER2,
							.layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						},
						{
							.attachment = ATTACHMENT_INTERMEDIATE_GBUFFER3,
							.layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						},
					},
					.colorAttachmentCount = 1,
					.pColorAttachments    = (VkAttachmentReference[1]){
						{
							.attachment = ATTACHMENT_INTERMEDIATE_COLOR,
							.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						},
					},
				},
				[SUBPASS_POST]    = {
					.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.inputAttachmentCount = 1,
					.pInputAttachments    = (VkAttachmentReference[1]){
						{
							.attachment = ATTACHMENT_INTERMEDIATE_COLOR,
							.layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
						},
					},
					.colorAttachmentCount = 1,
					.pColorAttachments    = (VkAttachmentReference[1]){
						{
							.attachment = ATTACHMENT_FRAMEBUFFER,
							.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						},
					},
				},
			},
			.dependencyCount = 2,
			.pDependencies   = (VkSubpassDependency[2]){
				{
					.srcSubpass      = SUBPASS_DEFERRED,
					.dstSubpass      = SUBPASS_DEFERRED_LIGHTING,
					.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					.dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
					.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
				},
				{
					.srcSubpass      = SUBPASS_DEFERRED_LIGHTING,
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
		&app->renderpass.renderPass
	);
	if ( result != 0 )
		return platform_throw_error ( -1, "vkCreateRenderPass failed (%u)", result );

	result = vkCreateRenderPass (
		app->device.device,
		&(VkRenderPassCreateInfo){
			.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments    = (VkAttachmentDescription[1]){
				[0] = {
					.format        = VK_FORMAT_D32_SFLOAT,
					.samples       = VK_SAMPLE_COUNT_1_BIT,
					.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp       = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					.finalLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
			},
			.subpassCount = 1,
			.pSubpasses   = (VkSubpassDescription[1]){
				[0] = {
					.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
					.colorAttachmentCount = 0,
					.pDepthStencilAttachment = &(VkAttachmentReference){
						.attachment = 0,
						.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
					},
				},
			},
			.dependencyCount = 0,
		},
		NULL,
		&app->shadowRenderpass.renderPass
	);
	if ( result != 0 )
		return platform_throw_error ( -1, "vkCreateRenderPass failed (%u)", result );

	//
	//
	//

	return 0;
}

int32_t app_destroy_renderpass ( app_t* app )
{
	vkDestroyRenderPass ( app->device.device, app->renderpass.renderPass, NULL );
	return 0;
}

////////////////////////////////////////
//

int32_t app_init_graphics_pipeline_prerequisites ( app_t* app )
{
	// In order to create the pipelines, we will first need to create the layouts depicting the
	// necessary inputs on the side of descriptors and push constants. These are used for binding
	// later on. If two pipelines use the same pipeline layouts, the bindings are guaranteed to be
	// compatible across both pipelines.

	VkResult vkResult = vkCreatePipelineLayout (
		app->device.device,
		&(VkPipelineLayoutCreateInfo){
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount         = 1,
			.pSetLayouts            = (VkDescriptorSetLayout[1]){ app->descriptorSetLayout[PIPELINE_DEFERRED] },
			.pushConstantRangeCount = 2,
			.pPushConstantRanges    = (VkPushConstantRange[2]){
				{
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.offset     = 0,
					.size       = 16 * sizeof ( float ),
				},
				{
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.offset     = 64,
					.size       = 16 * sizeof ( float ),
				},
			}
		},
		NULL,
		&app->pipelineLayout[PIPELINE_DEFERRED]
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreatePipelineLayout failed (%u)", vkResult );

	vkResult = vkCreatePipelineLayout (
		app->device.device,
		&(VkPipelineLayoutCreateInfo){
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount         = 1,
			.pSetLayouts            = (VkDescriptorSetLayout[1]){ app->descriptorSetLayout[PIPELINE_DEFERRED_LIGHTING] },
			.pushConstantRangeCount = 0,
		},
		NULL,
		&app->pipelineLayout[PIPELINE_DEFERRED_LIGHTING]
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreatePipelineLayout failed (%u)", vkResult );

	vkResult = vkCreatePipelineLayout (
		app->device.device,
		&(VkPipelineLayoutCreateInfo){
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount         = 1,
			.pSetLayouts            = (VkDescriptorSetLayout[1]){ app->descriptorSetLayout[PIPELINE_POST] },
			.pushConstantRangeCount = 1,
			.pPushConstantRanges    = (VkPushConstantRange[1]){
				{
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
					.offset     = 0,
					.size       = 1 * sizeof ( float ),
				},
			},
		},
		NULL,
		&app->pipelineLayout[PIPELINE_POST]
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreatePipelineLayout failed (%u)", vkResult );

	vkResult = vkCreatePipelineLayout (
		app->device.device,
		&(VkPipelineLayoutCreateInfo){
			.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount         = 0,
			.pSetLayouts            = NULL,
			.pushConstantRangeCount = 2,
			.pPushConstantRanges    = (VkPushConstantRange[2]){
				{
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.offset     = 0,
					.size       = 16 * sizeof ( float ),
				},
				{
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.offset     = 64,
					.size       = 16 * sizeof ( float ),
				},
			},
		},
		NULL,
		&app->pipelineLayoutShadow
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreatePipelineLayout failed (%u)", vkResult );

	// While pipelines can be created and destroyed without issue, it is advisable to use a
	// pipeline cache in order to allow the implementation to reuse as many pipeline properties
	// as possible, and potentially avoid needless pipeline construction time.

	// In addition, this pipeline cache can be written to disk and reloaded next time the app is
	// started to reduce startup time.

	vkResult = vkCreatePipelineCache (
		app->device.device,
		&(VkPipelineCacheCreateInfo){
			.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		},
		NULL,
		&app->pipelineCache
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreatePipelineCache failed (%u)", vkResult );

	return 0;
}

int32_t app_destroy_graphics_pipeline_prerequisites ( app_t* app )
{
	// In order to create the pipelines, we will first need to create the layouts depicting the
	// necessary inputs on the side of descriptors and push constants. These are used for binding
	// later on. If two pipelines use the same pipeline layouts, the bindings are guaranteed to be
	// compatible across both pipelines.

	//vkDestroyPipelineLayout ( app->device.device, app->pipelineLayout[PIPELINE_FORWARD], NULL );
	//vkDestroyPipelineLayout ( app->device.device, app->pipelineLayout[PIPELINE_POST], NULL );
	//vkDestroyPipelineCache ( app->device.device, app->pipelineCache, NULL );
	return 0;
}

////////////////////////////////////////
//

int32_t app_init_graphics_pipelines ( app_t* app, uint32_t windowWidth, uint32_t windowHeight )
{
	VkResult vkResult = VK_SUCCESS;

	// List the shader paths and the shader module they are intended to be loaded into
	// This is just done to ensure code duplication is reduced a bit more, as shader module
	// creation tends to not be that much code, but the amount of shader modules tend to scale
	// quite quickly.

	enum
	{
		SHADER_SHADOW_VERT,
		SHADER_DEFERRED_VERT,
		SHADER_DEFERRED_FRAG,
		SHADER_DEFERRED_LIGHTING_FRAG,
		SHADER_POST_VERT,
		SHADER_POST_FRAG,

		SHADER_COUNT
	};

	struct
	{
		const char* path;
		VkShaderModule outModule;
	} shaders[SHADER_COUNT] = {
		[SHADER_SHADOW_VERT]            = { .path = "shaders/shadow_v.spv", },
		[SHADER_DEFERRED_VERT]          = { .path = "shaders/forward_v.spv", },
		[SHADER_DEFERRED_FRAG]          = { .path = "shaders/deferred_f.spv", },
		[SHADER_DEFERRED_LIGHTING_FRAG] = { .path = "shaders/deferred_lighting_f.spv", },
		[SHADER_POST_VERT]              = { .path = "shaders/post_v.spv",    },
		[SHADER_POST_FRAG]              = { .path = "shaders/post_f.spv",    },
	};

	// Create the aforementioned shader modules. If any of these fail, check the documentation
	// for compiling the shaders, as you have probably skipped that step.

	for ( uint32_t i = 0; i < STATIC_ARRAY_LENGTH(shaders); i++ )
	{
		file_t file;
		if ( platform_file_load ( &file, shaders[i].path ) != 0 )
			return -1;

		vkResult = vkCreateShaderModule (
			app->device.device,
			&(VkShaderModuleCreateInfo){
				.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.codeSize = file.sizeInBytes,
				.pCode    = file.data,
			},
			NULL,
			&shaders[i].outModule
		);
		if ( vkResult != VK_SUCCESS )
			return platform_throw_error ( -1, "vkCreateShaderModule failed (%u)", vkResult );

		platform_file_close ( &file );
	}

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

	vkResult = vkCreateGraphicsPipelines (
		app->device.device,
		app->pipelineCache,
		PIPELINE_COUNT,
		(VkGraphicsPipelineCreateInfo[PIPELINE_COUNT]){
			[PIPELINE_DEFERRED]          = {
				.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.flags      = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
				.stageCount = 2,
				.pStages    = (VkPipelineShaderStageCreateInfo[2]){
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_VERTEX_BIT,
						.module = shaders[SHADER_DEFERRED_VERT].outModule,
						.pName  = "main",
					},
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
						.module = shaders[SHADER_DEFERRED_FRAG].outModule,
						.pName  = "main",
					},
				},
				.pVertexInputState = &(VkPipelineVertexInputStateCreateInfo){
					.sType             = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
					.vertexBindingDescriptionCount = 1,
					.pVertexBindingDescriptions    = (VkVertexInputBindingDescription[1]){
						{
							.binding   = 0,
							.stride    = sizeof ( bobj_vert ),
							.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
						},
					},
					.vertexAttributeDescriptionCount = VERTEX_ATTRIBUTE_COUNT,
					.pVertexAttributeDescriptions = 
						(VkVertexInputAttributeDescription[VERTEX_ATTRIBUTE_COUNT]){
						[VERTEX_ATTRIBUTE_POSITION] = {
							.location = VERTEX_ATTRIBUTE_POSITION,
							.binding  = 0,
							.format   = VK_FORMAT_R32G32B32_SFLOAT,
							.offset   = offsetof(bobj_vert, position),
						},
						[VERTEX_ATTRIBUTE_TEXCOORD] = {
							.location = VERTEX_ATTRIBUTE_TEXCOORD,
							.binding  = 0,
							.format   = VK_FORMAT_R32G32_SFLOAT,
							.offset   = offsetof(bobj_vert, texcoord),
						},
						[VERTEX_ATTRIBUTE_NORMAL] = {
							.location = VERTEX_ATTRIBUTE_NORMAL,
							.binding  = 0,
							.format   = VK_FORMAT_R32G32B32_SFLOAT,
							.offset   = offsetof(bobj_vert, normal),
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
							.width = (float)windowWidth, .height = (float)windowHeight,
							.minDepth = 0.0f, .maxDepth =  1.0f,
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
					.rasterizerDiscardEnable = VK_FALSE,
					.polygonMode             = VK_POLYGON_MODE_FILL,
					.cullMode                = VK_CULL_MODE_BACK_BIT,
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
					.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
					.logicOpEnable   = VK_FALSE,
					.attachmentCount = 3,
					.pAttachments    = (VkPipelineColorBlendAttachmentState[3]){
						{
							.blendEnable    = VK_FALSE,
							.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
								| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
						},
						{
							.blendEnable    = VK_FALSE,
							.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
								| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
						},
						{
							.blendEnable    = VK_FALSE,
							.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
								| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
						},
					},
				},
				.pDynamicState = NULL,
				.layout        = app->pipelineLayout[PIPELINE_DEFERRED],
				.renderPass    = app->renderpass.renderPass,
				.subpass       = SUBPASS_DEFERRED,
			},
			[PIPELINE_DEFERRED_LIGHTING] = {
				.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.flags      = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
				.stageCount = 2,
				.pStages    = (VkPipelineShaderStageCreateInfo[2]){
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_VERTEX_BIT,
						.module = shaders[SHADER_POST_VERT].outModule,
						.pName  = "main",
					},
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
						.module = shaders[SHADER_DEFERRED_LIGHTING_FRAG].outModule,
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
					.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
					.viewportCount = 1,
					.pViewports    = (VkViewport[1]){
						{
							.width = (float)windowWidth, .height = (float)windowHeight,
							.minDepth = 0.0f, .maxDepth =  1.0f,
						},
					},
					.scissorCount = 1,
					.pScissors    = (VkRect2D[1]){
						{ .extent.width = windowWidth, .extent.height = windowHeight },
					},
				},
				.pRasterizationState = &(VkPipelineRasterizationStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
					.depthClampEnable        = VK_FALSE,
					.rasterizerDiscardEnable = VK_FALSE,
					.polygonMode             = VK_POLYGON_MODE_FILL,
					.cullMode                = VK_CULL_MODE_FRONT_BIT,
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
					.depthTestEnable       = VK_FALSE,
					.depthWriteEnable      = VK_FALSE,
					.depthCompareOp        = VK_COMPARE_OP_LESS,
					.depthBoundsTestEnable = VK_FALSE,
					.stencilTestEnable     = VK_FALSE,
				},
				.pColorBlendState = &(VkPipelineColorBlendStateCreateInfo){
					.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
					.logicOpEnable   = VK_FALSE,
					.attachmentCount = 1,
					.pAttachments    = (VkPipelineColorBlendAttachmentState[1]){
						{
							.blendEnable = VK_FALSE,
							.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
								| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
						},
					},
				},
				.pDynamicState = NULL,
				.layout        = app->pipelineLayout[PIPELINE_DEFERRED_LIGHTING],
				.renderPass    = app->renderpass.renderPass,
				.subpass       = SUBPASS_DEFERRED_LIGHTING,
			},
			[PIPELINE_POST]              = {
				.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.flags      = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
				.stageCount = 2,
				.pStages    = (VkPipelineShaderStageCreateInfo[2]){
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_VERTEX_BIT,
						.module = shaders[SHADER_POST_VERT].outModule,
						.pName  = "main",
					},
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
						.module = shaders[SHADER_POST_FRAG].outModule,
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
					.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
					.viewportCount = 1,
					.pViewports    = (VkViewport[1]){
						{
							.width = (float)windowWidth, .height = (float)windowHeight,
							.minDepth = 0.0f, .maxDepth =  1.0f,
						},
					},
					.scissorCount = 1,
					.pScissors    = (VkRect2D[1]){
						{ .extent.width = windowWidth, .extent.height = windowHeight },
					},
				},
				.pRasterizationState = &(VkPipelineRasterizationStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
					.depthClampEnable        = VK_FALSE,
					.rasterizerDiscardEnable = VK_FALSE,
					.polygonMode             = VK_POLYGON_MODE_FILL,
					.cullMode                = VK_CULL_MODE_FRONT_BIT,
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
					.depthTestEnable       = VK_FALSE,
					.depthWriteEnable      = VK_FALSE,
					.depthCompareOp        = VK_COMPARE_OP_LESS,
					.depthBoundsTestEnable = VK_FALSE,
					.stencilTestEnable     = VK_FALSE,
				},
				.pColorBlendState = &(VkPipelineColorBlendStateCreateInfo){
					.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
					.logicOpEnable   = VK_FALSE,
					.attachmentCount = 1,
					.pAttachments    = (VkPipelineColorBlendAttachmentState[1]){
						{
							.blendEnable = VK_FALSE,
							.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
								| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
						},
					},
				},
				.pDynamicState = NULL,
				.layout        = app->pipelineLayout[PIPELINE_POST],
				.renderPass    = app->renderpass.renderPass,
				.subpass       = SUBPASS_POST,
			},
		},
		NULL,
		app->renderpass.pipeline
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateGraphicsPipelines failed (%u)", vkResult );

	//

	vkResult = vkCreateGraphicsPipelines (
		app->device.device,
		app->pipelineCache,
		1,
		(VkGraphicsPipelineCreateInfo[1]){
			[0] = {
				.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.flags      = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
				.stageCount = 1,
				.pStages    = (VkPipelineShaderStageCreateInfo[1]){
					{
						.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage  = VK_SHADER_STAGE_VERTEX_BIT,
						.module = shaders[SHADER_SHADOW_VERT].outModule,
						.pName  = "main",
					},
				},
				.pVertexInputState = &(VkPipelineVertexInputStateCreateInfo){
					.sType             = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
					.vertexBindingDescriptionCount = 1,
					.pVertexBindingDescriptions    = (VkVertexInputBindingDescription[1]){
						{
							.binding   = 0,
							.stride    = sizeof ( bobj_vert ),
							.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
						},
					},
					.vertexAttributeDescriptionCount = 1,
					.pVertexAttributeDescriptions = 
						(VkVertexInputAttributeDescription[1]){
						[0] = {
							.location = VERTEX_ATTRIBUTE_POSITION,
							.binding  = 0,
							.format   = VK_FORMAT_R32G32B32_SFLOAT,
							.offset   = offsetof(bobj_vert, position),
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
							.width = (float)SHADOW_MAP_WIDTH, .height = (float)SHADOW_MAP_HEIGHT,
							.minDepth = 0.0f, .maxDepth =  1.0f,
						},
					},
					.scissorCount = 1,
					.pScissors = (VkRect2D[1]){
						{ .extent.width = SHADOW_MAP_WIDTH, .extent.height = SHADOW_MAP_HEIGHT },
					},
				},
				.pRasterizationState = &(VkPipelineRasterizationStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
					.depthClampEnable        = VK_FALSE,
					.rasterizerDiscardEnable = VK_FALSE,
					.polygonMode             = VK_POLYGON_MODE_FILL,
					.cullMode                = VK_CULL_MODE_NONE,
					.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
					.depthBiasEnable         = VK_TRUE,
					.depthBiasConstantFactor = 5.0f,
					.depthBiasSlopeFactor    = 1.5f,
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
					.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
					.logicOpEnable   = VK_FALSE,
					.attachmentCount = 0,
				},
				.pDynamicState = NULL,
				.layout        = app->pipelineLayoutShadow,
				.renderPass    = app->shadowRenderpass.renderPass,
				.subpass       = 0,
			},
		},
		NULL,
		&app->shadowRenderpass.pipeline
	);

	// 

	for ( uint32_t i = 0; i < STATIC_ARRAY_LENGTH(shaders); i++ )
		vkDestroyShaderModule ( app->device.device, shaders[i].outModule, NULL );

	//
	//
	//

	return 0;
}

int32_t app_destroy_graphics_pipelines ( app_t* app )
{
	for ( uint32_t i = 0; i < PIPELINE_COUNT; i++ )
	{
		vkDestroyPipeline ( app->device.device, app->renderpass.pipeline[i], NULL );
	}
	return 0;
}

////////////////////////////////////////
//

int32_t app_init_static_resources ( app_t* app )
{
	// Create two samplers
	//
	// These will be used by shaders to determine how textures are to be read from shaders

	VkResult vkResult = vkCreateSampler (
		app->device.device,
		&(VkSamplerCreateInfo){
			.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter               = VK_FILTER_NEAREST,
			.minFilter               = VK_FILTER_NEAREST,
			.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.anisotropyEnable        = VK_FALSE,
			.minLod                  = 0.0f,
			.maxLod                  = 100.0f,
			.compareEnable           = VK_FALSE,
			.unnormalizedCoordinates = VK_FALSE,
		},
		NULL,
		&app->staticResources.samplerNearest
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateSampler failed (%u)", vkResult );

	vkResult = vkCreateSampler (
		app->device.device,
		&(VkSamplerCreateInfo){
			.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter               = VK_FILTER_LINEAR,
			.minFilter               = VK_FILTER_LINEAR,
			.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.anisotropyEnable        = VK_TRUE,
			.maxAnisotropy           = 16.0f,
			.minLod                  = 0.0f,
			.maxLod                  = 100.0f,
			.compareEnable           = VK_FALSE,
			.unnormalizedCoordinates = VK_FALSE,
		},
		NULL,
		&app->staticResources.samplerAnisotropic
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateSampler failed (%u)", vkResult );

	vkResult = vkCreateSampler (
		app->device.device,
		&(VkSamplerCreateInfo){
			.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter               = VK_FILTER_LINEAR,
			.minFilter               = VK_FILTER_LINEAR,
			.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST,
			.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			.anisotropyEnable        = VK_TRUE,
			.maxAnisotropy           = 16.0f,
			.minLod                  = 0.0f,
			.maxLod                  = 0.0f,
			.compareEnable           = VK_TRUE,
			.compareOp               = VK_COMPARE_OP_LESS_OR_EQUAL,
			.borderColor             = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		},
		NULL,
		&app->staticResources.samplerShadow
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateSampler failed (%u)", vkResult );

	// TODO

	VkImageViewCreateInfo shadowTargetImageViewCreateInfos[LIGHT_COUNT];
	
	vkutil_image_view_desc shadowTargetImageViews[LIGHT_COUNT+1] = {
		[LIGHT_COUNT] = {
			.outImageView = &app->staticResources.imageViewShadowArray,
			.createInfo   = &(VkImageViewCreateInfo){
				.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.viewType         = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
				.format           = VK_FORMAT_D32_SFLOAT,
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
					.levelCount = 1,
					.layerCount = LIGHT_COUNT,
				}
			},
		}
	};

	app->staticResources.imageViewShadowDepthAttachment =
		malloc ( LIGHT_COUNT * sizeof ( VkImageView ) );

	for ( uint32_t i = 0; i < LIGHT_COUNT; i++ )
	{
		shadowTargetImageViewCreateInfos[i] = (VkImageViewCreateInfo){
			.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.viewType         = VK_IMAGE_VIEW_TYPE_2D,
			.format           = VK_FORMAT_D32_SFLOAT,
			.subresourceRange = {
				.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
				.levelCount     = 1,
				.baseArrayLayer = i,
				.layerCount     = 1,
			}
		};
		shadowTargetImageViews[i] = (vkutil_image_view_desc){
			.outImageView = &app->staticResources.imageViewShadowDepthAttachment[i],
			.createInfo   = &shadowTargetImageViewCreateInfos[i],
		};
	}


	vkutil_image_desc staticTextureCreateInfo[STATIC_TEXTURE_COUNT] = {
		[STATIC_TEXTURE_DIFFUSE_DEFAULT] = {
			.outImage   = &app->staticResources.imageDummyDiffuse,
			.createInfo = &(VkImageCreateInfo){
				.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType     = VK_IMAGE_TYPE_2D,
				.format        = VK_FORMAT_R8G8B8A8_SRGB,
				.extent        = { 4, 4, 1 },
				.mipLevels     = 1,
				.arrayLayers   = 1,
				.samples       = VK_SAMPLE_COUNT_1_BIT,
				.tiling        = VK_IMAGE_TILING_OPTIMAL,
				.usage         = VK_IMAGE_USAGE_SAMPLED_BIT,
				.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			},
			.imageViewCount = 1, .imageViews = (vkutil_image_view_desc[1]){
				{
					.outImageView = &app->staticResources.imageViewDummyDiffuse,
					.createInfo   = &(VkImageViewCreateInfo){
						.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						.viewType         = VK_IMAGE_VIEW_TYPE_2D,
						.format           = VK_FORMAT_R8G8B8A8_SRGB,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.levelCount = 1,
							.layerCount = 1,
						}
					},
				},
			},
			.initialData = (uint32_t[16]){
				0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000, 
				0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 
				0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000, 
				0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
			},
			.accessMask = VK_ACCESS_SHADER_READ_BIT,
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		},
		[STATIC_TEXTURE_SHADOWMAP]       = {
			.outImage   = &app->staticResources.imageShadowArray,
			.createInfo = &(VkImageCreateInfo){
				.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType     = VK_IMAGE_TYPE_2D,
				.format        = VK_FORMAT_D32_SFLOAT,
				.extent        = { SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 1 },
				.mipLevels     = 1,
				.arrayLayers   = LIGHT_COUNT,
				.samples       = VK_SAMPLE_COUNT_1_BIT,
				.tiling        = VK_IMAGE_TILING_OPTIMAL,
				.usage         = VK_IMAGE_USAGE_SAMPLED_BIT
					| VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			},
			.imageViewCount = LIGHT_COUNT+1, .imageViews = shadowTargetImageViews,
			.accessMask = VK_ACCESS_SHADER_READ_BIT,
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		},
	};

	int32_t ret = vkutil_create_images_helper (
		app->device.device, app->commandBufferStaging, app->queues[QUEUE_MAIN].queue,
		&app->device.memoryProperties, STATIC_TEXTURE_COUNT, staticTextureCreateInfo,
		&app->staticResources.imageMemory
	);
	if ( ret != 0 )
		return platform_throw_error ( -1, "vkutil_create_images_helper failed (%d)", ret );

	// TODO

	uint32_t bufferSize =
		RVM_ALIGN_UP_POW2 (
			sizeof ( forward_vs_cb_t ),
			app->device.properties.limits.minUniformBufferOffsetAlignment
		) * RENDER_COMMAND_BUFFER_COUNT;

	VkMemoryRequirements bufferRequirements;
	vkCreateBuffer (
		app->device.device,
		&(VkBufferCreateInfo){
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size  = bufferSize,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		},
		NULL,
		&app->staticResources.lightBuffer
	);

	vkGetBufferMemoryRequirements (
		app->device.device, app->staticResources.lightBuffer, &bufferRequirements
	);

	ret = vkutil_multi_alloc_helper (
		app->device.device,
		&app->device.memoryProperties,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		1, &bufferRequirements,
		&app->staticResources.lightBufferMemory, NULL, NULL
	);
	if ( ret != 0 )
		return platform_throw_error ( -1, "vkutil_multi_alloc_helper failed (%d)", ret );

	vkResult = vkBindBufferMemory (
		app->device.device, app->staticResources.lightBuffer,
		app->staticResources.lightBufferMemory, 0
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkBindBufferMemory failed (%d)", ret );

	return 0;
}

int32_t app_destroy_static_resources ( app_t* app )
{
	vkDestroyImageView ( app->device.device, app->staticResources.imageViewDummyDiffuse, NULL );
	vkDestroyImage ( app->device.device, app->staticResources.imageDummyDiffuse, NULL );
	vkFreeMemory ( app->device.device, app->staticResources.imageMemory, NULL );

	vkDestroySampler ( app->device.device, app->staticResources.samplerNearest, NULL );
	vkDestroySampler ( app->device.device, app->staticResources.samplerAnisotropic, NULL );

	//for ( uint32_t i = 0; i < RENDER_COMMAND_BUFFER_COUNT; i++ )
	//	vkDestroyBuffer ( app->device.device, app->staticResources.transformBuffer[i], NULL );

	//vkFreeMemory ( app->device.device, app->staticResources.transformBufferMemory, NULL );

	return 0;
}

////////////////////////////////////////
//

int32_t app_init_renderpass_framebuffers ( app_t* app, uint32_t windowWidth, uint32_t windowHeight )
{
	app->renderpass.framebuffers = malloc ( app->swapchain.imageCount * sizeof ( VkFramebuffer ) );
	for ( uint32_t i = 0; i < app->swapchain.imageCount; i++ )
	{
		VkResult result = vkCreateFramebuffer (
			app->device.device,
			&(VkFramebufferCreateInfo){
				.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass      = app->renderpass.renderPass,
				.attachmentCount = ATTACHMENT_COUNT,
				.pAttachments    = (VkImageView[ATTACHMENT_COUNT]){
					[ATTACHMENT_INTERMEDIATE_GBUFFER1] = app->attachments.imageViewGBuffer[0],
					[ATTACHMENT_INTERMEDIATE_GBUFFER2] = app->attachments.imageViewGBuffer[1],
					[ATTACHMENT_INTERMEDIATE_GBUFFER3] = app->attachments.imageViewGBuffer[2],
					[ATTACHMENT_INTERMEDIATE_COLOR]    = app->attachments.imageViewIntermediateColor,
					[ATTACHMENT_INTERMEDIATE_DEPTH]    = app->attachments.imageViewIntermediateDepth,
					[ATTACHMENT_FRAMEBUFFER]           = app->swapchain.imageViews[i],
				},
				.width  = windowWidth,
				.height = windowHeight,
				.layers = 1,
			},
			NULL,
			&app->renderpass.framebuffers[i]
		);
		if ( result != 0 )
			return platform_throw_error ( -1, "vkCreateFramebuffer failed (%u)", result );
	}

	for ( uint32_t i = 0; i < LIGHT_COUNT; i++ )
	{
		VkResult result = vkCreateFramebuffer (
			app->device.device,
			&(VkFramebufferCreateInfo){
				.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass      = app->shadowRenderpass.renderPass,
				.attachmentCount = 1,
				.pAttachments    = (VkImageView[1]){
					[0] = app->staticResources.imageViewShadowDepthAttachment[i],
				},
				.width  = SHADOW_MAP_WIDTH,
				.height = SHADOW_MAP_HEIGHT,
				.layers = 1,
			},
			NULL,
			&app->shadowRenderpass.framebuffers[i]
		);
		if ( result != 0 )
			return platform_throw_error ( -1, "vkCreateFramebuffer failed (%u)", result );
	}
	
	return 0;
}

int32_t app_destroy_renderpass_framebuffers ( app_t* app )
{
	for ( uint32_t i = 0; i < app->swapchain.imageCount; i++ )
	{
		vkDestroyFramebuffer ( app->device.device, app->renderpass.framebuffers[i], NULL );
	}

	free ( app->renderpass.framebuffers );
	
	return 0;
}

////////////////////////////////////////
//

int32_t app_init_render_attachments ( app_t* app, uint32_t windowWidth, uint32_t windowHeight )
{
	vkutil_image_desc transientAttachmentCreateInfo[TRANSIENT_ATTACHMENT_COUNT] = {
		[TRANSIENT_ATTACHMENT_GBUFFER1] = {
			.outImage   = &app->attachments.imageGBuffer[0],
			.createInfo = &(VkImageCreateInfo){
				.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType     = VK_IMAGE_TYPE_2D,
				.format        = VK_FORMAT_R32G32B32A32_SFLOAT,
				.extent        = { windowWidth, windowHeight, 1 },
				.mipLevels     = 1,
				.arrayLayers   = 1,
				.samples       = VK_SAMPLE_COUNT_1_BIT,
				.tiling        = VK_IMAGE_TILING_OPTIMAL,
				.usage         = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
					| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
				.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
			.imageViewCount = 1, .imageViews = (vkutil_image_view_desc[1]){
				{
					.outImageView = &app->attachments.imageViewGBuffer[0],
					.createInfo   = &(VkImageViewCreateInfo){
						.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						.viewType         = VK_IMAGE_VIEW_TYPE_2D,
						.format           = VK_FORMAT_R32G32B32A32_SFLOAT,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.levelCount = 1,
							.layerCount = 1,
						}
					},
				},
			},
			.accessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		},
		[TRANSIENT_ATTACHMENT_GBUFFER2] = {
			.outImage   = &app->attachments.imageGBuffer[1],
			.createInfo = &(VkImageCreateInfo){
				.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType     = VK_IMAGE_TYPE_2D,
				.format        = VK_FORMAT_R32G32B32A32_SFLOAT,
				.extent        = { windowWidth, windowHeight, 1 },
				.mipLevels     = 1,
				.arrayLayers   = 1,
				.samples       = VK_SAMPLE_COUNT_1_BIT,
				.tiling        = VK_IMAGE_TILING_OPTIMAL,
				.usage         = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
					| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
				.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
			.imageViewCount = 1, .imageViews = (vkutil_image_view_desc[1]){
				{
					.outImageView = &app->attachments.imageViewGBuffer[1],
					.createInfo   = &(VkImageViewCreateInfo){
						.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						.viewType         = VK_IMAGE_VIEW_TYPE_2D,
						.format           = VK_FORMAT_R32G32B32A32_SFLOAT,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.levelCount = 1,
							.layerCount = 1,
						}
					},
				},
			},
			.accessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		},
		[TRANSIENT_ATTACHMENT_GBUFFER3] = {
			.outImage   = &app->attachments.imageGBuffer[2],
			.createInfo = &(VkImageCreateInfo){
				.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType     = VK_IMAGE_TYPE_2D,
				.format        = VK_FORMAT_R32G32B32A32_SFLOAT,
				.extent        = { windowWidth, windowHeight, 1 },
				.mipLevels     = 1,
				.arrayLayers   = 1,
				.samples       = VK_SAMPLE_COUNT_1_BIT,
				.tiling        = VK_IMAGE_TILING_OPTIMAL,
				.usage         = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
					| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
				.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
			.imageViewCount = 1, .imageViews = (vkutil_image_view_desc[1]){
				{
					.outImageView = &app->attachments.imageViewGBuffer[2],
					.createInfo   = &(VkImageViewCreateInfo){
						.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						.viewType         = VK_IMAGE_VIEW_TYPE_2D,
						.format           = VK_FORMAT_R32G32B32A32_SFLOAT,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.levelCount = 1,
							.layerCount = 1,
						}
					},
				},
			},
			.accessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		},
		[TRANSIENT_ATTACHMENT_COLOR] = {
			.outImage   = &app->attachments.imageIntermediateColor,
			.createInfo = &(VkImageCreateInfo){
				.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType     = VK_IMAGE_TYPE_2D,
				.format        = VK_FORMAT_R8G8B8A8_SRGB,
				.extent        = { windowWidth, windowHeight, 1 },
				.mipLevels     = 1,
				.arrayLayers   = 1,
				.samples       = VK_SAMPLE_COUNT_1_BIT,
				.tiling        = VK_IMAGE_TILING_OPTIMAL,
				.usage         = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
					| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
				.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
			.imageViewCount = 1, .imageViews = (vkutil_image_view_desc[1]){
				{
					.outImageView = &app->attachments.imageViewIntermediateColor,
					.createInfo   = &(VkImageViewCreateInfo){
						.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						.viewType         = VK_IMAGE_VIEW_TYPE_2D,
						.format           = VK_FORMAT_R8G8B8A8_SRGB,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
							.levelCount = 1,
							.layerCount = 1,
						}
					},
				},
			},
			.accessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		},
		[TRANSIENT_ATTACHMENT_DEPTH] = {
			.outImage     = &app->attachments.imageIntermediateDepth,
			.createInfo = &(VkImageCreateInfo){
				.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
				.imageType     = VK_IMAGE_TYPE_2D,
				.format        = VK_FORMAT_D32_SFLOAT,
				.extent        = { windowWidth, windowHeight, 1 },
				.mipLevels     = 1,
				.arrayLayers   = 1,
				.samples       = VK_SAMPLE_COUNT_1_BIT,
				.tiling        = VK_IMAGE_TILING_OPTIMAL,
				.usage         = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
					| VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			},
			.imageViewCount = 1, .imageViews = (vkutil_image_view_desc[1]){
				{
					.outImageView = &app->attachments.imageViewIntermediateDepth,
					.createInfo   = &(VkImageViewCreateInfo){
						.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						.viewType         = VK_IMAGE_VIEW_TYPE_2D,
						.format           = VK_FORMAT_D32_SFLOAT,
						.subresourceRange = {
							.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
							.levelCount = 1,
							.layerCount = 1,
						}
					},
				},
			},
			.accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
							| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		},
	};

	int32_t ret = vkutil_create_images_helper (
		app->device.device, app->commandBufferStaging, app->queues[QUEUE_MAIN].queue,
		&app->device.memoryProperties, TRANSIENT_ATTACHMENT_COUNT, transientAttachmentCreateInfo,
		&app->attachments.memory
	);

	vkUpdateDescriptorSets (
		app->device.device,
		2, (VkWriteDescriptorSet[2]){
			{
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = app->descriptorSet[PIPELINE_DEFERRED_LIGHTING],
				.dstBinding      = 0,
				.descriptorCount = 3,
				.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
				.pImageInfo      = (VkDescriptorImageInfo[3]){
					{
						.imageView   = app->attachments.imageViewGBuffer[0],
					},
					{
						.imageView   = app->attachments.imageViewGBuffer[1],
					},
					{
						.imageView   = app->attachments.imageViewGBuffer[2],
					},
				},
			},
			{
				.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				.dstSet          = app->descriptorSet[PIPELINE_POST],
				.dstBinding      = 0,
				.descriptorCount = 1,
				.descriptorType  = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
				.pImageInfo      = &(VkDescriptorImageInfo){
					.imageView   = app->attachments.imageViewIntermediateColor,
				},
			},
		},
		0, NULL
	);

	return 0;
}

int32_t app_destroy_render_attachments ( app_t* app )
{
	vkDestroyImageView ( app->device.device, app->attachments.imageViewIntermediateColor, NULL );
	vkDestroyImageView ( app->device.device, app->attachments.imageViewIntermediateDepth, NULL );

	vkDestroyImage ( app->device.device, app->attachments.imageIntermediateColor, NULL );
	vkDestroyImage ( app->device.device, app->attachments.imageIntermediateDepth, NULL );

	vkFreeMemory ( app->device.device, app->attachments.memory, NULL );

	return 0;
}

////////////////////////////////////////
//