carnivores_area_resources

[general]
use_second_lod = 1
detailmap_texture = "detailmap_default"

bound_west = 131
bound_east = 178
bound_north = 131
bound_south = 178

[sky]

use_sky_sphere = 1

dawn_sky_color = 119, 90, 82
dawn_sun_color = 250, 230, 170
dawn_shadows_color = 30, 20, 30
dawn_sky_sphere_texture = "sunset_grad_3a"
dawn_sky_plane_texture = "clear_sky_1"
dawn_sky_plane_opacity = 100
dawn_sky_plane_color = 64, 64, 64
dawn_clouds_shadows_texture = "area6_clouds"

day_sky_color = 110, 102, 95
day_sun_color = 255, 238, 222
day_shadows_color = 50, 30, 30
day_sky_sphere_texture = "grad5"
day_sky_plane_texture = "storm_sky_3"
day_sky_plane_opacity = 120
day_sky_plane_color = 202, 170, 150
day_clouds_shadows_texture = "area6_clouds"
day_clouds_speed_scale = 5
day_fog_start = 0.4
day_fog_end = 1

night_sky_color = 21, 26, 38
night_sun_color = 32, 68, 104
night_shadows_color = 16, 24, 82
night_sky_sphere_texture = "car_night_grad1"
night_sky_plane_texture = "clear_sky_1"
night_sky_plane_opacity = 32
night_sky_plane_color = 21, 26, 32
night_clouds_shadows_texture = "area6_clouds"

[tiles]
atlas_texture = "trophy_tiles"
atlas_tiles_in_row = 4
atlas_tiles_in_column = 4
sprites_atlas_texture = "area1_sprites"

[objects]
count = 16

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
	model = "area1_obj_24"
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

[sounds]
count = 2

sound_id 0
	file = "trophy_random_sound_0"

sound_id 1
	file = "trophy_random_sound_1"

[ambients]
count = 1

ambient_id 0
	file = "trophy_ambient_sound_0"
	random_sounds_count = 2

	random_sound_id 0
		random_sound_index = 0
		random_sound_volume = 0.784314
		random_sound_frequency = 15

	random_sound_id 1
		random_sound_index = 1
		random_sound_volume = 0.784314
		random_sound_frequency = 30

	volume = 1

[water]
count = 0

[fog]
count = 1

fog_id 0
	color = 117, 119, 119
	altitude = 0
	poisonous = 0
	distance = 30
	density = 100

