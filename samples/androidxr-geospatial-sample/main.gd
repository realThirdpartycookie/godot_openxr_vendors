extends Node3D

const HUD = preload("res://hud.gd")

@onready var viewport_2d_in_3d = %Viewport2Din3D
@onready var hud: HUD = viewport_2d_in_3d.get_scene_root()
@onready var camera: XRCamera3D = %XRCamera3D
@onready var timer: Timer = %Timer

const ANDROID_FINE_LOCATION_PERMISSION = "android.permission.ACCESS_FINE_LOCATION"

const GEOSPATIAL_STATES = {
	OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_INITIALIZATION_FAILED: "initialization failed",
	OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_RUNNING: "running",
	OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_STOPPED: "stopped",
}


func _ready() -> void:
	var geospatial_supported := OpenXRAndroidGeospatialExtension.is_geospatial_supported()
	hud.geospatial_supported_field.text = str(geospatial_supported)
	if not geospatial_supported:
		return

	var authenticated := await authenticate_with_google_cloud()
	hud.authenticated_field.text = str(authenticated)
	if not authenticated:
		return

	if ANDROID_FINE_LOCATION_PERMISSION in OS.get_granted_permissions():
		get_geographic_location()
	else:
		get_tree().on_request_permissions_result.connect(_on_request_permissions_result)
		OS.request_permission(ANDROID_FINE_LOCATION_PERMISSION)


func _on_request_permissions_result(permission: String, granted: bool) -> void:
	if permission == ANDROID_FINE_LOCATION_PERMISSION and granted:
		get_geographic_location()


func get_geographic_location() -> void:
	# If we want to confirm VPS availability before starting Geospatial,
	# then we need to get the approximate location without the VPS first.
	OpenXRAndroidGeospatialExtension.get_geographic_location(
		func(p_success: bool, p_latitude: float, p_longitude: float):
			if not p_success:
				print("Location not found.")
				hud.vps_availability_field.text = "error"
				return

			print("Location: ", p_latitude, ", ", p_longitude)

			check_vps_availability_and_start_geospatial.call_deferred(p_latitude, p_longitude)
	)


func check_vps_availability_and_start_geospatial(p_latitude: float, p_longitude: float) -> void:
	var vps_result := OpenXRAndroidGeospatialExtension.check_vps_availability(p_latitude, p_longitude)
	if not vps_result:
		print("Unable to check VPS availability")
		return

	await vps_result.completed
	if vps_result.get_status() != OpenXRFutureResult.RESULT_FINISHED:
		print("Getting VPS didn't finish: ", vps_result.get_status())
		hud.vps_availability_field.text = "error"
		return

	var vps_available: bool = vps_result.get_result_value() == OpenXRAndroidGeospatialExtension.VPS_AVAILABILITY_AVAILABLE
	hud.vps_availability_field.text = str(vps_available)
	if not vps_available:
		return

	OpenXRAndroidGeospatialExtension.openxr_android_geospatial_state_changed.connect(_on_geospatial_state_changed)
	OpenXRAndroidGeospatialExtension.start_geospatial()


func authenticate_with_google_cloud() -> bool:
	if not OpenXRAndroidGoogleCloudAuthExtension.is_enabled():
		print("Google Cloud Authentication Extension is not supported on this device.")
		return false

	var google_cloud_api_key := ""

	if FileAccess.file_exists("res://google_cloud_api_key.txt"):
		var file := FileAccess.open("res://google_cloud_api_key.txt", FileAccess.READ)
		if file:
			google_cloud_api_key = file.get_as_text().strip_edges()

	if google_cloud_api_key == "":
		print("Please set your Google Cloud API key in google_cloud_api_key.txt")
		return false

	var result := OpenXRAndroidGoogleCloudAuthExtension.set_google_cloud_auth_api_key(google_cloud_api_key)
	if result:
		await result.completed
		if result.get_result_value():
			print("Google Cloud API key set successfully.")
		else:
			print("Failed to set Google Cloud API key.")
			return false

	return true


func _on_geospatial_state_changed(p_state) -> void:
	hud.geospatial_state_field.text = GEOSPATIAL_STATES[p_state]

	if p_state == OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_RUNNING:
		# Start printing out pose information about the camera.
		timer.start()
	elif p_state == OpenXRAndroidGeospatialExtension.GEOSPATIAL_STATE_STOPPED:
		timer.stop()


func _on_timer_timeout() -> void:
	var hud: HUD = viewport_2d_in_3d.get_scene_root()
	var pose: OpenXRAndroidGeospatialPose = OpenXRAndroidGeospatialExtension.transform_to_geospatial_pose(camera.transform)

	var position_valid := pose.is_position_valid()
	hud.position_valid_field.text = str(position_valid)
	if position_valid:
		hud.latitude_field.text = str(pose.latitude)
		hud.longitude_field.text = str(pose.longitude)
		hud.horizontal_accuracy_field.text = str(pose.horizontal_accuracy)
		hud.vertical_accuracy_field.text = str(pose.vertical_accuracy)

	var orientation_valid := pose.is_orientation_valid()
	hud.orientation_valid_field.text = str(orientation_valid)
	if orientation_valid:
		hud.orientation_field.text = str(pose.orientation)
		hud.orientation_yaw_accuracy_field.text = str(pose.orientation_yaw_accuracy)
