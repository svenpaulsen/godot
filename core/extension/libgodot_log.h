/**************************************************************************/
/*  libgodot_log.h                                                        */
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

#include "core/error/error_macros.h"
#include "core/os/os.h"
#include "core/string/print_string.h"
#include "core/string/ustring.h"

#include <cstdio>

static LibGodotLogCallback log_callback = nullptr;
static void *log_user_data = nullptr;
static PrintHandlerList log_print_handler;
static ErrorHandlerList log_error_handler;
static bool log_handlers_registered = false;

static void _libgodot_print_handler(void *p_userdata, const String &p_string, bool p_error, bool p_rich) {
	if (log_callback) {
		CharString utf8 = p_string.utf8();
		log_callback(log_user_data, p_error ? LIBGODOT_LOG_LEVEL_ERROR : LIBGODOT_LOG_LEVEL_INFO, utf8.get_data());
	}
}

static void _libgodot_error_handler(void *p_userdata, const char *p_func, const char *p_file, int p_line, const char *p_error, const char *p_message, bool p_editor_notify, ErrorHandlerType p_type) {
	if (log_callback) {
		LibGodotLogLevel level;
		const char *prefix;
		switch (p_type) {
			case ERR_HANDLER_WARNING:
				level = LIBGODOT_LOG_LEVEL_WARNING;
				prefix = "WARNING";
				break;
			case ERR_HANDLER_SCRIPT:
				level = LIBGODOT_LOG_LEVEL_ERROR;
				prefix = "SCRIPT ERROR";
				break;
			case ERR_HANDLER_SHADER:
				level = LIBGODOT_LOG_LEVEL_ERROR;
				prefix = "SHADER ERROR";
				break;
			default:
				level = LIBGODOT_LOG_LEVEL_ERROR;
				prefix = "ERROR";
				break;
		}

		char buf[4096];
		if (p_message && p_message[0]) {
			snprintf(buf, sizeof(buf), "%s: %s\n   at: %s (%s:%d)", prefix, p_message, p_func, p_file, p_line);
		} else {
			snprintf(buf, sizeof(buf), "%s: %s\n   at: %s (%s:%d)", prefix, p_error, p_func, p_file, p_line);
		}
		log_callback(log_user_data, level, buf);
	}
}

static void _libgodot_apply_log_stdout() {
	if (log_callback) {
		OS *os_singleton = OS::get_singleton();
		if (os_singleton) {
			os_singleton->set_stdout_enabled(false);
			os_singleton->set_stderr_enabled(false);
		}
	}
}

static void _libgodot_set_log_callback_impl(LibGodotLogCallback p_callback, void *p_user_data) {
	if (log_handlers_registered) {
		remove_print_handler(&log_print_handler);
		remove_error_handler(&log_error_handler);
		log_handlers_registered = false;
	}

	log_callback = p_callback;
	log_user_data = p_user_data;

	if (p_callback) {
		log_print_handler.printfunc = _libgodot_print_handler;
		log_print_handler.userdata = nullptr;
		log_print_handler.next = nullptr;

		log_error_handler.errfunc = _libgodot_error_handler;
		log_error_handler.userdata = nullptr;
		log_error_handler.next = nullptr;

		add_print_handler(&log_print_handler);
		add_error_handler(&log_error_handler);
		log_handlers_registered = true;
	}

	OS *os_singleton = OS::get_singleton();
	if (os_singleton) {
		os_singleton->set_stdout_enabled(!p_callback);
		os_singleton->set_stderr_enabled(!p_callback);
	}
}
