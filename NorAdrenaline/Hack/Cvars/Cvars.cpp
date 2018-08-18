#include "../../Required.h"
#include <stdio.h>

CCvars cvar;
CFunctions func;

void CCvars::Init()
{
	hide_from_obs = false;

	aim = true; // Aim or not
	aim_teammates = false; // Deathmatch
	aim_target_selection = 1; // fov
	aim_hitbox = 6; // Hitbox of choice
	aim_multi_point = 0; // Multipoint
	aim_autowall = true; // Autowall
	aim_silent = true;
	aim_perfect_silent = true;
    aim_autoscope = true;
    aim_fov = 1.5;
    aim_hschance = 33;
    aim_autoshoot = false;

	fakelag = true;
	fakelag_while_shooting = false;
	fakelag_type = 4;
	fakelag_move = 4;
	fakelag_variance = 3.5;
	fakelag_limit = 7.5;

	autopistol = true;
	autoreload = false;
	fastzoom = true;

	nosmoke = false;
	remove_scope = false;
	crosshair = true;

	crosshair_r = 0;
	crosshair_g = 255;
	crosshair_b = 0;

	noflash = 0;
	thirdperson = 0;
	disable_render_teammates = false;

	esp = true;
	esp_behind = true;
	esp_teammates = false;

	esp_box = true;
	esp_box_ct_vis_r = 0;
	esp_box_ct_vis_g = 255;
	esp_box_ct_vis_b = 255;

	esp_box_t_vis_r = 255;
	esp_box_t_vis_g = 0;
	esp_box_t_vis_b = 0;

	esp_box_ct_invis_r = 0;
	esp_box_ct_invis_g = 255;
	esp_box_ct_invis_b = 255;

	esp_box_t_invis_r = 255;
	esp_box_t_invis_g = 0;
	esp_box_t_invis_b = 0;

	esp_box_outline = false;
	esp_name = true;

	esp_fake = true;
	bypass_trace_blockers = true;
	bypass_valid_blockers = true;
	esp_sound_minimum_volume = 0.5;

	esp_flags = false;
	esp_distance = false;
	esp_hitboxes = false;
	esp_sound = true;
	esp_alpha = 200;

	esp_screen = false;

    esp_weapon = 1;

	esp_bomb = true;
	esp_bomb_r = 255;
	esp_bomb_g = 255;
	esp_bomb_b = 255;

	grenade_trajectory = false;

	glow_players = false;

	glow_players_ct_r = 0;
	glow_players_ct_g = 0;
	glow_players_ct_b = 255;

	glow_players_t_r = 255;
	glow_players_t_g = 0;
	glow_players_t_b = 0;

	hud_clear = false;

	penetration_info = false;

	recoil_overlay_r = 255;
	recoil_overlay_g = 255;
	recoil_overlay_b = 0;

	spread_overlay_r = 255;
	spread_overlay_g = 0;
	spread_overlay_b = 0;

	chams = true;
	chams_behind_wall = false;
	chams_type = 1;

	chams_t_vis_r = 255;
	chams_t_vis_g = 0;
	chams_t_vis_b = 0;

	chams_t_invis_r = 255;
	chams_t_invis_g = 255;
	chams_t_invis_b = 0;

	chams_ct_vis_r = 0;
	chams_ct_vis_g = 255;
	chams_ct_vis_b = 255;

	chams_ct_invis_r = 0;
	chams_ct_invis_g = 255;
	chams_ct_invis_b = 255;

	spread_overlay = false;
	spread_overlay_old = false;
	recoil_overlay = false;

	draw_aim_fov = false;
	draw_aim_fov_r = 255;
	draw_aim_fov_g = 255;
	draw_aim_fov_b = 0;

	norecoil = true;
	norecoil_visual = false;

	nospread = true;
	nospread_method = 1;

	aa_yaw_while_running = 0;
	aa_yaw = 0;
	aa_yaw_static = 0;
	aa_pitch = 0;
	aa_edge = 0;
    aa_legit = 0;

	menu_legit_global_section = 1;
	menu_legit_sub_section = 1;
	menu_key = 45;
	menu_color_r = 215;
	menu_color_g = 111;
	menu_color_b = 234;

	bunnyhop = true;
	knifebot = false;
    autostrafe = true;

    sgs_temp = false;
    fastrun_temp = false;

	name_stealer = false;

	debug = false;
    status = true;
}

static double PRECISION = 0.00000000000001;

void CFunctions::Init()
{

}

void CFunctions::AdjustSpeed(double speed)
{
	static double LastSpeed = 1;

	if (speed != LastSpeed)
	{
		*(double*)g_Offsets.dwSpeedPointer = (speed * 1000);

		LastSpeed = speed;
	}
}
