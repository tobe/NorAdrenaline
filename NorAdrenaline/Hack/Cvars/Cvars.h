class CCvars;
class CCvars
{
public:
	void Init();

	bool hide_from_obs;

	bool aim;
	bool aim_teammates;
	float aim_target_selection;
	float aim_hitbox;
	float aim_multi_point;
	bool aim_autowall;
	bool aim_silent;
	bool aim_perfect_silent;
	bool aim_autoscope;
    float aim_fov;
    float aim_hschance;
    bool aim_autoshoot;

	bool fakelag;
	bool fakelag_while_shooting;
	float fakelag_type;
	float fakelag_move;
	float fakelag_variance;
	float fakelag_limit;
    bool fakelag_adaptive_ex_interp;

	bool autopistol;
	bool autoreload;
	bool fastzoom;

	bool nosmoke;
	bool remove_scope;
	bool crosshair;

	float crosshair_r;
	float crosshair_g;
	float crosshair_b;

	float thirdperson;

	bool disable_render_teammates;

	float noflash;

	bool esp;
	bool esp_behind;
	bool esp_teammates;

	bool esp_box;
	float esp_box_ct_vis_r;
	float esp_box_ct_vis_g;
	float esp_box_ct_vis_b;
	float esp_box_t_vis_r;
	float esp_box_t_vis_g;
	float esp_box_t_vis_b;
	float esp_box_ct_invis_r;
	float esp_box_ct_invis_g;
	float esp_box_ct_invis_b;
	float esp_box_t_invis_r;
	float esp_box_t_invis_g;
	float esp_box_t_invis_b;

	bool esp_box_outline;
	bool esp_name;

    float esp_weapon;

	bool esp_fake;
	bool bypass_trace_blockers;
	bool bypass_valid_blockers;
	float esp_sound_minimum_volume;

	bool esp_flags;
	bool esp_distance;
	bool esp_hitboxes;
	bool esp_sound;

	float esp_alpha;

	bool esp_screen;

	bool esp_bomb;
	float esp_bomb_r;
	float esp_bomb_g;
	float esp_bomb_b;

	bool hud_clear;

	bool penetration_info;

	float recoil_overlay_r;
	float recoil_overlay_g;
	float recoil_overlay_b;

	float spread_overlay_r;
	float spread_overlay_g;
	float spread_overlay_b;

	bool grenade_trajectory;

	bool chams;
	bool chams_behind_wall;
	float chams_type;

	float chams_t_vis_r;
	float chams_t_vis_g;
	float chams_t_vis_b;

	float chams_t_invis_r;
	float chams_t_invis_g;
	float chams_t_invis_b;

	float chams_ct_vis_r;
	float chams_ct_vis_g;
	float chams_ct_vis_b;

	float chams_ct_invis_r;
	float chams_ct_invis_g;
	float chams_ct_invis_b;

	bool glow_players;

	float glow_players_ct_r;
	float glow_players_ct_g;
	float glow_players_ct_b;

	float glow_players_t_r;
	float glow_players_t_g;
	float glow_players_t_b;

	bool spread_overlay;
	bool spread_overlay_old;
	bool recoil_overlay;

	bool draw_aim_fov;
	float draw_aim_fov_r;
	float draw_aim_fov_g;
	float draw_aim_fov_b;

	bool norecoil;
	bool norecoil_visual;

	bool nospread;
	float nospread_method;

	float aa_yaw_while_running;
	float aa_yaw;
	float aa_yaw_static;
	float aa_pitch;
	float aa_edge;
    float aa_legit;

	float menu_legit_global_section;
	float menu_legit_sub_section;

	int menu_key;

	float menu_color_r;
	float menu_color_g;
	float menu_color_b;

	bool bunnyhop;
	bool knifebot;
    bool autostrafe;

    bool sgs_temp;
    bool fastrun_temp;

	float name_stealer;

	bool debug;
    bool status;
};
extern CCvars cvar;

class CFunctions
{
public:
	void Init();

	void AdjustSpeed(double speed);
};
extern CFunctions func;