carnivores_area_resources

[general]
detailmap_texture = "detailmap_default"

[sky]

use_sky_sphere = 1

dawn_sky_color = 136, 120, 104
dawn_sun_color = 255, 229, 204
dawn_shadows_color = 30, 20, 30
dawn_sky_sphere_texture = "car_sunset_grad2"
dawn_sky_plane_texture = "storm_sky_3"
dawn_sky_plane_opacity = 150
dawn_sky_plane_color = 255, 140, 110
dawn_clouds_shadows_texture = "car_clouds2"
dawn_clouds_speed_scale = 2
dawn_fog_start = 0.65
dawn_fog_end = 1

day_sky_color = 104, 125, 136
day_sun_color = 255, 229, 204
day_shadows_color = 50, 50, 30
day_sky_sphere_texture = "grad5"
day_sky_plane_texture = "storm_sky_3"
day_sky_plane_opacity = 190
day_sky_plane_color = 192, 170, 150
day_clouds_shadows_texture = "car_clouds2"
day_clouds_speed_scale = 2
day_fog_start = 0.4
day_fog_end = 1

night_sky_color = 21, 26, 38
night_sun_color = 32, 68, 104
night_shadows_color = 16, 24, 68
night_sky_sphere_texture = "car_night_grad1"
night_sky_plane_texture = "clear_sky_1"
night_sky_plane_opacity = 128
night_sky_plane_color = 21, 26, 44
night_clouds_shadows_texture = "car_clouds2"
night_clouds_speed_scale = 4
night_fog_start = 0
night_fog_end = 0.8

[tiles]
atlas_texture = "area2_tiles"
atlas_tiles_in_row = 16
atlas_tiles_in_column = 8
sprites_atlas_texture = "area2_sprites"
grass_atlas_texture = "area_grass_B"
grass_mask_texture = "area2_grass_mask"

[birthplaces]
count = 16

birthplace_id 0
	position = 167974, 0, -124745
	alpha = 353
	beta = 0

birthplace_id 1
	position = 164423, 0, -135760
	alpha = 322
	beta = 4

birthplace_id 2
	position = 153658, 0, -170345
	alpha = 97
	beta = 10

birthplace_id 3
	position = 140776, 0, -188185
	alpha = 305
	beta = 0

birthplace_id 4
	position = 93886, 0, -205114
	alpha = 28
	beta = 8

birthplace_id 5
	position = 116376, 0, -219775
	alpha = 146
	beta = 0

birthplace_id 6
	position = 180476, 0, -209362
	alpha = 99
	beta = 6

birthplace_id 7
	position = 215857, 0, -169697
	alpha = 145
	beta = 4

birthplace_id 8
	position = 243872, 0, -151441
	alpha = 196
	beta = 4

birthplace_id 9
	position = 100463, 0, -164520
	alpha = 124
	beta = 7

birthplace_id 10
	position = 132170, 0, -84947
	alpha = 161
	beta = -4

birthplace_id 11
	position = 125741, 0, -64574
	alpha = 274
	beta = 8

birthplace_id 12
	position = 96204, 0, -104075
	alpha = 175
	beta = 7

birthplace_id 13
	position = 102272, 0, -125643
	alpha = 28
	beta = 9

birthplace_id 14
	position = 234109, 0, -199341
	alpha = 76
	beta = -9

birthplace_id 15
	position = 210038, 0, -229677
	alpha = 318
	beta = 13

[objects]
count = 34

object_id 0
	model = "area2_obj_0"
	use_lod = 1
	lod_sprite_index = 0
	lod_dist_factor = 120000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 264
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 64
	shadow_circle_intensity = 16

object_id 1
	model = "area2_obj_1"
	use_lod = 1
	lod_sprite_index = 1
	lod_dist_factor = 240000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 414
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 64
	shadow_circle_intensity = 16

object_id 2
	model = "area2_obj_2"
	use_lod = 1
	lod_sprite_index = 2
	lod_dist_factor = 120000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 252
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 64
	shadow_circle_intensity = 16

object_id 3
	model = "area2_obj_3"
	use_lod = 1
	lod_sprite_index = 3
	lod_dist_factor = 120000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 252
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 64
	shadow_circle_intensity = 16

object_id 4
	model = "area2_obj_4"
	use_lod = 1
	lod_sprite_index = 4
	lod_dist_factor = 47110.2
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -16
	bound_top = 212
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 64
	shadow_circle_intensity = 16

object_id 5
	model = "area2_obj_5"
	use_lod = 1
	lod_sprite_index = 5
	lod_dist_factor = 47110.2
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -16
	bound_top = 212
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 64
	shadow_circle_intensity = 16

object_id 6
	model = "area2_obj_6"
	use_lod = 1
	lod_sprite_index = 6
	lod_dist_factor = 1.6e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = 0
	bound_top = 1232
	shadow_line_length = 800
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 24

object_id 7
	model = "area2_obj_7"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 1000
	bound_top = 3054
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 8
	model = "area2_obj_8"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 1022
	bound_top = 3054
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 9
	model = "area2_obj_9"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 1000
	bound_top = 1610
	use_custom_light = 1
	custom_light_intensity = 64
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 800
	shadow_circle_intensity = 32

object_id 10
	model = "area2_obj_10"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = 1000
	bound_top = 1610
	use_custom_light = 1
	custom_light_intensity = 64
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 800
	shadow_circle_intensity = 32

object_id 11
	model = "area2_obj_11"
	use_lod = 1
	lod_sprite_index = 7
	lod_dist_factor = 3.5e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2272
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 300
	shadow_circle_intensity = 16

object_id 12
	model = "area2_obj_12"
	use_lod = 1
	lod_sprite_index = 8
	lod_dist_factor = 3.5e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2370
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 300
	shadow_circle_intensity = 16

object_id 13
	model = "area2_obj_13"
	use_lod = 1
	lod_sprite_index = 9
	lod_dist_factor = 2.80687e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2400
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 14
	model = "area2_obj_14"
	use_lod = 1
	lod_sprite_index = 10
	lod_dist_factor = 2.80687e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2400
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 15
	model = "area2_obj_15"
	use_lod = 1
	lod_sprite_index = 11
	lod_dist_factor = 3.5e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2272
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 300
	shadow_circle_intensity = 16

object_id 16
	model = "area2_obj_16"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 1128
	shadow_line_length = 800
	shadow_line_intensity = 24
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 17
	model = "area2_obj_17"
	use_lod = 1
	lod_sprite_index = 12
	lod_dist_factor = 500000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 1230
	shadow_line_length = 800
	shadow_line_intensity = 24
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 18
	model = "area2_obj_18"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 316
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 300
	shadow_circle_intensity = 24

object_id 19
	model = "area2_obj_19"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 400
	bound_bottom = -78
	bound_top = 348
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 300
	shadow_circle_intensity = 24

object_id 20
	model = "area2_obj_20"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 400
	bound_bottom = -78
	bound_top = 348
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 300
	shadow_circle_intensity = 20

object_id 21
	model = "area2_obj_21"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 500
	bound_bottom = -110
	bound_top = 666
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 300
	shadow_circle_intensity = 24

object_id 22
	model = "area2_obj_22"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2048
	shadow_line_length = 512
	shadow_line_intensity = 0
	shadow_circle_radius = 1024
	shadow_circle_intensity = 24

object_id 23
	model = "area2_obj_23"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -12
	bound_top = 2060
	shadow_line_length = 512
	shadow_line_intensity = 0
	shadow_circle_radius = 1024
	shadow_circle_intensity = 0

object_id 24
	model = "area2_obj_24"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2352
	shadow_line_length = 2048
	shadow_line_intensity = 32
	shadow_circle_radius = 0
	shadow_circle_intensity = 24

object_id 25
	model = "area2_obj_25"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -140
	bound_top = 462
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 26
	model = "area2_obj_26"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 1536
	use_custom_light = 1
	custom_light_intensity = 140
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 1200
	shadow_circle_intensity = 32

object_id 27
	model = "area2_obj_27"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 1312
	use_custom_light = 1
	custom_light_intensity = 128
	shadow_line_length = 0
	shadow_line_intensity = 40
	shadow_circle_radius = 0
	shadow_circle_intensity = 16

object_id 28
	model = "area2_obj_28"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 1304
	use_custom_light = 1
	custom_light_intensity = 128
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 29
	model = "area2_obj_29"
	use_lod = 0
	face_collision_detection = 1
	bound_radius = 0
	bound_bottom = 0
	bound_top = 1304
	use_custom_light = 1
	custom_light_intensity = 128
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 30
	model = "area2_obj_30"
	use_lod = 0
	radial_collision_detection = 1
	bound_radius = 800
	bound_bottom = -70
	bound_top = 432
	use_custom_light = 1
	custom_light_intensity = 128
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 380
	shadow_circle_intensity = 24

object_id 31
	model = "area2_obj_31"
	use_lod = 1
	lod_sprite_index = 13
	lod_dist_factor = 400000
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

object_id 32
	model = "area2_obj_32"
	use_lod = 1
	lod_sprite_index = 14
	lod_dist_factor = 400000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -4
	bound_top = 434
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 33
	model = "area2_obj_33"
	use_lod = 1
	lod_sprite_index = 15
	lod_dist_factor = 320000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 332
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

[sounds]
count = 25

sound_id 0
	file = "area2_random_sound_0"

sound_id 1
	file = "area2_random_sound_1"

sound_id 2
	file = "area2_random_sound_2"

sound_id 3
	file = "area2_random_sound_3"

sound_id 4
	file = "area2_random_sound_4"

sound_id 5
	file = "area2_random_sound_5"

sound_id 6
	file = "area2_random_sound_6"

sound_id 7
	file = "area2_random_sound_7"

sound_id 8
	file = "area2_random_sound_8"

sound_id 9
	file = "area2_random_sound_9"

sound_id 10
	file = "area2_random_sound_10"

sound_id 11
	file = "area2_random_sound_11"

sound_id 12
	file = "area2_random_sound_12"

sound_id 13
	file = "area2_random_sound_13"

sound_id 14
	file = "area2_random_sound_14"

sound_id 15
	file = "area2_random_sound_15"

sound_id 16
	file = "area2_random_sound_16"

sound_id 17
	file = "area2_random_sound_17"

sound_id 18
	file = "area2_random_sound_18"

sound_id 19
	file = "area2_random_sound_19"

sound_id 20
	file = "area2_random_sound_20"

sound_id 21
	file = "area2_random_sound_21"

sound_id 22
	file = "area2_random_sound_22"

sound_id 23
	file = "area2_random_sound_23"

sound_id 24
	file = "area2_random_sound_24"

[ambients]
count = 5

ambient_id 0
	file = "area2_ambient_sound_0"
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
		random_sound_index = 13
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 14
		random_sound_index = 14
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 15
		random_sound_index = 15
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 1
	file = "area2_ambient_sound_1"
	random_sounds_count = 11

	random_sound_id 0
		random_sound_index = 0
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
		random_sound_index = 9
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 12
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 5
		random_sound_index = 14
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 6
		random_sound_index = 15
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 7
		random_sound_index = 16
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 8
		random_sound_index = 17
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

	volume = 0.501961

ambient_id 2
	file = "area2_ambient_sound_2"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 3
	file = "area2_ambient_sound_3"
	random_sounds_count = 7

	random_sound_id 0
		random_sound_index = 20
		random_sound_volume = 0.501961
		random_sound_frequency = 5

	random_sound_id 1
		random_sound_index = 21
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
		random_sound_index = 12
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 6
		random_sound_index = 13
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 4
	file = "area2_ambient_sound_4"
	random_sounds_count = 3

	random_sound_id 0
		random_sound_index = 22
		random_sound_volume = 0.501961
		random_sound_frequency = 2
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 23
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 24
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.501961

[water]
count = 7

water_id 0
	tile_index = 0
	level = 39
	opacity = 1
	night_color = 11, 30, 40

water_id 1
	tile_index = 0
	level = 64
	opacity = 1

water_id 2
	tile_index = 0
	level = 56
	opacity = 1

water_id 3
	tile_index = 0
	level = 43
	opacity = 1

water_id 4
	tile_index = 7
	level = 43
	opacity = 1

water_id 5
	tile_index = 65
	level = 70
	opacity = 1

water_id 6
	tile_index = 65
	level = 64
	opacity = 1

[fog]
count = 4

fog_id 0
	color = 127, 127, 127
	altitude = 0
	poisonous = 0
	distance = 512
	density = 200

fog_id 1
	color = 77, 90, 95
	altitude = 68
	poisonous = 0
	distance = 512
	density = 128

fog_id 2
	color = 90, 93, 97
	altitude = 148
	poisonous = 0
	distance = 1024
	density = 150

fog_id 3
	color = 88, 83, 77
	altitude = 63
	poisonous = 0
	distance = 768
	density = 128

