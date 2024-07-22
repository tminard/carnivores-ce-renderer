carnivores_area_resources

[general]
detailmap_texture = "detailmap_default"

[sky]

use_sky_sphere = 1

dawn_sky_color = 113, 97, 74
dawn_sun_color = 200, 180, 120
dawn_shadows_color = 50, 20, 30
dawn_sky_sphere_texture = "car_sunset_grad3"
dawn_sky_plane_texture = "clear_sky_1"
dawn_sky_plane_opacity = 240
dawn_sky_plane_color = 96, 64, 64
dawn_clouds_shadows_texture = "area6_clouds"
dawn_clouds_speed_scale = 5
dawn_fog_start = 0.4
dawn_fog_end = 1

day_sky_color = 80, 112, 125
day_sun_color = 255, 238, 222
day_shadows_color = 50, 30, 30
day_sky_sphere_texture = "grad5"
day_sky_plane_texture = "storm_sky_3"
day_sky_plane_opacity = 48
day_sky_plane_color = 192, 170, 150
day_clouds_shadows_texture = "area6_clouds"
day_clouds_speed_scale = 2
day_fog_start = 0.7
day_fog_end = 1

night_sky_color = 21, 26, 40
night_sun_color = 32, 68, 104
night_shadows_color = 8, 8, 16
night_sky_sphere_texture = "car_night_grad2"
night_sky_plane_texture = "storm_sky_3"
night_sky_plane_opacity = 160
night_sky_plane_color = 21, 26, 48
night_clouds_shadows_texture = "area6_clouds"
night_clouds_speed_scale = 6
night_fog_start = 0
night_fog_end = 0.8

[tiles]
atlas_texture = "area4_tiles"
atlas_tiles_in_row = 16
atlas_tiles_in_column = 8
sprites_atlas_texture = "area4_sprites"
grass_atlas_texture = "area_grass_A"
grass_mask_texture = "area4_grass_mask"

[birthplaces]
count = 16

birthplace_id 0
	position = 79517, 0, -145817
	alpha = 169
	beta = 1

birthplace_id 1
	position = 62127, 0, -132401
	alpha = 230
	beta = 10

birthplace_id 2
	position = 36459, 0, -117973
	alpha = 274
	beta = 0

birthplace_id 3
	position = 44424, 0, -213458
	alpha = 210
	beta = 8

birthplace_id 4
	position = 92523, 0, -194804
	alpha = 174
	beta = 0

birthplace_id 5
	position = 200956, 0, -230646
	alpha = 206
	beta = 6

birthplace_id 6
	position = 177792, 0, -220967
	alpha = 229
	beta = 0

birthplace_id 7
	position = 240390, 0, -140745
	alpha = 240
	beta = -1

birthplace_id 8
	position = 220564, 0, -130499
	alpha = 229
	beta = 5

birthplace_id 9
	position = 159520, 0, -82049
	alpha = 229
	beta = 0

birthplace_id 10
	position = 139162, 0, -110644
	alpha = 252
	beta = 1

birthplace_id 11
	position = 109762, 0, -107131
	alpha = 254
	beta = 2

birthplace_id 12
	position = 70434, 0, -64454
	alpha = 186
	beta = -9

birthplace_id 13
	position = 54069, 0, -34665
	alpha = 231
	beta = 8

birthplace_id 14
	position = 39310, 0, -51590
	alpha = 221
	beta = 1

birthplace_id 15
	position = 57529, 0, -157440
	alpha = 246
	beta = 7

[objects]
count = 36

object_id 0
	model = "area4_obj_0"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 0
	bound_bottom = -1472
	bound_top = 5950
	static_light = 1
	use_custom_light = 1
	custom_light_intensity = 80
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 1000
	shadow_circle_intensity = 40

object_id 1
	model = "area4_obj_1"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 0
	bound_bottom = -1472
	bound_top = 5950
	static_light = 1
	use_custom_light = 1
	custom_light_intensity = 80
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 1000
	shadow_circle_intensity = 40

object_id 2
	model = "area4_obj_2"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2352
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 3
	model = "area4_obj_3"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2352
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 4
	model = "area4_obj_4"
	use_lod = 1
	lod_sprite_index = 0
	lod_dist_factor = 985138
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 0
	bound_top = 1554
	shadow_line_length = 1280
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 24

object_id 5
	model = "area4_obj_5"
	use_lod = 1
	lod_sprite_index = 1
	lod_dist_factor = 702897
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 0
	bound_top = 1484
	shadow_line_length = 1280
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 24

object_id 6
	model = "area4_obj_6"
	use_lod = 1
	lod_sprite_index = 2
	lod_dist_factor = 1.01035e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 4
	bound_top = 1218
	shadow_line_length = 800
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 24

object_id 7
	model = "area4_obj_7"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 0
	bound_bottom = -638
	bound_top = 690
	use_custom_light = 1
	custom_light_intensity = 180
	static_light = 1
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 0
	shadow_circle_intensity = 20

object_id 8
	model = "area4_obj_8"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 0
	bound_bottom = -272
	bound_top = -96
	use_custom_light = 1
	custom_light_intensity = 128
	static_light = 1
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 1000
	shadow_circle_intensity = 20

object_id 9
	model = "area4_obj_9"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 0
	bound_bottom = -26
	bound_top = 290
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 10
	model = "area4_obj_10"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 0
	bound_bottom = -26
	bound_top = 290
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 11
	model = "area4_obj_11"
	use_lod = 1
	lod_sprite_index = 3
	lod_dist_factor = 640000
	face_collision_detection = 1
	bound_radius = 0
	bound_bottom = -108
	bound_top = 506
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 24

object_id 12
	model = "area4_obj_12"
	use_lod = 1
	lod_sprite_index = 4
	lod_dist_factor = 1.78543e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -30
	bound_top = 1904
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 24

object_id 13
	model = "area4_obj_13"
	use_lod = 1
	lod_sprite_index = 5
	lod_dist_factor = 1.88741e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -90
	bound_top = 2012
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 24

object_id 14
	model = "area4_obj_14"
	use_lod = 1
	lod_sprite_index = 6
	lod_dist_factor = 3.73463e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2272
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 24

object_id 15
	model = "area4_obj_15"
	use_lod = 1
	lod_sprite_index = 7
	lod_dist_factor = 1.7967e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 0
	bound_top = 1232
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 24

object_id 16
	model = "area4_obj_16"
	use_lod = 1
	lod_sprite_index = 8
	lod_dist_factor = 800000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 516
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 17
	model = "area4_obj_17"
	use_lod = 1
	lod_sprite_index = 9
	lod_dist_factor = 800000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 516
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 18
	model = "area4_obj_18"
	use_lod = 1
	lod_sprite_index = 10
	lod_dist_factor = 800000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 534
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 19
	model = "area4_obj_19"
	use_lod = 1
	lod_sprite_index = 11
	lod_dist_factor = 600000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 262
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 80
	shadow_circle_intensity = 16

object_id 20
	model = "area4_obj_20"
	use_lod = 1
	lod_sprite_index = 12
	lod_dist_factor = 400000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 300
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 80
	shadow_circle_intensity = 16

object_id 21
	model = "area4_obj_21"
	use_lod = 1
	lod_sprite_index = 13
	lod_dist_factor = 200000
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = -28
	bound_top = 222
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 22
	model = "area4_obj_22"
	use_lod = 1
	lod_sprite_index = 14
	lod_dist_factor = 200000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 474
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 23
	model = "area4_obj_23"
	use_lod = 1
	lod_sprite_index = 15
	lod_dist_factor = 200000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 474
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 24
	model = "area4_obj_24"
	use_lod = 1
	lod_sprite_index = 16
	lod_dist_factor = 400000
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = -18
	bound_top = 266
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 25
	model = "area4_obj_25"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -36
	bound_top = 152
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 24

object_id 26
	model = "area4_obj_26"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = -76
	bound_top = 110
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 24

object_id 27
	model = "area4_obj_27"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 320
	bound_bottom = -160
	bound_top = 204
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 24

object_id 28
	model = "area4_obj_28"
	use_lod = 1
	lod_sprite_index = 17
	lod_dist_factor = 200000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 182
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 29
	model = "area4_obj_29"
	use_lod = 1
	lod_sprite_index = 18
	lod_dist_factor = 80000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 194
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 30
	model = "area4_obj_30"
	use_lod = 1
	lod_sprite_index = 19
	lod_dist_factor = 80000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 264
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 31
	model = "area4_obj_31"
	use_lod = 1
	lod_sprite_index = 20
	lod_dist_factor = 120000
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

object_id 32
	model = "area4_obj_32"
	use_lod = 1
	lod_sprite_index = 21
	lod_dist_factor = 47110.2
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -16
	bound_top = 212
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 33
	model = "area4_obj_33"
	use_lod = 1
	lod_sprite_index = 22
	lod_dist_factor = 172021
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 414
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16

object_id 34
	model = "area4_obj_34"
	use_lod = 0
	transparent = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -42
	bound_top = 396
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 35
	model = "area4_obj_35"
	use_lod = 0
	transparent = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -42
	bound_top = 420
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

[sounds]
count = 35

sound_id 0
	file = "area4_random_sound_0"

sound_id 1
	file = "area4_random_sound_1"

sound_id 2
	file = "area4_random_sound_2"

sound_id 3
	file = "area4_random_sound_3"

sound_id 4
	file = "area4_random_sound_4"

sound_id 5
	file = "area4_random_sound_5"

sound_id 6
	file = "area4_random_sound_6"

sound_id 7
	file = "area4_random_sound_7"

sound_id 8
	file = "area4_random_sound_8"

sound_id 9
	file = "area4_random_sound_9"

sound_id 10
	file = "area4_random_sound_10"

sound_id 11
	file = "area4_random_sound_11"

sound_id 12
	file = "area4_random_sound_12"

sound_id 13
	file = "area4_random_sound_13"

sound_id 14
	file = "area4_random_sound_14"

sound_id 15
	file = "area4_random_sound_15"

sound_id 16
	file = "area4_random_sound_16"

sound_id 17
	file = "area4_random_sound_17"

sound_id 18
	file = "area4_random_sound_18"

sound_id 19
	file = "area4_random_sound_19"

sound_id 20
	file = "area4_random_sound_20"

sound_id 21
	file = "area4_random_sound_21"

sound_id 22
	file = "area4_random_sound_22"

sound_id 23
	file = "area4_random_sound_23"

sound_id 24
	file = "area4_random_sound_24"

sound_id 25
	file = "area4_random_sound_25"

sound_id 26
	file = "area4_random_sound_26"

sound_id 27
	file = "area4_random_sound_27"

sound_id 28
	file = "area4_random_sound_28"

sound_id 29
	file = "area4_random_sound_29"

sound_id 30
	file = "area4_random_sound_30"

sound_id 31
	file = "area4_random_sound_31"

sound_id 32
	file = "area4_random_sound_32"

sound_id 33
	file = "area4_random_sound_33"

sound_id 34
	file = "area4_random_sound_34"

[ambients]
count = 6

ambient_id 0
	file = "area4_ambient_sound_0"
	random_sounds_count = 16

	random_sound_id 0
		random_sound_index = 6
		random_sound_volume = 0.501961
		random_sound_frequency = 2
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 7
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 8
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 3
		random_sound_index = 10
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 11
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 5
		random_sound_index = 12
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 6
		random_sound_index = 16
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 7
		random_sound_index = 17
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 8
		random_sound_index = 18
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 9
		random_sound_index = 19
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 10
		random_sound_index = 20
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 11
		random_sound_index = 21
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 12
		random_sound_index = 23
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 13
		random_sound_index = 24
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 14
		random_sound_index = 25
		random_sound_volume = 0.501961
		random_sound_frequency = 30
		random_sound_daytime_only = 1

	random_sound_id 15
		random_sound_index = 26
		random_sound_volume = 0.501961
		random_sound_frequency = 30
		random_sound_daytime_only = 1

	volume = 0.501961

ambient_id 1
	file = "area4_ambient_sound_1"
	random_sounds_count = 16

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.501961
		random_sound_frequency = 2
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 2
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 3
		random_sound_index = 3
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 4
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 5
		random_sound_index = 5
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 6
		random_sound_index = 6
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 7
		random_sound_index = 7
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 8
		random_sound_index = 12
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 9
		random_sound_index = 18
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 10
		random_sound_index = 19
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 11
		random_sound_index = 20
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 12
		random_sound_index = 21
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 13
		random_sound_index = 24
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 14
		random_sound_index = 25
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 15
		random_sound_index = 32
		random_sound_volume = 0.501961
		random_sound_frequency = 30
		random_sound_daytime_only = 1

	volume = 0.501961

ambient_id 2
	file = "area4_ambient_sound_2"
	random_sounds_count = 5

	random_sound_id 0
		random_sound_index = 31
		random_sound_volume = 0.501961
		random_sound_frequency = 2
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 13
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 14
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 3
		random_sound_index = 33
		random_sound_volume = 0.313726
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 34
		random_sound_volume = 0.313726
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 3
	file = "area4_ambient_sound_3"
	random_sounds_count = 16

	random_sound_id 0
		random_sound_index = 30
		random_sound_volume = 0.501961
		random_sound_frequency = 2

	random_sound_id 1
		random_sound_index = 29
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 28
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 3
		random_sound_index = 27
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 25
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 5
		random_sound_index = 24
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 6
		random_sound_index = 6
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 7
		random_sound_index = 7
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 8
		random_sound_index = 8
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 9
		random_sound_index = 9
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 10
		random_sound_index = 10
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 11
		random_sound_index = 11
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 12
		random_sound_index = 12
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 13
		random_sound_index = 16
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 14
		random_sound_index = 17
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 15
		random_sound_index = 18
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 4
	file = "area4_ambient_sound_4"
	random_sounds_count = 4

	random_sound_id 0
		random_sound_index = 27
		random_sound_volume = 0.501961
		random_sound_frequency = 5

	random_sound_id 1
		random_sound_index = 28
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 29
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 3
		random_sound_index = 30
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 5
	file = "area4_ambient_sound_5"
	random_sounds_count = 16

	random_sound_id 0
		random_sound_index = 17
		random_sound_volume = 0.501961
		random_sound_frequency = 2
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 6
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 7
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 3
		random_sound_index = 8
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 9
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 5
		random_sound_index = 10
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 6
		random_sound_index = 11
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 7
		random_sound_index = 12
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 8
		random_sound_index = 16
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 9
		random_sound_index = 18
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 10
		random_sound_index = 20
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 11
		random_sound_index = 21
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 12
		random_sound_index = 23
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 13
		random_sound_index = 24
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 14
		random_sound_index = 25
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 15
		random_sound_index = 26
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.501961

[water]
count = 4

water_id 0
	tile_index = 4
	level = 40
	opacity = 1
	night_color = 11, 30, 40

water_id 1
	tile_index = 4
	level = 35
	opacity = 1

water_id 2
	tile_index = 4
	level = 32
	opacity = 1

water_id 3
	tile_index = 46
	level = 55
	opacity = 2

[fog]
count = 4

fog_id 0
	color = 127, 127, 127
	altitude = 0
	poisonous = 0
	distance = 512
	density = 200

fog_id 1
	color = 74, 99, 115
	altitude = 45
	poisonous = 0
	distance = 1024
	density = 128

fog_id 2
	color = 102, 106, 111
	altitude = 8
	poisonous = 0
	distance = 1024
	density = 128

fog_id 3
	color = 99, 99, 90
	altitude = 24
	poisonous = 0
	distance = 512
	density = 128

