/**************************************************************************/
/*  libgodot_display_server.h                                             */
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

#include "gdextension_interface.gen.h"

#ifdef __cplusplus
extern "C" {
#endif

// Export macros for DLL visibility
#if defined(_MSC_VER) || defined(__MINGW32__)
#define LIBGODOT_DS_API __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define LIBGODOT_DS_API __attribute__((visibility("default")))
#else
#define LIBGODOT_DS_API
#endif

// ============================================================================
// Constants
// ============================================================================

#define LIBGODOT_MAIN_WINDOW_ID 0
#define LIBGODOT_INVALID_WINDOW_ID -1

// ============================================================================
// Enumerations (matching Godot's internal values)
// ============================================================================

typedef enum {
	LIBGODOT_WINDOW_EVENT_MOUSE_ENTER = 0,
	LIBGODOT_WINDOW_EVENT_MOUSE_EXIT = 1,
	LIBGODOT_WINDOW_EVENT_FOCUS_IN = 2,
	LIBGODOT_WINDOW_EVENT_FOCUS_OUT = 3,
	LIBGODOT_WINDOW_EVENT_CLOSE_REQUEST = 4,
	LIBGODOT_WINDOW_EVENT_GO_BACK_REQUEST = 5,
	LIBGODOT_WINDOW_EVENT_DPI_CHANGE = 6,
	LIBGODOT_WINDOW_EVENT_TITLEBAR_CHANGE = 7,
} LibGodotWindowEvent;

typedef enum {
	LIBGODOT_MOUSE_MODE_VISIBLE = 0,
	LIBGODOT_MOUSE_MODE_HIDDEN = 1,
	LIBGODOT_MOUSE_MODE_CAPTURED = 2,
	LIBGODOT_MOUSE_MODE_CONFINED = 3,
	LIBGODOT_MOUSE_MODE_CONFINED_HIDDEN = 4,
} LibGodotMouseMode;

typedef enum {
	LIBGODOT_CURSOR_ARROW = 0,
	LIBGODOT_CURSOR_IBEAM = 1,
	LIBGODOT_CURSOR_POINTING_HAND = 2,
	LIBGODOT_CURSOR_CROSS = 3,
	LIBGODOT_CURSOR_WAIT = 4,
	LIBGODOT_CURSOR_BUSY = 5,
	LIBGODOT_CURSOR_DRAG = 6,
	LIBGODOT_CURSOR_CAN_DROP = 7,
	LIBGODOT_CURSOR_FORBIDDEN = 8,
	LIBGODOT_CURSOR_VSIZE = 9,
	LIBGODOT_CURSOR_HSIZE = 10,
	LIBGODOT_CURSOR_BDIAGSIZE = 11,
	LIBGODOT_CURSOR_FDIAGSIZE = 12,
	LIBGODOT_CURSOR_MOVE = 13,
	LIBGODOT_CURSOR_VSPLIT = 14,
	LIBGODOT_CURSOR_HSPLIT = 15,
	LIBGODOT_CURSOR_HELP = 16,
	LIBGODOT_CURSOR_MAX = 17,
} LibGodotCursorShape;

typedef enum {
	LIBGODOT_HANDLE_DISPLAY = 0,
	LIBGODOT_HANDLE_WINDOW = 1,
	LIBGODOT_HANDLE_WINDOW_VIEW = 2,
	LIBGODOT_HANDLE_OPENGL_CONTEXT = 3,
	LIBGODOT_HANDLE_EGL_DISPLAY = 4,
	LIBGODOT_HANDLE_EGL_CONFIG = 5,
} LibGodotHandleType;

typedef enum {
	LIBGODOT_VSYNC_DISABLED = 0,
	LIBGODOT_VSYNC_ENABLED = 1,
	LIBGODOT_VSYNC_ADAPTIVE = 2,
	LIBGODOT_VSYNC_MAILBOX = 3,
} LibGodotVSyncMode;

// Key modifier flags (can be combined with bitwise OR)
typedef enum {
	LIBGODOT_KEY_MOD_NONE = 0,
	LIBGODOT_KEY_MOD_SHIFT = 1 << 0,
	LIBGODOT_KEY_MOD_ALT = 1 << 1,
	LIBGODOT_KEY_MOD_CTRL = 1 << 2,
	LIBGODOT_KEY_MOD_META = 1 << 3,
	LIBGODOT_KEY_MOD_CAPS_LOCK = 1 << 4,
	LIBGODOT_KEY_MOD_NUM_LOCK = 1 << 5,
} LibGodotKeyModifier;

// Mouse button flags (can be combined with bitwise OR)
typedef enum {
	LIBGODOT_MOUSE_BUTTON_NONE = 0,
	LIBGODOT_MOUSE_BUTTON_LEFT = 1 << 0,
	LIBGODOT_MOUSE_BUTTON_RIGHT = 1 << 1,
	LIBGODOT_MOUSE_BUTTON_MIDDLE = 1 << 2,
	LIBGODOT_MOUSE_BUTTON_XBUTTON1 = 1 << 3,
	LIBGODOT_MOUSE_BUTTON_XBUTTON2 = 1 << 4,
} LibGodotMouseButtonMask;

// Mouse button indices
typedef enum {
	LIBGODOT_MOUSE_BUTTON_INDEX_LEFT = 1,
	LIBGODOT_MOUSE_BUTTON_INDEX_RIGHT = 2,
	LIBGODOT_MOUSE_BUTTON_INDEX_MIDDLE = 3,
	LIBGODOT_MOUSE_BUTTON_INDEX_WHEEL_UP = 4,
	LIBGODOT_MOUSE_BUTTON_INDEX_WHEEL_DOWN = 5,
	LIBGODOT_MOUSE_BUTTON_INDEX_WHEEL_LEFT = 6,
	LIBGODOT_MOUSE_BUTTON_INDEX_WHEEL_RIGHT = 7,
	LIBGODOT_MOUSE_BUTTON_INDEX_XBUTTON1 = 8,
	LIBGODOT_MOUSE_BUTTON_INDEX_XBUTTON2 = 9,
} LibGodotMouseButtonIndex;

// Key location
typedef enum {
	LIBGODOT_KEY_LOCATION_UNSPECIFIED = 0,
	LIBGODOT_KEY_LOCATION_LEFT = 1,
	LIBGODOT_KEY_LOCATION_RIGHT = 2,
} LibGodotKeyLocation;

// ============================================================================
// Display Server Interface
// Callbacks from Godot -> Your Application
// ============================================================================

typedef struct LibGodotDisplayServerInterface {
	void *user_data; // Passed as first argument to all callbacks

	// ========================================================================
	// Identity (required)
	// ========================================================================

	const char *(*get_name)(void *user_data);

	// ========================================================================
	// Screen information (required)
	// ========================================================================

	int (*get_screen_count)(void *user_data);
	int (*get_primary_screen)(void *user_data);
	void (*get_screen_position)(void *user_data, int p_screen, int *r_x, int *r_y);
	void (*get_screen_size)(void *user_data, int p_screen, int *r_width, int *r_height);
	int (*get_screen_dpi)(void *user_data, int p_screen);
	float (*get_screen_scale)(void *user_data, int p_screen);
	float (*get_screen_refresh_rate)(void *user_data, int p_screen);

	// ========================================================================
	// Window management (required)
	// ========================================================================

	void (*get_window_position)(void *user_data, int p_window_id, int *r_x, int *r_y);
	void (*get_window_size)(void *user_data, int p_window_id, int *r_width, int *r_height);
	void (*set_window_size)(void *user_data, int p_window_id, int p_width, int p_height);
	void (*set_window_position)(void *user_data, int p_window_id, int p_x, int p_y);
	GDExtensionBool (*window_can_draw)(void *user_data, int p_window_id);
	GDExtensionBool (*window_is_focused)(void *user_data, int p_window_id);

	// ========================================================================
	// Event processing (required)
	// ========================================================================

	void (*process_events)(void *user_data);

	// ========================================================================
	// Rendering (required for visual output)
	// ========================================================================

	/**
	 * Return a native handle for rendering integration.
	 * For LIBGODOT_HANDLE_WINDOW_VIEW:
	 *   - macOS: Return CAMetalLayer* (for Metal) or CALayer* (for OpenGL)
	 *   - Windows: Return HWND
	 *   - Linux: Return Window (X11) or wl_surface* (Wayland)
	 */
	void *(*get_native_handle)(void *user_data, LibGodotHandleType p_handle_type, int p_window_id);

	/**
	 * Called after rendering is complete. For OpenGL, this is where you'd swap buffers.
	 * For Metal/Vulkan, this may be a no-op if the rendering context handles presentation.
	 */
	void (*swap_buffers)(void *user_data);

	// ========================================================================
	// Mouse (optional - implement for mouse support)
	// ========================================================================

	void (*mouse_set_mode)(void *user_data, LibGodotMouseMode p_mode);
	LibGodotMouseMode (*mouse_get_mode)(void *user_data);
	void (*warp_mouse)(void *user_data, int p_x, int p_y);
	void (*get_mouse_position)(void *user_data, int *r_x, int *r_y);
	unsigned int (*get_mouse_button_state)(void *user_data); // Returns LibGodotMouseButtonMask

	// ========================================================================
	// Cursor (optional)
	// ========================================================================

	void (*cursor_set_shape)(void *user_data, LibGodotCursorShape p_shape);
	LibGodotCursorShape (*cursor_get_shape)(void *user_data);

	// ========================================================================
	// Clipboard (optional)
	// ========================================================================

	void (*clipboard_set)(void *user_data, const char *p_text);
	const char *(*clipboard_get)(void *user_data);
	GDExtensionBool (*clipboard_has)(void *user_data);

	// ========================================================================
	// Window properties (optional)
	// ========================================================================

	void (*set_window_title)(void *user_data, int p_window_id, const char *p_title);
	void (*set_window_min_size)(void *user_data, int p_window_id, int p_width, int p_height);
	void (*get_window_min_size)(void *user_data, int p_window_id, int *r_width, int *r_height);
	void (*set_window_max_size)(void *user_data, int p_window_id, int p_width, int p_height);
	void (*get_window_max_size)(void *user_data, int p_window_id, int *r_width, int *r_height);

	// ========================================================================
	// IME (optional - for text input)
	// ========================================================================

	void (*window_set_ime_active)(void *user_data, int p_window_id, GDExtensionBool p_active);
	void (*window_set_ime_position)(void *user_data, int p_window_id, int p_x, int p_y);

	// ========================================================================
	// VSync (optional)
	// ========================================================================

	void (*set_vsync_mode)(void *user_data, int p_window_id, LibGodotVSyncMode p_mode);
	LibGodotVSyncMode (*get_vsync_mode)(void *user_data, int p_window_id);

	// ========================================================================
	// Misc (optional)
	// ========================================================================

	void (*beep)(void *user_data);

} LibGodotDisplayServerInterface;

// ============================================================================
// API Functions - Interface Setup
// ============================================================================

/**
 * @name libgodot_display_server_set_interface
 * @since 4.5
 *
 * Set the external display server interface.
 * Must be called BEFORE libgodot_create_godot_instance().
 * The interface struct must remain valid for the lifetime of the Godot instance.
 * Pass NULL to clear the interface.
 *
 * @param p_interface Pointer to your display server interface implementation.
 */
LIBGODOT_DS_API void libgodot_display_server_set_interface(LibGodotDisplayServerInterface *p_interface);

/**
 * @name libgodot_display_server_get_interface
 * @since 4.5
 *
 * Get the currently set external display server interface.
 *
 * @return The current interface, or NULL if not set.
 */
LIBGODOT_DS_API LibGodotDisplayServerInterface *libgodot_display_server_get_interface(void);

// ============================================================================
// API Functions - Push Events INTO Godot
// Call these from your application's event handling code.
// ============================================================================

/**
 * @name libgodot_display_server_push_window_event
 * @since 4.5
 *
 * Push a window event (focus change, close request, etc.) into Godot.
 *
 * @param p_instance The GodotInstance (can be NULL to use singleton DisplayServer).
 * @param p_event The window event type.
 * @param p_window_id The window ID (use LIBGODOT_MAIN_WINDOW_ID for main window).
 */
LIBGODOT_DS_API void libgodot_display_server_push_window_event(
		GDExtensionObjectPtr p_instance,
		LibGodotWindowEvent p_event,
		int p_window_id);

/**
 * @name libgodot_display_server_notify_window_size_changed
 * @since 4.5
 *
 * Notify Godot that the window size has changed.
 *
 * @param p_instance The GodotInstance (can be NULL).
 * @param p_window_id The window ID.
 * @param p_width New width in pixels.
 * @param p_height New height in pixels.
 */
LIBGODOT_DS_API void libgodot_display_server_notify_window_size_changed(
		GDExtensionObjectPtr p_instance,
		int p_window_id,
		int p_width,
		int p_height);

/**
 * @name libgodot_display_server_push_key_event
 * @since 4.5
 *
 * Push a keyboard event into Godot.
 *
 * @param p_instance The GodotInstance (can be NULL).
 * @param p_keycode The Godot keycode (see @GlobalScope Key enum).
 * @param p_physical_keycode The physical keycode based on keyboard position.
 * @param p_key_label The localized key label.
 * @param p_unicode The Unicode character (0 if not applicable).
 * @param p_location Key location (left/right for modifier keys).
 * @param p_pressed True if key is pressed, false if released.
 * @param p_echo True if this is a key repeat event.
 * @param p_modifiers Modifier key state (LibGodotKeyModifier flags).
 */
LIBGODOT_DS_API void libgodot_display_server_push_key_event(
		GDExtensionObjectPtr p_instance,
		int p_keycode,
		int p_physical_keycode,
		int p_key_label,
		unsigned int p_unicode,
		LibGodotKeyLocation p_location,
		GDExtensionBool p_pressed,
		GDExtensionBool p_echo,
		unsigned int p_modifiers);

/**
 * @name libgodot_display_server_push_mouse_button_event
 * @since 4.5
 *
 * Push a mouse button event into Godot.
 *
 * @param p_instance The GodotInstance (can be NULL).
 * @param p_button_index The mouse button (LibGodotMouseButtonIndex).
 * @param p_pressed True if button is pressed, false if released.
 * @param p_x Mouse X position in window coordinates.
 * @param p_y Mouse Y position in window coordinates.
 * @param p_modifiers Modifier key state (LibGodotKeyModifier flags).
 * @param p_button_mask Current state of all mouse buttons (LibGodotMouseButtonMask).
 */
LIBGODOT_DS_API void libgodot_display_server_push_mouse_button_event(
		GDExtensionObjectPtr p_instance,
		LibGodotMouseButtonIndex p_button_index,
		GDExtensionBool p_pressed,
		int p_x,
		int p_y,
		unsigned int p_modifiers,
		unsigned int p_button_mask);

/**
 * @name libgodot_display_server_push_mouse_motion_event
 * @since 4.5
 *
 * Push a mouse motion event into Godot.
 *
 * @param p_instance The GodotInstance (can be NULL).
 * @param p_x Mouse X position in window coordinates.
 * @param p_y Mouse Y position in window coordinates.
 * @param p_relative_x Relative X movement since last event.
 * @param p_relative_y Relative Y movement since last event.
 * @param p_modifiers Modifier key state (LibGodotKeyModifier flags).
 * @param p_button_mask Current state of all mouse buttons (LibGodotMouseButtonMask).
 */
LIBGODOT_DS_API void libgodot_display_server_push_mouse_motion_event(
		GDExtensionObjectPtr p_instance,
		int p_x,
		int p_y,
		int p_relative_x,
		int p_relative_y,
		unsigned int p_modifiers,
		unsigned int p_button_mask);

/**
 * @name libgodot_display_server_push_mouse_wheel_event
 * @since 4.5
 *
 * Push a mouse wheel/scroll event into Godot.
 *
 * @param p_instance The GodotInstance (can be NULL).
 * @param p_delta_x Horizontal scroll amount.
 * @param p_delta_y Vertical scroll amount.
 * @param p_x Mouse X position in window coordinates.
 * @param p_y Mouse Y position in window coordinates.
 * @param p_modifiers Modifier key state (LibGodotKeyModifier flags).
 */
LIBGODOT_DS_API void libgodot_display_server_push_mouse_wheel_event(
		GDExtensionObjectPtr p_instance,
		float p_delta_x,
		float p_delta_y,
		int p_x,
		int p_y,
		unsigned int p_modifiers);

/**
 * @name libgodot_display_server_push_input_text
 * @since 4.5
 *
 * Push text input into Godot (for IME/unicode text entry).
 *
 * @param p_instance The GodotInstance (can be NULL).
 * @param p_text The text to input (UTF-8 encoded).
 */
LIBGODOT_DS_API void libgodot_display_server_push_input_text(
		GDExtensionObjectPtr p_instance,
		const char *p_text);

#ifdef __cplusplus
}
#endif
