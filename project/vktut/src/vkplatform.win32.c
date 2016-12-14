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

#ifndef _WIN32
#error Wait... This is not Windows!
#endif

// We need to define VK_USE_PLATFORM_WIN32_KHR in order to use Windows specific functions
// This is not required outside of the platform-specific implementation
#define VK_USE_PLATFORM_WIN32_KHR 1
#include "vkbase.h"
#include <Windows.h>
#include <stdio.h>
#include <assert.h>

////////////////////////////////////////
// Windows defines. Mostly for windows. Heh. Heh.

#define WINDOW_WIDTH       1920
#define WINDOW_HEIGHT      1080
#define THROW_ERROR_DIALOG 1

////////////////////////////////////////
// Platform-specific data structure

typedef struct platform_data_s
{
	app_t* app;
	uint32_t exitRequested;
	uint32_t initialized;
} platform_data_t;

typedef struct platform_window_s
{
	HWND window;
	WNDCLASSEX wndClass;
	HINSTANCE instance;
	platform_data_t* platformData;
} platform_window_t;

////////////////////////////////////////
// Platform-specific application functions

int32_t platform_log_warning ( const char* warningFormat, ... )
{
	va_list va;
	va_start ( va, warningFormat );
	vprintf ( warningFormat, va );
	va_end ( va );
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

	char* buffer = _alloca ( l + 1 );

	va_start ( va, humanReadableFormat );
	int l2 = vsnprintf ( buffer, l+1, humanReadableFormat, va );
	va_end ( va );

	if ( l2 != l )
	{
		assert ( 0 );	// Shouldn't get here, assert if we do
		return code;	// Shouldn't fail here ideally
	}

	MessageBox ( NULL, buffer, "Error", MB_OK | MB_ICONERROR );
	return code;
#else
	// Just return the code
	return code;
#endif
}


int32_t platform_get_timestamp ( timestamp_t* outTimestamp )
{
	LARGE_INTEGER tick = { 0 };
	QueryPerformanceCounter ( &tick );
	*outTimestamp = (timestamp_t)tick.QuadPart;
	return 0;
}

int32_t platform_get_timestamp_freq ( timestamp_t* outFreq )
{
	LARGE_INTEGER freq = { 1 };	// To prevent /0 in case of failure
	QueryPerformanceFrequency ( &freq );
	*outFreq = freq.QuadPart;
	return 0;
}

////////////////////////////////////////
// Platform-specific I/O functions

int32_t platform_file_load ( file_t* outFile, const char* path )
{
	FILE* file;

	// We would like to prefix assets/ before the path, as the assets are provided in bin/assets/
	// whereas the executable is in bin/. The Android version has the "assets" folder as an asset
	// folder, and using their somewhat odd asset management system it does not need the assets/
	// prefix, so we don't do it in the application.
	size_t len = 7 + strlen ( path ) + 1;
	char* fullPath = _alloca ( len );
	strcpy_s ( fullPath, len, "assets/" );
	strcat_s ( fullPath, len, path );

	errno_t err = fopen_s ( &file, fullPath, "rb" );
	if ( err != 0 )
		return platform_throw_error ( -1, "Could not open file %s", fullPath );
	fseek ( file, 0, SEEK_END );
	long size = ftell ( file );
	fseek ( file, 0, SEEK_SET );
	
	uint8_t* data = malloc ( size );
	fread ( data, 1, size, file );
	fclose ( file );

	outFile->data = data;
	outFile->sizeInBytes = size;

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
	char buffer[32];
	snprintf ( buffer, sizeof ( buffer ), "%s", name );

	outFile->platform = NULL;
	errno_t err = fopen_s ( (FILE**)&outFile->platform, buffer, "wb" );
	if ( err != 0 )
		return -1;

	return 0;
}

int32_t platform_log_file_print ( log_file_t* file, const char* format, ... )
{
	va_list va;
	va_start(va,format);
	vfprintf_s ( (FILE*)file->platform, format, va );
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

static LRESULT CALLBACK WndProc ( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{
	platform_window_t* wnd = (platform_window_t*)GetWindowLongPtr ( window, GWLP_USERDATA );

	switch ( message )
	{
	case WM_SIZE:
		{
			if ( wnd->platformData->initialized != 1 )
				break;

			uint32_t width  = LOWORD(lParam);
			uint32_t height = HIWORD(lParam);
			int32_t ret = app_resize ( wnd->platformData->app, width, height );
			if ( ret != 0 )
				platform_throw_error ( ret, "app_resize failed" );
			return 0;
		}
		break;

	case WM_CLOSE:
		DestroyWindow ( window );
		return 0;

	case WM_DESTROY:
		wnd->platformData->exitRequested = 1;
		return 0;
	}
	return DefWindowProc ( window, message, wParam, lParam );
}

////////////////////////////////////////
// Platform-specific application functions

int32_t platform_window_create ( window_t* outWindow, void* userdata )
{
	platform_window_t* window = calloc ( 1, sizeof ( platform_window_t ) );
	window->platformData = userdata;

	HRESULT hr = S_OK;

	// Create window type
	window->instance = GetModuleHandle(NULL);
	window->wndClass = (WNDCLASSEX){
		.style         = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc   = WndProc,
		.hInstance     = window->instance,
		.hIcon         = LoadIcon(NULL, IDI_WINLOGO),
		.hIconSm       = LoadIcon(NULL, IDI_WINLOGO),
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH),
		.lpszClassName = "VulkanWindow",
		.cbSize        = sizeof(WNDCLASSEX),
	};
	hr = RegisterClassEx ( &window->wndClass );
	if ( !SUCCEEDED ( hr ) )
		return platform_throw_error ( -1, "RegisterClassEx failed with code %u", hr );

	// Resize the desired window size to account for window edges & top bar
	RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	BOOL success = AdjustWindowRect ( &windowRect, WS_OVERLAPPEDWINDOW, FALSE );
	if ( success == FALSE )
		return platform_throw_error( -2, "AdjustWindowRect failed with code %u", GetLastError ( ));

	// Create the window
	window->window = CreateWindowEx (
		WS_EX_APPWINDOW,
		"VulkanWindow", "Vulkan example", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, window->instance, NULL
	);
	if ( window->window == NULL )
		return platform_throw_error ( -3, "CreateWindowEx failed with code %u", GetLastError ( ) );

	SetWindowLongPtr ( window->window, GWLP_USERDATA, (LONG_PTR)window );
	BOOL ret = ShowWindow ( window->window, SW_SHOW );

	// Output platform structure and success code
	outWindow->platform = window;
	return 0;
}

int32_t platform_window_get_size ( window_t* window, uint32_t* outWidth, uint32_t* outHeight )
{
	platform_window_t* platformWindow = window->platform;
	RECT rect;
	BOOL result = GetClientRect ( platformWindow->window, &rect );
	if ( result != TRUE )
		return platform_throw_error ( -1, "GetClientRect failed with code %u", GetLastError ( ) );
	*outWidth  = rect.right - rect.left;
	*outHeight = rect.bottom - rect.top;
	return 0;
}

////////////////////////////////////////
// Platform-specific Vulkan functions

int32_t platform_vulkan_get_required_instance_layers (
	VkLayerProperties* layers, uint32_t layerCount, const char*** outLayers, uint32_t* outCount
)
{
#if VK_ENABLE_DEBUG
	static const char* Layers[] = {
		//"VK_LAYER_LUNARG_standard_validation"
		"VK_LAYER_LUNARG_core_validation",
		"VK_LAYER_LUNARG_swapchain",
		"VK_LAYER_LUNARG_image",
		"VK_LAYER_GOOGLE_unique_objects",
		"VK_LAYER_LUNARG_parameter_validation",
		"VK_LAYER_LUNARG_object_tracker",
	};
	*outLayers = Layers;
	*outCount  = sizeof ( Layers ) / sizeof ( Layers[0] );
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
		"VK_KHR_win32_surface",		// Allows for platform-specific output targets
#if VK_ENABLE_DEBUG
		"VK_EXT_debug_report",		// Allows for debug output functionality
#endif
	};

	// VK_EXT_debug_report is supported, and (to be) enabled in the case of VK_ENABLE_DEBUG
	// As such we pass "debug supported" as non-zero (supported) as opposed to zero (not supported)
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
	VkResult vkResult = vkCreateWin32SurfaceKHR (
		instance,
		&(VkWin32SurfaceCreateInfoKHR){
			.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = platformWindow->instance,
			.hwnd      = platformWindow->window,
		},
		NULL,
		outSurface
	);
	if ( vkResult != VK_SUCCESS )
		return platform_throw_error ( -1, "vkCreateWin32SurfaceKHR failed with code %u", vkResult);
	return 0;
}

////////////////////////////////////////
// Entry point

int main ( int argc, char* argv[] )
{
	platform_data_t data = { 0 };
	int32_t ret = app_init ( &data.app, &data );
	if ( ret != 0 )
		return ret;

	data.initialized = 1;

	LARGE_INTEGER cur, prev;
	QueryPerformanceFrequency ( &prev );
	QueryPerformanceCounter ( &cur );

	double tickToSec = 1.0 / prev.QuadPart;

	while ( 1 )
	{
		prev = cur;

		MSG msg;
		while ( PeekMessage ( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage ( &msg );
			DispatchMessage ( &msg );
		}

		if ( data.exitRequested )
			break;

		QueryPerformanceCounter ( &cur );
		LONGLONG delta = cur.QuadPart - prev.QuadPart;

		app_render ( data.app, tickToSec * delta );
	}

	return app_free ( data.app );
}