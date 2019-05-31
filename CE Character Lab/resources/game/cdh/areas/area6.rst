carnivores_area_resources

[general]
detailmap_texture = "detailmap_default"

[sky]

use_sky_sphere = 1

dawn_sky_color = 90, 80, 72
dawn_sun_color = 250, 230, 170
dawn_shadows_color = 30, 20, 30
dawn_sky_sphere_texture = "car_sunset_grad3"
dawn_sky_plane_texture = "storm_sky_3"
dawn_sky_plane_opacity = 200
dawn_sky_plane_color = 104, 74, 64
dawn_clouds_shadows_texture = "car_clouds1"
dawn_clouds_speed_scale = 2
dawn_fog_start = 0.4
dawn_fog_end = 1

day_sky_color = 104, 125, 136
day_sun_color = 255, 238, 222
day_shadows_color = 30, 20, 10
day_sky_sphere_texture = "grad5"
day_sky_plane_texture = "storm_sky_3"
day_sky_plane_opacity = 180
day_sky_plane_color = 150, 150, 150
day_clouds_shadows_texture = "car_clouds1"
day_clouds_speed_scale = 2
day_fog_start = 0.3
day_fog_end = 1

night_sky_color = 21, 26, 38
night_sun_color = 32, 68, 104
night_shadows_color = 12, 28, 62
night_sky_sphere_texture = "car_night_grad2"
night_sky_plane_texture = "car_night_sky"
night_sky_plane_opacity = 80
night_sky_plane_color = 21, 26, 32
night_clouds_shadows_texture = "car_clouds1"
night_clouds_speed_scale = 1
night_fog_start = 0
night_fog_end = 0.8

[tiles]
atlas_texture = "area6_tiles"
atlas_tiles_in_row = 16
atlas_tiles_in_column = 8
sprites_atlas_texture = "area6_sprites"
grass_atlas_texture = "area_grass_A"
grass_mask_texture = "area6_grass_mask"

[birthplaces]
count = 16

birthplace_id 0
	position = 221442, 0, -162399
	alpha = 293
	beta = -13

birthplace_id 1
	position = 203675, 0, -167185
	alpha = 192
	beta = 0

birthplace_id 2
	position = 216161, 0, -142780
	alpha = 162
	beta = -2

birthplace_id 3
	position = 222058, 0, -127202
	alpha = 207
	beta = -1

birthplace_id 4
	position = 215880, 0, -108977
	alpha = 189
	beta = 1

birthplace_id 5
	position = 201915, 0, -79741
	alpha = 245
	beta = 5

birthplace_id 6
	position = 219982, 0, -26386
	alpha = 264
	beta = 7

birthplace_id 7	
	position = 178616, 0, -47379
	alpha = 273
	beta = 7

birthplace_id 8
	position = 100507, 0, -40036
	alpha = 253
	beta = -8

birthplace_id 9
	position = 67027, 0, -36424
	alpha = 288
	beta = 4

birthplace_id 10
	position = 65631, 0, -208118
	alpha = 130
	beta = -8

birthplace_id 11
	position = 114623, 0, -161633
	alpha = 193
	beta = 5

birthplace_id 12
	position = 162432, 0, -209362
	alpha = 293
	beta = 10

birthplace_id 13
	position = 178479, 0, -234453
	alpha = 215
	beta = -2

birthplace_id 14
	position = 170352, 0, -221582
	alpha = 213
	beta = 8

birthplace_id 15
	position = 227749, 0, -135759
	alpha = 192
	beta = 1

[objects]
count = 40

object_id 0
	model = "area6_obj_0"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 692
	bound_top = 2086
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 1
	model = "area6_obj_1"
	use_lod = 1
	lod_sprite_index = 0
	lod_dist_factor = 4e+006
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 186
	bound_top = 974
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 2
	model = "area6_obj_2"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -26
	bound_top = 2072
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 3
	model = "area6_obj_3"
	use_lod = 0
	transparent = 0
	bound_radius = 256
	bound_bottom = 796
	bound_top = 1552
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 4
	model = "area6_obj_4"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 796
	bound_top = 1552
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 5
	model = "area6_obj_5"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -12
	bound_top = 18
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 6
	model = "area6_obj_6"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -6
	bound_top = 146
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 7
	model = "area6_obj_7"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -6
	bound_top = 80
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 8
	model = "area6_obj_8"
	use_lod = 1
	lod_sprite_index = 1
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

object_id 9
	model = "area6_obj_9"
	use_lod = 1
	lod_sprite_index = 2
	lod_dist_factor = 1.64257e+006
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 1128
	shadow_line_length = 800
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 8

object_id 10
	model = "area6_obj_10"
	use_lod = 1
	lod_sprite_index = 3
	lod_dist_factor = 191401
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 316
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 11
	model = "area6_obj_11"
	use_lod = 1
	lod_sprite_index = 4
	lod_dist_factor = 2.80687e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 160
	bound_bottom = 0
	bound_top = 2400
	shadow_line_length = 1024
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 12
	model = "area6_obj_12"
	use_lod = 0
	transparent = 0
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -4
	bound_top = 346
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 13
	model = "area6_obj_13"
	use_lod = 1
	lod_sprite_index = 5
	lod_dist_factor = 3.5e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 180
	bound_bottom = 0
	bound_top = 2272
	shadow_line_length = 400
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 14
	model = "area6_obj_14"
	use_lod = 1
	lod_sprite_index = 6
	lod_dist_factor = 240000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -4
	bound_top = 314
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 13

object_id 15
	model = "area6_obj_15"
	use_lod = 1
	lod_sprite_index = 7
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
	shadow_circle_intensity = 13

object_id 16
	model = "area6_obj_16"
	use_lod = 1
	lod_sprite_index = 8
	lod_dist_factor = 3.41591e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2002
	shadow_line_length = 800
	shadow_line_intensity = 24
	shadow_circle_radius = 256
	shadow_circle_intensity = 16

object_id 17
	model = "area6_obj_17"
	use_lod = 1
	lod_sprite_index = 9
	lod_dist_factor = 2.5e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 2400
	shadow_line_length = 400
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 18
	disabled = 1

object_id 19
	disabled = 1

object_id 20
	disabled = 1

object_id 21
	disabled = 1

object_id 22
	model = "area6_obj_22"
	use_lod = 1
	lod_sprite_index = 10
	lod_dist_factor = 2e+006
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 140
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 23
	model = "area6_obj_23"
	use_lod = 1
	lod_sprite_index = 11
	lod_dist_factor = 2e+006
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 140
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 24
	model = "area6_obj_24"
	use_lod = 1
	lod_sprite_index = 12
	lod_dist_factor = 2e+006
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = 0
	bound_top = 192
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 25
	model = "area6_obj_25"
	use_lod = 1
	lod_sprite_index = 13
	lod_dist_factor = 320000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -2
	bound_top = 332
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 8

object_id 26
	model = "area6_obj_26"
	use_lod = 1
	lod_sprite_index = 14
	lod_dist_factor = 240000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -2
	bound_top = 644
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 27
	model = "area6_obj_27"
	use_lod = 1
	lod_sprite_index = 15
	lod_dist_factor = 5e+006
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -26
	bound_top = 290
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 28
	model = "area6_obj_28"
	use_lod = 1
	lod_sprite_index = 16
	lod_dist_factor = 5e+006
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -26
	bound_top = 290
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 29
	model = "area6_obj_29"
	use_lod = 1
	lod_sprite_index = 17
	lod_dist_factor = 1.6e+006
	face_collision_detection = 1
	bound_radius = 256
	bound_bottom = -108
	bound_top = 506
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 30
	model = "area6_obj_30"
	use_lod = 1
	lod_sprite_index = 18
	lod_dist_factor = 1.78543e+006
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -30
	bound_top = 1904
	shadow_line_length = 800
	shadow_line_intensity = 24
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 31
	model = "area6_obj_31"
	use_lod = 1
	lod_sprite_index = 19
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

object_id 32
	model = "area6_obj_32"
	use_lod = 1
	lod_sprite_index = 20
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

object_id 33
	model = "area6_obj_33"
	use_lod = 1
	lod_sprite_index = 21
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

object_id 34
	model = "area6_obj_34"
	use_lod = 1
	lod_sprite_index = 22
	lod_dist_factor = 200000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -28
	bound_top = 222
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 35
	model = "area6_obj_35"
	use_lod = 1
	lod_sprite_index = 23
	lod_dist_factor = 200000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -28
	bound_top = 222
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 36
	model = "area6_obj_36"
	use_lod = 1
	lod_sprite_index = 24
	lod_dist_factor = 200000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -18
	bound_top = 266
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 37
	model = "area6_obj_37"
	use_lod = 1
	lod_sprite_index = 25
	lod_dist_factor = 400000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -160
	bound_top = 204
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 38
	model = "area6_obj_38"
	use_lod = 1
	lod_sprite_index = 26
	lod_dist_factor = 400000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -76
	bound_top = 110
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

object_id 39
	model = "area6_obj_39"
	use_lod = 1
	lod_sprite_index = 27
	lod_dist_factor = 400000
	radial_collision_detection = 1
	bound_radius = 256
	bound_bottom = -36
	bound_top = 152
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16

[sounds]
count = 16

sound_id 0
	file = "area6_random_sound_0"

sound_id 1
	file = "area6_random_sound_1"

sound_id 2
	file = "area6_random_sound_2"

sound_id 3
	file = "area6_random_sound_3"

sound_id 4
	file = "area6_random_sound_4"

sound_id 5
	file = "area6_random_sound_5"

sound_id 6
	file = "area6_random_sound_6"

sound_id 7
	file = "area6_random_sound_7"

sound_id 8
	file = "area6_random_sound_8"

sound_id 9
	file = "area6_random_sound_9"

sound_id 10
	file = "area6_random_sound_10"

sound_id 11
	file = "area6_random_sound_11"

sound_id 12
	file = "area6_random_sound_12"

sound_id 13
	file = "area6_random_sound_13"

sound_id 14
	file = "area6_random_sound_14"

sound_id 15
	file = "area6_random_sound_15"

[ambients]
count = 5

ambient_id 0
	file = "area6_ambient_sound_0"
	random_sounds_count = 14

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.376471
		random_sound_frequency = 3
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.12549
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 3
		random_sound_volume = 0.501961
		random_sound_frequency = 2

	random_sound_id 3
		random_sound_index = 4
		random_sound_volume = 0.0784314
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 5
		random_sound_volume = 0.156863
		random_sound_frequency = 20

	random_sound_id 5
		random_sound_index = 6
		random_sound_volume = 0.501961
		random_sound_frequency = 10

	random_sound_id 6
		random_sound_index = 9
		random_sound_volume = 0.156863
		random_sound_frequency = 20

	random_sound_id 7
		random_sound_index = 8
		random_sound_volume = 0.376471
		random_sound_frequency = 15

	random_sound_id 8
		random_sound_index = 10
		random_sound_volume = 0.25098
		random_sound_frequency = 30

	random_sound_id 9
		random_sound_index = 11
		random_sound_volume = 0.25098
		random_sound_frequency = 30

	random_sound_id 10
		random_sound_index = 13
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 11
		random_sound_index = 12
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 12
		random_sound_index = 15
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 13
		random_sound_index = 14
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 1
	file = "area6_ambient_sound_1"
	random_sounds_count = 0
	volume = 0.501961

ambient_id 2
	file = "area6_ambient_sound_2"
	random_sounds_count = 6

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.313726
		random_sound_frequency = 5

	random_sound_id 1
		random_sound_index = 6
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 7
		random_sound_volume = 0.156863
		random_sound_frequency = 30

	random_sound_id 3
		random_sound_index = 10
		random_sound_volume = 0.352941
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 11
		random_sound_volume = 0.156863
		random_sound_frequency = 30

	random_sound_id 5
		random_sound_index = 15
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.501961

ambient_id 3
	file = "area6_ambient_sound_3"
	random_sounds_count = 14

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.352941
		random_sound_frequency = 30

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.196078
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 2
		random_sound_volume = 0.117647
		random_sound_frequency = 30

	random_sound_id 3
		random_sound_index = 3
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 5
		random_sound_volume = 0.27451
		random_sound_frequency = 30

	random_sound_id 5
		random_sound_index = 6
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 6
		random_sound_index = 7
		random_sound_volume = 0.156863
		random_sound_frequency = 30

	random_sound_id 7
		random_sound_index = 8
		random_sound_volume = 0.235294
		random_sound_frequency = 30

	random_sound_id 8
		random_sound_index = 9
		random_sound_volume = 0.27451
		random_sound_frequency = 30

	random_sound_id 9
		random_sound_index = 10
		random_sound_volume = 0.352941
		random_sound_frequency = 30

	random_sound_id 10
		random_sound_index = 12
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 11
		random_sound_index = 13
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 12
		random_sound_index = 14
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	random_sound_id 13
		random_sound_index = 15
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.25098

ambient_id 4
	file = "area6_ambient_sound_4"
	random_sounds_count = 5

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.25098
		random_sound_frequency = 7
		random_sound_daytime_only = 1

	random_sound_id 1
		random_sound_index = 3
		random_sound_volume = 0.25098
		random_sound_frequency = 30

	random_sound_id 2
		random_sound_index = 6
		random_sound_volume = 0.25098
		random_sound_frequency = 30

	random_sound_id 3
		random_sound_index = 10
		random_sound_volume = 0.25098
		random_sound_frequency = 30

	random_sound_id 4
		random_sound_index = 12
		random_sound_volume = 0.501961
		random_sound_frequency = 30

	volume = 0.501961

[water]
count = 16

water_id 0
	tile_index = 10
	level = 25
	opacity = 1
	color = 0, 26, 23
	night_color = 11, 30, 40

water_id 1
	tile_index = 0
	level = 86
	opacity = 1

water_id 2
	tile_index = 10
	level = 70
	opacity = 1

water_id 3
	tile_index = 10
	level = 65
	opacity = 1

water_id 4
	tile_index = 10
	level = 81
	opacity = 1

water_id 5
	tile_index = 0
	level = 47
	opacity = 1

water_id 6
	tile_index = 0
	level = 20
	opacity = 1

water_id 7
	tile_index = 10
	level = 50
	opacity = 1

water_id 8
	tile_index = 10
	level = 44
	opacity = 1

water_id 9
	tile_index = 10
	level = 41
	opacity = 1

water_id 10
	tile_index = 10
	level = 42
	opacity = 1

water_id 11
	tile_index = 10
	level = 47
	opacity = 1

water_id 12
	tile_index = 0
	level = 28
	opacity = 1

water_id 13
	tile_index = 0
	level = 21
	opacity = 1

water_id 14
	tile_index = 10
	level = 28
	opacity = 1

water_id 15
	tile_index = 0
	level = 32
	opacity = 1

[fog]
count = 1

fog_id 0
	color = 127, 127, 127
	altitude = 0
	poisonous = 0
	distance = 512
	density = 200

