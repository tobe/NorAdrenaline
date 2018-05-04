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

	deque<unsigned int> Hitboxes;

	if (cvar.aim_hitbox == 1)//"Head", "Neck", "Chest", "Stomach"
	{
		Hitboxes.push_back(11);
	}
	else if (cvar.aim_hitbox == 2)
	{
		Hitboxes.push_back(10);
	}
	else if (cvar.aim_hitbox == 3)
	{
		Hitboxes.push_back(7);
	}
	else if (cvar.aim_hitbox == 4)
	{
		Hitboxes.push_back(0);
	}
	else if (cvar.aim_hitbox == 5)//All
	{
		for (unsigned int j = 0; j <= 11; j++)
			Hitboxes.push_front(j);

		for (unsigned int k = 12; k < g_Local.iMaxHitboxes; k++)
			Hitboxes.push_back(k);
	}
	else if (cvar.aim_hitbox == 6)//Vital
	{
        unsigned int vitalHitboxes[] = {0, 7, 8, 9, 10, 12, 17};
        for(auto &hitbox : vitalHitboxes)
            Hitboxes.push_front(hitbox);
	}

	if (Hitboxes.empty())
		return;

	unsigned int m_iTarget = 0;
	int m_iHitbox = -1;
	int m_iPoint = -1;

	float m_flBestFOV = 180;
	float m_flBestDist = 8192;

	for (unsigned int id = 1; id <= g_Engine.GetMaxClients(); ++id)
	{
		if (id == g_Local.iIndex)
			continue;

		if (!g_Player[id].bAlive)
			continue;

		if (g_Player[id].bFriend)
			continue;

		if (!g_Player[id].bVisible)
			continue;

		if (!cvar.aim_teammates && g_Player[id].iTeam == g_Local.iTeam)
			continue;

		for (auto &&hitbox : Hitboxes)
		{
			if (g_PlayerExtraInfoList[id].bHitboxVisible[hitbox])
			{
				m_iHitbox = hitbox;
				break;
			}
		}

		if (m_iHitbox == -1)
		{
			for (auto &&hitbox : Hitboxes)
			{
				if (cvar.aim_multi_point > 0)
				{
					if (cvar.aim_multi_point == 1 && hitbox != 11)
						continue;

					if (cvar.aim_multi_point == 2 && hitbox > 11)
						continue;

					for (unsigned int point = 0; point <= 8; ++point)
					{
						if (g_PlayerExtraInfoList[id].bHitboxPointsVisible[hitbox][point] && !g_PlayerExtraInfoList[id].bHitboxVisible[hitbox])
						{
							m_iPoint = point;
							m_iHitbox = hitbox;
							break;
						}
					}
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

	if (cvar.aim_autoscope && IsCurWeaponSniper() && g_Local.iFOV == DEFAULT_FOV)
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
        if(!(cmd->buttons & IN_ATTACK) || m_flBestFOV > cvar.aim_fov && cvar.aim_silent) return;

		if (cvar.aim_perfect_silent)
		{
			g_Utils.MakeAngle(false, QAimAngles, cmd);
			g_Utils.bSendpacket(false);

            //g_Engine.pfnConsolePrint("initial choke!");

            /*if(++this->choked < cvar.aim_psilent_ticks) {
                g_Engine.Con_NPrintf(2, "choked: %i", this->choked);
                g_Utils.bSendpacket(false);
            } else {
                g_Engine.Con_NPrintf(2, "not choking anymore");
                g_Utils.bSendpacket(true);
                //g_Utils.MakeAngle(false, QMyAngles, cmd);
                this->choked = 0;
            }*/
		}
		else {
			g_Utils.MakeAngle(false, QAimAngles, cmd);

			if (!cvar.aim_silent)
				g_Engine.SetViewAngles(QAimAngles);
		}
	}
}