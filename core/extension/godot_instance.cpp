/**************************************************************************/
/*  godot_instance.cpp                                                    */
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

#include "godot_instance.h"

#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/extension/gdextension.h"
#include "core/extension/gdextension_manager.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/os/main_loop.h"
#include "main/main.h"
#include "servers/display/display_server.h"

void GodotInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start"), &GodotInstance::start);
	ClassDB::bind_method(D_METHOD("is_started"), &GodotInstance::is_started);
	ClassDB::bind_method(D_METHOD("iteration"), &GodotInstance::iteration);
	ClassDB::bind_method(D_METHOD("focus_in"), &GodotInstance::focus_in);
	ClassDB::bind_method(D_METHOD("focus_out"), &GodotInstance::focus_out);
	ClassDB::bind_method(D_METHOD("pause"), &GodotInstance::pause);
	ClassDB::bind_method(D_METHOD("resume"), &GodotInstance::resume);
	ClassDB::bind_method(D_METHOD("load_project", "path"), &GodotInstance::load_project);
	ClassDB::bind_method(D_METHOD("unload_project"), &GodotInstance::unload_project);
	ClassDB::bind_method(D_METHOD("reload_project", "path"), &GodotInstance::reload_project);
}

GodotInstance::GodotInstance() {
}

GodotInstance::~GodotInstance() {
}

Error GodotInstance::_ensure_setup() {
	if (setup_done) {
		return OK;
	}

	Error err = Main::setup2();
	if (err == OK) {
		setup_done = true;
	}
	return err;
}

void GodotInstance::load_project_extensions() {
	if (Engine::get_singleton()->is_recovery_mode_hint()) {
		return;
	}

	GDExtensionManager *ext_manager = GDExtensionManager::get_singleton();
	if (!ext_manager) {
		return;
	}

	// Snapshot currently loaded extensions before loading project extensions.
	HashSet<String> pre_existing;
	for (const String &path : ext_manager->get_loaded_extensions()) {
		pre_existing.insert(path);
	}

	// Read and load extensions from the project's extension_list.cfg.
	String config_path = GDExtension::get_extension_list_config_file();
	Ref<FileAccess> f = FileAccess::open(config_path, FileAccess::READ);
	while (f.is_valid() && !f->eof_reached()) {
		String ext_path = f->get_line().strip_edges();
		if (ext_path.is_empty()) {
			continue;
		}

		GDExtensionManager::LoadStatus status = ext_manager->load_extension(ext_path);
		if (status == GDExtensionManager::LOAD_STATUS_FAILED) {
			ERR_PRINT(vformat("Error loading project extension: '%s'.", ext_path));
			continue;
		}

		// Track only newly loaded extensions (not pre-existing).
		if (status == GDExtensionManager::LOAD_STATUS_OK) {
			// Resolve the path as it appears in the manager (may differ from config).
			for (const String &loaded_path : ext_manager->get_loaded_extensions()) {
				if (!pre_existing.has(loaded_path) && !project_loaded_extensions.has(loaded_path)) {
					project_loaded_extensions.insert(loaded_path);
				}
			}
		}
	}

	print_verbose(vformat("GodotInstance::load_project_extensions() loaded %d extention(s)", project_loaded_extensions.size()));
}

void GodotInstance::unload_project_extensions() {
	if (Engine::get_singleton()->is_recovery_mode_hint()) {
		return;
	}

	GDExtensionManager *ext_manager = GDExtensionManager::get_singleton();
	if (!ext_manager) {
		project_loaded_extensions.clear();
		return;
	}

	print_verbose(vformat("GodotInstance::unload_project_extensions() unloading %d extention(s)", project_loaded_extensions.size()));

	for (const String &ext_path : project_loaded_extensions) {
		GDExtensionManager::LoadStatus status = ext_manager->unload_extension(ext_path);
		if (status == GDExtensionManager::LOAD_STATUS_FAILED) {
			ERR_PRINT(vformat("Error unloading project extension: '%s'.", ext_path));
		} else if (status == GDExtensionManager::LOAD_STATUS_NOT_LOADED) {
			// Already unloaded, not an error.
			print_verbose(vformat("Project extension already unloaded: '%s'.", ext_path));
		} else if (status == GDExtensionManager::LOAD_STATUS_NEEDS_RESTART) {
			WARN_PRINT(vformat("Project extension requires restart to fully unload: '%s'.", ext_path));
		}
	}

	project_loaded_extensions.clear();
}

bool GodotInstance::initialize(GDExtensionInitializationFunction p_init_func) {
	print_verbose("Godot Instance initialization");
	if (!p_init_func) {
		return true;
	}
	GDExtensionManager *gdextension_manager = GDExtensionManager::get_singleton();
	GDExtensionConstPtr<const GDExtensionInitializationFunction> ptr((const GDExtensionInitializationFunction *)&p_init_func);
	GDExtensionManager::LoadStatus status = gdextension_manager->load_extension_from_function("libgodot://main", ptr);
	return status == GDExtensionManager::LoadStatus::LOAD_STATUS_OK;
}

bool GodotInstance::start() {
	print_verbose("GodotInstance::start()");
	if (_ensure_setup() != OK) {
		return false;
	}

	// If a project was preloaded via load_project(), ignore the original command line to avoid
	// reusing stale arguments (like a previous --path) on subsequent runs.
	if (!current_project_args.is_empty()) {
		Main::stop_project();
		started = Main::start(current_project_args, true) == EXIT_SUCCESS;
	} else {
		started = Main::start() == EXIT_SUCCESS;
	}

	if (started) {
		OS::get_singleton()->get_main_loop()->initialize();
		project_loaded = true;
	}
	return started;
}

bool GodotInstance::is_started() {
	return started;
}

bool GodotInstance::iteration() {
	if (!started || OS::get_singleton()->get_main_loop() == nullptr) {
		return true;
	}

	DisplayServer::get_singleton()->process_events();
	return Main::iteration();
}

void GodotInstance::stop() {
	print_verbose("GodotInstance::stop()");
	if (started || project_loaded) {
		Main::stop_project();
	}
	started = false;
	project_loaded = false;
}

bool GodotInstance::load_project(const String &p_path) {
	print_verbose("GodotInstance::load_project()");
	ERR_FAIL_COND_V(p_path.is_empty(), false);

	if (_ensure_setup() != OK) {
		return false;
	}

	// Clean up any running project before loading a new one.
	Main::stop_project();
	unload_project_extensions();

	String project_dir = p_path;
	String main_pack;
	if (project_dir.ends_with(".pck")) {
		main_pack = project_dir;
		project_dir = ".";
	}

	bool has_project_file = FileAccess::exists(project_dir.path_join("project.godot"));
	print_verbose(vformat("GodotInstance::load_project() dir=%s pck=%s pro=%d res=%s", project_dir, main_pack, has_project_file, ProjectSettings::get_singleton()->get_resource_path()));

	Ref<DirAccess> project_da = DirAccess::open(project_dir);
	if (project_da.is_null()) {
		ERR_PRINT(vformat("Project directory does not exist: %s", project_dir));
		return false;
	}

	project_dir = project_da->get_current_dir();
	if (!project_da->file_exists(project_dir.path_join("project.godot")) && !project_da->file_exists(project_dir.path_join("project.binary"))) {
		ERR_PRINT(vformat("No project.godot or project.binary found in %s", project_dir));
		return false;
	}

	Ref<DirAccess> cwd_da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	if (cwd_da.is_null()) {
		ERR_PRINT("Failed to create DirAccess for filesystem");
		return false;
	}
	Error chdir_err = cwd_da->change_dir(project_dir);
	if (chdir_err != OK) {
		ERR_PRINT(vformat("Failed to change directory to %s, error %d", project_dir, chdir_err));
		return false;
	}

	ProjectSettings::get_singleton()->set_resource_path(project_dir);

	Error err = ProjectSettings::get_singleton()->setup(project_dir, main_pack, true, false);
	if (err != OK) {
		ERR_PRINT(vformat("Failed to setup project at %s with error code %d", project_dir, err));
		return false;
	}

	load_project_extensions();

	current_project_path = p_path;
	current_project_args.clear();
	current_project_args.push_back(p_path);

	started = Main::start(current_project_args, true) == EXIT_SUCCESS;
	project_loaded = started;
	if (started && OS::get_singleton()->get_main_loop()) {
		OS::get_singleton()->get_main_loop()->initialize();
	}
	return started;
}

void GodotInstance::unload_project() {
	print_verbose("GodotInstance::unload_project()");
	if (started || project_loaded) {
		Main::stop_project();
	}

	unload_project_extensions();

	started = false;
	project_loaded = false;
	current_project_path = String();
	current_project_args.clear();
}

bool GodotInstance::reload_project(const String &p_path) {
	unload_project();
	return load_project(p_path);
}

void GodotInstance::focus_out() {
	print_verbose("GodotInstance::focus_out()");
	if (started) {
		if (OS::get_singleton()->get_main_loop()) {
			OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_APPLICATION_FOCUS_OUT);
		}
	}
}

void GodotInstance::focus_in() {
	print_verbose("GodotInstance::focus_in()");
	if (started) {
		if (OS::get_singleton()->get_main_loop()) {
			OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_APPLICATION_FOCUS_IN);
		}
	}
}

void GodotInstance::pause() {
	print_verbose("GodotInstance::pause()");
	if (started) {
		if (OS::get_singleton()->get_main_loop()) {
			OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_APPLICATION_PAUSED);
		}
	}
}

void GodotInstance::resume() {
	print_verbose("GodotInstance::resume()");
	if (started) {
		if (OS::get_singleton()->get_main_loop()) {
			OS::get_singleton()->get_main_loop()->notification(MainLoop::NOTIFICATION_APPLICATION_RESUMED);
		}
	}
}
