carnivores_area_resources

[general]
detailmap_texture = "detailmap_default"

[sky]

use_sky_sphere = 1

dawn_sky_color = 119, 90, 82
dawn_sun_color = 250, 230, 170
dawn_shadows_color = 30, 20, 30
dawn_sky_sphere_texture = "sunset_grad_3a"
dawn_sky_plane_texture = "storm_sky_3"
dawn_sky_plane_opacity = 80
dawn_sky_plane_color = 48, 20, 30
dawn_clouds_shadows_texture = "area6_clouds"
dawn_clouds_speed_scale = 1
dawn_fog_start = 0.3
dawn_fog_end = 1

day_sky_color = 68, 87, 112
day_sun_color = 255, 251, 231
day_shadows_color = 30, 50, 80
day_sky_sphere_texture = "grad5"
day_sky_plane_texture = "clear_sky_1"
day_sky_plane_opacity = 160
day_sky_plane_color = 200, 200, 240
day_clouds_shadows_texture = "area6_clouds"
day_clouds_speed_scale = 3
day_fog_start = 0.3
day_fog_end = 1

night_sky_color = 21, 26, 38
night_sun_color = 32, 68, 104
night_shadows_color = 16, 24, 82
night_sky_sphere_texture = "car_night_grad1"
night_sky_plane_texture = "storm_sky_3"
night_sky_plane_opacity = 190
night_sky_plane_color = 21, 26, 48
night_clouds_shadows_texture = "area6_clouds"
night_clouds_speed_scale = 6
night_fog_start = 0
night_fog_end = 0.8

[tiles]
atlas_texture = "area3_tiles"
atlas_tiles_in_row = 8
atlas_tiles_in_column = 8
sprites_atlas_texture = "area3_sprites"
grass_atlas_texture = "area_grass_A"
grass_mask_texture = "area3_grass_mask"

[birthplaces]
count = 16

birthplace_id 0
	position = 20102, 0, -127188
	alpha = 165
	beta = 6

birthplace_id 1
	position = 153453, 0, -128166
	alpha = 263
	beta = 9

birthplace_id 2
	position = 189269, 0, -68856
	alpha = 160
	beta = 6

birthplace_id 3
	position = 90035, 0, -23612
	alpha = 268
	beta = 9

birthplace_id 4
	position = 205150, 0, -54048
	alpha = 169
	beta = 6

birthplace_id 5
	position = 246250, 0, -68731
	alpha = 302
	beta = 6

birthplace_id 6
	position = 212599, 0, -126381
	alpha = 195
	beta = 9

birthplace_id 7
	position = 168259, 0, -221333
	alpha = 254
	beta = 12

birthplace_id 8
	position = 113452, 0, -231968
	alpha = 236
	beta = 6

birthplace_id 9
	position = 76211, 0, -227661
	alpha = 250
	beta = -5

birthplace_id 10
	position = 83573, 0, -188800
	alpha = 154
	beta = 10

birthplace_id 11
	position = 14663, 0, -172962
	alpha = 178
	beta = 12

birthplace_id 12
	position = 62773, 0, -43962
	alpha = 181
	beta = 4

birthplace_id 13
	position = 30203, 0, -41914
	alpha = 228
	beta = 0

birthplace_id 14
	position = 77805, 0, -29212
	alpha = 140
	beta = -1

birthplace_id 15
	position = 121660, 0, -51683
	alpha = 151
	beta = 5

[objects]
count = 21

object_id 0
	model = "area3_obj_0"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 840
	bound_bottom = -52
	bound_top = 3406
	use_custom_light = 1
	custom_light_intensity = 164
	static_light = 1
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 512
	shadow_circle_intensity = 16

object_id 1
	model = "area3_obj_1"
	use_lod = 0
	transparent = 0
	radial_collision_detection = 1
	bound_radius = 760
	bound_bottom = -56
	bound_top = 3416
	use_custom_light = 1
	custom_light_intensity = 164
	static_light = 1
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 2
	model = "area3_obj_2"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -182
	bound_top = 288
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 3
	model = "area3_obj_3"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -638
	bound_top = 690
	use_custom_light = 1
	custom_light_intensity = 164
	static_light = 1
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 512
	shadow_circle_intensity = 32

object_id 4
	model = "area3_obj_4"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -272
	bound_top = -96
	use_custom_light = 1
	custom_light_intensity = 96
	static_light = 1
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 1024
	shadow_circle_intensity = 48

object_id 5
	model = "area3_obj_5"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -158
	bound_top = 2740
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 6
	model = "area3_obj_6"
	use_lod = 1
	lod_sprite_index = 0
	lod_dist_factor = 400000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -4
	bound_top = 432
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 7
	model = "area3_obj_7"
	use_lod = 1
	lod_sprite_index = 1
	lod_dist_factor = 160000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 252
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 8
	model = "area3_obj_8"
	use_lod = 1
	lod_sprite_index = 2
	lod_dist_factor = 100000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 252
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 9
	model = "area3_obj_9"
	use_lod = 1
	lod_sprite_index = 3
	lod_dist_factor = 240000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 230
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 10
	model = "area3_obj_10"
	use_lod = 1
	lod_sprite_index = 4
	lod_dist_factor = 1.7967e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 1232
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 11
	model = "area3_obj_11"
	use_lod = 1
	lod_sprite_index = 5
	lod_dist_factor = 60000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -16
	bound_top = 212
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 12
	model = "area3_obj_12"
	use_lod = 1
	lod_sprite_index = 6
	lod_dist_factor = 500000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 644
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 13
	model = "area3_obj_13"
	use_lod = 1
	lod_sprite_index = 7
	lod_dist_factor = 2.4e+006
	radial_collision_detection = 1
	bound_radius = 180
	bound_bottom = 0
	bound_top = 450
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 10

object_id 14
	model = "area3_obj_14"
	use_lod = 1
	lod_sprite_index = 8
	lod_dist_factor = 2.4e+006
	radial_collision_detection = 1
	bound_radius = 180
	bound_bottom = 0
	bound_top = 376
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 12

object_id 15
	model = "area3_obj_15"
	use_lod = 1
	lod_sprite_index = 9
	lod_dist_factor = 800000
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = -6
	bound_top = 714
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 12

object_id 16
	model = "area3_obj_16"
	use_lod = 1
	lod_sprite_index = 10
	lod_dist_factor = 320000
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -10
	bound_top = 168
	static_light = 1
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 17
	model = "area3_obj_17"
	use_lod = 1
	lod_sprite_index = 11
	lod_dist_factor = 3.41591e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 180
	bound_bottom = 0
	bound_top = 2002
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 18
	model = "area3_obj_18"
	use_lod = 1
	lod_sprite_index = 12
	lod_dist_factor = 5e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 180
	bound_bottom = 0
	bound_top = 2002
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 19
	model = "area3_obj_19"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -26
	bound_top = 290
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 20
	model = "area3_obj_20"
	use_lod = 1
	lod_sprite_index = 13
	lod_dist_factor = 400000
	transparent = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -108
	bound_top = 506
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

[sounds]
count = 12

sound_id 0
	file = "area3_random_sound_0"

sound_id 1
	file = "area3_random_sound_1"

sound_id 2
	file = "area3_random_sound_2"

sound_id 3
	file = "area3_random_sound_3"

sound_id 4
	file = "area3_random_sound_4"

sound_id 5
	file = "area3_random_sound_5"

sound_id 6
	file = "area3_random_sound_6"

sound_id 7
	file = "area3_random_sound_7"

sound_id 8
	file = "area3_random_sound_8"

sound_id 9
	file = "area3_random_sound_9"

sound_id 10
	file = "area3_random_sound_10"

sound_id 11
	file = "area3_random_sound_11"

[ambients]
count = 15

ambient_id 0
	file = "area3_ambient_sound_0"
	random_sounds_count = 9

	random_sound_id 0
		random_sound_index = 2
		random_sound_volume = 0.235294
		random_sound_frequency = 5
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 4
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 2
		random_sound_index = 5
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 3
		random_sound_index = 6
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 4
		random_sound_index = 7
		random_sound_volume = 0.235294
		random_sound_frequency = 9

	random_sound_id 5
		random_sound_index = 8
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 6
		random_sound_index = 9
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 7
		random_sound_index = 10
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 8
		random_sound_index = 11
		random_sound_volume = 0.235294
		random_sound_frequency = 9

	volume = 0.501961

ambient_id 1
	file = "area3_ambient_sound_1"
	random_sounds_count = 4

	random_sound_id 0
		random_sound_index = 2
		random_sound_volume = 0.235294
		random_sound_frequency = 5
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 3
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 2
		random_sound_index = 4
		random_sound_volume = 0.235294
		random_sound_frequency = 10

	random_sound_id 3
		random_sound_index = 6
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	volume = 0.501961

ambient_id 2
	file = "area3_ambient_sound_2"
	random_sounds_count = 2

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	volume = 0.501961

ambient_id 3
	file = "area3_ambient_sound_3"
	random_sounds_count = 2

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	volume = 0.501961

ambient_id 4
	file = "area3_ambient_sound_4"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 5
	file = "area3_ambient_sound_5"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 6
	file = "area3_ambient_sound_6"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 7
	file = "area3_ambient_sound_7"
	random_sounds_count = 2

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.235294
		random_sound_frequency = 5

	volume = 0.501961

ambient_id 8
	file = "area3_ambient_sound_8"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 9
	file = "area3_ambient_sound_9"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 10
	file = "area3_ambient_sound_10"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 11
	file = "area3_ambient_sound_11"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 12
	file = "area3_ambient_sound_12"
	random_sounds_count = 0
	volume = 0.313726

ambient_id 13
	file = "area3_ambient_sound_13"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 14
	file = "area3_ambient_sound_14"
	random_sounds_count = 0
	volume = 0.501961

[water]
count = 5

water_id 0
	tile_index = 0
	level = 50
	opacity = 0.9
	color = 0, 26, 23
	night_color = 11, 20, 40

water_id 1
	tile_index = 0
	level = 52
	opacity = 0.2

water_id 2
	tile_index = 0
	level = 80
	opacity = 0.7

water_id 3
	tile_index = 0
	level = 175
	opacity = 0.2

water_id 4
	tile_index = 0
	level = 28
	opacity = 0.7

[fog]
count = 3

fog_id 0
	color = 127, 127, 127
	altitude = 0
	poisonous = 0
	distance = 512
	density = 200

fog_id 1
	color = 84, 62, 43
	altitude = 50
	poisonous = 0
	distance = 512
	density = 128

fog_id 2
	color = 54, 88, 94
	altitude = 60
	poisonous = 0
	distance = 512
	density = 200

