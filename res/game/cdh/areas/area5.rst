carnivores_area_resources

[general]
detailmap_texture = "detailmap_default"

[sky]

use_sky_sphere = 1

dawn_sky_color = 123, 97, 74
dawn_sun_color = 250, 230, 170
dawn_shadows_color = 50, 20, 30
dawn_sky_sphere_texture = "car_sunset_grad2"
dawn_sky_plane_texture = "car_sunset_sky"
dawn_sky_plane_opacity = 240
dawn_sky_plane_color = 64, 64, 96
dawn_clouds_shadows_texture = "area5_clouds"
dawn_clouds_speed_scale = 2
dawn_fog_start = 0.4
dawn_fog_end = 1

day_sky_color = 99, 116, 140
day_sun_color = 225, 208, 202
day_shadows_color = 20, 30, 50
day_sky_sphere_texture = "grad5"
day_sky_plane_texture = "clear_sky_1"
day_sky_plane_opacity = 230
day_sky_plane_color = 150, 150, 150
day_clouds_shadows_texture = "area5_clouds"
day_clouds_speed_scale = 2
day_fog_start = 0.7
day_fog_end = 1

night_sky_color = 20, 26, 38
night_sun_color = 32, 68, 104
night_shadows_color = 22, 38, 82
night_sky_sphere_texture = "car_night_grad2"
night_sky_plane_texture = "storm_sky_3"
night_sky_plane_opacity = 180
night_sky_plane_color = 31, 36, 52
night_clouds_shadows_texture = "area5_clouds"
night_clouds_speed_scale = 4
night_fog_start = 0
night_fog_end = 0.8

[tiles]
atlas_texture = "area5_tiles"
atlas_tiles_in_row = 16
atlas_tiles_in_column = 8
sprites_atlas_texture = "area5_sprites"
grass_atlas_texture = "area_grass_B"
grass_mask_texture = "area5_grass_mask"

[birthplaces]
count = 16

birthplace_id 0
	position = 50292, 0, -38606
	alpha = 13
	beta = 9

birthplace_id 1
	position = 84907, 0, -41666
	alpha = 346
	beta = -2

birthplace_id 2
	position = 138352, 0, -65331
	alpha = 136
	beta = 2

birthplace_id 3
	position = 194632, 0, -70591
	alpha = 19
	beta = -7

birthplace_id 4
	position = 217468, 0, -72832
	alpha = 296
	beta = 9

birthplace_id 5
	position = 218426, 0, -123961
	alpha = 280
	beta = -9

birthplace_id 6
	position = 201604, 0, -177669
	alpha = 356
	beta = -9

birthplace_id 7
	position = 187300, 0, -195907
	alpha = 325
	beta = 14

birthplace_id 8
	position = 186967, 0, -229923
	alpha = 65
	beta = 1

birthplace_id 9
	position = 169731, 0, -224433
	alpha = 261
	beta = -5

birthplace_id 10
	position = 140399, 0, -218627
	alpha = 256
	beta = -9

birthplace_id 11
	position = 123087, 0, -197591
	alpha = 227
	beta = -5

birthplace_id 12
	position = 68108, 0, -197968
	alpha = 299
	beta = 4

birthplace_id 13
	position = 34233, 0, -229070
	alpha = 194
	beta = 5

birthplace_id 14
	position = 162884, 0, -194190
	alpha = 272
	beta = 0

birthplace_id 15
	position = 149601, 0, -106587
	alpha = 178
	beta = 7

[objects]
count = 24

object_id 0
	model = "area5_obj_0"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 128
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 1
	model = "area5_obj_1"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 120
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 2
	model = "area5_obj_2"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 192
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 3
	model = "area5_obj_3"
	use_lod = 1
	lod_sprite_index = 0
	lod_dist_factor = 400000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 300
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 4
	model = "area5_obj_4"
	use_lod = 1
	lod_sprite_index = 1
	lod_dist_factor = 120000
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

object_id 5
	model = "area5_obj_5"
	use_lod = 1
	lod_sprite_index = 2
	lod_dist_factor = 240000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 414
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 6
	model = "area5_obj_6"
	use_lod = 1
	lod_sprite_index = 3
	lod_dist_factor = 2.4e+006
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 344
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 32

object_id 7
	model = "area5_obj_7"
	use_lod = 1
	lod_sprite_index = 4
	lod_dist_factor = 2.4e+006
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 356
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 32

object_id 8
	model = "area5_obj_8"
	use_lod = 1
	lod_sprite_index = 5
	lod_dist_factor = 400000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -16
	bound_top = 518
	shadow_line_length = 0
	shadow_line_intensity = 32
	shadow_circle_radius = 128
	shadow_circle_intensity = 32

object_id 9
	model = "area5_obj_9"
	use_lod = 1
	lod_sprite_index = 6
	lod_dist_factor = 985138
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 0
	bound_top = 1554
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 10
	model = "area5_obj_10"
	use_lod = 1
	lod_sprite_index = 7
	lod_dist_factor = 3.4876e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2002
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 11
	model = "area5_obj_11"
	use_lod = 1
	lod_sprite_index = 8
	lod_dist_factor = 1.2e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 0
	bound_top = 1484
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 12
	model = "area5_obj_12"
	use_lod = 1
	lod_sprite_index = 9
	lod_dist_factor = 4e+006
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -26
	bound_top = 290
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 13
	model = "area5_obj_13"
	use_lod = 1
	lod_sprite_index = 10
	lod_dist_factor = 400000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -28
	bound_top = 222
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 14
	model = "area5_obj_14"
	use_lod = 1
	lod_sprite_index = 11
	lod_dist_factor = 400000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -18
	bound_top = 266
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 15
	model = "area5_obj_15"
	use_lod = 1
	lod_sprite_index = 12
	lod_dist_factor = 400000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -28
	bound_top = 222
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 16
	model = "area5_obj_16"
	use_lod = 1
	lod_sprite_index = 13
	lod_dist_factor = 1.2e+006
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -108
	bound_top = 506
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 17
	model = "area5_obj_17"
	use_lod = 1
	lod_sprite_index = 14
	lod_dist_factor = 400000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 474
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 18
	model = "area5_obj_18"
	use_lod = 1
	lod_sprite_index = 15
	lod_dist_factor = 400000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 474
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 19
	model = "area5_obj_19"
	use_lod = 1
	lod_sprite_index = 16
	lod_dist_factor = 400000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 474
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 20
	model = "area5_obj_20"
	use_lod = 1
	lod_sprite_index = 17
	lod_dist_factor = 1.2e+006
	transparent = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -160
	bound_top = 204
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 21
	model = "area5_obj_21"
	use_lod = 1
	lod_sprite_index = 18
	lod_dist_factor = 400000
	transparent = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -76
	bound_top = 110
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 22
	model = "area5_obj_22"
	use_lod = 1
	lod_sprite_index = 19
	lod_dist_factor = 400000
	transparent = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -36
	bound_top = 152
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 23
	model = "area5_obj_23"
	use_lod = 1
	lod_sprite_index = 20
	lod_dist_factor = 1.01035e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 4
	bound_top = 1218
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

[sounds]
count = 33

sound_id 0
	file = "area5_random_sound_0"

sound_id 1
	file = "area5_random_sound_1"

sound_id 2
	file = "area5_random_sound_2"

sound_id 3
	file = "area5_random_sound_3"

sound_id 4
	file = "area5_random_sound_4"

sound_id 5
	file = "area5_random_sound_5"

sound_id 6
	file = "area5_random_sound_6"

sound_id 7
	file = "area5_random_sound_7"

sound_id 8
	file = "area5_random_sound_8"

sound_id 9
	file = "area5_random_sound_9"

sound_id 10
	file = "area5_random_sound_10"

sound_id 11
	file = "area5_random_sound_11"

sound_id 12
	file = "area5_random_sound_12"

sound_id 13
	file = "area5_random_sound_13"

sound_id 14
	file = "area5_random_sound_14"

sound_id 15
	file = "area5_random_sound_15"

sound_id 16
	file = "area5_random_sound_16"

sound_id 17
	file = "area5_random_sound_17"

sound_id 18
	file = "area5_random_sound_18"

sound_id 19
	file = "area5_random_sound_19"

sound_id 20
	file = "area5_random_sound_20"

sound_id 21
	file = "area5_random_sound_21"

sound_id 22
	file = "area5_random_sound_22"

sound_id 23
	file = "area5_random_sound_23"

sound_id 24
	file = "area5_random_sound_24"

sound_id 25
	file = "area5_random_sound_25"

sound_id 26
	file = "area5_random_sound_26"

sound_id 27
	file = "area5_random_sound_27"

sound_id 28
	file = "area5_random_sound_28"

sound_id 29
	file = "area5_random_sound_29"

sound_id 30
	file = "area5_random_sound_30"

sound_id 31
	file = "area5_random_sound_31"

sound_id 32
	file = "area5_random_sound_32"

[ambients]
count = 10

ambient_id 0
	file = "area5_ambient_sound_0"
	random_sounds_count = 16

	random_sound_id 0
		random_sound_index = 8
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 1
		random_sound_index = 9
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 2
		random_sound_index = 10
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 3
		random_sound_index = 11
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 4
		random_sound_index = 13
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 5
		random_sound_index = 14
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 6
		random_sound_index = 15
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 7
		random_sound_index = 16
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 8
		random_sound_index = 22
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 9
		random_sound_index = 23
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 10
		random_sound_index = 24
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 11
		random_sound_index = 25
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 12
		random_sound_index = 26
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 13
		random_sound_index = 17
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 14
		random_sound_index = 18
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 15
		random_sound_index = 19
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	volume = 0.25098

ambient_id 1
	file = "area5_ambient_sound_1"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 2
	file = "area5_ambient_sound_2"
	random_sounds_count = 14

	random_sound_id 0
		random_sound_index = 22
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 1
		random_sound_index = 23
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 2
		random_sound_index = 24
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 3
		random_sound_index = 25
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 4
		random_sound_index = 26
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 5
		random_sound_index = 27
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 6
		random_sound_index = 28
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 7
		random_sound_index = 17
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 8
		random_sound_index = 18
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 9
		random_sound_index = 19
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 10
		random_sound_index = 20
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 11
		random_sound_index = 8
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 12
		random_sound_index = 9
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 13
		random_sound_index = 10
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	volume = 0.501961

ambient_id 3
	file = "area5_ambient_sound_3"
	random_sounds_count = 11

	random_sound_id 0
		random_sound_index = 22
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 1
		random_sound_index = 23
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 2
		random_sound_index = 24
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 3
		random_sound_index = 25
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 4
		random_sound_index = 26
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 5
		random_sound_index = 27
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 6
		random_sound_index = 28
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 7
		random_sound_index = 9
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 8
		random_sound_index = 10
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 9
		random_sound_index = 8
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 10
		random_sound_index = 15
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	volume = 0.501961

ambient_id 4
	file = "area5_ambient_sound_4"
	random_sounds_count = 10

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 2
		random_sound_index = 2
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 3
		random_sound_index = 22
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 4
		random_sound_index = 23
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 5
		random_sound_index = 24
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 6
		random_sound_index = 25
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 7
		random_sound_index = 26
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 8
		random_sound_index = 27
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 9
		random_sound_index = 28
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	volume = 0.501961

ambient_id 5
	file = "area5_ambient_sound_5"
	random_sounds_count = 16

	random_sound_id 0
		random_sound_index = 22
		random_sound_volume = 0.25098
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 23
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 2
		random_sound_index = 24
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 3
		random_sound_index = 25
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 4
		random_sound_index = 26
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 5
		random_sound_index = 9
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 6
		random_sound_index = 10
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 7
		random_sound_index = 11
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 8
		random_sound_index = 13
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 9
		random_sound_index = 14
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 10
		random_sound_index = 16
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 11
		random_sound_index = 17
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 12
		random_sound_index = 18
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 13
		random_sound_index = 20
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 14
		random_sound_index = 19
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	random_sound_id 15
		random_sound_index = 27
		random_sound_volume = 0.25098
		random_sound_frequency = 3

	volume = 0.501961

ambient_id 6
	file = "area5_ambient_sound_6"
	random_sounds_count = 2

	random_sound_id 0
		random_sound_index = 1
		random_sound_volume = 0.25098
		random_sound_frequency = 10

	random_sound_id 1
		random_sound_index = 2
		random_sound_volume = 0.25098
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 7
	file = "area5_ambient_sound_7"
	random_sounds_count = 7

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.25098
		random_sound_frequency = 10

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.25098
		random_sound_frequency = 10

	random_sound_id 2
		random_sound_index = 22
		random_sound_volume = 0.25098
		random_sound_frequency = 10

	random_sound_id 3
		random_sound_index = 23
		random_sound_volume = 0.25098
		random_sound_frequency = 10

	random_sound_id 4
		random_sound_index = 24
		random_sound_volume = 0.25098
		random_sound_frequency = 10

	random_sound_id 5
		random_sound_index = 25
		random_sound_volume = 0.25098
		random_sound_frequency = 10

	random_sound_id 6
		random_sound_index = 26
		random_sound_volume = 0.25098
		random_sound_frequency = 10

	volume = 0.501961

ambient_id 8
	file = "area5_ambient_sound_8"
	random_sounds_count = 13

	random_sound_id 0
		random_sound_index = 3
		random_sound_volume = 0.25098
		random_sound_frequency = 5
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 4
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 2
		random_sound_index = 5
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 3
		random_sound_index = 6
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 4
		random_sound_index = 7
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 5
		random_sound_index = 22
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 6
		random_sound_index = 23
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 7
		random_sound_index = 24
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 8
		random_sound_index = 25
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 9
		random_sound_index = 26
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 10
		random_sound_index = 27
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 11
		random_sound_index = 28
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	random_sound_id 12
		random_sound_index = 29
		random_sound_volume = 0.25098
		random_sound_frequency = 5

	volume = 0.501961

ambient_id 9
	file = "area5_ambient_sound_9"
	random_sounds_count = 0
	volume = 0.501961

[water]
count = 8

water_id 0
	tile_index = 0
	level = 60
	opacity = 1


water_id 1
	tile_index = 7
	level = 80
	opacity = 1
	color = 6, 24, 28
	night_color = 0, 25, 40

water_id 2
	tile_index = 0
	level = 106
	opacity = 1

water_id 3
	tile_index = 2
	level = 94
	opacity = 2

water_id 4
	tile_index = 2
	level = 68
	opacity = 0

water_id 5
	tile_index = 2
	level = 49
	opacity = 1

water_id 6
	tile_index = 2
	level = 108
	opacity = 1

water_id 7
	tile_index = 7
	level = 75
	opacity = 1

[fog]
count = 2

fog_id 0
	color = 125, 125, 111
	altitude = 60
	poisonous = 0
	distance = 512
	density = 200

fog_id 1
	color = 192, 77, 23
	altitude = 60
	poisonous = 1
	distance = 512
	density = 200

