carnivores_area_resources

[general]
use_second_lod = 1
additional_objects_rise = -2

bound_west = 30
bound_south = 998

[sky]
use_sky_sphere = 1

dawn_sky_color = 90, 80, 72
dawn_sun_color = 200, 156, 114
dawn_shadows_color = 30, 20, 30
dawn_sky_sphere_texture = "car_sunset_grad2"
dawn_sky_plane_texture = "car_sunset_sky"
dawn_sky_plane_opacity = 255
dawn_sky_plane_color = 74, 64, 86
dawn_clouds_shadows_texture = "area5_clouds"
dawn_clouds_speed_scale = 2
dawn_fog_start = 0.4
dawn_fog_end = 1

day_sky_color = 99, 116, 140
day_sun_color = 230, 210, 190
day_shadows_color = 50, 30, 20
day_sky_sphere_texture = "grad5"
day_sky_plane_texture = "storm_sky_3"
day_sky_plane_opacity = 30
day_sky_plane_color = 250, 250, 250
day_clouds_shadows_texture = "area5_clouds"
day_clouds_speed_scale = 2
day_fog_start = 0.6
day_fog_end = 1

night_sky_color = 21, 26, 38
night_sun_color = 32, 68, 104
night_shadows_color = 8, 8, 16
night_sky_sphere_texture = "car_night_grad1"
night_sky_plane_texture = "car_night_sky"
night_sky_plane_opacity = 180
night_sky_plane_color = 21, 26, 42
night_clouds_shadows_texture = "area5_clouds"
night_clouds_speed_scale = 2
night_fog_start = 0
night_fog_end = 0.9

[tiles]
atlas_texture = "area7_tiles"
atlas_tiles_in_row = 8
atlas_tiles_in_column = 8
sprites_atlas_texture = "area7_sprites"
grass_atlas_texture = "area_grass_A"
grass_mask_texture = "area7_grass_mask"

[birthplaces]
count = 13

birthplace_id 0
	position = 20798, 0, -18084
	alpha = 121
	beta = -5
	
birthplace_id 1
	position = 45344, 0, -84620
	alpha = 51
	beta = -5
	
birthplace_id 2
	position = 104067, 0, -62635
	alpha = 87
	beta = -2
	
birthplace_id 3
	position = 214933, 0, -66185
	alpha = 180
	beta = -22
	
birthplace_id 4
	position = 163193, 0, -64119
	alpha = 264
	beta = -10
	
birthplace_id 5
	position = 87931, 0, -160518
	alpha = 262
	beta = 0
	
birthplace_id 6
	position = 108328, 0, -189495
	alpha = 206
	beta = -2
	
birthplace_id 7
	position = 162915, 0, -178215
	alpha = 76
	beta = 0
	
birthplace_id 8
	position = 225237, 0, -184675
	alpha = 100
	beta = -7
	
birthplace_id 9
	position = 231935, 0, -244545
	alpha = 125
	beta = -4
	
birthplace_id 10
	position = 248863, 0, -116184
	alpha = 170
	beta = 1
	
birthplace_id 11
	position = 241564, 0, -89447
	alpha = 132
	beta = -4
	
birthplace_id 12
	position = 219006, 0, -56338
	alpha = 99
	beta = -9


[objects]
count = 17

object_id 0
	disabled = 1

object_id 1
	model = "af_wot_00"
	use_lod = 1
	lod_sprite_index = 1
	lod_dist_factor = 120000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -43
	bound_top = 570
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16
		
object_id 2
	model = "af_wot_01"
	use_lod = 1
	lod_sprite_index = 2
	lod_dist_factor = 120000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -35
	bound_top = 361
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16
	
object_id 3
	model = "af_wot_02"
	use_lod = 1
	lod_sprite_index = 3
	lod_dist_factor = 120000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -30
	bound_top = 413
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16
	
object_id 4
	model = "af_wot_03"
	use_lod = 1
	lod_sprite_index = 4
	lod_dist_factor = 120000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -41
	bound_top = 390
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16
	
object_id 5
	model = "af_plm_00"
	use_lod = 1
	lod_sprite_index = 5
	lod_dist_factor = 3000000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = -7
	bound_top = 1676
	shadow_line_length = 1024
	shadow_line_intensity = 16
	shadow_circle_radius = 256
	shadow_circle_intensity = 16
	
object_id 6
	model = "af_plm_02"
	use_lod = 1
	lod_sprite_index = 6
	lod_dist_factor = 5000000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = -16
	bound_top = 2776
	shadow_line_length = 1600
	shadow_line_intensity = 16
	shadow_circle_radius = 256
	shadow_circle_intensity = 16
	
object_id 7
	model = "af_plm_03"
	use_lod = 1
	lod_sprite_index = 7
	lod_dist_factor = 3000000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = -31
	bound_top = 2388
	shadow_line_length = 1600
	shadow_line_intensity = 16
	shadow_circle_radius = 256
	shadow_circle_intensity = 16
	
object_id 8
	model = "af_plm_04"
	use_lod = 1
	lod_sprite_index = 8
	lod_dist_factor = 3000000
	wind_waving = 1
	radial_collision_detection = 1
	bound_radius = 128
	bound_bottom = -25
	bound_top = 1716
	shadow_line_length = 1600
	shadow_line_intensity = 16
	shadow_circle_radius = 256
	shadow_circle_intensity = 16
	
object_id 9
	model = "af_pbush_00"
	use_lod = 1
	lod_sprite_index = 9
	lod_dist_factor = 480000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -40
	bound_top = 602
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16
	
object_id 10
	model = "af_pbush_01"
	use_lod = 1
	lod_sprite_index = 10
	lod_dist_factor = 480000
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -15
	bound_top = 494
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16
	
object_id 11
	model = "af_pbush_02"
	use_lod = 1
	lod_sprite_index = 11
	lod_dist_factor = 480000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -15
	bound_top = 519
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16
	
object_id 12
	model = "af_pbush_03"
	use_lod = 1
	lod_sprite_index = 12
	lod_dist_factor = 480000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -15
	bound_top = 519
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16
	
object_id 13
	model = "af_pbush_04"
	use_lod = 1
	lod_sprite_index = 13
	lod_dist_factor = 480000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -15
	bound_top = 423
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16
	
object_id 14
	model = "af_pbush_05"
	use_lod = 1
	lod_sprite_index = 14
	lod_dist_factor = 480000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -15
	bound_top = 336
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 200
	shadow_circle_intensity = 16
	
object_id 15
	model = "af_pig_00"
	use_lod = 1
	lod_sprite_index = 15
	lod_dist_factor = 2000000
	wind_waving = 1
	make_noise = 1
	radial_collision_detection = 1
	bound_radius = 0
	bound_bottom = -15
	bound_top = 713
	shadow_line_length = 0
	shadow_line_intensity = 16
	shadow_circle_radius = 128
	shadow_circle_intensity = 16
	
object_id 16
	model = "monolith"
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
	
[sounds]
count = 15

sound_id 0
	file = "b_bird_ziablik"

sound_id 1
	file = "b_bird_yellohammer"

sound_id 2
	file = "b_bird_cardinal"

sound_id 3
	file = "b_bird_robin1"

sound_id 4
	file = "b_tree_frog4"

sound_id 5
	file = "b_tree_frog2"

sound_id 6
	file = "b_fly2"

sound_id 7
	file = "b_fly1"

sound_id 8
	file = "b_criket10"

sound_id 9
	file = "b_criket1"

sound_id 10
	file = "b_criket2_multi"

sound_id 11
	file = "b_criket14cliks"

sound_id 12
	file = "b_ape4_siamang"

sound_id 13
	file = "b_ape1"

sound_id 14
	file = "b_lemur"

[ambients]
count = 4

ambient_id 0
	file = "area7_ambient_sound_0"
	random_sounds_count = 15
	
	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.65
		random_sound_frequency = 90
		random_sound_daytime_only = 1
		
	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.65
		random_sound_frequency = 90
		random_sound_daytime_only = 1

	random_sound_id 2
		random_sound_index = 2
		random_sound_volume = 0.65
		random_sound_frequency = 90
		random_sound_daytime_only = 1
		
	random_sound_id 3
		random_sound_index = 3
		random_sound_volume = 0.65
		random_sound_frequency = 90
		random_sound_daytime_only = 1
		
	random_sound_id 4
		random_sound_index = 4
		random_sound_volume = 0.65
		random_sound_frequency = 90
		random_sound_daytime_only = 1
		
	random_sound_id 5
		random_sound_index = 5
		random_sound_volume = 0.65
		random_sound_frequency = 90
		
	random_sound_id 6
		random_sound_index = 6
		random_sound_volume = 0.65
		random_sound_frequency = 180
		
	random_sound_id 7
		random_sound_index = 7
		random_sound_volume = 0.65
		random_sound_frequency = 180
		
	random_sound_id 8
		random_sound_index = 8
		random_sound_volume = 0.65
		random_sound_frequency = 30
		
	random_sound_id 9
		random_sound_index = 9
		random_sound_volume = 0.65
		random_sound_frequency = 30
		
	random_sound_id 10
		random_sound_index = 10
		random_sound_volume = 0.65
		random_sound_frequency = 30
		
	random_sound_id 11
		random_sound_index = 11
		random_sound_volume = 0.65
		random_sound_frequency = 30
		
	random_sound_id 12
		random_sound_index = 12
		random_sound_volume = 0.65
		random_sound_frequency = 90
		
	random_sound_id 13
		random_sound_index = 13
		random_sound_volume = 0.65
		random_sound_frequency = 90
		
	random_sound_id 14
		random_sound_index = 14
		random_sound_volume = 0.65
		random_sound_frequency = 90
		
	volume = 0.30

ambient_id 1
	file = "area7_ambient_sound_1"
	random_sounds_count = 9
	
	random_sound_id 0
		random_sound_index = 6
		random_sound_volume = 0.65
		random_sound_frequency = 60
		
	random_sound_id 1
		random_sound_index = 7
		random_sound_volume = 0.65
		random_sound_frequency = 60
		
	random_sound_id 2
		random_sound_index = 11
		random_sound_volume = 0.65
		random_sound_frequency = 60
		
	random_sound_id 3
		random_sound_index = 4
		random_sound_volume = 0.65
		random_sound_frequency = 60
		
	random_sound_id 4
		random_sound_index = 5
		random_sound_volume = 0.65
		random_sound_frequency = 60
		
	random_sound_id 5
		random_sound_index = 0
		random_sound_volume = 0.65
		random_sound_frequency = 60
		
	random_sound_id 6
		random_sound_index = 1
		random_sound_volume = 0.65
		random_sound_frequency = 60
		
	random_sound_id 7
		random_sound_index = 2
		random_sound_volume = 0.65
		random_sound_frequency = 60
		
	random_sound_id 8
		random_sound_index = 3
		random_sound_volume = 0.65
		random_sound_frequency = 60
		
	volume = 0.125

ambient_id 2
	file = "area7_ambient_sound_2"
	random_sounds_count = 0
	volume = 0.125

ambient_id 3
	file = "area7_ambient_sound_3"
	random_sounds_count = 0
	volume = 1

[water]
count = 3

water_id 0
	tile_index = 0
	level = 30
	opacity = 1
	night_color = 11, 30, 40

water_id 1
	tile_index = 0
	level = 47
	opacity = 1

water_id 2
	tile_index = 0
	level = 35
	opacity = 1

[fog]
count = 0
