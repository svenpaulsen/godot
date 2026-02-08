/**************************************************************************/
/*  libgodot_macos.mm                                                     */
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

#include "os_macos.h"

#include "core/extension/godot_instance.h"
#include "core/extension/libgodot.h"
#include "main/main.h"
#include "servers/display/display_server.h"

static OS_MacOS *os = nullptr;

static GodotInstance *instance = nullptr;

GDExtensionObjectPtr libgodot_create_godot_instance(int p_argc, char *p_argv[], GDExtensionInitializationFunction p_init_func) {
	ERR_FAIL_COND_V_MSG(instance != nullptr, nullptr, "Only one Godot Instance may be created.");

	uint32_t remaining_args = p_argc - 1;
	os = new OS_MacOS_NSApp(p_argv[0], remaining_args, remaining_args > 0 ? &p_argv[1] : nullptr);

	@autoreleasepool {
		Error err = Main::setup(p_argv[0], remaining_args, remaining_args > 0 ? &p_argv[1] : nullptr, false, true);
		if (err != OK) {
			return nullptr;
		}

		instance = memnew(GodotInstance);
		if (!instance->initialize(p_init_func)) {
			memdelete(instance);
			instance = nullptr;
			return nullptr;
		}

		return (GDExtensionObjectPtr)instance;
	}
}

void libgodot_destroy_godot_instance(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	if (instance == godot_instance) {
		godot_instance->stop();
		memdelete(godot_instance);
		instance = nullptr;
		Main::cleanup();
	}
}

bool libgodot_start_godot_instance(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND_V(godot_instance == nullptr, false);

	return godot_instance->start();
}

bool libgodot_is_godot_instance_started(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND_V(godot_instance == nullptr, false);

	return godot_instance->is_started();
}

bool libgodot_iteration_godot_instance(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND_V(godot_instance == nullptr, false);

	return godot_instance->iteration();
}

void libgodot_stop_godot_instance(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND(godot_instance == nullptr);

	godot_instance->stop();
}

void libgodot_focus_in_godot_instance(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND(godot_instance == nullptr);

	godot_instance->focus_in();
}

void libgodot_focus_out_godot_instance(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND(godot_instance == nullptr);

	godot_instance->focus_out();
}

void libgodot_pause_godot_instance(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND(godot_instance == nullptr);

	godot_instance->pause();
}

void libgodot_resume_godot_instance(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND(godot_instance == nullptr);

	godot_instance->resume();
}

bool libgodot_load_project(GDExtensionObjectPtr p_godot_instance, const char *p_project_path) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND_V(godot_instance == nullptr, false);
	ERR_FAIL_COND_V(p_project_path == nullptr, false);

	return godot_instance->load_project(String(p_project_path));
}

void libgodot_unload_project(GDExtensionObjectPtr p_godot_instance) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND(godot_instance == nullptr);

	godot_instance->unload_project();
}

bool libgodot_reload_project(GDExtensionObjectPtr p_godot_instance, const char *p_project_path) {
	GodotInstance *godot_instance = (GodotInstance *)p_godot_instance;
	ERR_FAIL_COND_V(godot_instance == nullptr, false);
	ERR_FAIL_COND_V(p_project_path == nullptr, false);

	return godot_instance->reload_project(String(p_project_path));
}

uint64_t libgodot_window_get_native_handle(int32_t p_handle_type, int32_t p_window_id) {
	DisplayServer *ds = DisplayServer::get_singleton();
	if (ds == nullptr) {
		return 0;
	}

	DisplayServerEnums::HandleType ht = static_cast<DisplayServerEnums::HandleType>(p_handle_type);
	DisplayServerEnums::WindowID wid = static_cast<DisplayServerEnums::WindowID>(p_window_id);

	int64_t native = ds->window_get_native_handle(ht, wid);
	if (native == 0) {
		return 0;
	}

	return static_cast<uint64_t>(native);
}
