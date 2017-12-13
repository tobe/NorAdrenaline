#include "../../Required.h"

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

void CSystems::AutoStrafe(struct usercmd_s *cmd)
{
    #define STRAFE_DIR 1
    #define STRAFE_SPEED 69.0
    #define STRAFE_FPS 1
    #define STRAFE_FASTRUN 1
    #define STRAFE_SIDEMOVE 437.8928
    #define STRAFE_ANGLE 30.0
    if(!cvar.autostrafe) return;

    if(!(pmove->flags & FL_ONGROUND) && (pmove->movetype != 5) /*&& !g_Local.bSlowDown*/) {
        float dir = 0.0f;

        int dir_value = STRAFE_DIR;

        if(dir_value == 1)dir = 0 * M_PI / 180.0f;
        else if(dir_value == 2)	dir = 90 * M_PI / 180.0f;
        else if(dir_value == 3)	dir = 180 * M_PI / 180.0f;
        else if(dir_value == 4)	dir = -90 * M_PI / 180.0f;

        if(g_Local.flSpeed < 15.0f) {
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
            else
                cmd->forwardmove = 900;
        } else {
            float va_speed = atan2(pmove->velocity.y, pmove->velocity.x);

            float va[3] = {};
            g_Engine.GetViewAngles(va);

            float adif = va_speed - va[1] * M_PI / 180.0f - dir;

            adif = sin(adif);
            adif = atan2(adif, sqrt(1 - adif*adif));

            cmd->sidemove = (STRAFE_SIDEMOVE)*(adif > 0 ? 1 : -1);
            cmd->forwardmove = 0;

            float angle;
            float osin, ocos, nsin, ncos;

            angle = cmd->viewangles.y * M_PI / 180.0f;
            osin = sin(angle);
            ocos = cos(angle);

            angle = 2.0f * cmd->viewangles.y * M_PI / 180.0f - va_speed + dir;
            nsin = sin(angle);
            ncos = cos(angle);

            cmd->forwardmove = cmd->sidemove * (osin * ncos - ocos * nsin);
            cmd->sidemove *= osin * nsin + ocos * ncos;

            float fs = 0;
            if(atan2(STRAFE_ANGLE / g_Local.flSpeed, 1.0f) >= abs(adif)) {
                Vector vBodyDirection;

                if(dir_value & 1)
                    vBodyDirection = g_Local.vForward;
                else
                    vBodyDirection = g_Local.vRight;

                vBodyDirection[2] = 0;
                vBodyDirection = vBodyDirection.Normalize();

                // TODO: pow base 2?
                float vel = pow(vBodyDirection[0] * pmove->velocity[0], 2) + pow(vBodyDirection[1] * pmove->velocity[1], 2);

                fs = sqrt(STRAFE_SPEED * 100000 / vel);
            }

            cmd->forwardmove += fs;
        }

        float sdmw = cmd->sidemove;
        float fdmw = cmd->forwardmove;

        switch(STRAFE_DIR) {
            case 1:
            cmd->forwardmove = fdmw;
            cmd->sidemove = sdmw;
            break;
            case 2:
            cmd->forwardmove = -sdmw;
            cmd->sidemove = fdmw;
            break;
            case 3:
            cmd->forwardmove = -fdmw;
            cmd->sidemove = -sdmw;
            break;
            case 4:
            cmd->forwardmove = sdmw;
            cmd->sidemove = -fdmw;
            break;
        }
    }

	/*cmd->buttons &= ~(IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK);

	if (cmd->sidemove < 0.f)
		cmd->buttons |= IN_MOVELEFT;
	else
		cmd->buttons |= IN_MOVERIGHT;

	if (cmd->forwardmove < 0.f)
		cmd->buttons |= IN_BACK;
	else
		cmd->buttons |= IN_FORWARD;*/
}