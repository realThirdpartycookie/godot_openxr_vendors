/**************************************************************************/
/*  openxr_android_viewport_feathering_extension.cpp                      */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
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

#include "extensions/openxr_android_viewport_feathering_extension.h"

#include <godot_cpp/classes/open_xr_interface.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/classes/xr_server.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidViewportFeatheringExtension *OpenXRAndroidViewportFeatheringExtension::singleton = nullptr;

OpenXRAndroidViewportFeatheringExtension *OpenXRAndroidViewportFeatheringExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRAndroidViewportFeatheringExtension());
	}
	return singleton;
}

OpenXRAndroidViewportFeatheringExtension::OpenXRAndroidViewportFeatheringExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidViewportFeatheringExtension singleton already exists.");

	request_extensions[XR_ANDROID_VIEWPORT_FEATHERING_EXTENSION_NAME] = &android_viewport_feathering_ext;
	singleton = this;
}

OpenXRAndroidViewportFeatheringExtension::~OpenXRAndroidViewportFeatheringExtension() {
	cleanup();
	singleton = nullptr;
}

void OpenXRAndroidViewportFeatheringExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRAndroidViewportFeatheringExtension::is_enabled);
	ClassDB::bind_method(D_METHOD("get_display_safe_area", "view"), &OpenXRAndroidViewportFeatheringExtension::get_display_safe_area);
}

bool OpenXRAndroidViewportFeatheringExtension::is_enabled() const {
	return android_viewport_feathering_ext;
}

Rect2i OpenXRAndroidViewportFeatheringExtension::get_display_safe_area(uint32_t p_view) const {
	ERR_FAIL_COND_V(!android_viewport_feathering_ext, Rect2i());
	ERR_FAIL_UNSIGNED_INDEX_V(p_view, viewport_feathering_config.size(), Rect2i());

	XRServer *xr_server = XRServer::get_singleton();
	ERR_FAIL_NULL_V(xr_server, Rect2i());

	Ref<OpenXRInterface> xr_interface = xr_server->find_interface("OpenXR");
	ERR_FAIL_COND_V(xr_interface.is_null(), Rect2i());

	Vector2 render_target_size = xr_interface->get_render_target_size();
	const XrViewportFeatheringConfigViewANDROID &vfc = viewport_feathering_config[p_view];

	return Rect2i(
			vfc.insetLeft,
			vfc.insetTop,
			int(render_target_size.x) - (vfc.insetLeft + vfc.insetRight),
			int(render_target_size.y) - (vfc.insetTop + vfc.insetBottom));
}

void OpenXRAndroidViewportFeatheringExtension::_on_instance_destroyed() {
	cleanup();
}

void OpenXRAndroidViewportFeatheringExtension::_prepare_view_configuration(int32_t p_view_count) {
	if (!android_viewport_feathering_ext) {
		return;
	}

	ERR_FAIL_COND(p_view_count < 0);

	viewport_feathering_config.resize(p_view_count);
	for (XrViewportFeatheringConfigViewANDROID &config : viewport_feathering_config) {
		config.type = XR_TYPE_VIEWPORT_FEATHERING_CONFIG_VIEW_ANDROID;
		config.next = nullptr;
		config.insetLeft = 0;
		config.insetRight = 0;
		config.insetTop = 0;
		config.insetBottom = 0;
	}
}

uint64_t OpenXRAndroidViewportFeatheringExtension::_set_view_configuration_and_get_next_pointer(uint32_t p_view, void *p_next_pointer) {
	if (!android_viewport_feathering_ext) {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}

	ERR_FAIL_UNSIGNED_INDEX_V(p_view, viewport_feathering_config.size(), reinterpret_cast<uint64_t>(p_next_pointer));

	viewport_feathering_config[p_view].next = p_next_pointer;
	return reinterpret_cast<uint64_t>(&viewport_feathering_config[p_view]);
}

void OpenXRAndroidViewportFeatheringExtension::_print_view_configuration_info(int32_t p_view) const {
	if (!android_viewport_feathering_ext) {
		return;
	}

	ERR_FAIL_UNSIGNED_INDEX(p_view, viewport_feathering_config.size());

	const XrViewportFeatheringConfigViewANDROID &config = viewport_feathering_config[p_view];

	print_line(" - viewport feathering inset left: ", config.insetLeft);
	print_line(" - viewport feathering inset right: ", config.insetRight);
	print_line(" - viewport feathering inset top: ", config.insetTop);
	print_line(" - viewport feathering inset bottom: ", config.insetBottom);
}

void OpenXRAndroidViewportFeatheringExtension::cleanup() {
	android_viewport_feathering_ext = false;
	viewport_feathering_config.clear();
}

Dictionary OpenXRAndroidViewportFeatheringExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}
