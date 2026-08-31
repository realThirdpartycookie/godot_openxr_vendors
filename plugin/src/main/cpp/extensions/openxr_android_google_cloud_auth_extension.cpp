/**************************************************************************/
/*  openxr_android_google_cloud_auth_extension.cpp                        */
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

#include "extensions/openxr_android_google_cloud_auth_extension.h"

#include <godot_cpp/classes/open_xr_future_extension.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

OpenXRAndroidGoogleCloudAuthExtension *OpenXRAndroidGoogleCloudAuthExtension::singleton = nullptr;

OpenXRAndroidGoogleCloudAuthExtension *OpenXRAndroidGoogleCloudAuthExtension::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRAndroidGoogleCloudAuthExtension());
	}
	return singleton;
}

OpenXRAndroidGoogleCloudAuthExtension::OpenXRAndroidGoogleCloudAuthExtension() :
		OpenXRExtensionWrapper() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRAndroidGoogleCloudAuthExtension singleton already exists.");

	request_extensions[XR_ANDROID_GOOGLE_CLOUD_AUTH_EXTENSION_NAME] = &android_google_cloud_auth_ext;
	singleton = this;
}

OpenXRAndroidGoogleCloudAuthExtension::~OpenXRAndroidGoogleCloudAuthExtension() {
	cleanup();
	singleton = nullptr;
}

void OpenXRAndroidGoogleCloudAuthExtension::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_enabled"), &OpenXRAndroidGoogleCloudAuthExtension::is_enabled);
	ClassDB::bind_method(D_METHOD("is_authenticated"), &OpenXRAndroidGoogleCloudAuthExtension::is_authenticated);
	ClassDB::bind_method(D_METHOD("set_google_cloud_auth_api_key", "api_key"), &OpenXRAndroidGoogleCloudAuthExtension::set_google_cloud_auth_api_key);
	ClassDB::bind_method(D_METHOD("set_google_cloud_auth_token", "auth_token"), &OpenXRAndroidGoogleCloudAuthExtension::set_google_cloud_auth_token);
	ClassDB::bind_method(D_METHOD("set_google_cloud_auth_keyless"), &OpenXRAndroidGoogleCloudAuthExtension::set_google_cloud_auth_keyless);
}

bool OpenXRAndroidGoogleCloudAuthExtension::is_enabled() const {
	return android_google_cloud_auth_ext;
}

bool OpenXRAndroidGoogleCloudAuthExtension::is_authenticated() const {
	return authenticated;
}

Ref<OpenXRFutureResult> OpenXRAndroidGoogleCloudAuthExtension::set_google_cloud_auth_api_key(const String &p_api_key) {
	ERR_FAIL_COND_V_MSG(!android_google_cloud_auth_ext, nullptr, "XR_ANDROID_google_cloud_auth extension is not enabled.");

	OpenXRFutureExtension *future_api = OpenXRFutureExtension::get_singleton();
	ERR_FAIL_NULL_V(future_api, nullptr);

	CharString api_key_ascii = p_api_key.ascii();

	XrGoogleCloudAuthInfoApiKeyANDROID auth_info = {
		XR_TYPE_GOOGLE_CLOUD_AUTH_INFO_API_KEY_ANDROID, // type
		nullptr, // next
		api_key_ascii.get_data(), // apiKey
	};

	XrFutureEXT future = XR_NULL_HANDLE;
	XrResult result = xrSetGoogleCloudAuthAsyncANDROID(SESSION, (const XrGoogleCloudAuthInfoBaseHeaderANDROID *)&auth_info, &future);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to set Google Cloud auth API key: ", get_openxr_api()->get_error_string(result));
		return nullptr;
	}

	return future_api->register_future(reinterpret_cast<uint64_t>(future), callable_mp(this, &OpenXRAndroidGoogleCloudAuthExtension::_on_auth_complete));
}

Ref<OpenXRFutureResult> OpenXRAndroidGoogleCloudAuthExtension::set_google_cloud_auth_token(const String &p_auth_token) {
	ERR_FAIL_COND_V_MSG(!android_google_cloud_auth_ext, nullptr, "XR_ANDROID_google_cloud_auth extension is not enabled.");

	OpenXRFutureExtension *future_api = OpenXRFutureExtension::get_singleton();
	ERR_FAIL_NULL_V(future_api, nullptr);

	CharString auth_token_ascii = p_auth_token.ascii();

	XrGoogleCloudAuthInfoTokenANDROID auth_info = {
		XR_TYPE_GOOGLE_CLOUD_AUTH_INFO_TOKEN_ANDROID, // type
		nullptr, // next
		auth_token_ascii.get_data(), // authToken
	};

	XrFutureEXT future = XR_NULL_HANDLE;
	XrResult result = xrSetGoogleCloudAuthAsyncANDROID(SESSION, (const XrGoogleCloudAuthInfoBaseHeaderANDROID *)&auth_info, &future);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to set Google Cloud auth API token: ", get_openxr_api()->get_error_string(result));
		return nullptr;
	}

	return future_api->register_future(reinterpret_cast<uint64_t>(future), callable_mp(this, &OpenXRAndroidGoogleCloudAuthExtension::_on_auth_complete));
}

Ref<OpenXRFutureResult> OpenXRAndroidGoogleCloudAuthExtension::set_google_cloud_auth_keyless() {
	ERR_FAIL_COND_V_MSG(!android_google_cloud_auth_ext, nullptr, "XR_ANDROID_google_cloud_auth extension is not enabled.");

	OpenXRFutureExtension *future_api = OpenXRFutureExtension::get_singleton();
	ERR_FAIL_NULL_V(future_api, nullptr);

	XrGoogleCloudAuthInfoKeylessANDROID auth_info = {
		XR_TYPE_GOOGLE_CLOUD_AUTH_INFO_KEYLESS_ANDROID, // type
		nullptr, // next
	};

	XrFutureEXT future = XR_NULL_HANDLE;
	XrResult result = xrSetGoogleCloudAuthAsyncANDROID(SESSION, (const XrGoogleCloudAuthInfoBaseHeaderANDROID *)&auth_info, &future);
	if (XR_FAILED(result)) {
		UtilityFunctions::printerr("Failed to set Google Cloud auth keyless: ", get_openxr_api()->get_error_string(result));
		return nullptr;
	}

	return future_api->register_future(reinterpret_cast<uint64_t>(future), callable_mp(this, &OpenXRAndroidGoogleCloudAuthExtension::_on_auth_complete));
}

void OpenXRAndroidGoogleCloudAuthExtension::_on_auth_complete(const Ref<OpenXRFutureResult> &p_future) {
	XrFutureCompletionEXT completion = {
		XR_TYPE_FUTURE_COMPLETION_EXT, // type
		nullptr, // next
		XR_RESULT_MAX_ENUM, // futureResult
	};

	XrResult result = xrSetGoogleCloudAuthCompleteANDROID(SESSION, (XrFutureEXT)p_future->get_future(), &completion);
	authenticated = XR_SUCCEEDED(result) && XR_SUCCEEDED(completion.futureResult);
	p_future->set_result_value(authenticated);
}

void OpenXRAndroidGoogleCloudAuthExtension::_on_instance_created(uint64_t p_instance) {
	if (android_google_cloud_auth_ext) {
		bool result = initialize_android_google_cloud_auth_extension((XrInstance)p_instance);
		if (!result) {
			UtilityFunctions::printerr("Failed to initialize XR_ANDROID_google_cloud_auth extension");
			android_google_cloud_auth_ext = false;
		}
	}
}

void OpenXRAndroidGoogleCloudAuthExtension::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRAndroidGoogleCloudAuthExtension::initialize_android_google_cloud_auth_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrSetGoogleCloudAuthAsyncANDROID);
	GDEXTENSION_INIT_XR_FUNC_V(xrSetGoogleCloudAuthCompleteANDROID);

	return true;
}

void OpenXRAndroidGoogleCloudAuthExtension::cleanup() {
	android_google_cloud_auth_ext = false;
}

Dictionary OpenXRAndroidGoogleCloudAuthExtension::_get_requested_extensions(uint64_t p_xr_version) {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}
