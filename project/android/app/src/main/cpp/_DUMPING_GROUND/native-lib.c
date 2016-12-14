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

////////////////////////////////////////
// Setting defines

#define SHOW_ERROR_DIALOG      1	// 1 to show dialog upon error, 0 to just return code
#define WINDOW_WIDTH           640
#define WINDOW_HEIGHT          480
#define ENABLE_VK_DEBUG_REPORT 0

static const char* VulkanInstanceLayers[] = {
        "VK_LAYER_LUNARG_standard_validation"
};
static const char* VulkanInstanceExtensions[] = {
        "VK_KHR_surface",
#if defined ( _WIN32 )
        "VK_KHR_win32_surface",
#elif defined ( __ANDROID__ )
        "VK_KHR_android_surface",
#else
#error Unknown platform
#endif
#if ENABLE_VK_DEBUG_REPORT
        "VK_EXT_debug_report",	// Enables debug output
#endif
};

static const char* VulkanDeviceExtensions[] = {
        "VK_KHR_swapchain",
};

////////////////////////////////////////
// Platform-specific includes

#if defined ( _WIN32 )
#define VK_USE_PLATFORM_WIN32_KHR 1
#include <vulkan\vulkan.h>
#include <Windows.h>
#define alloca _alloca
#elif defined ( __ANDROID__ )
#define VK_USE_PLATFORM_ANDROID_KHR 1

#if __ANDROID_API__ <= 23
// API level 23 does not contain core support for Vulkan, although some phones have Vulkan
// support regardless to be loaded dynamically. vulkan_wrapper is Google's general solution
// for this. If API level 24+ is targeted, vulkan.h can be loaded directly
#include <vulkan_wrapper.h>
#else
#include <vulkan/vulkan.h>
#endif

#include <alloca.h>
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include "android_native_app_glue.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))
#else
#error Unknown platform
#endif

////////////////////////////////////////
// Dependent includes

#include <assert.h>
#include <stdio.h>

////////////////////////////////////////
// Application function declarations

typedef struct app_s app_t;

int32_t app_throw_error ( int32_t code, const char* humanReadableFormat, ... );

int32_t app_create_window ( app_t* app );
int32_t app_create_vulkan ( app_t* app );

////////////////////////////////////////
// Application data container

typedef struct app_s
{
    // Platform-specific data
    struct
    {
#if defined ( _WIN32 )
        HWND window;
		WNDCLASSEX wndClass;
		HINSTANCE instance;
#elif defined ( __ANDROID__ )
        struct android_app* app;
#else
#error Unknown platform
#endif
    } platform;

    // Vulkan instance data
    struct
    {
        VkInstance instance;
        VkSurfaceKHR surface;
        VkDebugReportCallbackEXT debugCallback;
    } vkInstance;

    // Vulkan device data
    struct
    {
        VkDevice device;
        VkSwapchainKHR swapchain;
    } vkDevice;
} app_t;

////////////////////////////////////////
// Entry point

int32_t app_run ( void* arg )
{
    int32_t result = 0;

    // Null-initialize an empty app_t structure
    app_t app = { 0 };

#if defined ( __ANDROID__ )
    // We will store the value of the android_app structure in the app_t structure
    // This way we can refer to it all the way through the application
    app.platform.app = arg;
#endif

    result = app_create_window ( &app );
    if ( result != 0 )
        return result;

    result = app_create_vulkan ( &app );
    if ( result != 0 )
        return result;

    return 0;
}

#if defined ( _WIN32 )
int main ( int argc, char* argv[] )
{
    app_run(NULL);
}
#elif defined ( __ANDROID__ )
static void startup_cmd_handler(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
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

void android_main(struct android_app* state)
{
#if __ANDROID_API__ <= 23
    InitVulkan();
#endif
    state->userData = NULL;
    state->onAppCmd = startup_cmd_handler;

    int ident, events;
    struct android_poll_source* source;

    // If not animating, we will block forever waiting for events.
    // If animating, we loop until all events are read, then continue
    // to draw the next frame of animation.
    while (state->window == NULL && (ident=ALooper_pollAll(-1, NULL, &events, (void**)&source)) >= 0)
    {
        if ( source != NULL )
            source->process ( state, source );
    }

    app_run(state);
}
#endif

////////////////////////////////////////
// Application functions

int32_t app_throw_error ( int32_t code, const char* humanReadableFormat, ... )
{
#if SHOW_ERROR_DIALOG
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

#if defined ( _WIN32 )
        MessageBox ( NULL, buffer, "Error", MB_OK | MB_ICONERROR );
#elif defined ( __ANDROID__ )
    LOGE("%s", buffer);
#else
#error Unknown platform
#endif
    return code;
#else
    // Just return the code
	return code;
#endif
}

#ifdef _WIN32
LRESULT CALLBACK WndProc ( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message )
	{
	case WM_CLOSE:
		DestroyWindow ( window );
		return 0;

	case WM_DESTROY:
		PostQuitMessage ( 1 );
		return 0;
	}
	return DefWindowProc ( window, message, wParam, lParam );
}
#endif

int32_t app_create_window ( app_t* app )
{
#ifdef _WIN32
    HRESULT hr = S_OK;

	app->platform.instance = GetModuleHandle(NULL);

	// Create window type
	app->platform.wndClass = (WNDCLASSEX){
		.style         = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc   = WndProc,
		.hInstance     = app->platform.instance,
		.hIcon         = LoadIcon(NULL, IDI_WINLOGO),
		.hIconSm       = LoadIcon(NULL, IDI_WINLOGO),
		.hCursor       = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH),
		.lpszClassName = "VulkanWindow",
		.cbSize        = sizeof(WNDCLASSEX),
	};
	hr = RegisterClassEx ( &app->platform.wndClass );
	if ( !SUCCEEDED ( hr ) )
		return app_throw_error ( -1, "RegisterClassEx failed with code %u", hr );

	// Resize the desired window size to account for window edges & top bar
	RECT windowRect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	BOOL success = AdjustWindowRect ( &windowRect, WS_OVERLAPPEDWINDOW, FALSE );
	if ( success == FALSE )
		return app_throw_error ( -2, "AdjustWindowRect failed with code %u", GetLastError ( ) );

	// Create the window
	app->platform.window = CreateWindowEx (
		WS_EX_APPWINDOW,
		"VulkanWindow", "Vulkan example", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, app->platform.instance, NULL
	);
	if ( app->platform.window == NULL )
		return app_throw_error ( -3, "CreateWindowEx failed with code %u", GetLastError ( ) );
#elif defined ( __ANDROID__ )
    // No window creation code required, seeing as well... There is no such thing
#else
#error Not implemented on this platform
#endif

    return 0;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
                    uint64_t srcObject, size_t location, int32_t msgCode,
                    const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
    app_throw_error ( msgCode, "[VULKAN] Debug callback invoked\n"
                              "Flags: %u\n"
                              "Object type: %u\n"
                              "Source object: 0x%X\n"
                              "Location: %llu\n"
                              "Message code: %d\n"
                              "Layer prefix: %s\n"
                              "Message: %s",
                      msgFlags, objType, srcObject, location, msgCode, pLayerPrefix, pMsg
    );

    // We did not "handle" the error
    return 0;
}

#define STATIC_ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

int32_t app_create_vulkan ( app_t* app )
{
    VkResult vkResult = VK_SUCCESS;

    // Query the amount of layers and extensions available in the installed version of the VulkanRT
    uint32_t layerCount = 0, extensionCount = 0;
    vkResult = vkEnumerateInstanceLayerProperties ( &layerCount, NULL );
    if ( vkResult != VK_SUCCESS )
        return app_throw_error ( -1, "vkEnumerateInstanceLayerProperties failed (%u)", vkResult );
    vkResult = vkEnumerateInstanceExtensionProperties ( NULL, &extensionCount, NULL );
    if ( vkResult != VK_SUCCESS )
        return app_throw_error ( -1, "vkEnumerateInstanceExtensionProperties failed (%u)", vkResult );

    // Allocate space on the stack for the layers and extensions
    VkLayerProperties* layers         = alloca ( layerCount     * sizeof ( VkLayerProperties ) );
    VkExtensionProperties* extensions = alloca ( extensionCount * sizeof ( VkExtensionProperties ) );

    // Obtain all layers and extensions
    vkResult = vkEnumerateInstanceLayerProperties ( &layerCount, layers );
    if ( vkResult != VK_SUCCESS )
        return app_throw_error ( -1, "vkEnumerateInstanceLayerProperties failed (%u)", vkResult );
    vkResult = vkEnumerateInstanceExtensionProperties ( NULL, &extensionCount, extensions );
    if ( vkResult != VK_SUCCESS )
        return app_throw_error ( -1, "vkEnumerateInstanceExtensionProperties failed (%u)", vkResult );

    // Verify availability of required layers and extensions
#if defined ( _WIN32 )
    // Did not get layers to work on Android just yet. I should get it to, though...~
    for ( uint32_t i = 0; i < STATIC_ARRAY_LENGTH(VulkanInstanceLayers); i++ )
    {
        uint32_t found = 0;
        for ( uint32_t j = 0; j < layerCount; j++ )
        {
            if ( strcmp ( layers[j].layerName, VulkanInstanceLayers[i] ) == 0 )
            {
                found = 1;
                break;
            }
        }

        if ( !found )
            return app_throw_error ( -1, "Required layer %s not supported", VulkanInstanceLayers[i] );
    }
#endif

    for ( uint32_t i = 0; i < STATIC_ARRAY_LENGTH(VulkanInstanceExtensions); i++ )
    {
        uint32_t found = 0;
        for ( uint32_t j = 0; j < extensionCount; j++ )
        {
            if ( strcmp ( extensions[j].extensionName, VulkanInstanceExtensions[i] ) == 0 )
            {
                found = 1;
                break;
            }
        }

        if ( !found )
            return app_throw_error ( -1, "Required extension %s not supported", VulkanInstanceExtensions[i] );
    }

    // Initialize Vulkan
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
#ifndef __ANDROID__
                    .enabledLayerCount       = STATIC_ARRAY_LENGTH ( VulkanInstanceLayers ),
                    .ppEnabledLayerNames     = VulkanInstanceLayers,
#endif
                    .enabledExtensionCount   = STATIC_ARRAY_LENGTH ( VulkanInstanceExtensions ),
                    .ppEnabledExtensionNames = VulkanInstanceExtensions,
            },
            NULL,	// We are not currently interested in a custom memory allocator
            &app->vkInstance.instance
    );
    if ( vkResult != VK_SUCCESS )
        return app_throw_error ( -1, "vkCreateInstance failed with error %u", vkResult );

#if ENABLE_VK_DEBUG_REPORT
    // For the purposes of this function, we would need to call vkCreateDebugReportCallbackEXT
    // However, as an extension (suffix EXT) it is not included in the Vulkan loader, and instead
    // a function in the Vulkan runtime.

    // In order to call this function, we will need to query it first.

    PFN_vkCreateDebugReportCallbackEXT createDebugReportFunc = (PFN_vkCreateDebugReportCallbackEXT)
            vkGetInstanceProcAddr ( app->vkInstance.instance, "vkCreateDebugReportCallbackEXT" );

    vkResult = createDebugReportFunc (
            app->vkInstance.instance,
            &(VkDebugReportCallbackCreateInfoEXT){
                    .sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
                    .flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
                    .pfnCallback = VulkanDebugCallback,
                    .pUserData   = app,
            },
            NULL,
            &app->vkInstance.debugCallback
    );
    if ( vkResult != VK_SUCCESS )
        return app_throw_error ( -1, "vkCreateDebugReportCallbackEXT failed with error %u", vkResult );
#endif

#if defined ( _WIN32 )
    vkResult = vkCreateWin32SurfaceKHR (
		app->vkInstance.instance,
		&(VkWin32SurfaceCreateInfoKHR){
			.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = app->platform.instance,
			.hwnd      = app->platform.window,
		},
		NULL,
		&app->vkInstance.surface
	);
#elif defined ( __ANDROID__ )
    vkResult = vkCreateAndroidSurfaceKHR (
            app->vkInstance.instance,
            &(VkAndroidSurfaceCreateInfoKHR){
                    .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
                    .window = app->platform.app->window,
            },
            NULL,
            &app->vkInstance.surface
    );
#else
#error Unknown platform
#endif
    if ( vkResult != VK_SUCCESS )
        return app_throw_error ( -1, "vkCreateWin32SurfaceKHR failed with error %u", vkResult );





    return 0;
}



#if 0
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include "android_native_app_glue.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    //glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    // Just fill the screen with a color.
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
                 ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                               engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                               engine->accelerometerSensor, (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                                engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
                                                                 ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
                                                              state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                                      (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                                                       &event, 1) > 0) {
                        LOGI("accelerometer: x=%f y=%f z=%f",
                             event.acceleration.x, event.acceleration.y,
                             event.acceleration.z);
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}
#endif

#if 0
#include <jni.h>
#include <string>

extern "C"
jstring
Java_nl_nhtv_vktut_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}*/
#endif