/**************************************************************************/
/*  resource_format_loader_raw.cpp                                        */
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

#include "resource_format_loader_raw.h"

#include "core/io/file_access.h"
#include "core/io/image.h"
#include "scene/resources/image_texture.h"

#include "modules/modules_enabled.gen.h"

#ifdef MODULE_MP3_ENABLED
#include "modules/mp3/audio_stream_mp3.h"
#endif

#include "scene/resources/audio_stream_wav.h"
#include "scene/resources/font.h"

static bool _is_image_ext(const String &ext) {
	return ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "webp" || ext == "bmp" || ext == "tga";
}

static bool _is_audio_ext(const String &ext) {
	return ext == "mp3" || ext == "wav" || ext == "ogg";
}

static bool _is_font_ext(const String &ext) {
	return ext == "ttf" || ext == "otf" || ext == "woff" || ext == "woff2" || ext == "pfb" || ext == "pfm";
}

Ref<Resource> ResourceFormatLoaderRaw::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_use_sub_threads, float *r_progress, CacheMode p_cache_mode) {
	String ext = p_path.get_extension().to_lower();

	// --- Images → ImageTexture ---
	if (_is_image_ext(ext)) {
		Ref<Image> img;
		img.instantiate();
		Error err = img->load(p_path);
		if (err != OK) {
			if (r_error) {
				*r_error = err;
			}
			return Ref<Resource>();
		}
		Ref<ImageTexture> tex = ImageTexture::create_from_image(img);
		if (r_error) {
			*r_error = OK;
		}
		return tex;
	}

	// --- MP3 → AudioStreamMP3 ---
#ifdef MODULE_MP3_ENABLED
	if (ext == "mp3") {
		Ref<AudioStreamMP3> stream = AudioStreamMP3::load_from_file(p_path);
		if (stream.is_null()) {
			if (r_error) {
				*r_error = ERR_FILE_CANT_READ;
			}
			return Ref<Resource>();
		}
		if (r_error) {
			*r_error = OK;
		}
		return stream;
	}
#endif

	// --- WAV → AudioStreamWAV ---
	if (ext == "wav") {
		Ref<AudioStreamWAV> stream = AudioStreamWAV::load_from_file(p_path, Dictionary());
		if (stream.is_null()) {
			if (r_error) {
				*r_error = ERR_FILE_CANT_READ;
			}
			return Ref<Resource>();
		}
		if (r_error) {
			*r_error = OK;
		}
		return stream;
	}

	// --- TTF/OTF → FontFile ---
	if (_is_font_ext(ext)) {
		Ref<FontFile> font;
		font.instantiate();
		Error err = font->load_dynamic_font(p_path);
		if (err != OK) {
			if (r_error) {
				*r_error = err;
			}
			return Ref<Resource>();
		}
		if (r_error) {
			*r_error = OK;
		}
		return font;
	}

	if (r_error) {
		*r_error = ERR_FILE_UNRECOGNIZED;
	}
	return Ref<Resource>();
}

void ResourceFormatLoaderRaw::get_recognized_extensions(List<String> *p_extensions) const {
	// Images
	p_extensions->push_back("png");
	p_extensions->push_back("jpg");
	p_extensions->push_back("jpeg");
	p_extensions->push_back("webp");
	p_extensions->push_back("bmp");
	p_extensions->push_back("tga");
	// Audio
	p_extensions->push_back("mp3");
	p_extensions->push_back("wav");
	// Fonts
	p_extensions->push_back("ttf");
	p_extensions->push_back("otf");
	p_extensions->push_back("woff");
	p_extensions->push_back("woff2");
}

bool ResourceFormatLoaderRaw::handles_type(const String &p_type) const {
	return p_type == "Texture2D" || p_type == "ImageTexture" || p_type == "Image" ||
			p_type == "AudioStream" || p_type == "AudioStreamMP3" || p_type == "AudioStreamWAV" ||
			p_type == "Font" || p_type == "FontFile";
}

String ResourceFormatLoaderRaw::get_resource_type(const String &p_path) const {
	String ext = p_path.get_extension().to_lower();
	if (_is_image_ext(ext)) {
		return "ImageTexture";
	}
	if (ext == "mp3") {
		return "AudioStreamMP3";
	}
	if (ext == "wav") {
		return "AudioStreamWAV";
	}
	if (_is_font_ext(ext)) {
		return "FontFile";
	}
	return String();
}
