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

#ifndef __ANDROID__
#error Wait... This is not Android!
#endif

// We need to define VK_USE_PLATFORM_WIN32_KHR in order to use Windows specific functions
// This is not required outside of the platform-specific implementation
#define VK_USE_PLATFORM_ANDROID_KHR 1
#include "vkbase.h"
#include <alloca.h>
#include <jni.h>
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <dlfcn.h>

#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))

////////////////////////////////////////
// Android defines

#define THROW_ERROR_DIALOG 1
#define STATIC_ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

////////////////////////////////////////
// Platform-specific data structure

typedef struct platform_window_s
{
	struct android_app* app;
} platform_window_t;

static struct android_app* APP;	// I seriously don't like this, but adding a parameter for all
// platforms is less preferable

////////////////////////////////////////
// Platform-specific application functions

int32_t platform_log_warning ( const char* humanReadableFormat, ... )
{
	// The code below does the following:
	// 1. Get all variable params in variable "va"
	// 2. Run printf on a NULL buffer to get the total text length
	// 3. Allocate enough bytes (text_length + NULL character) to store the string from the stack
	// 4. Reset variable params
	// 5. Actually print to the buffer
	// 6. Show result in a dialog box

	va_list va;
	va_start ( va, humanReadableFormat );
	int l = vsnprintf ( NULL, 0, humanReadableFormat, va );
	va_end ( va );

	if ( l < 0 )
	{
		assert ( 0 );	// Shouldn't get here, assert if we do
		return -1;	// Printf failed, just return the code
	}

	char* buffer = alloca ( l + 1 );

	va_start ( va, humanReadableFormat );
	int l2 = vsnprintf ( buffer, l+1, humanReadableFormat, va );
	va_end ( va );

	if ( l2 != l )
	{
		assert ( 0 );	// Shouldn't get here, assert if we do
		return -2;	// Shouldn't fail here ideally
	}

	LOGW("%s", buffer);

	return 0;
}

int32_t platform_throw_error ( int32_t code, const char* humanReadableFormat, ... )
{
#if THROW_ERROR_DIALOG
	// The code below does the following:
	// 1. Get all variable params in variable "va"
	// 2. Run printf on a NULL buffer to get the total text length
	// 3. Allocate enough bytes (text_length + NULL character) to store the string from the stack
	// 4. Reset variable params
	// 5. Actually print to the buffer
	// 6. Show result in a dialog box

	va_list va;
	va_start ( va, humanReadableFormat );
	int l = vsnprintf ( NULL, 0, humanReadableFormat, va );
	va_end ( va );

	if ( l < 0 )
	{
		assert ( 0 );	// Shouldn't get here, assert if we do
		return code;	// Printf failed, just return the code
	}

	char* buffer = alloca ( l + 1 );

	va_start ( va, humanReadableFormat );
	int l2 = vsnprintf ( buffer, l+1, humanReadableFormat, va );
	va_end ( va );

	if ( l2 != l )
	{
		assert ( 0 );	// Shouldn't get here, assert if we do
		return code;	// Shouldn't fail here ideally
	}

#if 1
	LOGE("%s", buffer);
#else
	JNIEnv* env = APP->activity->env;



	jclass actClass = (*env)->FindClass ( env, "nl/nhtv/vktut/WrappedNativeActivity" );
	jmethodID dialogFunc = (*env)->GetMethodID ( env, actClass, "ShowPopup", "(Ljava/lang/String;)V");
	jstring jstr = (*env)->NewStringUTF ( env, buffer );
	(*env)->CallVoidMethod ( env, APP->activity->clazz, dialogFunc, jstr );
	// Get the method that you want to call
	//jmethodID messageMe = env->GetMethodID(clazz, "messageMe", "(Ljava/lang/String;)V");
	// Call the method on the object
	//jobject result = env->CallVMethod(jstr, messageMe);
#endif
	return code;
#else
	// Just return the code
	return code;
#endif
}

int32_t platform_get_timestamp ( timestamp_t* outTimestamp )
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	*outTimestamp = ts.tv_sec * 1000000000 + ts.tv_nsec;
	return 0;
}

int32_t platform_get_timestamp_freq ( timestamp_t* outTimestampFreq )
{
	*outTimestampFreq = 1000000000;
	return 0;
}

////////////////////////////////////////
// Platform-specific I/O functions

int32_t platform_file_load ( file_t* outFile, const char* path )
{
	AAsset* file = AAssetManager_open( APP->activity->assetManager, path, AASSET_MODE_BUFFER );

	if ( file == NULL )
		platform_throw_error( -1, "Unable to open file %s", path );

	outFile->sizeInBytes = AAsset_getLength64 ( file );

	uint8_t* buffer = malloc ( outFile->sizeInBytes + 1 );
	buffer[outFile->sizeInBytes] = '\0';
	memcpy ( buffer, AAsset_getBuffer( file ), outFile->sizeInBytes );

	AAsset_close ( file );

	outFile->data = buffer;

	return 0;
}

int32_t platform_file_close ( file_t* file )
{
	if ( file->data == NULL )
		return platform_throw_error ( -1, "Attempting to close a NULL file" );
	free ( file->data );
	file->data        = NULL;
	file->sizeInBytes = 0;
	return 0;
}

int32_t platform_log_file_create ( log_file_t* outFile, const char* name )
{
	char buffer[256];
	snprintf ( buffer, sizeof ( buffer ), "%s/%s", APP->activity->externalDataPath, name );

	outFile->platform = NULL;
	outFile->platform = fopen ( buffer, "wb" );
	if ( outFile->platform == NULL )
		return -1;

	return 0;
}

int32_t platform_log_file_print ( log_file_t* file, const char* format, ... )
{
	va_list va;
	va_start(va,format);
	vfprintf ( (FILE*)file->platform, format, va );
	va_end(va);
	return 0;
}

int32_t platform_log_file_write ( log_file_t* file, const void* data, uint64_t size )
{
	fwrite ( data, size, 1, (FILE*)file->platform );
	return 0;
}

int32_t platform_log_file_close ( log_file_t* file )
{
	fclose ( (FILE*)file->platform );
	return 0;
}

////////////////////////////////////////
// Platform-specific callback functions

static void startup_cmd_handler(struct android_app* app, int32_t cmd)
{
	switch (cmd) {
		case APP_CMD_SAVE_STATE:
		case APP_CMD_GAINED_FOCUS:
		case APP_CMD_LOST_FOCUS:
		case APP_CMD_TERM_WINDOW:
			break;
		case APP_CMD_INIT_WINDOW:
			break;
	}
}

////////////////////////////////////////
// Platform-specific application functions

int32_t platform_window_create ( window_t* outWindow, void* _app )
{
	// Since Android does not use a window manager in order to create windows, but instead creates
	// a window for us to be able to use, we just wait for Android to give us a window to use.
	// It is not the nicest method for creating a window, but alas.

	struct android_app* app = _app;
	platform_window_t* window = malloc ( sizeof ( platform_window_t ) );
	window->app = app;

	app->onAppCmd = startup_cmd_handler;

	int ident, events;
	struct android_poll_source* source;

	// If not animating, we will block forever waiting for events.
	// If animating, we loop until all events are read, then continue
	// to draw the next frame of animation.
	while (app->window == NULL && (ident=ALooper_pollAll(-1, NULL, &events, (void**)&source)) >= 0)
	{
		if ( source != NULL )
			source->process ( app, source );
	}

	outWindow->platform = window;
	return 0;
}

int32_t platform_window_get_size ( window_t* window, uint32_t* outWidth, uint32_t* outHeight )
{
	platform_window_t* platformWindow = window->platform;
	*outWidth  = ANativeWindow_getWidth  ( platformWindow->app->window );
	*outHeight = ANativeWindow_getHeight ( platformWindow->app->window );
	return 0;
}

////////////////////////////////////////
// Platform-specific Vulkan functions

int32_t platform_vulkan_get_required_instance_layers (
		VkLayerProperties* layers, uint32_t layerCount, const char*** outLayers, uint32_t* outCount
)
{
#if VK_ENABLE_DEBUG
	// Whereas VK_LAYER_LUNARG_standard_validation would be ideal, this meta-layer is not supported
	// on the Android implementation I tested. As such, the validation layers I did find are used
	// instead.
	static const char* RequiredLayers[] = {
		"VK_LAYER_GOOGLE_threading",
		"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_object_tracker",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_LUNARG_image",
		"VK_LAYER_LUNARG_parameter_validation",
		//"VK_LAYER_GOOGLE_unique_objects",	// CRASHES ON vkCreateSwapchain
	};
	static const char* OptionalLayers[] = {
		0,//"VK_LAYER_ARM_MGD",	// ARM Mali graphics debugger
	};
	static const char* Layers[STATIC_ARRAY_LENGTH(RequiredLayers) + STATIC_ARRAY_LENGTH(OptionalLayers)];

	uint32_t cnt = 0;//STATIC_ARRAY_LENGTH(RequiredLayers);
	memcpy ( Layers, RequiredLayers, sizeof ( RequiredLayers ) );
	for ( uint32_t i = 0; i < STATIC_ARRAY_LENGTH(OptionalLayers); i++ )
	{
		if ( OptionalLayers[i] == NULL )
			continue;
		for ( uint32_t j = 0; j < layerCount; j++ )
		{
			if ( strcmp ( OptionalLayers[i], layers[j].layerName ) == 0 )
			{
				Layers[cnt++] = OptionalLayers[i];
				break;
			}
		}
	}

	//vkEnumerateInstanceExtensionProperties ( "VK_LAYER_ARM_MGD", &cnt, NULL );

	*outLayers = Layers;
	*outCount  = cnt;
	return 0;
#else
	// Since layers are for the most part intended for debug purposes, we have no specific
	// layers we would like to enable in this case.
	// Some layers do have utility outside of debug builds, such as the Steam overlay, but these
	// are usually in the registry as "implicit" layers, and as such included by default
	*outLayers = NULL;
	*outCount  = 0;
	return 0;
#endif
}

int32_t platform_vulkan_get_required_instance_extensions (
	const char*** outExtensions, uint32_t* outCount, uint32_t* outDebugSupported
)
{
	// While in the case of layers we do not have any specific preference in the non-debug case
	// for extensions we need the layers having to do with output at the very least.
	static const char* Extensions[] = {
		"VK_KHR_surface",			// Allows for output targets (window render targets)
		"VK_KHR_android_surface",	// Allows for platform-specific output targets
	};

	// VK_EXT_debug_report is supported, but not reported by the Android implementation I tested
#if VK_ENABLE_DEBUG
	*outDebugSupported = 1;
#else
	*outDebugSupported = 0;
#endif

	*outExtensions = Extensions;
	*outCount      = sizeof ( Extensions ) / sizeof ( Extensions[0] );
	return 0;
}

int32_t platform_vulkan_get_required_device_extensions (
		const char*** outExtensions, uint32_t* outCount
)
{
	static const char* Extensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	*outExtensions = Extensions;
	*outCount      = sizeof ( Extensions ) / sizeof ( Extensions[0] );
	return 0;
}

int32_t platform_vulkan_create_surface (
	VkSurfaceKHR* outSurface, VkInstance instance, window_t* window
)
{
	platform_window_t* platformWindow = window->platform;
	VkResult vkResult = vkCreateAndroidSurfaceKHR (
		instance,
		&(VkAndroidSurfaceCreateInfoKHR){
			.sType  = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
			.window = platformWindow->app->window,
		},
		NULL,
		outSurface
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error (
			-1, "vkCreateAndroidSurfaceKHR failed with code %u", vkResult
		);
	return 0;
}

////////////////////////////////////////
// Entry point

void android_main(struct android_app* androidApp)
{
#if __ANDROID_API__ <= 23
	InitVulkan();
#endif

	APP = androidApp;

	app_t* app;
	androidApp->userData = app;

	int32_t ret = app_init ( &app, androidApp );
	if ( ret != 0 )
		return;

	struct timespec prev;
	clock_gettime(CLOCK_MONOTONIC, &prev);

	while ( 1 )
	{
		int ident, events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		do {
			while ((ident = ALooper_pollAll(0, NULL, &events, (void **) &source)) >= 0) {
				if (source != NULL)
					source->process(androidApp, source);
			}
		} while ( !androidApp->window && !androidApp->destroyRequested );

		if ( androidApp->destroyRequested )
			break;

		struct timespec cur;
		clock_gettime(CLOCK_MONOTONIC, &cur);

		double pt = prev.tv_sec + 0.000000001 * prev.tv_nsec;
		double ct =  cur.tv_sec + 0.000000001 *  cur.tv_nsec;

		//double dt = (cur.tv_sec - prev.tv_sec) + (0.000000001 * (cur.tv_nsec - prev.tv_nsec));
		double dt = ct - pt;
		prev = cur;

		app_render ( app, dt );
	}

	app_free ( app );
	androidApp->running = 0;
}