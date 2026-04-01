/**************************************************************************/
/*  libgodot_status.h                                                    */
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

#include "libgodot.h"

#include "core/os/mutex.h"

#include <atomic>

/*
 * Engine lifecycle status tracking for libgodot.
 *
 * The status follows this state machine:
 *
 *   create_instance()       _ensure_setup()          load_project() / start()
 *   UNINITIALIZED ──► CORE_READY ──► SERVERS_READY ──► PROJECT_LOADING ──► RUNNING
 *                          │                ▲                                  │
 *                          │                │       unload_project()           │
 *                          │                └── IDLE ◄── PROJECT_UNLOADING ◄──┘
 *                          │
 *                          └──► ERROR  (on any failure)
 *
 *   stop_instance() from any active state:
 *     ──► STOPPING ──► STOPPED
 *
 * Status descriptions:
 *
 *   UNINITIALIZED     — No engine instance exists yet. This is the initial state
 *                       before libgodot_create_godot_instance() is called.
 *
 *   CORE_READY        — Main::setup() completed successfully. Core types are
 *                       registered, project settings are parsed, and command-line
 *                       arguments are processed. The rendering and audio servers
 *                       are NOT yet initialized.
 *
 *   SERVERS_READY     — Main::setup2() completed. All servers are initialized:
 *                       display (window), rendering (Metal/Vulkan/D3D12),
 *                       audio, physics, input, text, and scene types.
 *                       The engine is ready to load a project.
 *                       Equivalent to IDLE on first entry.
 *
 *   PROJECT_LOADING   — A project is being loaded via load_project() or start().
 *                       Sub-phases reported via the detail string include:
 *                       "Preparing project", "Loading project settings",
 *                       "Loading extensions", "Starting main scene".
 *
 *   RUNNING           — The main loop is initialized and the project is active.
 *                       Call libgodot_iteration_godot_instance() to drive frames.
 *                       Shader compilation happens lazily during early frames;
 *                       use libgodot_get_shader_compilations_pending() to monitor.
 *
 *   PROJECT_UNLOADING — The current project is being torn down via unload_project().
 *                       Scene tree, main loop, and project extensions are freed.
 *
 *   IDLE              — Engine is fully running but no project is loaded.
 *                       Ready for another load_project() call.
 *
 *   STOPPING          — The engine is shutting down via stop/destroy. The main loop
 *                       is being finalized.
 *
 *   STOPPED           — The engine has been fully stopped. The instance should be
 *                       destroyed with libgodot_destroy_godot_instance().
 *
 *   ERROR             — A fatal error occurred during any phase. The detail string
 *                       provides additional context about the failure.
 */

inline LibGodotStatusCallback status_callback = nullptr;
inline void *status_user_data = nullptr;
inline std::atomic<LibGodotStatus> current_status{ LIBGODOT_STATUS_UNINITIALIZED };
inline Mutex status_callback_mutex;

inline void _libgodot_set_status(LibGodotStatus p_status, const char *p_detail = nullptr) {
	current_status.store(p_status, std::memory_order_release);
	MutexLock lock(status_callback_mutex);
	if (status_callback) {
		status_callback(status_user_data, p_status, p_detail);
	}
}

inline LibGodotStatus _libgodot_get_status() {
	return current_status.load(std::memory_order_acquire);
}

inline void _libgodot_set_status_callback_impl(LibGodotStatusCallback p_callback, void *p_user_data) {
	MutexLock lock(status_callback_mutex);
	status_callback = p_callback;
	status_user_data = p_user_data;
}
