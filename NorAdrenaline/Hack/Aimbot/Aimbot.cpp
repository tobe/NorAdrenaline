#include "../../Required.h"

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

    switch((int)cvar.aim_hitbox) {
        case 1:
            m_iHitbox = 11; // head
        break;
        case 2:
            m_iHitbox = 10; // neck
        break;
        case 3:
            m_iHitbox = 7; // stomach
        break;
        case 4:
            m_iHitbox = 4; // rightleg
            // dodat 4 vidit sta sa 7 i 0.
        break;
    }

	float m_flBestFOV = cvar.aim_fov;
	float m_flBestDist = FLT_MAX;

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

        // Multi point aimbot
		if (cvar.aim_multi_point > 0)
		{
			if (cvar.aim_multi_point == 1 && m_iHitbox != 11)
				continue;

			if (cvar.aim_multi_point == 2 && m_iHitbox > 11)
				continue;

			for (unsigned int point = 0; point <= 8; ++point)
			{
				if (g_PlayerExtraInfoList[id].bHitboxPointsVisible[m_iHitbox][point] && !g_PlayerExtraInfoList[id].bHitboxVisible[m_iHitbox])
				{
					m_iPoint = point;
					m_iHitbox = m_iHitbox;
					break;
				}
			}
		}

		if (m_iHitbox < 0 || m_iHitbox > g_Local.iMaxHitboxes)
			continue;

		if (g_Player[id].bPriority)
		{
			m_iTarget = id;
			break;
		}
		//"Field of view", "Distance", "Cycle"
		if (cvar.aim_target_selection == 1)
		{
			if (g_PlayerExtraInfoList[id].fHitboxFOV[m_iHitbox] < m_flBestFOV)
			{
				m_flBestFOV = g_PlayerExtraInfoList[id].fHitboxFOV[m_iHitbox];
				m_iTarget = id;
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
				if (g_Player[id].flDist < m_flBestDist)
				{
					m_flBestFOV = g_PlayerExtraInfoList[id].fHitboxFOV[m_iHitbox];
					m_flBestDist = g_Player[id].flDist;
					m_iTarget = id;
				}
			}
		}
	}

    if(m_iTarget <= 0) return;

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

        // We have to be in attack to process the rest. And also check the fov.
        if(!(cmd->buttons & IN_ATTACK) && (cvar.aim_silent || cvar.aim_perfect_silent)) return;

		if (cvar.aim_perfect_silent && m_flBestFOV <= cvar.aim_fov)
		{
			g_Utils.MakeAngle(false, QAimAngles, cmd);
			g_Utils.bSendpacket(false);
		}
		else { // No FoV check for rage aim
			g_Utils.MakeAngle(false, QAimAngles, cmd);

			if (!cvar.aim_silent)
				g_Engine.SetViewAngles(QAimAngles);
		}
	}
}