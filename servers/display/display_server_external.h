/**************************************************************************/
/*  display_server_external.h                                             */
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

#include "servers/display/display_server.h"

#include "core/extension/libgodot_display_server.h"

class InputEvent;
class RenderingContextDriver;
class RenderingDevice;

class DisplayServerExternal : public DisplayServer {
	GDCLASS(DisplayServerExternal, DisplayServer)

private:
	static LibGodotDisplayServerInterface *interface;

	// Rendering
	String rendering_driver;
#if defined(RD_ENABLED)
	RenderingContextDriver *rendering_context = nullptr;
	RenderingDevice *rendering_device = nullptr;
#endif

	// Window state
	Size2i window_size;
	Point2i window_position;
	DisplayServerEnums::WindowMode window_mode = DisplayServerEnums::WINDOW_MODE_WINDOWED;

	// Cursor state
	DisplayServerEnums::CursorShape cursor_shape = DisplayServerEnums::CURSOR_ARROW;
	DisplayServerEnums::MouseMode mouse_mode = DisplayServerEnums::MOUSE_MODE_VISIBLE;

	// Callbacks
	HashMap<DisplayServerEnums::WindowID, ObjectID> window_attached_instance_id;
	HashMap<DisplayServerEnums::WindowID, Callable> window_event_callbacks;
	HashMap<DisplayServerEnums::WindowID, Callable> window_resize_callbacks;
	HashMap<DisplayServerEnums::WindowID, Callable> input_event_callbacks;
	HashMap<DisplayServerEnums::WindowID, Callable> input_text_callbacks;
	HashMap<DisplayServerEnums::WindowID, Callable> drop_files_callbacks;

	// Input dispatch
	static void _dispatch_input_events(const Ref<InputEvent> &p_event);
	void _send_input_event(const Ref<InputEvent> &p_event, DisplayServerEnums::WindowID p_window_id) const;
	void _send_window_event(DisplayServerEnums::WindowEvent p_event, DisplayServerEnums::WindowID p_window_id) const;

public:
	// Helper to convert modifier flags (public for C API access)
	static BitField<KeyModifierMask> _WrapModifiers(unsigned int p_modifiers);
	static BitField<MouseButtonMask> _WrapButtonMask(unsigned int p_button_mask);
	static void register_external_driver();
	static void set_interface(LibGodotDisplayServerInterface *p_interface);
	static LibGodotDisplayServerInterface *get_interface();
	static bool has_interface();

	static DisplayServer *create_func(const String &p_rendering_driver, DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, DisplayServerEnums::Context p_context, int64_t p_parent_window, Error &r_error);
	static Vector<String> get_rendering_drivers_func();

	// ========================================================================
	// Event pushing (called from libgodot C API)
	// ========================================================================

	void push_window_event(DisplayServerEnums::WindowEvent p_event, DisplayServerEnums::WindowID p_window);
	void push_input_event(const Ref<InputEvent> &p_event);
	void push_input_text(const String &p_text, DisplayServerEnums::WindowID p_window);
	void notify_window_size_changed(DisplayServerEnums::WindowID p_window, int p_width, int p_height);

	// ========================================================================
	// DisplayServer interface - Core
	// ========================================================================

	virtual bool has_feature(DisplayServerEnums::Feature p_feature) const override;
	virtual String get_name() const override;

	// ========================================================================
	// DisplayServer interface - Screen
	// ========================================================================

	virtual int get_screen_count() const override;
	virtual int get_primary_screen() const override;
	virtual Point2i screen_get_position(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override;
	virtual Size2i screen_get_size(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override;
	virtual Rect2i screen_get_usable_rect(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override;
	virtual int screen_get_dpi(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override;
	virtual float screen_get_scale(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override;
	virtual float screen_get_refresh_rate(int p_screen = DisplayServerEnums::SCREEN_OF_MAIN_WINDOW) const override;

	// ========================================================================
	// DisplayServer interface - Window list
	// ========================================================================

	virtual Vector<DisplayServerEnums::WindowID> get_window_list() const override;
	virtual DisplayServerEnums::WindowID get_window_at_screen_position(const Point2i &p_position) const override;

	// ========================================================================
	// DisplayServer interface - Window instance
	// ========================================================================

	virtual void window_attach_instance_id(ObjectID p_instance, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual ObjectID window_get_attached_instance_id(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	// ========================================================================
	// DisplayServer interface - Window callbacks
	// ========================================================================

	virtual void window_set_rect_changed_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual void window_set_window_event_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual void window_set_input_event_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual void window_set_input_text_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual void window_set_drop_files_callback(const Callable &p_callable, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;

	// ========================================================================
	// DisplayServer interface - Window properties
	// ========================================================================

	virtual void window_set_title(const String &p_title, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;

	virtual int window_get_current_screen(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;
	virtual void window_set_current_screen(int p_screen, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;

	virtual Point2i window_get_position(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;
	virtual Point2i window_get_position_with_decorations(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;
	virtual void window_set_position(const Point2i &p_position, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;

	virtual void window_set_transient(DisplayServerEnums::WindowID p_window, DisplayServerEnums::WindowID p_parent) override;

	virtual void window_set_max_size(const Size2i p_size, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual Size2i window_get_max_size(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;
	virtual void window_set_min_size(const Size2i p_size, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual Size2i window_get_min_size(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	virtual void window_set_size(const Size2i p_size, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual Size2i window_get_size(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;
	virtual Size2i window_get_size_with_decorations(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	virtual void window_set_mode(DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual DisplayServerEnums::WindowMode window_get_mode(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	virtual bool window_is_maximize_allowed(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	virtual void window_set_flag(DisplayServerEnums::WindowFlags p_flag, bool p_enabled, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual bool window_get_flag(DisplayServerEnums::WindowFlags p_flag, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	virtual void window_request_attention(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual void window_move_to_foreground(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual bool window_is_focused(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	virtual bool window_can_draw(DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;
	virtual bool can_any_window_draw() const override;

	// ========================================================================
	// DisplayServer interface - Events
	// ========================================================================

	virtual void process_events() override;

	// ========================================================================
	// DisplayServer interface - Mouse
	// ========================================================================

	virtual void mouse_set_mode(DisplayServerEnums::MouseMode p_mode) override;
	virtual DisplayServerEnums::MouseMode mouse_get_mode() const override;
	virtual void warp_mouse(const Point2i &p_position) override;
	virtual Point2i mouse_get_position() const override;
	virtual BitField<MouseButtonMask> mouse_get_button_state() const override;

	// ========================================================================
	// DisplayServer interface - Clipboard
	// ========================================================================

	virtual void clipboard_set(const String &p_text) override;
	virtual String clipboard_get() const override;
	virtual bool clipboard_has() const override;

	// ========================================================================
	// DisplayServer interface - Cursor
	// ========================================================================

	virtual void cursor_set_shape(DisplayServerEnums::CursorShape p_shape) override;
	virtual DisplayServerEnums::CursorShape cursor_get_shape() const override;
	virtual void cursor_set_custom_image(const Ref<Resource> &p_cursor, DisplayServerEnums::CursorShape p_shape = DisplayServerEnums::CURSOR_ARROW, const Vector2 &p_hotspot = Vector2()) override;

	// ========================================================================
	// DisplayServer interface - IME
	// ========================================================================

	virtual void window_set_ime_active(const bool p_active, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual void window_set_ime_position(const Point2i &p_pos, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;

	// ========================================================================
	// DisplayServer interface - VSync
	// ========================================================================

	virtual void window_set_vsync_mode(DisplayServerEnums::VSyncMode p_vsync_mode, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) override;
	virtual DisplayServerEnums::VSyncMode window_get_vsync_mode(DisplayServerEnums::WindowID p_window) const override;

	// ========================================================================
	// DisplayServer interface - Native handles
	// ========================================================================

	virtual int64_t window_get_native_handle(DisplayServerEnums::HandleType p_handle_type, DisplayServerEnums::WindowID p_window = DisplayServerEnums::MAIN_WINDOW_ID) const override;

	// ========================================================================
	// DisplayServer interface - Rendering
	// ========================================================================

	virtual void swap_buffers() override;

	// ========================================================================
	// DisplayServer interface - Misc
	// ========================================================================

	virtual void beep() const override;

	// ========================================================================
	// Constructor / Destructor
	// ========================================================================

	DisplayServerExternal(const String &p_rendering_driver, DisplayServerEnums::WindowMode p_mode, DisplayServerEnums::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, DisplayServerEnums::Context p_context, Error &r_error);
	~DisplayServerExternal();
};
