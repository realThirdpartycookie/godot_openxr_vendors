/**************************************************************************/
/*  openxr_android_geospatial_extension.cpp                               */
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

#include "extensions/openxr_android_geospatial_extension.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/open_xr_future_extension.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "extensions/openxr_android_google_cloud_auth_extension.h"

using namespace godot;

OpenXRAndroidGeospatialExtension *OpenXRAndroidGeospatialExtension::singleton = nullptr;

OpenXRAndroidGeospatialExtension *OpenXRAndroidGeospatialExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRAndroidGeospatialExtension());
	}
	return singleton;
}

OpenXRAndroidGeospatialExtension::OpenXRAndroidGeospatialExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidGeospatialExtension singleton already exists.");

	request_extensions[XR_ANDROID_GEOSPATIAL_EXTENSION_NAME] = &android_geospatial_ext;
	singleton = this;
}

OpenXRAndroidGeospatialExtension::~OpenXRAndroidGeospatialExtension() {
	cleanup();
	singleton = nullptr;
}

void OpenXRAndroidGeospatialExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_geospatial_supported"), &OpenXRAndroidGeospatialExtension::is_geospatial_supported);
	ClassDB::bind_method(D_METHOD("get_geographic_location", "callback"), &OpenXRAndroidGeospatialExtension::get_geographic_location);
	ClassDB::bind_method(D_METHOD("check_vps_availability", "latitude", "longitude"), &OpenXRAndroidGeospatialExtension::check_vps_availability);
	ClassDB::bind_method(D_METHOD("start_geospatial"), &OpenXRAndroidGeospatialExtension::start_geospatial);
	ClassDB::bind_method(D_METHOD("stop_geospatial"), &OpenXRAndroidGeospatialExtension::stop_geospatial);
	ClassDB::bind_method(D_METHOD("get_geospatial_state"), &OpenXRAndroidGeospatialExtension::get_geospatial_state);
	ClassDB::bind_method(D_METHOD("transform_to_geospatial_pose", "transform"), &OpenXRAndroidGeospatialExtension::transform_to_geospatial_pose);
	ClassDB::bind_method(D_METHOD("geospatial_pose_to_transform", "pose"), &OpenXRAndroidGeospatialExtension::geospatial_pose_to_transform);

	BIND_ENUM_CONSTANT(GEOSPATIAL_STATE_STOPPED);
	BIND_ENUM_CONSTANT(GEOSPATIAL_STATE_RUNNING);
	BIND_ENUM_CONSTANT(GEOSPATIAL_STATE_INITIALIZATION_FAILED);

	BIND_ENUM_CONSTANT(VPS_AVAILABILITY_UNAVAILABLE);
	BIND_ENUM_CONSTANT(VPS_AVAILABILITY_AVAILABLE);

	ADD_SIGNAL(MethodInfo("openxr_android_geospatial_state_changed", PropertyInfo(Variant::INT, "state")));
}

bool OpenXRAndroidGeospatialExtension::is_geospatial_supported() const {
	return android_geospatial_ext && system_geospatial_properties.supportsGeospatial;
}

void OpenXRAndroidGeospatialExtension::get_geographic_location(const Callable &p_callback) {
	Object *godot_openxr_location_finder = Engine::get_singleton()->get_singleton("GodotOpenXRUtilitiesInternal");
	if (!godot_openxr_location_finder) {
		p_callback.call_deferred(false, 0.0f, 0.0f);
	}

	godot_openxr_location_finder->call("findLocation", p_callback);
}

Ref<OpenXRFutureResult> OpenXRAndroidGeospatialExtension::check_vps_availability(double p_latitude, double p_longitude) {
	ERR_FAIL_COND_V_MSG(!is_geospatial_supported(), nullptr, "Android geospatial isn't supported");

	OpenXRAndroidGoogleCloudAuthExtension *google_cloud_auth = OpenXRAndroidGoogleCloudAuthExtension::get_singleton();
	ERR_FAIL_COND_V_MSG(!google_cloud_auth || !google_cloud_auth->is_authenticated(), nullptr, "Must be authenticated with OpenXRAndroidGoogleCloudAuthExtension to use geospatial");

	OpenXRFutureExtension *future_api = OpenXRFutureExtension::get_singleton();
	ERR_FAIL_NULL_V(future_api, nullptr);

	XrFutureEXT future = XR_NULL_HANDLE;
	XrResult result = xrCheckVpsAvailabilityAsyncANDROID(SESSION, p_latitude, p_longitude, &future);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to check geospatial VPS availability: ", get_openxr_api()->get_error_string(result));
		return nullptr;
	}

	return future_api->register_future(reinterpret_cast<uint64_t>(future), callable_mp(this, &OpenXRAndroidGeospatialExtension::_on_check_vps_availability_complete));
}

void OpenXRAndroidGeospatialExtension::_on_check_vps_availability_complete(const Ref<OpenXRFutureResult> &p_future) {
	XrVPSAvailabilityCheckCompletionANDROID completion = {
		XR_TYPE_VPS_AVAILABILITY_CHECK_COMPLETION_ANDROID, // type
		nullptr, // next
		XR_RESULT_MAX_ENUM, // futureResult,
		XR_VPS_AVAILABILITY_UNAVAILABLE_ANDROID, // availability
	};

	XrResult result = xrCheckVpsAvailabilityCompleteANDROID(SESSION, (XrFutureEXT)p_future->get_future(), &completion);

	VpsAvailability availability = VPS_AVAILABILITY_UNAVAILABLE;
	if (XR_SUCCEEDED(result) && XR_SUCCEEDED(completion.futureResult) && completion.availability == XR_VPS_AVAILABILITY_AVAILABLE_ANDROID) {
		availability = VPS_AVAILABILITY_AVAILABLE;
	}
	p_future->set_result_value(availability);
}

void OpenXRAndroidGeospatialExtension::start_geospatial() {
	ERR_FAIL_COND(!is_geospatial_supported());
	ERR_FAIL_COND(geospatial_tracker != XR_NULL_HANDLE);

	OpenXRAndroidGoogleCloudAuthExtension *google_cloud_auth = OpenXRAndroidGoogleCloudAuthExtension::get_singleton();
	ERR_FAIL_COND_MSG(!google_cloud_auth || !google_cloud_auth->is_authenticated(), "Must be authenticated with OpenXRAndroidGoogleCloudAuthExtension to use geospatial");

	XrGeospatialTrackerCreateInfoANDROID create_info = {
		XR_TYPE_GEOSPATIAL_TRACKER_CREATE_INFO_ANDROID, // type
		nullptr, // next
	};

	XrResult result = xrCreateGeospatialTrackerANDROID(SESSION, &create_info, &geospatial_tracker);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to create geospatial tracker: ", get_openxr_api()->get_error_string(result));
	}
}

void OpenXRAndroidGeospatialExtension::stop_geospatial() {
	ERR_FAIL_COND(!is_geospatial_supported());
	ERR_FAIL_COND(geospatial_tracker == XR_NULL_HANDLE);

	XrResult result = xrDestroyGeospatialTrackerANDROID(geospatial_tracker);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to destroy geospatial tracker: ", get_openxr_api()->get_error_string(result));
	}

	geospatial_tracker = XR_NULL_HANDLE;

	GeospatialState old_geospatial_state = geospatial_state;
	geospatial_state = GEOSPATIAL_STATE_STOPPED;
	if (geospatial_state != old_geospatial_state) {
		emit_signal("openxr_android_geospatial_state_changed", geospatial_state);
	}
}

Ref<OpenXRAndroidGeospatialPose> OpenXRAndroidGeospatialExtension::transform_to_geospatial_pose(const Transform3D &p_transform) {
	ERR_FAIL_COND_V_MSG(!is_geospatial_supported(), nullptr, "Android geospatial isn't supported");
	ERR_FAIL_COND_V_MSG(geospatial_tracker == XR_NULL_HANDLE, nullptr, "Geospatial tracker is not started");

	Quaternion q = p_transform.basis.get_quaternion();
	XrPosef xr_pose = {
		{ (float)q.x, (float)q.y, (float)q.z, (float)q.w }, // orientation
		{ (float)p_transform.origin.x, (float)p_transform.origin.y, (float)p_transform.origin.z }, // position
	};

	XrGeospatialPoseFromPoseLocateInfoANDROID locate_info = {
		XR_TYPE_GEOSPATIAL_POSE_FROM_POSE_LOCATE_INFO_ANDROID, // type
		nullptr, // next
		(XrSpace)get_openxr_api()->get_play_space(), // space
		(XrTime)get_openxr_api()->get_predicted_display_time(), // time
		xr_pose, // pose
	};

	XrGeospatialPoseResultANDROID pose_result = {
		XR_TYPE_GEOSPATIAL_POSE_RESULT_ANDROID, // type
		nullptr, // next
		0, // poseFlags
		{}, // geospatialPose
		0.0, // horizontalAccuracy
		0.0, // verticalAccuracy
		0.0, // orientationYawAccuracy
	};

	XrResult result = xrLocateGeospatialPoseFromPoseANDROID(geospatial_tracker, &locate_info, &pose_result);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to locate geospatial pose from pose: ", get_openxr_api()->get_error_string(result));
		return nullptr;
	}

	XrGeospatialPoseANDROID &geo_pose = pose_result.geospatialPose;
	Quaternion orientation(geo_pose.eastUpSouthOrientation.x, geo_pose.eastUpSouthOrientation.y, geo_pose.eastUpSouthOrientation.z, geo_pose.eastUpSouthOrientation.w);

	Ref<OpenXRAndroidGeospatialPose> geospatial_pose = OpenXRAndroidGeospatialPose::create(orientation, geo_pose.latitude, geo_pose.longitude, geo_pose.altitude);
	geospatial_pose->set_accuracy(
			(pose_result.poseFlags & XR_GEOSPATIAL_POSE_ORIENTATION_VALID_BIT_ANDROID) != 0,
			(pose_result.poseFlags & XR_GEOSPATIAL_POSE_POSITION_VALID_BIT_ANDROID) != 0,
			pose_result.horizontalAccuracy,
			pose_result.verticalAccuracy,
			pose_result.orientationYawAccuracy);

	return geospatial_pose;
}

Transform3D OpenXRAndroidGeospatialExtension::geospatial_pose_to_transform(const Ref<OpenXRAndroidGeospatialPose> &p_pose) {
	ERR_FAIL_COND_V_MSG(!is_geospatial_supported(), Transform3D(), "Android geospatial isn't supported");
	ERR_FAIL_COND_V_MSG(geospatial_tracker == XR_NULL_HANDLE, Transform3D(), "Geospatial tracker is not started");
	ERR_FAIL_COND_V_MSG(p_pose.is_null(), Transform3D(), "Geospatial pose is null");

	Quaternion orientation = p_pose->get_orientation();
	XrGeospatialPoseANDROID geo_pose = {
		{ (float)orientation.x, (float)orientation.y, (float)orientation.z, (float)orientation.w }, // eastUpSouthOrientation
		p_pose->get_latitude(), // latitude
		p_pose->get_longitude(), // longitude
		p_pose->get_altitude(), // altitude
	};

	XrGeospatialPoseLocateInfoANDROID locate_info = {
		XR_TYPE_GEOSPATIAL_POSE_LOCATE_INFO_ANDROID, // type
		nullptr, // next
		(XrSpace)get_openxr_api()->get_play_space(), // space
		(XrTime)get_openxr_api()->get_predicted_display_time(), // time
		geo_pose, // geospatialPose
	};

	XrSpaceLocation location = {
		XR_TYPE_SPACE_LOCATION, // type
		nullptr, // next
		0, // locationFlags
		{}, // pose
	};

	XrResult result = xrLocateGeospatialPoseANDROID(geospatial_tracker, &locate_info, &location);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to locate geospatial pose: ", get_openxr_api()->get_error_string(result));
		return Transform3D();
	}

	return OpenXRUtilities::xrPosef_to_godot_transform3d(location.pose);
}

void OpenXRAndroidGeospatialExtension::_on_instance_created(uint64_t p_instance) {
	if (android_geospatial_ext) {
		bool result = initialize_android_geospatial_extension((XrInstance)p_instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize XR_ANDROID_geospatial extension");
			android_geospatial_ext = false;
		}
	}
}

void OpenXRAndroidGeospatialExtension::_on_instance_destroyed() {
	cleanup();
}

uint64_t OpenXRAndroidGeospatialExtension::_set_system_properties_and_get_next_pointer(void *p_next_pointer) {
	if (android_geospatial_ext) {
		system_geospatial_properties.next = p_next_pointer;
		return reinterpret_cast<uint64_t>(&system_geospatial_properties);
	} else {
		return reinterpret_cast<uint64_t>(p_next_pointer);
	}
}

bool OpenXRAndroidGeospatialExtension::_on_event_polled(const void *p_event) {
	if (!android_geospatial_ext) {
		return false;
	}

	if (static_cast<const XrEventDataBuffer *>(p_event)->type == XR_TYPE_EVENT_DATA_GEOSPATIAL_TRACKER_STATE_CHANGED_ANDROID) {
		XrEventDataGeospatialTrackerStateChangedANDROID *geospatial_event = (XrEventDataGeospatialTrackerStateChangedANDROID *)p_event;
		GeospatialState new_geospatial_state = geospatial_state;
		switch (geospatial_event->state) {
			case XR_GEOSPATIAL_TRACKER_STATE_STOPPED_ANDROID: {
				new_geospatial_state = GEOSPATIAL_STATE_STOPPED;
			} break;
			case XR_GEOSPATIAL_TRACKER_STATE_RUNNING_ANDROID: {
				new_geospatial_state = GEOSPATIAL_STATE_RUNNING;
			} break;
			case XR_GEOSPATIAL_TRACKER_STATE_INITIALIZATION_FAILED_ANDROID: {
				new_geospatial_state = GEOSPATIAL_STATE_INITIALIZATION_FAILED;
			} break;
			default: {
				UtilityFunctions::printerr("Received unknown geospatial tracker state: " + itos(geospatial_event->state));
			} break;
		}
		if (new_geospatial_state != geospatial_state) {
			geospatial_state = new_geospatial_state;
			emit_signal("openxr_android_geospatial_state_changed", geospatial_state);
		}
		return true;
	}

	return false;
}

bool OpenXRAndroidGeospatialExtension::initialize_android_geospatial_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrCreateGeospatialTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrDestroyGeospatialTrackerANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrLocateGeospatialPoseFromPoseANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrLocateGeospatialPoseANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrCheckVpsAvailabilityAsyncANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrCheckVpsAvailabilityCompleteANDROID);

	return true;
}

void OpenXRAndroidGeospatialExtension::cleanup() {
	if (geospatial_tracker != XR_NULL_HANDLE) {
		xrDestroyGeospatialTrackerANDROID(geospatial_tracker);
		geospatial_tracker = XR_NULL_HANDLE;
		geospatial_state = GEOSPATIAL_STATE_STOPPED;
	}

	android_geospatial_ext = false;
}

Dictionary OpenXRAndroidGeospatialExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}
