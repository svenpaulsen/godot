/**************************************************************************/
/*  display_server_external.cpp                                           */
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

#include "display_server_external.h"

#include "core/config/project_settings.h"
#include "core/input/input.h"

#if defined(RD_ENABLED)
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#include "servers/rendering/rendering_device.h"

#if defined(METAL_ENABLED)
#include "drivers/metal/rendering_context_driver_metal.h"
#endif

#if defined(VULKAN_ENABLED)
// Note: macOS Vulkan is not supported in DisplayServerExternal because it requires
// Objective-C++ compilation. Use Metal on macOS instead (preferred API).
#if defined(WINDOWS_ENABLED)
#include "platform/windows/rendering_context_driver_vulkan_windows.h"
#elif defined(LINUXBSD_ENABLED)
#ifdef X11_ENABLED
#include "platform/linuxbsd/x11/rendering_context_driver_vulkan_x11.h"
#endif
#ifdef WAYLAND_ENABLED
#include "platform/linuxbsd/wayland/rendering_context_driver_vulkan_wayland.h"
#endif
#endif
#endif // VULKAN_ENABLED

#if defined(D3D12_ENABLED)
#include "drivers/d3d12/rendering_context_driver_d3d12.h"
#endif

#endif // RD_ENABLED

#if defined(GLES3_ENABLED)
#include "drivers/gles3/rasterizer_gles3.h"
#endif

// X11 headers define CursorShape as a macro, which conflicts with DisplayServer::CursorShape.
#ifdef CursorShape
#undef CursorShape
#endif

// Static interface pointer
LibGodotDisplayServerInterface *DisplayServerExternal::interface = nullptr;

// ============================================================================
// Static methods
// ============================================================================

void DisplayServerExternal::register_external_driver() {
	register_create_function("external", create_func, get_rendering_drivers_func);
}

void DisplayServerExternal::set_interface(LibGodotDisplayServerInterface *p_interface) {
	interface = p_interface;
}

LibGodotDisplayServerInterface *DisplayServerExternal::get_interface() {
	return interface;
}

bool DisplayServerExternal::has_interface() {
	return interface != nullptr;
}

DisplayServer *DisplayServerExternal::create_func(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error) {
	DisplayServer *ds = memnew(DisplayServerExternal(p_rendering_driver, p_mode, p_vsync_mode, p_flags, p_position, p_resolution, p_screen, p_context, r_error));
	if (r_error != OK) {
		ERR_PRINT("Failed to create DisplayServerExternal");
	}
	return ds;
}

Vector<String> DisplayServerExternal::get_rendering_drivers_func() {
	Vector<String> drivers;
#if defined(VULKAN_ENABLED) && !defined(MACOS_ENABLED)
	// Note: Vulkan on macOS requires Objective-C++, use Metal instead
	drivers.push_back("vulkan");
#endif
#if defined(D3D12_ENABLED)
	drivers.push_back("d3d12");
#endif
#if defined(METAL_ENABLED)
	drivers.push_back("metal");
#endif
#if defined(GLES3_ENABLED)
	drivers.push_back("opengl3");
#endif
	return drivers;
}

// ============================================================================
// Helper methods
// ============================================================================

BitField<KeyModifierMask> DisplayServerExternal::_WrapModifiers(unsigned int p_modifiers) {
	BitField<KeyModifierMask> modifiers;
	if (p_modifiers & LIBGODOT_KEY_MOD_SHIFT) {
		modifiers.set_flag(KeyModifierMask::SHIFT);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_ALT) {
		modifiers.set_flag(KeyModifierMask::ALT);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_CTRL) {
		modifiers.set_flag(KeyModifierMask::CTRL);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_META) {
		modifiers.set_flag(KeyModifierMask::META);
	}
	return modifiers;
}

BitField<MouseButtonMask> DisplayServerExternal::_WrapButtonMask(unsigned int p_button_mask) {
	BitField<MouseButtonMask> mask;
	if (p_button_mask & LIBGODOT_MOUSE_BUTTON_LEFT) {
		mask.set_flag(MouseButtonMask::LEFT);
	}
	if (p_button_mask & LIBGODOT_MOUSE_BUTTON_RIGHT) {
		mask.set_flag(MouseButtonMask::RIGHT);
	}
	if (p_button_mask & LIBGODOT_MOUSE_BUTTON_MIDDLE) {
		mask.set_flag(MouseButtonMask::MIDDLE);
	}
	if (p_button_mask & LIBGODOT_MOUSE_BUTTON_XBUTTON1) {
		mask.set_flag(MouseButtonMask::MB_XBUTTON1);
	}
	if (p_button_mask & LIBGODOT_MOUSE_BUTTON_XBUTTON2) {
		mask.set_flag(MouseButtonMask::MB_XBUTTON2);
	}
	return mask;
}

void DisplayServerExternal::_dispatch_input_events(const Ref<InputEvent> &p_event) {
	DisplayServerExternal *ds = static_cast<DisplayServerExternal *>(DisplayServer::get_singleton());
	if (ds) {
		Ref<InputEventFromWindow> event_from_window = p_event;
		WindowID window_id = MAIN_WINDOW_ID;
		if (event_from_window.is_valid()) {
			window_id = event_from_window->get_window_id();
		}
		ds->_send_input_event(p_event, window_id);
	}
}

void DisplayServerExternal::_send_input_event(const Ref<InputEvent> &p_event, WindowID p_window_id) const {
	const Callable *cb = input_event_callbacks.getptr(p_window_id);
	if (cb && cb->is_valid()) {
		cb->call(p_event);
	}
}

void DisplayServerExternal::_send_window_event(WindowEvent p_event, WindowID p_window_id) const {
	const Callable *cb = window_event_callbacks.getptr(p_window_id);
	if (cb && cb->is_valid()) {
		cb->call(int(p_event));
	}
}

// ============================================================================
// Event pushing (called from libgodot C API)
// ============================================================================

void DisplayServerExternal::push_window_event(WindowEvent p_event, WindowID p_window) {
	_send_window_event(p_event, p_window);
}

void DisplayServerExternal::push_input_event(const Ref<InputEvent> &p_event) {
	if (p_event.is_valid()) {
		Input::get_singleton()->parse_input_event(p_event);
	}
}

void DisplayServerExternal::push_input_text(const String &p_text, WindowID p_window) {
	const Callable *cb = input_text_callbacks.getptr(p_window);
	if (cb && cb->is_valid()) {
		cb->call(p_text);
	}
}

void DisplayServerExternal::notify_window_size_changed(WindowID p_window, int p_width, int p_height) {
	window_size = Size2i(p_width, p_height);

#if defined(RD_ENABLED)
	if (rendering_context) {
		rendering_context->window_set_size(p_window, p_width, p_height);
	}
#endif

	const Callable *cb = window_resize_callbacks.getptr(p_window);
	if (cb && cb->is_valid()) {
		Rect2i rect(Point2i(), window_size);
		cb->call(rect);
	}
}

// ============================================================================
// DisplayServer interface - Core
// ============================================================================

bool DisplayServerExternal::has_feature(Feature p_feature) const {
	switch (p_feature) {
		case FEATURE_MOUSE:
		case FEATURE_CURSOR_SHAPE:
		case FEATURE_CLIPBOARD:
		case FEATURE_SWAP_BUFFERS:
			return true;
		default:
			return false;
	}
}

String DisplayServerExternal::get_name() const {
	if (interface && interface->get_name) {
		const char *name = interface->get_name(interface->user_data);
		if (name) {
			return String::utf8(name);
		}
	}
	return "external";
}

// ============================================================================
// DisplayServer interface - Screen
// ============================================================================

int DisplayServerExternal::get_screen_count() const {
	if (interface && interface->get_screen_count) {
		return interface->get_screen_count(interface->user_data);
	}
	return 1;
}

int DisplayServerExternal::get_primary_screen() const {
	if (interface && interface->get_primary_screen) {
		return interface->get_primary_screen(interface->user_data);
	}
	return 0;
}

Point2i DisplayServerExternal::screen_get_position(int p_screen) const {
	p_screen = _get_screen_index(p_screen);
	if (interface && interface->get_screen_position) {
		int x = 0, y = 0;
		interface->get_screen_position(interface->user_data, p_screen, &x, &y);
		return Point2i(x, y);
	}
	return Point2i(0, 0);
}

Size2i DisplayServerExternal::screen_get_size(int p_screen) const {
	p_screen = _get_screen_index(p_screen);
	if (interface && interface->get_screen_size) {
		int w = 0, h = 0;
		interface->get_screen_size(interface->user_data, p_screen, &w, &h);
		return Size2i(w, h);
	}
	return window_size;
}

Rect2i DisplayServerExternal::screen_get_usable_rect(int p_screen) const {
	return Rect2i(screen_get_position(p_screen), screen_get_size(p_screen));
}

int DisplayServerExternal::screen_get_dpi(int p_screen) const {
	p_screen = _get_screen_index(p_screen);
	if (interface && interface->get_screen_dpi) {
		return interface->get_screen_dpi(interface->user_data, p_screen);
	}
	return 96;
}

float DisplayServerExternal::screen_get_scale(int p_screen) const {
	p_screen = _get_screen_index(p_screen);
	if (interface && interface->get_screen_scale) {
		return interface->get_screen_scale(interface->user_data, p_screen);
	}
	return 1.0f;
}

float DisplayServerExternal::screen_get_refresh_rate(int p_screen) const {
	p_screen = _get_screen_index(p_screen);
	if (interface && interface->get_screen_refresh_rate) {
		return interface->get_screen_refresh_rate(interface->user_data, p_screen);
	}
	return 60.0f;
}

// ============================================================================
// DisplayServer interface - Window List
// ============================================================================

Vector<DisplayServer::WindowID> DisplayServerExternal::get_window_list() const {
	Vector<WindowID> list;
	list.push_back(MAIN_WINDOW_ID);
	return list;
}

DisplayServer::WindowID DisplayServerExternal::get_window_at_screen_position(const Point2i &p_position) const {
	return MAIN_WINDOW_ID;
}

// ============================================================================
// DisplayServer interface - Window instance
// ============================================================================

void DisplayServerExternal::window_attach_instance_id(ObjectID p_instance, WindowID p_window) {
	window_attached_instance_id[p_window] = p_instance;
}

ObjectID DisplayServerExternal::window_get_attached_instance_id(WindowID p_window) const {
	const ObjectID *id = window_attached_instance_id.getptr(p_window);
	if (id) {
		return *id;
	}
	return ObjectID();
}

// ============================================================================
// DisplayServer interface - Window callbacks
// ============================================================================

void DisplayServerExternal::window_set_rect_changed_callback(const Callable &p_callable, WindowID p_window) {
	window_resize_callbacks[p_window] = p_callable;
}

void DisplayServerExternal::window_set_window_event_callback(const Callable &p_callable, WindowID p_window) {
	window_event_callbacks[p_window] = p_callable;
}

void DisplayServerExternal::window_set_input_event_callback(const Callable &p_callable, WindowID p_window) {
	input_event_callbacks[p_window] = p_callable;
}

void DisplayServerExternal::window_set_input_text_callback(const Callable &p_callable, WindowID p_window) {
	input_text_callbacks[p_window] = p_callable;
}

void DisplayServerExternal::window_set_drop_files_callback(const Callable &p_callable, WindowID p_window) {
	drop_files_callbacks[p_window] = p_callable;
}

// ============================================================================
// DisplayServer interface - Window properties
// ============================================================================

void DisplayServerExternal::window_set_title(const String &p_title, WindowID p_window) {
	if (interface && interface->set_window_title) {
		interface->set_window_title(interface->user_data, p_window, p_title.utf8().get_data());
	}
}

int DisplayServerExternal::window_get_current_screen(WindowID p_window) const {
	return 0;
}

void DisplayServerExternal::window_set_current_screen(int p_screen, WindowID p_window) {
	// Not supported in external mode
}

Point2i DisplayServerExternal::window_get_position(WindowID p_window) const {
	if (interface && interface->get_window_position) {
		int x = 0, y = 0;
		interface->get_window_position(interface->user_data, p_window, &x, &y);
		return Point2i(x, y);
	}
	return window_position;
}

Point2i DisplayServerExternal::window_get_position_with_decorations(WindowID p_window) const {
	return window_get_position(p_window);
}

void DisplayServerExternal::window_set_position(const Point2i &p_position, WindowID p_window) {
	if (interface && interface->set_window_position) {
		interface->set_window_position(interface->user_data, p_window, p_position.x, p_position.y);
	}
	window_position = p_position;
}

void DisplayServerExternal::window_set_transient(WindowID p_window, WindowID p_parent) {
	// Not supported in external mode
}

void DisplayServerExternal::window_set_max_size(const Size2i p_size, WindowID p_window) {
	if (interface && interface->set_window_max_size) {
		interface->set_window_max_size(interface->user_data, p_window, p_size.x, p_size.y);
	}
}

Size2i DisplayServerExternal::window_get_max_size(WindowID p_window) const {
	if (interface && interface->get_window_max_size) {
		int w = 0, h = 0;
		interface->get_window_max_size(interface->user_data, p_window, &w, &h);
		return Size2i(w, h);
	}
	return Size2i();
}

void DisplayServerExternal::window_set_min_size(const Size2i p_size, WindowID p_window) {
	if (interface && interface->set_window_min_size) {
		interface->set_window_min_size(interface->user_data, p_window, p_size.x, p_size.y);
	}
}

Size2i DisplayServerExternal::window_get_min_size(WindowID p_window) const {
	if (interface && interface->get_window_min_size) {
		int w = 0, h = 0;
		interface->get_window_min_size(interface->user_data, p_window, &w, &h);
		return Size2i(w, h);
	}
	return Size2i();
}

void DisplayServerExternal::window_set_size(const Size2i p_size, WindowID p_window) {
	if (interface && interface->set_window_size) {
		interface->set_window_size(interface->user_data, p_window, p_size.x, p_size.y);
	}
	window_size = p_size;

#if defined(RD_ENABLED)
	if (rendering_context) {
		rendering_context->window_set_size(p_window, p_size.x, p_size.y);
	}
#endif

	// Notify about resize
	const Callable *cb = window_resize_callbacks.getptr(p_window);
	if (cb && cb->is_valid()) {
		Rect2i rect(Point2i(), window_size);
		cb->call(rect);
	}
}

Size2i DisplayServerExternal::window_get_size(WindowID p_window) const {
	if (interface && interface->get_window_size) {
		int w = 0, h = 0;
		interface->get_window_size(interface->user_data, p_window, &w, &h);
		return Size2i(w, h);
	}
	return window_size;
}

Size2i DisplayServerExternal::window_get_size_with_decorations(WindowID p_window) const {
	return window_get_size(p_window);
}

void DisplayServerExternal::window_set_mode(WindowMode p_mode, WindowID p_window) {
	window_mode = p_mode;
}

DisplayServer::WindowMode DisplayServerExternal::window_get_mode(WindowID p_window) const {
	return window_mode;
}

bool DisplayServerExternal::window_is_maximize_allowed(WindowID p_window) const {
	return false;
}

void DisplayServerExternal::window_set_flag(WindowFlags p_flag, bool p_enabled, WindowID p_window) {
	// Flags not supported in external mode
}

bool DisplayServerExternal::window_get_flag(WindowFlags p_flag, WindowID p_window) const {
	return false;
}

void DisplayServerExternal::window_request_attention(WindowID p_window) {
	// Not supported in external mode
}

void DisplayServerExternal::window_move_to_foreground(WindowID p_window) {
	// Not supported in external mode
}

bool DisplayServerExternal::window_is_focused(WindowID p_window) const {
	if (interface && interface->window_is_focused) {
		return interface->window_is_focused(interface->user_data, p_window);
	}
	return true;
}

bool DisplayServerExternal::window_can_draw(WindowID p_window) const {
	if (interface && interface->window_can_draw) {
		return interface->window_can_draw(interface->user_data, p_window);
	}
	return true;
}

bool DisplayServerExternal::can_any_window_draw() const {
	return window_can_draw(MAIN_WINDOW_ID);
}

// ============================================================================
// DisplayServer interface - Events
// ============================================================================

void DisplayServerExternal::process_events() {
	if (interface && interface->process_events) {
		interface->process_events(interface->user_data);
	}

	Input::get_singleton()->flush_buffered_events();
}

// ============================================================================
// DisplayServer interface - Mouse
// ============================================================================

void DisplayServerExternal::mouse_set_mode(MouseMode p_mode) {
	if (interface && interface->mouse_set_mode) {
		interface->mouse_set_mode(interface->user_data, (LibGodotMouseMode)p_mode);
	}
	mouse_mode = p_mode;
}

DisplayServer::MouseMode DisplayServerExternal::mouse_get_mode() const {
	if (interface && interface->mouse_get_mode) {
		return (MouseMode)interface->mouse_get_mode(interface->user_data);
	}
	return mouse_mode;
}

void DisplayServerExternal::warp_mouse(const Point2i &p_position) {
	if (interface && interface->warp_mouse) {
		interface->warp_mouse(interface->user_data, p_position.x, p_position.y);
	}
	Input::get_singleton()->set_mouse_position(p_position);
}

Point2i DisplayServerExternal::mouse_get_position() const {
	if (interface && interface->get_mouse_position) {
		int x = 0, y = 0;
		interface->get_mouse_position(interface->user_data, &x, &y);
		return Point2i(x, y);
	}
	return Input::get_singleton()->get_mouse_position();
}

BitField<MouseButtonMask> DisplayServerExternal::mouse_get_button_state() const {
	if (interface && interface->get_mouse_button_state) {
		unsigned int state = interface->get_mouse_button_state(interface->user_data);
		return _WrapButtonMask(state);
	}
	return Input::get_singleton()->get_mouse_button_mask();
}

// ============================================================================
// DisplayServer interface - Clipboard
// ============================================================================

void DisplayServerExternal::clipboard_set(const String &p_text) {
	if (interface && interface->clipboard_set) {
		interface->clipboard_set(interface->user_data, p_text.utf8().get_data());
	}
}

String DisplayServerExternal::clipboard_get() const {
	if (interface && interface->clipboard_get) {
		const char *text = interface->clipboard_get(interface->user_data);
		if (text) {
			return String::utf8(text);
		}
	}
	return String();
}

bool DisplayServerExternal::clipboard_has() const {
	if (interface && interface->clipboard_has) {
		return interface->clipboard_has(interface->user_data);
	}
	return !clipboard_get().is_empty();
}

// ============================================================================
// DisplayServer interface - Cursor
// ============================================================================

void DisplayServerExternal::cursor_set_shape(CursorShape p_shape) {
	if (interface && interface->cursor_set_shape) {
		interface->cursor_set_shape(interface->user_data, (LibGodotCursorShape)p_shape);
	}
	cursor_shape = p_shape;
}

DisplayServer::CursorShape DisplayServerExternal::cursor_get_shape() const {
	if (interface && interface->cursor_get_shape) {
		return (CursorShape)interface->cursor_get_shape(interface->user_data);
	}
	return cursor_shape;
}

void DisplayServerExternal::cursor_set_custom_image(const Ref<Resource> &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot) {
	// Custom cursor images not supported in external mode
}

// ============================================================================
// DisplayServer interface - IME
// ============================================================================

void DisplayServerExternal::window_set_ime_active(const bool p_active, WindowID p_window) {
	if (interface && interface->window_set_ime_active) {
		interface->window_set_ime_active(interface->user_data, p_window, p_active);
	}
}

void DisplayServerExternal::window_set_ime_position(const Point2i &p_pos, WindowID p_window) {
	if (interface && interface->window_set_ime_position) {
		interface->window_set_ime_position(interface->user_data, p_window, p_pos.x, p_pos.y);
	}
}

// ============================================================================
// DisplayServer interface - VSync
// ============================================================================

void DisplayServerExternal::window_set_vsync_mode(VSyncMode p_vsync_mode, WindowID p_window) {
	if (interface && interface->set_vsync_mode) {
		interface->set_vsync_mode(interface->user_data, p_window, (LibGodotVSyncMode)p_vsync_mode);
	}
}

DisplayServer::VSyncMode DisplayServerExternal::window_get_vsync_mode(WindowID p_window) const {
	if (interface && interface->get_vsync_mode) {
		return (VSyncMode)interface->get_vsync_mode(interface->user_data, p_window);
	}
	return VSYNC_ENABLED;
}

// ============================================================================
// DisplayServer interface - Native handles
// ============================================================================

int64_t DisplayServerExternal::window_get_native_handle(HandleType p_handle_type, WindowID p_window) const {
	if (interface && interface->get_native_handle) {
		void *handle = interface->get_native_handle(interface->user_data, (LibGodotHandleType)p_handle_type, p_window);
		return (int64_t)handle;
	}
	return 0;
}

// ============================================================================
// DisplayServer interface - Rendering
// ============================================================================

void DisplayServerExternal::swap_buffers() {
	if (interface && interface->swap_buffers) {
		interface->swap_buffers(interface->user_data);
	}
}

// ============================================================================
// DisplayServer interface - Misc
// ============================================================================

void DisplayServerExternal::beep() const {
	if (interface && interface->beep) {
		interface->beep(interface->user_data);
	}
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

DisplayServerExternal::DisplayServerExternal(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Context p_context, Error &r_error) {
	r_error = OK;

	if (!interface) {
		ERR_PRINT("DisplayServerExternal: No interface set! Call libgodot_display_server_set_interface() before creating Godot instance.");
		r_error = ERR_UNCONFIGURED;
		return;
	}

	// Store initial window state
	window_size = p_resolution;
	if (p_position) {
		window_position = *p_position;
	}
	window_mode = p_mode;
	rendering_driver = p_rendering_driver;

	// Set up input dispatch
	Input::get_singleton()->set_event_dispatch_function(_dispatch_input_events);

	// Get the native view/layer from the external interface
	void *native_layer = nullptr;
	if (interface->get_native_handle) {
		native_layer = interface->get_native_handle(interface->user_data, LIBGODOT_HANDLE_WINDOW_VIEW, MAIN_WINDOW_ID);
	}

	if (!native_layer) {
		ERR_PRINT("DisplayServerExternal: get_native_handle(LIBGODOT_HANDLE_WINDOW_VIEW) returned NULL. Rendering will not work.");
		r_error = ERR_CANT_CREATE;
		return;
	}

#if defined(RD_ENABLED)
	// Create the appropriate rendering context based on the driver and platform
#if defined(METAL_ENABLED)
	if (rendering_driver == "metal") {
		rendering_context = memnew(RenderingContextDriverMetal);
	}
#endif

#if defined(VULKAN_ENABLED)
	// Note: Vulkan on macOS requires Objective-C++, use Metal instead
#if !defined(MACOS_ENABLED)
	if (rendering_driver == "vulkan") {
#if defined(WINDOWS_ENABLED)
		rendering_context = memnew(RenderingContextDriverVulkanWindows);
#elif defined(LINUXBSD_ENABLED)
#if defined(X11_ENABLED)
		// For now, default to X11 for Linux. Wayland support would require
		// additional logic to detect which display server is in use.
		rendering_context = memnew(RenderingContextDriverVulkanX11);
#elif defined(WAYLAND_ENABLED)
		rendering_context = memnew(RenderingContextDriverVulkanWayland);
#endif
#endif
	}
#endif // !MACOS_ENABLED
#endif // VULKAN_ENABLED

#if defined(D3D12_ENABLED)
	if (rendering_driver == "d3d12") {
		rendering_context = memnew(RenderingContextDriverD3D12);
	}
#endif

	if (rendering_context) {
		if (rendering_context->initialize() != OK) {
			memdelete(rendering_context);
			rendering_context = nullptr;
			r_error = ERR_CANT_CREATE;
			ERR_FAIL_MSG("Could not initialize " + rendering_driver);
		}
	}

	if (rendering_context) {
		// Create a window with the external layer using platform-specific data
		union {
#if defined(METAL_ENABLED)
			RenderingContextDriverMetal::WindowPlatformData metal;
#endif
#if defined(VULKAN_ENABLED) && !defined(MACOS_ENABLED)
#if defined(WINDOWS_ENABLED)
			RenderingContextDriverVulkanWindows::WindowPlatformData vulkan_windows;
#elif defined(LINUXBSD_ENABLED)
#if defined(X11_ENABLED)
			RenderingContextDriverVulkanX11::WindowPlatformData vulkan_x11;
#endif
#if defined(WAYLAND_ENABLED)
			RenderingContextDriverVulkanWayland::WindowPlatformData vulkan_wayland;
#endif
#endif
#endif // VULKAN_ENABLED && !MACOS_ENABLED
#if defined(D3D12_ENABLED)
			RenderingContextDriverD3D12::WindowPlatformData d3d12;
#endif
		} wpd;

		Error err = ERR_UNAVAILABLE;

#if defined(METAL_ENABLED)
		if (rendering_driver == "metal") {
			wpd.metal.layer = native_layer;
			err = rendering_context->window_create(MAIN_WINDOW_ID, &wpd);
		}
#endif

#if defined(VULKAN_ENABLED) && !defined(MACOS_ENABLED)
		if (rendering_driver == "vulkan") {
#if defined(WINDOWS_ENABLED)
			wpd.vulkan_windows.window = (HWND)native_layer;
			wpd.vulkan_windows.instance = GetModuleHandle(nullptr);
			err = rendering_context->window_create(MAIN_WINDOW_ID, &wpd);
#elif defined(LINUXBSD_ENABLED)
#if defined(X11_ENABLED)
			// For X11, native_layer is the X11 Window, we also need the display
			wpd.vulkan_x11.window = (::Window)(uintptr_t)native_layer;
			void *x11_display = interface->get_native_handle(interface->user_data, LIBGODOT_HANDLE_DISPLAY, MAIN_WINDOW_ID);
			wpd.vulkan_x11.display = (::Display *)x11_display;
			err = rendering_context->window_create(MAIN_WINDOW_ID, &wpd);
#elif defined(WAYLAND_ENABLED)
			// For Wayland, native_layer is the wl_surface
			void *wayland_display = interface->get_native_handle(interface->user_data, LIBGODOT_HANDLE_DISPLAY, MAIN_WINDOW_ID);
			wpd.vulkan_wayland.surface = (struct wl_surface *)native_layer;
			wpd.vulkan_wayland.display = (struct wl_display *)wayland_display;
			err = rendering_context->window_create(MAIN_WINDOW_ID, &wpd);
#endif
#endif
		}
#endif // VULKAN_ENABLED && !MACOS_ENABLED

#if defined(D3D12_ENABLED)
		if (rendering_driver == "d3d12") {
			wpd.d3d12.window = (HWND)native_layer;
			err = rendering_context->window_create(MAIN_WINDOW_ID, &wpd);
		}
#endif

		if (err != OK) {
			memdelete(rendering_context);
			rendering_context = nullptr;
			r_error = ERR_CANT_CREATE;
			ERR_FAIL_MSG(vformat("Can't create a %s window", rendering_driver));
		}

		// Set window size and vsync
		rendering_context->window_set_size(MAIN_WINDOW_ID, p_resolution.width, p_resolution.height);
		rendering_context->window_set_vsync_mode(MAIN_WINDOW_ID, p_vsync_mode);

		// Initialize the rendering device
		rendering_device = memnew(RenderingDevice);
		rendering_device->initialize(rendering_context, MAIN_WINDOW_ID);
		rendering_device->screen_create(MAIN_WINDOW_ID);

		RendererCompositorRD::make_current();
	}
#endif // RD_ENABLED

#if defined(GLES3_ENABLED)
	if (rendering_driver == "opengl3") {
		RasterizerGLES3::make_current(true);
	}
#endif
}

DisplayServerExternal::~DisplayServerExternal() {
#if defined(RD_ENABLED)
	if (rendering_device) {
		memdelete(rendering_device);
		rendering_device = nullptr;
	}

	if (rendering_context) {
		memdelete(rendering_context);
		rendering_context = nullptr;
	}
#endif
}

// ============================================================================
// C API implementation
// ============================================================================

void libgodot_display_server_set_interface(LibGodotDisplayServerInterface *p_interface) {
	DisplayServerExternal::set_interface(p_interface);
}

LibGodotDisplayServerInterface *libgodot_display_server_get_interface(void) {
	return DisplayServerExternal::get_interface();
}

void libgodot_display_server_push_window_event(GDExtensionObjectPtr p_instance, LibGodotWindowEvent p_event, int p_window_id) {
	DisplayServerExternal *ds = Object::cast_to<DisplayServerExternal>(DisplayServer::get_singleton());
	if (ds) {
		ds->push_window_event((DisplayServer::WindowEvent)p_event, p_window_id);
	}
}

void libgodot_display_server_notify_window_size_changed(GDExtensionObjectPtr p_instance, int p_window_id, int p_width, int p_height) {
	DisplayServerExternal *ds = Object::cast_to<DisplayServerExternal>(DisplayServer::get_singleton());
	if (ds) {
		ds->notify_window_size_changed(p_window_id, p_width, p_height);
	}
}

void libgodot_display_server_push_key_event(GDExtensionObjectPtr p_instance, int p_keycode, int p_physical_keycode, int p_key_label, unsigned int p_unicode, LibGodotKeyLocation p_location, GDExtensionBool p_pressed, GDExtensionBool p_echo, unsigned int p_modifiers) {
	DisplayServerExternal *ds = Object::cast_to<DisplayServerExternal>(DisplayServer::get_singleton());
	if (!ds) {
		return;
	}

	Ref<InputEventKey> event;
	event.instantiate();

	event->set_window_id(DisplayServer::MAIN_WINDOW_ID);
	event->set_keycode((Key)p_keycode);
	event->set_physical_keycode((Key)p_physical_keycode);
	event->set_key_label((Key)p_key_label);
	event->set_unicode(p_unicode);
	event->set_location((KeyLocation)p_location);
	event->set_pressed(p_pressed);
	event->set_echo(p_echo);

	// Set modifiers
	if (p_modifiers & LIBGODOT_KEY_MOD_SHIFT) {
		event->set_shift_pressed(true);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_ALT) {
		event->set_alt_pressed(true);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_CTRL) {
		event->set_ctrl_pressed(true);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_META) {
		event->set_meta_pressed(true);
	}

	ds->push_input_event(event);
}

void libgodot_display_server_push_mouse_button_event(GDExtensionObjectPtr p_instance, LibGodotMouseButtonIndex p_button_index, GDExtensionBool p_pressed, int p_x, int p_y, unsigned int p_modifiers, unsigned int p_button_mask) {
	DisplayServerExternal *ds = Object::cast_to<DisplayServerExternal>(DisplayServer::get_singleton());
	if (!ds) {
		return;
	}

	Ref<InputEventMouseButton> event;
	event.instantiate();

	event->set_window_id(DisplayServer::MAIN_WINDOW_ID);
	event->set_button_index((MouseButton)p_button_index);
	event->set_pressed(p_pressed);
	event->set_position(Vector2(p_x, p_y));
	event->set_global_position(Vector2(p_x, p_y));
	event->set_button_mask(DisplayServerExternal::_WrapButtonMask(p_button_mask));

	// Set modifiers
	if (p_modifiers & LIBGODOT_KEY_MOD_SHIFT) {
		event->set_shift_pressed(true);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_ALT) {
		event->set_alt_pressed(true);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_CTRL) {
		event->set_ctrl_pressed(true);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_META) {
		event->set_meta_pressed(true);
	}

	ds->push_input_event(event);
}

void libgodot_display_server_push_mouse_motion_event(GDExtensionObjectPtr p_instance, int p_x, int p_y, int p_relative_x, int p_relative_y, unsigned int p_modifiers, unsigned int p_button_mask) {
	DisplayServerExternal *ds = Object::cast_to<DisplayServerExternal>(DisplayServer::get_singleton());
	if (!ds) {
		return;
	}

	Ref<InputEventMouseMotion> event;
	event.instantiate();

	event->set_window_id(DisplayServer::MAIN_WINDOW_ID);
	event->set_position(Vector2(p_x, p_y));
	event->set_global_position(Vector2(p_x, p_y));
	event->set_relative(Vector2(p_relative_x, p_relative_y));
	event->set_button_mask(DisplayServerExternal::_WrapButtonMask(p_button_mask));

	// Set modifiers
	if (p_modifiers & LIBGODOT_KEY_MOD_SHIFT) {
		event->set_shift_pressed(true);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_ALT) {
		event->set_alt_pressed(true);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_CTRL) {
		event->set_ctrl_pressed(true);
	}
	if (p_modifiers & LIBGODOT_KEY_MOD_META) {
		event->set_meta_pressed(true);
	}

	ds->push_input_event(event);
}

void libgodot_display_server_push_mouse_wheel_event(GDExtensionObjectPtr p_instance, float p_delta_x, float p_delta_y, int p_x, int p_y, unsigned int p_modifiers) {
	DisplayServerExternal *ds = Object::cast_to<DisplayServerExternal>(DisplayServer::get_singleton());
	if (!ds) {
		return;
	}

	// Create mouse button events for wheel
	if (p_delta_y != 0) {
		Ref<InputEventMouseButton> event;
		event.instantiate();

		event->set_window_id(DisplayServer::MAIN_WINDOW_ID);
		event->set_button_index(p_delta_y > 0 ? MouseButton::WHEEL_UP : MouseButton::WHEEL_DOWN);
		event->set_factor(Math::abs(p_delta_y));
		event->set_position(Vector2(p_x, p_y));
		event->set_global_position(Vector2(p_x, p_y));

		// Set modifiers
		if (p_modifiers & LIBGODOT_KEY_MOD_SHIFT) {
			event->set_shift_pressed(true);
		}
		if (p_modifiers & LIBGODOT_KEY_MOD_ALT) {
			event->set_alt_pressed(true);
		}
		if (p_modifiers & LIBGODOT_KEY_MOD_CTRL) {
			event->set_ctrl_pressed(true);
		}
		if (p_modifiers & LIBGODOT_KEY_MOD_META) {
			event->set_meta_pressed(true);
		}

		// Press
		event->set_pressed(true);
		ds->push_input_event(event);

		// Release
		Ref<InputEventMouseButton> release_event = event->duplicate();
		release_event->set_pressed(false);
		ds->push_input_event(release_event);
	}

	if (p_delta_x != 0) {
		Ref<InputEventMouseButton> event;
		event.instantiate();

		event->set_window_id(DisplayServer::MAIN_WINDOW_ID);
		event->set_button_index(p_delta_x > 0 ? MouseButton::WHEEL_RIGHT : MouseButton::WHEEL_LEFT);
		event->set_factor(Math::abs(p_delta_x));
		event->set_position(Vector2(p_x, p_y));
		event->set_global_position(Vector2(p_x, p_y));

		// Set modifiers
		if (p_modifiers & LIBGODOT_KEY_MOD_SHIFT) {
			event->set_shift_pressed(true);
		}
		if (p_modifiers & LIBGODOT_KEY_MOD_ALT) {
			event->set_alt_pressed(true);
		}
		if (p_modifiers & LIBGODOT_KEY_MOD_CTRL) {
			event->set_ctrl_pressed(true);
		}
		if (p_modifiers & LIBGODOT_KEY_MOD_META) {
			event->set_meta_pressed(true);
		}

		// Press
		event->set_pressed(true);
		ds->push_input_event(event);

		// Release
		Ref<InputEventMouseButton> release_event = event->duplicate();
		release_event->set_pressed(false);
		ds->push_input_event(release_event);
	}
}

void libgodot_display_server_push_input_text(GDExtensionObjectPtr p_instance, const char *p_text) {
	DisplayServerExternal *ds = Object::cast_to<DisplayServerExternal>(DisplayServer::get_singleton());
	if (ds && p_text) {
		ds->push_input_text(String::utf8(p_text), DisplayServer::MAIN_WINDOW_ID);
	}
}
