#include "../../Required.h"
#include <time.h>

CSystems g_Systems;

void ComputeMove(int id, float forwardmove, float sidemove)
{
	Vector vecVelocity = g_Player[id].vVelocity;

	float wtf_const = 33.4;

	float flSpeed = vecVelocity.Length2D() * wtf_const;

	cl_entity_s *ent = g_Engine.GetEntityByIndex(id);

	if (flSpeed > 0 && ent)
	{
		QAngle QAngles = ent->angles;

		Vector vecVelocityRotated;
		vecVelocityRotated.VectorRotate(vecVelocity, QAngles);

		forwardmove = vecVelocityRotated[0] * wtf_const;
		sidemove = -1 * vecVelocityRotated[1] * wtf_const;

		if (forwardmove > 250)
			forwardmove = 250;
		
		if (sidemove > 250)
			sidemove = 250;
	}
	else {
		forwardmove = 0;
		sidemove = 0;
	}
}

void CSystems::RandomizeSteamID() {
    DWORD dwMask = 0x5A448; // steamclient.dll+5A448
    DWORD dwSteamID = 0x5AC4C; // steamclient.dll+5AC4C
    DWORD dwFlag = 0x5B4D4; // steamclient.dll+5B4D
    DWORD dwBase = reinterpret_cast<DWORD>(GetModuleHandle("steamclient.dll"));
    char szMask[14];

    srand(static_cast<unsigned int>(time(NULL)));
    char *szChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    for(uint8_t i = 0; i < 14; i++) {
        szMask[i] = szChars[rand() % strlen(szChars)];
    }

    native_memwrite(dwBase + dwMask, reinterpret_cast<uintptr_t>(szMask), sizeof(szMask));

    DWORD dwOldProtect;
    VirtualProtect(LPVOID(dwBase + dwFlag), 1, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    *reinterpret_cast<uint8_t *>(dwBase + dwFlag) = 0;
    VirtualProtect(LPVOID(dwBase + dwFlag), 1, dwOldProtect, &dwOldProtect);

    g_pConsole->Printf("Randomized. New mask: %s", szMask);
}

void CSystems::KnifeBot(struct usercmd_s *cmd)
{
	if (IsCurWeaponKnife() && CanAttack() && cvar.knifebot)
	{
		int id = NULL;
		float flDist = 8192.f;

		for (int i = 1; i <= g_Engine.GetMaxClients(); i++)
		{
			if (i == g_Local.iIndex)
				continue;

			if (!g_Player[i].bAlive)
				continue;

			if (g_Player[i].bFriend)
				continue;

			if (!g_Player[i].bVisible)
				continue;

			if (g_Player[i].iTeam == g_Local.iTeam)
				continue;

			if (g_Player[i].flDist < flDist || id == NULL)
			{
				flDist = g_Player[i].flDist;
				id = i;
			}
		}

		if(id > 0)
		{
			cl_entity_s *ent = g_Engine.GetEntityByIndex(id);

			if (!ent) 
				return;

			int fDidHit = FALSE;

			Vector vecPlayerOrigin = ent->origin + g_Player[id].vVelocity * g_Player[id].flFrametime;

			if (vecPlayerOrigin.Distance(pmove->origin) < 64)
			{
				// hit
				fDidHit = TRUE;

				float flDamage = 65.0f;

				Vector vecForwardPlayer, vecForward;

				Vector vecSrc = g_Local.vEye;

				QAngle QAimAngle;

				g_Utils.VectorAngles(vecPlayerOrigin - vecSrc, QAimAngle);

				QAimAngle.Normalize();

				g_Engine.pfnAngleVectors(QAimAngle, vecForward, 0, 0);
				g_Engine.pfnAngleVectors(ent->angles, vecForwardPlayer, 0, 0);

				vecForwardPlayer.Normalize();
				vecForward.Normalize();

				float flDot = vecForward.Dot2D(vecForwardPlayer);

				//Triple the damage if we are stabbing them in the back.
				if (flDot > 0.80f)
				{
					flDamage *= 3.0f;
				}

				if (fDidHit) 
				{
					g_Utils.MakeAngle(false, QAimAngle, cmd);

					cmd->buttons |= IN_ATTACK2;
				}
			}
		}
	}
}

void CSystems::BunnyHop(struct usercmd_s *cmd)
{
	if (cvar.bunnyhop)
	{
		if (cmd->buttons&IN_JUMP)
		{
			cmd->buttons &= ~IN_JUMP;

			if (pmove->flags & FL_ONGROUND || pmove->waterlevel >= 2)
			{
				cmd->buttons |= IN_JUMP;
			}
		}
	}
}

void CSystems::StandUpGroundStrafe(struct usercmd_s *cmd) {
    if(!cvar.sgs_temp) return;

    static int gs_state = 0;

    if(pmove->flFallVelocity > 0) {
        if((g_Local.flGroundAngle < 5) && (g_Local.flHeight <= 0.000001f || pmove->flags&FL_ONGROUND)) { func.AdjustSpeed(0.0001); }

        if(pmove->flFallVelocity >= 140) {
            if(g_Local.flHeight <= 30) {
                cmd->buttons |= IN_DUCK;
            }
        }
    }

    if(gs_state == 0 && (pmove->flags & FL_ONGROUND)) {
        if((g_Local.flGroundAngle<5) && (g_Local.flHeight <= 0.000001f || pmove->flags&FL_ONGROUND)) { func.AdjustSpeed(0.0001); }

        cmd->buttons |= IN_DUCK;

        gs_state = 1;
    } else if(gs_state == 1) {
        cmd->buttons &= ~IN_DUCK;

        if((g_Local.flGroundAngle<5) && (g_Local.flHeight <= 0.000001f || pmove->flags&FL_ONGROUND)) { func.AdjustSpeed(1); }

        gs_state = 0;
    }
}

void CSystems::Fastrun(struct usercmd_s *cmd) {
    if(cvar.fastrun_temp && (pmove->flags & FL_ONGROUND)) {
        if(!(cmd->buttons&(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT))) return;

        if(cmd->buttons&IN_MOVELEFT && cmd->buttons&IN_MOVERIGHT)
            cmd->buttons &= ~(IN_MOVELEFT | IN_MOVERIGHT);

        if(cmd->buttons&IN_FORWARD && cmd->buttons&IN_BACK)
            cmd->buttons &= ~(IN_FORWARD | IN_BACK);

        float aaddtova;

        if(cmd->buttons&IN_FORWARD) {
            if(cmd->buttons&IN_MOVELEFT)
                aaddtova = 45.0f;
            else if(cmd->buttons&IN_MOVERIGHT)
                aaddtova = -45.0f;
            else
                aaddtova = 0.0f;
        } else if(cmd->buttons&IN_BACK) {
            if(cmd->buttons&IN_MOVELEFT)
                aaddtova = 135.0f;
            else if(cmd->buttons&IN_MOVERIGHT)
                aaddtova = -135.0f;
            else
                aaddtova = 180.0f;
        } else if(cmd->buttons&IN_MOVELEFT)
            aaddtova = 90.0f;
        else if(cmd->buttons&IN_MOVERIGHT)
            aaddtova = -90.0f;

        if(g_Local.flVelocity < 78) {
            if(cmd->buttons&IN_FORWARD) {
                if(cmd->buttons&IN_MOVELEFT) {
                    cmd->forwardmove = 900;
                    cmd->sidemove = -900;
                } else if(cmd->buttons&IN_MOVERIGHT) {
                    cmd->forwardmove = 900;
                    cmd->sidemove = 900;
                } else
                    cmd->forwardmove = 900;
            } else if(cmd->buttons&IN_BACK) {
                if(cmd->buttons&IN_MOVELEFT) {
                    cmd->forwardmove = -900;
                    cmd->sidemove = -900;
                } else if(cmd->buttons&IN_MOVERIGHT) {
                    cmd->forwardmove = -900;
                    cmd->sidemove = 900;
                } else
                    cmd->forwardmove = -900;
            } else if(cmd->buttons&IN_MOVELEFT)
                cmd->sidemove = -900;
            else if(cmd->buttons&IN_MOVERIGHT)
                cmd->sidemove = 900;

            return;
        }

        bool dir_rig = true;

        float yaw = atan2(pmove->velocity.y, pmove->velocity.x) * 180.0 / M_PI;

        yaw -= aaddtova;
        yaw = cmd->viewangles.y - yaw;

        int fadif = yaw;

        fadif = (fadif + 360) % 360;
        dir_rig = fadif <= 180;

        float x = 135.06374825035877480180939653038* (dir_rig ? -1 : +1);
        float y = 267.87643402987823669025530311859;

        #define DegToRad(Angle)((M_PI/180.0)*Angle)
        float a = DegToRad(aaddtova);
        float sinA = sin(a);
        float cosA = cos(a);

        cmd->sidemove = x * cosA - y * sinA;
        cmd->forwardmove = y * cosA + x * sinA;

        x = cmd->sidemove;
        y = cmd->forwardmove;


        float newa = DegToRad(-yaw);
        sinA = sin(newa);
        cosA = cos(newa);

        cmd->sidemove = x * cosA - y * sinA;
        cmd->forwardmove = y * cosA + x * sinA;
    }
}

int CSystems::DetectStrafeDir(struct usercmd_s *cmd) {
    if(cmd->buttons&IN_FORWARD) {
        return 1;
    }
    if(cmd->buttons&IN_MOVERIGHT) {
        return 2;
    }
    if(cmd->buttons&IN_BACK) {
        return 3;
    }
    if(cmd->buttons&IN_MOVELEFT) {
        return 4;
    }
}

void CSystems::AutoStrafe(struct usercmd_s *cmd)
{
    if(!cvar.autostrafe || cvar.sgs_temp) return;

}