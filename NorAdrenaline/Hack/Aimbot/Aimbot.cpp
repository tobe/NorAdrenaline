#include "../../Required.h"
#include <sstream>

CAimBot g_AimBot;

void SmoothAimAngles(QAngle MyViewAngles, QAngle AimAngles, QAngle &OutAngles, float Smoothing, bool bSpiral, float SpiralX, float SpiralY)
{
	if (Smoothing < 1)
	{
		OutAngles = AimAngles;
		return;
	}

	OutAngles = AimAngles - MyViewAngles;

	OutAngles.Normalize();

	Vector vecViewAngleDelta = OutAngles;

	if (bSpiral & SpiralX != 0 && SpiralY != 0)
		vecViewAngleDelta += Vector(vecViewAngleDelta.y / SpiralX, vecViewAngleDelta.x / SpiralY, 0.0f);

	if (!isnan(Smoothing))
		vecViewAngleDelta /= Smoothing;

	OutAngles = MyViewAngles + vecViewAngleDelta;

	OutAngles.Normalize();
}

void CAimBot::Run(struct usercmd_s *cmd)
{
	if (cvar.aim)
	{
		Aimbot(cmd);
	}
}

void CAimBot::Aimbot(struct usercmd_s *cmd)
{
	if (!cvar.aim && !IsCurWeaponGun() || !CanAttack())
		return;

    unsigned int m_iTarget = 0;
    int m_iHitbox = -1;
    int m_iPoint  = -1;
    deque<unsigned int> Hitboxes;

    switch((int)cvar.aim_hitbox) {
        case 1:
            m_iHitbox = 11; // head
        break;
        case 2:
            m_iHitbox = 10; // neck
        break;
        case 3:
            m_iHitbox = 9;  // low head
        break;
        case 4:
            m_iHitbox = 8;  // chest
        break;
        case 5:
            m_iHitbox = 7; // stomach
        break;
        case 6:
        { // Vital (Multipoint)
            uint8_t vitalHitboxes[] = {7, 8, 9, 10, 11};
            for(uint8_t i = 0; i < sizeof(vitalHitboxes); i++)
                Hitboxes.push_back(vitalHitboxes[i]);
        }break;
        case 7: // All (Multipoint)
            for(unsigned int j = 0; j <= 11; j++)
                Hitboxes.push_back(j);

            for(unsigned int k = 12; k < g_Local.iMaxHitboxes; k++)
                Hitboxes.push_back(k);
        break;
    }

	float m_flBestFOV = cvar.aim_fov;
    this->m_flCurrentFOV = 0;
	float m_flBestDist = 8192.f;

	for (unsigned int id = 1; id <= g_Engine.GetMaxClients(); ++id)
	{
		if (id == g_Local.iIndex)
			continue;

		if (!g_Player[id].bAlive)
			continue;

		if (g_Player[id].bFriend)
			continue;

        if(!g_Player[id].bVisible && !cvar.aim_autowall)
            continue;

		if (!cvar.aim_teammates && g_Player[id].iTeam == g_Local.iTeam)
			continue;

        // Prioritize visible hitboxes (Multihitboxes / Multipoints)
        if(!Hitboxes.empty()) {
            for(auto &&hitbox : Hitboxes) {    
                if(g_PlayerExtraInfoList[id].bHitboxVisible[hitbox]) {
                    m_iHitbox = hitbox;
                    break;
                }
            }
        }

        // Multi point aimbot
		if (cvar.aim_multi_point > 0)
		{
            for(auto &&hitbox : Hitboxes) {
                if(cvar.aim_multi_point == 1 && m_iHitbox != 11)
                    continue;

                if(cvar.aim_multi_point == 2 && m_iHitbox > 11)
                    continue;

                for(unsigned int point = 0; point <= 8; point++) {
                    if(g_PlayerExtraInfoList[id].bHitboxPointsVisible[m_iHitbox][point] && !g_PlayerExtraInfoList[id].bHitboxVisible[m_iHitbox]) {
                        m_iPoint = point;
                        m_iHitbox = m_iHitbox;
                        break;
                    }
                }
            }
		}

        // Got no hitbox (visible, multipoint, etc...).
        // Default to head if not using a sniper, chest otherwise.
        // Useful for wall banging against the head
        if(m_iHitbox < 0)
            m_iHitbox = IsCurWeaponSniper() ? 8 : 11;

		if (m_iHitbox < 0 || m_iHitbox > g_Local.iMaxHitboxes)
			continue;

		if (cvar.aim_target_selection == 1)
		{
            // Get enemy W2S
            float enemyPosition[2];
            if(!g_Utils.bCalcScreen(g_PlayerExtraInfoList[id].vHitbox[m_iHitbox], enemyPosition)) continue; // Not on screen

            cl_entity_s *ent = g_Engine.GetEntityByIndex(id);

            if(!ent)
                return;

            Vector Top = Vector(ent->origin.x, ent->origin.y, ent->origin.z + ent->curstate.mins.z);
            Vector Bot = Vector(ent->origin.x, ent->origin.y, ent->origin.z + ent->curstate.maxs.z);

            float ScreenTop[2], ScreenBot[2];

            bool m_bScreenTop = g_Utils.bCalcScreen(Top, ScreenTop);
            bool m_bScreenBot = g_Utils.bCalcScreen(Bot, ScreenBot);

            if(m_bScreenTop && m_bScreenBot) {
                float _h = ScreenBot[1] - ScreenTop[1];
                float box_height = g_Player[id].bDucked ? _h : _h * 0.9f;
                float box_width = box_height * 0.3f;

                int x = ScreenTop[0];
                int y = ScreenTop[1];
                int w = box_width;
                int h = box_height;
                int x0 = x - w;
                int y0 = y;
                int x1 = x + w;
                int y1 = y + h;

                if(x1 < x0)
                    swap(x1, x0);

                if(y1 < y0)
                    swap(y1, y0);


                static float my_x = g_Screen.iWidth * 0.5f;
                static float my_y = g_Screen.iHeight * 0.5f;

                // Inside
                if((my_x >= x0 && my_x <= x1) && (my_y >= y0 && my_y <= y1)) {
                    m_iTarget = id;
                }
            }
		}
		else if (cvar.aim_target_selection == 2)
		{
			if (g_Player[id].flDist < m_flBestDist)
			{
				m_flBestDist = g_Player[id].flDist;
				m_iTarget = id;
			}
		}
		else if (cvar.aim_target_selection == 3)
		{
			if (g_PlayerExtraInfoList[id].fHitboxFOV[m_iHitbox] < m_flBestFOV)
			{
				m_flBestFOV = g_PlayerExtraInfoList[id].fHitboxFOV[m_iHitbox];
				m_flBestDist = g_Player[id].flDist;
				m_iTarget = id;
			}
		}
	}

    if(m_iTarget <= 0) return;

    // Register the FoV and check it
    this->m_flCurrentFOV = m_flBestFOV;
    if(cvar.aim_target_selection == 3 && (m_flBestFOV > cvar.aim_fov))
        return;

    // Legit headshot
    if(cvar.aim_hschance > 0) {
        uint8_t randomValue = rand() % 100;
        if(randomValue <= cvar.aim_hschance && !IsCurWeaponSniper()) {
            m_iHitbox = 11;
        }
    }

    // Always headshot prioritized targets
    if(g_Player[m_iTarget].bPriority) {
        m_iHitbox = 11;
    }

    // Quickscope
	if (cvar.aim_autoscope && IsCurWeaponSniper() && (cmd->buttons & IN_ATTACK) && g_Local.iFOV == DEFAULT_FOV)
	{
		cmd->buttons |= IN_ATTACK2;
	}
	else if (CanAttack())
	{
		QAngle QMyAngles, QAimAngles, QSmoothAngles;
		Vector vAimOrigin;

		if(m_iPoint >= 0 && m_iPoint < 8)
			vAimOrigin = g_PlayerExtraInfoList[m_iTarget].vHitboxPoints[m_iHitbox][m_iPoint];
		else
			vAimOrigin = g_PlayerExtraInfoList[m_iTarget].vHitbox[m_iHitbox];

		g_Engine.GetViewAngles(QMyAngles);

		g_Utils.VectorAngles(vAimOrigin - g_Local.vEye, QAimAngles);

        // We have to be in attack to process the rest.
        if(!(cmd->buttons & IN_ATTACK) && (cvar.aim_silent || cvar.aim_perfect_silent)) return;

		if (cvar.aim_perfect_silent)
		{
			g_Utils.MakeAngle(false, QAimAngles, cmd);
			g_Utils.bSendpacket(false);
		}
		else {
			g_Utils.MakeAngle(false, QAimAngles, cmd);

			if (!cvar.aim_silent)
				g_Engine.SetViewAngles(QAimAngles);
		}
	}
}