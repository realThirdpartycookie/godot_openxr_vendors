/**************************************************************************/
/*  util.cpp                                                              */
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

#include "util.h"

#include <openxr/internal/xr_linear.h>
#include <openxr/openxr.h>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/projection.hpp>

using namespace godot;

StringName OpenXRUtilities::uuid_to_string_name(const XrUuid &p_uuid) {
	// This code was originally copied from
	// godot core's modules/openxr/openxr_util.cpp, OpenXRUtil::string_from_xruuid()
	// to ensure identical output.
	// It was then modified to use a char[] buffer for efficiency.

	// +1 for null terminator
	char ret[XR_UUID_SIZE * 2 + 1];
	char *ret_ptr = &ret[0];
	bool is_zero = true;

	for (int i = 0; i < XR_UUID_SIZE; i++) {
		ERR_FAIL_COND_V(XR_UUID_SIZE * 2 <= ret_ptr - &ret[0], "");

		is_zero = is_zero && p_uuid.data[i] == 0;

		char a = (p_uuid.data[i] & 0xF0) >> 4;
		char b = p_uuid.data[i] & 0x0F;

		if (a < 10) {
			*ret_ptr = '0' + a;
		} else {
			*ret_ptr = 'a' + a - 10;
		}

		++ret_ptr;
		if (b < 10) {
			*ret_ptr = '0' + b;
		} else {
			*ret_ptr = 'a' + b - 10;
		}

		++ret_ptr;
	}

	ret[XR_UUID_SIZE * 2] = '\0';
	return is_zero ? "" : String(ret);
}

void OpenXRUtilities::xrMatrix4x4f_to_godot_projection(XrMatrix4x4f *m, godot::Projection &p) {
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			p.columns[j][i] = m->m[j * 4 + i];
		}
	}
}

Transform3D OpenXRUtilities::xrPosef_to_godot_transform3d(const XrPosef &pose) {
	Transform3D out;
	out.origin.x = pose.position.x;
	out.origin.y = pose.position.y;
	out.origin.z = pose.position.z;
	out.basis = Basis{ Quaternion{ pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w } };
	return out;
}

Vector3 OpenXRUtilities::XrVector3f_to_godot_vector3(const XrVector3f &vector) {
	Vector3 out;
	out.x = vector.x;
	out.y = vector.y;
	out.z = vector.z;
	return out;
}

XrUuid OpenXRUtilities::string_name_to_uuid(const StringName &p_uuid_str) {
	// This code was copied from
	// godot core's modules/openxr/openxr_util.cpp, OpenXRUtil::xruuid_from_string()
	// to ensure identical output.
	// It was then modified to handle uuids with hyphens.
	String uuid = p_uuid_str;
	XrUuid new_uuid = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	int len = uuid.length();
	if (len == 0) {
		return new_uuid;
	} else if (len != (2 * XR_UUID_SIZE)) {
		// For compatibility, accept uuid strings that contain
		// hyphens, like: "ffffffff-ffff-ffff-ffff-ffffffffffff"
		uuid = uuid.remove_char('-');
		len = uuid.length();
		if (len != (2 * XR_UUID_SIZE)) {
			WARN_PRINT("OpenXR: Unexpected UUID length: " + String::num_int64(len) + " != " + String::num_int64(2 * XR_UUID_SIZE));
		}
	}

	int j = 0;
	const char32_t *uuid_ptr = uuid.ptr();
	for (int i = 0; i < XR_UUID_SIZE; i++) {
		uint8_t val = 0;

		// 2 chars per byte.
		for (int k = 0; k < 2; k++) {
			if (j < len) {
				val <<= 4;

				char32_t c = uuid_ptr[j++];
				if (c >= '0' && c <= '9') {
					val += uint8_t(c - '0');
				} else if (c >= 'a' && c <= 'f') {
					val += uint8_t(10 + c - 'a');
				} else if (c >= 'A' && c <= 'F') {
					val += uint8_t(10 + c - 'A');
				} else {
					WARN_PRINT("OpenXR: Unexpected character in UUID: " + String::num_int64(c));
				}
			}
		}

		new_uuid.data[i] = val;
	}

	return new_uuid;
}

bool OpenXRUtilities::supports_runtime_permissions() {
	OS *os = OS::get_singleton();
	ERR_FAIL_NULL_V(os, false);
	return os->has_feature("android");
}
