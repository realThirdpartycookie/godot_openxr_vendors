/**************************************************************************/
/*  openxr_android_geospatial_pose.cpp                                    */
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

#include "classes/openxr_android_geospatial_pose.h"

using namespace godot;

void OpenXRAndroidGeospatialPose::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_orientation", "orientation"), &OpenXRAndroidGeospatialPose::set_orientation);
	ClassDB::bind_method(D_METHOD("get_orientation"), &OpenXRAndroidGeospatialPose::get_orientation);

	ClassDB::bind_method(D_METHOD("set_latitude", "latitude"), &OpenXRAndroidGeospatialPose::set_latitude);
	ClassDB::bind_method(D_METHOD("get_latitude"), &OpenXRAndroidGeospatialPose::get_latitude);

	ClassDB::bind_method(D_METHOD("set_longitude", "longitude"), &OpenXRAndroidGeospatialPose::set_longitude);
	ClassDB::bind_method(D_METHOD("get_longitude"), &OpenXRAndroidGeospatialPose::get_longitude);

	ClassDB::bind_method(D_METHOD("set_altitude", "altitude"), &OpenXRAndroidGeospatialPose::set_altitude);
	ClassDB::bind_method(D_METHOD("get_altitude"), &OpenXRAndroidGeospatialPose::get_altitude);

	ClassDB::bind_static_method("OpenXRAndroidGeospatialPose", D_METHOD("create", "orientation", "latitude", "longitude", "altitude"), &OpenXRAndroidGeospatialPose::create);

	ClassDB::bind_method(D_METHOD("is_orientation_valid"), &OpenXRAndroidGeospatialPose::is_orientation_valid);
	ClassDB::bind_method(D_METHOD("is_position_valid"), &OpenXRAndroidGeospatialPose::is_position_valid);

	ClassDB::bind_method(D_METHOD("get_horizontal_accuracy"), &OpenXRAndroidGeospatialPose::get_horizontal_accuracy);
	ClassDB::bind_method(D_METHOD("get_vertical_accuracy"), &OpenXRAndroidGeospatialPose::get_vertical_accuracy);
	ClassDB::bind_method(D_METHOD("get_orientation_yaw_accuracy"), &OpenXRAndroidGeospatialPose::get_orientation_yaw_accuracy);

	ADD_PROPERTY(PropertyInfo(Variant::QUATERNION, "orientation"), "set_orientation", "get_orientation");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "latitude"), "set_latitude", "get_latitude");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "longitude"), "set_longitude", "get_longitude");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "altitude"), "set_altitude", "get_altitude");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "horizontal_accuracy"), "", "get_horizontal_accuracy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "vertical_accuracy"), "", "get_vertical_accuracy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "orientation_yaw_accuracy"), "", "get_orientation_yaw_accuracy");
}

void OpenXRAndroidGeospatialPose::set_orientation(const Quaternion &p_orientation) {
	orientation = p_orientation;
}

Quaternion OpenXRAndroidGeospatialPose::get_orientation() const {
	return orientation;
}

void OpenXRAndroidGeospatialPose::set_latitude(double p_latitude) {
	latitude = p_latitude;
}

double OpenXRAndroidGeospatialPose::get_latitude() const {
	return latitude;
}

void OpenXRAndroidGeospatialPose::set_longitude(double p_longitude) {
	longitude = p_longitude;
}

double OpenXRAndroidGeospatialPose::get_longitude() const {
	return longitude;
}

void OpenXRAndroidGeospatialPose::set_altitude(double p_altitude) {
	altitude = p_altitude;
}

double OpenXRAndroidGeospatialPose::get_altitude() const {
	return altitude;
}
