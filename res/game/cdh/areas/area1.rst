carnivores_area_resources

[general]
detailmap_texture = "detailmap_default"

[sky]

use_sky_sphere = 1

dawn_sky_color = 119, 90, 82
dawn_sun_color = 250, 230, 170
dawn_shadows_color = 30, 20, 30
dawn_sky_sphere_texture = "sunset_grad_3a"
dawn_sky_plane_texture = "clear_sky_1"
dawn_sky_plane_opacity = 128
dawn_sky_plane_color = 64, 44, 44
dawn_clouds_shadows_texture = "area6_clouds"
dawn_clouds_speed_scale = 1
dawn_fog_start = 0.6
dawn_fog_end = 1

day_sky_color = 80, 112, 125
day_sun_color = 255, 238, 222
day_shadows_color = 30, 50, 80
day_sky_sphere_texture = "grad5"
day_sky_plane_texture = "storm_sky_3"
day_sky_plane_opacity = 142
day_sky_plane_color = 255, 255, 255
day_clouds_shadows_texture = "area6_clouds"
day_clouds_speed_scale = 2
day_fog_start = 0.8
day_fog_end = 1

night_sky_color = 21, 26, 38
night_sun_color = 32, 68, 104
night_shadows_color = 16, 24, 82
night_sky_sphere_texture = "car_night_grad1"
night_sky_plane_texture = "clear_sky_1"
night_sky_plane_opacity = 255
night_sky_plane_color = 21, 26, 44
night_clouds_shadows_texture = "area6_clouds"
night_clouds_speed_scale = 4
night_fog_start = 0
night_fog_end = 0.8

[tiles]
atlas_texture = "area1_tiles"
atlas_tiles_in_row = 8
atlas_tiles_in_column = 8
sprites_atlas_texture = "area1_sprites"
grass_atlas_texture = "area_grass_A"
grass_mask_texture = "area1_grass_mask"

[birthplaces]
count = 16

birthplace_id 0
	position = 220808, 0, -36092
	alpha = 309
	beta = 1

birthplace_id 1
	position = 194579, 0, -72121
	alpha = 307
	beta = -11

birthplace_id 2
	position = 219143, 0, -109956
	alpha = 313
	beta = 10

birthplace_id 3
	position = 200708, 0, -110903
	alpha = 223
	beta = 9

birthplace_id 4
	position = 179478, 0, -109807
	alpha = 238
	beta = -4

birthplace_id 5
	position = 186056, 0, -153025
	alpha = 167
	beta = -5

birthplace_id 6
	position = 158962, 0, -193294
	alpha = 284
	beta = 1

birthplace_id 7
	position = 196630, 0, -190723
	alpha = 134
	beta = 4

birthplace_id 8
	position = 106953, 0, -122954
	alpha = 262
	beta = -1

birthplace_id 9
	position = 61763, 0, -95143
	alpha = 343
	beta = 5

birthplace_id 10
	position = 48611, 0, -68845
	alpha = 346
	beta = 0

birthplace_id 11
	position = 34447, 0, -127753
	alpha = 17
	beta = 0

birthplace_id 12
	position = 27219, 0, -204387
	alpha = 163
	beta = -5

birthplace_id 13
	position = 55899, 0, -207838
	alpha = 201
	beta = 2

birthplace_id 14
	position = 92615, 0, -179005
	alpha = 170
	beta = 4

birthplace_id 15
	position = 128685, 0, -197702
	alpha = 73
	beta = 2

[objects]
count = 33

object_id 0
	model = "area1_obj_0"
	use_lod = 1
	lod_sprite_index = 0
	lod_dist_factor = 80000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 182
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 1
	model = "area1_obj_1"
	use_lod = 1
	lod_sprite_index = 1
	lod_dist_factor = 150000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 284
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 2
	model = "area1_obj_2"
	use_lod = 1
	lod_sprite_index = 2
	lod_dist_factor = 80000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 264
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 100
	shadow_circle_intensity = 16

object_id 3
	model = "area1_obj_3"
	use_lod = 1
	lod_sprite_index = 3
	lod_dist_factor = 105000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 252
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 100
	shadow_circle_intensity = 16

object_id 4
	model = "area1_obj_4"
	use_lod = 1
	lod_sprite_index = 4
	lod_dist_factor = 172000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 414
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 100
	shadow_circle_intensity = 16

object_id 5
	model = "area1_obj_5"
	use_lod = 1
	lod_sprite_index = 5
	lod_dist_factor = 47000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -16
	bound_top = 212
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 100
	shadow_circle_intensity = 16

object_id 6
	model = "area1_obj_6"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -4
	bound_top = 346
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 7
	model = "area1_obj_7"
	use_lod = 1
	lod_sprite_index = 6
	lod_dist_factor = 200000
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 0
	bound_top = 1230
	shadow_line_length = 768
	shadow_line_intensity = 32
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 8
	model = "area1_obj_8"
	use_lod = 1
	lod_sprite_index = 7
	lod_dist_factor = 1.64257e+006
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 0
	bound_top = 1128
	shadow_line_length = 768
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 9
	model = "area1_obj_9"
	use_lod = 1
	lod_sprite_index = 8
	lod_dist_factor = 120000
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 316
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 10
	model = "area1_obj_10"
	use_lod = 1
	lod_sprite_index = 9
	lod_dist_factor = 360000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 348
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 11
	model = "area1_obj_11"
	use_lod = 1
	lod_sprite_index = 10
	lod_dist_factor = 2e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2400
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 12
	model = "area1_obj_12"
	use_lod = 1
	lod_sprite_index = 11
	lod_dist_factor = 4e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 2
	bound_top = 2808
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 384
	shadow_circle_intensity = 8

object_id 13
	model = "area1_obj_13"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 650
	bound_bottom = -98
	bound_top = 3388
	use_custom_light = 1
	custom_light_intensity = 128
	static_light = 1
	shadow_line_length = 0
	shadow_line_intensity = 32
	shadow_circle_radius = 700
	shadow_circle_intensity = 32

object_id 14
	model = "area1_obj_14"
	use_lod = 1
	lod_sprite_index = 12
	lod_dist_factor = 2.8e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2400
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 15
	model = "area1_obj_15"
	use_lod = 1
	lod_sprite_index = 13
	lod_dist_factor = 400000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -4
	bound_top = 432
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 256
	shadow_circle_intensity = 10

object_id 16
	model = "area1_obj_16"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 1200
	bound_bottom = -318
	bound_top = 642
	use_custom_light = 1
	custom_light_intensity = 128
	static_light = 1
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 17
	model = "area1_obj_17"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 320
	bound_bottom = -76
	bound_top = 184
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 18
	model = "area1_obj_18"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -88
	bound_top = 256
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 8
	use_custom_light = 1
	custom_light_intensity = 160

object_id 19
	model = "area1_obj_19"
	use_lod = 1
	lod_sprite_index = 14
	lod_dist_factor = 200000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -10
	bound_top = 662
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 20
	model = "area1_obj_20"
	use_lod = 1
	lod_sprite_index = 15
	lod_dist_factor = 600000
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -6
	bound_top = 62
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 21
	model = "area1_obj_21"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -118
	bound_top = -44
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 22
	model = "area1_obj_22"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -6
	bound_top = 946
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 23
	model = "area1_obj_23"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -6
	bound_top = 946
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 24
	model = "area1_obj_24"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 1024
	bound_bottom = 2598
	bound_top = 4772
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 25
	model = "area1_obj_25"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 856
	use_custom_light = 1
	custom_light_intensity = 128
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 26
	model = "area1_obj_26"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 856
	use_custom_light = 1
	custom_light_intensity = 128
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 27
	model = "area1_obj_27"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 856
	use_custom_light = 1
	custom_light_intensity = 128
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 28
	model = "area1_obj_28"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 856
	use_custom_light = 1
	custom_light_intensity = 128
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 29
	model = "area1_obj_29"
	use_lod = 1
	lod_sprite_index = 16
	lod_dist_factor = 8e+006
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 510
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 30
	model = "area1_obj_30"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 400
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 31
	model = "area1_obj_31"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -124
	bound_top = 312
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 32
	model = "area1_obj_32"
	use_lod = 1
	lod_sprite_index = 17
	lod_dist_factor = 160000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 252
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

[sounds]
count = 12

sound_id 0
	file = "area1_random_sound_0"

sound_id 1
	file = "area1_random_sound_1"

sound_id 2
	file = "area1_random_sound_2"

sound_id 3
	file = "area1_random_sound_3"

sound_id 4
	file = "area1_random_sound_4"

sound_id 5
	file = "area1_random_sound_5"

sound_id 6
	file = "area1_random_sound_6"

sound_id 7
	file = "area1_random_sound_7"

sound_id 8
	file = "area1_random_sound_8"

sound_id 9
	file = "area1_random_sound_9"

sound_id 10
	file = "area1_random_sound_10"

sound_id 11
	file = "area1_random_sound_11"

[ambients]
count = 14

ambient_id 0
	file = "area1_ambient_sound_0"
	random_sounds_count = 8

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.235294
		random_sound_frequency = 5
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 2
		random_sound_index = 2
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 3
		random_sound_index = 3
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 4
		random_sound_index = 4
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 5
		random_sound_index = 7
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 6
		random_sound_index = 10
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	random_sound_id 7
		random_sound_index = 11
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 1
	file = "area1_ambient_sound_1"
	random_sounds_count = 0
	volume = 1.00392

ambient_id 2
	file = "area1_ambient_sound_2"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 3
	file = "area1_ambient_sound_3"
	random_sounds_count = 4

	random_sound_id 0
		random_sound_index = 4
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 1
		random_sound_index = 0
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 2
		random_sound_index = 2
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 3
		random_sound_index = 11
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	volume = 0.313726

ambient_id 4
	file = "area1_ambient_sound_4"
	random_sounds_count = 4

	random_sound_id 0
		random_sound_index = 5
		random_sound_volume = 0.235294
		random_sound_frequency = 5
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 6
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 2
		random_sound_index = 7
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 3
		random_sound_index = 11
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 5
	file = "area1_ambient_sound_5"
	random_sounds_count = 7

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.235294
		random_sound_frequency = 3

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.235294
		random_sound_frequency = 4

	random_sound_id 2
		random_sound_index = 2
		random_sound_volume = 0.235294
		random_sound_frequency = 4

	random_sound_id 3
		random_sound_index = 3
		random_sound_volume = 0.235294
		random_sound_frequency = 4

	random_sound_id 4
		random_sound_index = 4
		random_sound_volume = 0.235294
		random_sound_frequency = 4

	random_sound_id 5
		random_sound_index = 7
		random_sound_volume = 0.235294
		random_sound_frequency = 4

	random_sound_id 6
		random_sound_index = 11
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 6
	file = "area1_ambient_sound_6"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 7
	file = "area1_ambient_sound_7"
	random_sounds_count = 3

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	random_sound_id 1
		random_sound_index = 2
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 11
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 8
	file = "area1_ambient_sound_8"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 9
	file = "area1_ambient_sound_9"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 10
	file = "area1_ambient_sound_10"
	random_sounds_count = 5

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.235294
		random_sound_frequency = 4

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.235294
		random_sound_frequency = 4

	random_sound_id 2
		random_sound_index = 2
		random_sound_volume = 0.235294
		random_sound_frequency = 4

	random_sound_id 3
		random_sound_index = 10
		random_sound_volume = 0.235294
		random_sound_frequency = 3

	random_sound_id 4
		random_sound_index = 11
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 11
	file = "area1_ambient_sound_11"
	random_sounds_count = 3

	random_sound_id 0
		random_sound_index = 8
		random_sound_volume = 0.235294
		random_sound_frequency = 5
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 9
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 2
		random_sound_index = 11
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 12
	file = "area1_ambient_sound_12"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 13
	file = "area1_ambient_sound_13"
	random_sounds_count = 0
	volume = 0.501961

[water]
count = 7

water_id 0
	tile_index = 0
	level = 60
	opacity = 1
	color = 0, 26, 23
	night_color = 0, 25, 40

water_id 1
	tile_index = 0
	level = 71
	opacity = 0.5
	color = 2, 41, 31

water_id 2
	tile_index = 1
	level = 65
	opacity = 1
	color = 25, 87, 75
	night_color = 11, 70, 60


water_id 3
	tile_index = 0
	level = 77
	opacity = 1

water_id 4
	tile_index = 1
	level = 77
	opacity = 1
	color = 56, 96, 67

water_id 5
	tile_index = 1
	level = 55
	opacity = 1

water_id 6
	tile_index = 2
	level = 72
	opacity = 1
	color = 56, 46, 7
	night_color = 38, 36, 27

[fog]
count = 7

fog_id 0
	color = 127, 127, 127
	altitude = 0
	poisonous = 0
	distance = 512
	density = 200

fog_id 1
	color = 79, 77, 56
	altitude = 80
	poisonous = 0
	distance = 200
	density = 200

fog_id 2
	color = 73, 52, 32
	altitude = 70
	poisonous = 0
	distance = 100
	density = 200

fog_id 3
	color = 79, 77, 56
	altitude = 70
	poisonous = 0
	distance = 200
	density = 200

fog_id 4
	color = 79, 77, 56
	altitude = 90
	poisonous = 0
	distance = 200
	density = 200

fog_id 5
	color = 79, 77, 56
	altitude = 90
	poisonous = 0
	distance = 200
	density = 200

fog_id 6
	color = 79, 77, 56
	altitude = 95
	poisonous = 0
	distance = 200
	density = 200

