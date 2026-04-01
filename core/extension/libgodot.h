/**************************************************************************/
/*  libgodot.h                                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/extension/gdextension_interface.gen.h"
#include "core/extension/libgodot_display_server.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Export macros for DLL visibility
#if defined(_MSC_VER) || defined(__MINGW32__)
#define LIBGODOT_API __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define LIBGODOT_API __attribute__((visibility("default")))
#else
#define LIBGODOT_API
#endif

typedef enum {
	LIBGODOT_LOG_LEVEL_INFO = 0,
	LIBGODOT_LOG_LEVEL_WARNING = 1,
	LIBGODOT_LOG_LEVEL_ERROR = 2,
} LibGodotLogLevel;

typedef enum {
	LIBGODOT_STATUS_UNINITIALIZED = 0,
	LIBGODOT_STATUS_CORE_READY = 1,
	LIBGODOT_STATUS_SERVERS_READY = 2,
	LIBGODOT_STATUS_WARMING_UP = 3,
	LIBGODOT_STATUS_PROJECT_LOADING = 4,
	LIBGODOT_STATUS_RUNNING = 5,
	LIBGODOT_STATUS_PROJECT_UNLOADING = 6,
	LIBGODOT_STATUS_IDLE = 7,
	LIBGODOT_STATUS_STOPPING = 8,
	LIBGODOT_STATUS_STOPPED = 9,
	LIBGODOT_STATUS_ERROR = 10,
} LibGodotStatus;

/**
 * Callback type for receiving engine status changes.
 *
 * @param p_user_data User data pointer passed to libgodot_set_status_callback.
 * @param p_status The new engine status.
 * @param p_detail Optional detail string (UTF-8) describing the sub-phase, or NULL.
 */
typedef void (*LibGodotStatusCallback)(void *p_user_data, LibGodotStatus p_status, const char *p_detail);

/**
 * Callback type for receiving engine log messages.
 *
 * @param p_user_data User data pointer passed to libgodot_set_log_callback.
 * @param p_level The severity level of the message.
 * @param p_message The log message as a UTF-8 string.
 */
typedef void (*LibGodotLogCallback)(void *p_user_data, LibGodotLogLevel p_level, const char *p_message);

/**
 * @name libgodot_create_godot_instance
 * @since 4.6
 *
 * Creates a new Godot instance.
 *
 * @param p_argc The number of command line arguments.
 * @param p_argv The C-style array of command line arguments.
 * @param p_init_func Optional GDExtension initialization function for registering custom
 *        native classes/types with the engine. Pass NULL if you just want to run a project
 *        without extending the engine from the host application.
 *
 * @return A pointer to created \ref GodotInstance GDExtension object or nullptr if there was an error.
 */
LIBGODOT_API GDExtensionObjectPtr libgodot_create_godot_instance(int p_argc, char *p_argv[], GDExtensionInitializationFunction p_init_func);

/**
 * @name libgodot_destroy_godot_instance
 * @since 4.6
 *
 * Destroys an existing Godot instance.
 *
 * @param p_godot_instance The reference to the GodotInstance object to destroy.
 *
 */
LIBGODOT_API void libgodot_destroy_godot_instance(GDExtensionObjectPtr p_godot_instance);

/**
 * Sets up and starts the engine. If successful, it also initializes the main loop.
 */
LIBGODOT_API bool libgodot_start_godot_instance(GDExtensionObjectPtr p_godot_instance);

/**
 * Returns whether the engine has already been started.
 */
LIBGODOT_API bool libgodot_is_godot_instance_started(GDExtensionObjectPtr p_godot_instance);

/**
 * Runs one iteration of the engine: processes display events and updates the main loop.
 */
LIBGODOT_API bool libgodot_iteration_godot_instance(GDExtensionObjectPtr p_godot_instance);

/**
 * Finalizes the main loop and stops the engine instance.
 */
LIBGODOT_API void libgodot_stop_godot_instance(GDExtensionObjectPtr p_godot_instance);

/**
 * Sends a notification that the application has gained focus.
 */
LIBGODOT_API void libgodot_focus_in_godot_instance(GDExtensionObjectPtr p_godot_instance);

/**
 * Sends a notification that the application has lost focus.
 */
LIBGODOT_API void libgodot_focus_out_godot_instance(GDExtensionObjectPtr p_godot_instance);

/**
 * Sends a notification that the application has been paused.
 */
LIBGODOT_API void libgodot_pause_godot_instance(GDExtensionObjectPtr p_godot_instance);

/**
 * Sends a notification that the application has resumed after being paused.
 */
LIBGODOT_API void libgodot_resume_godot_instance(GDExtensionObjectPtr p_godot_instance);

/**
 * Loads and starts a Godot project from the given path (project folder or .pck).
 * The engine must have been created with libgodot_create_godot_instance() first.
 */
LIBGODOT_API bool libgodot_load_project(GDExtensionObjectPtr p_godot_instance, const char *p_project_path);

/**
 * Stops the currently running project and frees its resources without tearing down the engine.
 */
LIBGODOT_API void libgodot_unload_project(GDExtensionObjectPtr p_godot_instance);

/**
 * Convenience helper to stop the current project (if any) and load the next one.
 */
LIBGODOT_API bool libgodot_reload_project(GDExtensionObjectPtr p_godot_instance, const char *p_project_path);

/**
 * Returns the native window handle for a given handle type and window ID.
 *
 * @param p_handle_type The type of handle to retrieve (DisplayServer::HandleType as int32_t).
 * @param p_window_id The window ID (DisplayServer::WindowID as int32_t).
 *
 * @return The native handle as uint64_t, or 0 if not available.
 */
LIBGODOT_API uint64_t libgodot_window_get_native_handle(int32_t p_handle_type, int32_t p_window_id);

/**
 * Pre-compiles all built-in engine shaders (Metal, Vulkan, D3D12) asynchronously.
 * Ensures servers are initialized (calls _ensure_setup if needed), then dispatches
 * all embedded shader variant compilations to background worker threads.
 *
 * Status transitions: CORE_READY -> SERVERS_READY -> WARMING_UP -> IDLE.
 * The transition from WARMING_UP to IDLE happens automatically when all shader
 * compilations complete. Monitor progress with libgodot_get_shader_compilations_pending().
 *
 * Call this after libgodot_create_godot_instance() and before libgodot_load_project()
 * to avoid shader compilation stalls during the first rendered frames.
 *
 * @return true if warmup was successfully started, false on error.
 */
LIBGODOT_API bool libgodot_warmup(GDExtensionObjectPtr p_godot_instance);

/**
 * Returns the number of shader compilation tasks currently in progress.
 * Returns 0 when all currently requested shaders have been compiled.
 * Note: new shaders may be triggered at any time when new materials or effects are first used.
 */
LIBGODOT_API int32_t libgodot_get_shader_compilations_pending(void);

/**
 * Returns the total number of shader variants compiled since the last load_project().
 */
LIBGODOT_API int32_t libgodot_get_shader_compilations_total(void);

/**
 * Returns the current engine lifecycle status.
 */
LIBGODOT_API LibGodotStatus libgodot_get_status(GDExtensionObjectPtr p_godot_instance);

/**
 * Sets a callback to receive engine status changes.
 * Only one callback may be active at a time; setting a new one replaces the previous.
 * Passing NULL removes the callback.
 *
 * @param p_callback The callback function, or NULL to remove.
 * @param p_user_data Opaque pointer forwarded to the callback.
 */
LIBGODOT_API void libgodot_set_status_callback(LibGodotStatusCallback p_callback, void *p_user_data);

/**
 * Sets a callback to receive engine log messages (print, warning, error).
 * Can be called before libgodot_create_godot_instance() to capture early messages.
 * Only one callback may be active at a time; setting a new one replaces the previous.
 * When a callback is set, the engine's default stdout/stderr output is suppressed.
 * Passing NULL restores normal console output.
 *
 * @param p_callback The callback function, or NULL to remove and restore console output.
 * @param p_user_data Opaque pointer forwarded to the callback.
 */
LIBGODOT_API void libgodot_set_log_callback(LibGodotLogCallback p_callback, void *p_user_data);

#ifdef __cplusplus
}
#endif // __cplusplus
