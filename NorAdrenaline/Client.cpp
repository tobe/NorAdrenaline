#include "Required.h"

CGlobalsVars g_pGlobals;
SCREENINFO g_Screen;

static float ticks = 0;

void AntiSnapshot() { g_pGlobals.bSnapshot = true; }
void AntiScreenshot() { g_pGlobals.bScreenshot = true; }
void AntiScreen()
{
	if (g_pGlobals.bSnapshot || g_pGlobals.bScreenshot)
	{
		g_pGlobals.ScreenTimer--;

		if (g_pGlobals.ScreenTimer <= 0)
		{
			if (g_pGlobals.bSnapshot)
			{
				Snapshot_s();
				g_pGlobals.bSnapshot = false;
			}
			else if (g_pGlobals.bScreenshot)
			{
				Screenshot_s();
				g_pGlobals.bScreenshot = false;
			}

			g_pGlobals.ScreenTimer = 5;
		}
	}
}

void WINAPI PaintTraversePanel(vgui::IPanel* vguiPanel, bool forceRepaint, bool allowForce)
{
	PanelHook.UnHook();

	g_pIPanel->PaintTraverse(vguiPanel, forceRepaint, allowForce);

	const char* PanelName = g_pIPanel->GetName(vguiPanel);

	if (strstr(PanelName, "StaticPanel") && !cvar.hide_from_obs && g_Engine.pfnGetCvarFloat("r_norefresh") == 0)
	{
		g_Visuals.Run();
		g_Menu.Run();

		g_pISurface->DrawSetTextFont(ESP);
		g_pISurface->DrawSetTextColor(0, 0, 0, 0);
		g_pISurface->DrawSetTextPos(1, 1);
		g_pISurface->DrawPrintText((wchar_t*)__func__, wcslen((wchar_t*)__func__));//Fix rendering

		g_pISurface->DrawFlushText();

		AntiScreen();
	}
	else if (strstr(PanelName, "LoadingDialog") || strstr(PanelName, "BasePanel"))
	{
		g_pGlobals.dwLoadingFinished = GetTickCount();
	}

	PanelHook.ReHook();
}

void CL_CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	g_Client.CL_CreateMove(frametime, cmd, active);

    func.AdjustSpeed(1);

    ticks += frametime;

	World.Update(frametime, cmd);

	if (g_Local.bAlive)
	{
		UpdateWeaponData();

		g_Systems.KnifeBot(cmd);
		g_Systems.BunnyHop(cmd);
		g_Systems.AutoStrafe(cmd);
        g_Systems.StandUpGroundStrafe(cmd);
        g_Systems.Fastrun(cmd);

        g_Misc.FastZoom(cmd); // before aimbot

		g_AimBot.Run(cmd);

		g_Misc.AutoReload(cmd);
		g_Misc.AutoPistol(cmd);

		ItemPostFrame(cmd);// do weapon stuff

		g_NoRecoil.CL_CreateMove(cmd);
		g_NoSpread.CL_CreateMove(cmd);

		g_Misc.AntiAim(cmd);
		g_Misc.FakeLag(cmd);
		g_Misc.NameStealer();
	}
}

void HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_Client.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);

	ItemPreFrame(from, to, cmd, runfuncs, time, random_seed);
}

void V_CalcRefdef(struct ref_params_s *pparams)
{
	g_Local.vPunchangle = pparams->punchangle;
	g_Local.vForward = pparams->forward;
	g_Local.vRight = pparams->right;
	g_Local.vUp = pparams->up;

	g_NoRecoil.V_CalcRefdef(pparams);
	g_NoSpread.V_CalcRefdef(pparams);

	g_Client.V_CalcRefdef(pparams);

	g_Misc.ThirdPerson(pparams);
}

void HUD_Frame(double time)
{
	g_Engine.pfnGetScreenInfo(&g_Screen);

	g_Engine.pNetAPI->Status(&(g_Local.status));

	World.UpdateMapInfo();

	g_Client.HUD_Frame(time);
}

void HUD_Frame_init(double time)
{
	g_Screen.iSize = sizeof(SCREENINFO);
	g_Engine.pfnGetScreenInfo(&g_Screen);

	g_pGlobals.ScreenTimer = 5;
	g_pGlobals.bScreenshot = false;
	g_pGlobals.bSnapshot = false;

	g_pGlobals.dwLoadingFinished = 0;

	pcmd_t pSnapshot = CommandByName("snapshot");
	pcmd_t pScreenshot = CommandByName("screenshot");

	Snapshot_s = (Snapshot_t)pSnapshot->function;
	Screenshot_s = (Snapshot_t)pScreenshot->function;

	pSnapshot->function = (xcommand_t)AntiSnapshot;
	pScreenshot->function = (xcommand_t)AntiScreenshot;

	World.Reset();
	World.ClearMapInfo();

	cvar.Init();
	func.Init();

	func.LoadCvars();

	HookOpenGL();

	if (!g_pConsole->IsConsoleVisible())
		g_pConsole->Activate();

    // Patch interpolation
    g_Offsets.PatchInterpolation();

    // Register SGS
    g_pEngine->pfnAddCommand("+zw_gstrafe", []() {
        cvar.sgs_temp = true;
    });
    g_pEngine->pfnAddCommand("-zw_gstrafe", []() {
        cvar.sgs_temp = false;
    });
    // Register fastrun
    g_pEngine->pfnAddCommand("+zw_fastrun", []() {
        cvar.fastrun_temp = true;
    });
    g_pEngine->pfnAddCommand("-zw_fastrun", []() {
        cvar.fastrun_temp = false;
    });
    // Register steamid randomization
    g_pEngine->pfnAddCommand("na_randsteam", []() {
        g_Systems.RandomizeSteamID();
    });

    // Patch rates
    g_Engine.pfnClientCmd("rate 999999;cl_updaterate 1000;cl_cmdrate 1000;cl_rate 9999;ex_interp 0.1");

    // Randomize steamid
    g_Systems.RandomizeSteamID();

	g_pConsole->DPrintf("\n\tNorAdrenaline loaded.\n");

	g_pClient->HUD_Frame = HUD_Frame;

	g_Client.HUD_Frame(time);
}

int CL_IsThirdPerson(void)
{
	if (!cvar.hide_from_obs && !g_pGlobals.bSnapshot && !g_pGlobals.bScreenshot && cvar.thirdperson > 0 && g_Local.bAlive)
		return 1;

	return g_Client.CL_IsThirdPerson();
}

void HUD_PlayerMoveInit(struct playermove_s *ppmove)
{
	PM_InitTextureTypes(ppmove);

	return g_Client.HUD_PlayerMoveInit(ppmove);
}

int StudioCheckBBox() { return 1; }

void StudioSetRemapColors(int top, int bottom)
{
	cl_entity_s *ent = g_Studio.GetCurrentEntity();

	World.GetHitboxes(ent);

	g_Studio.StudioSetRemapColors(top, bottom);
}

int pfnDrawUnicodeCharacter(int x, int y, short number, int r, int g, int b, unsigned long hfont)
{
	if (!cvar.hide_from_obs && cvar.hud_clear && !g_pGlobals.bSnapshot && !g_pGlobals.bScreenshot)
		return 1;

	return g_Engine.pfnDrawUnicodeCharacter(x, y, number, r, g, b, hfont);
}

int HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname)
{
	if (!cvar.hide_from_obs && cvar.esp && g_Utils.IsPlayer(ent) && g_Player[ent->index].bAlive && !g_pGlobals.bSnapshot && !g_pGlobals.bScreenshot)
	{
		if ((!cvar.esp_teammates || cvar.disable_render_teammates) && g_Player[ent->index].iTeam == g_Local.iTeam)
			return g_Client.HUD_AddEntity(type, ent, modelname);
	}

	return g_Client.HUD_AddEntity(type, ent, modelname);
}

void HookUserMessages()
{
	pResetHUD = HookUserMsg("ResetHUD", ResetHUD);
	pBombDrop = HookUserMsg("BombDrop", BombDrop);
	pBattery = HookUserMsg("Battery", Battery);
	pHealth = HookUserMsg("Health", Health);
	pDeathMsg = HookUserMsg("DeathMsg", DeathMsg);
	pSetFOV = HookUserMsg("SetFOV", SetFOV);
	pTeamInfo = HookUserMsg("TeamInfo", TeamInfo);
	pScoreAttrib = HookUserMsg("ScoreAttrib", ScoreAttrib);
}

int HUD_GetHullBounds(int hullnum, float* mins, float* maxs)
{
	if (hullnum == 1)
		maxs[2] = 32.0f;//https://gist.github.com/WPMGPRoSToTeMa/2e6e0454654f9e4ca22ee3e987051b57
	return 1;
}

void CL_Move() //Create and send the command packet to the server
{
    g_Utils.bSendpacket(true);

	CL_Move_s();
}

void Netchan_TransmitBits(void *chan, int length, byte *data) {
    //g_Engine.pfnConsolePrint("Netchan_TransmitBits")

    Netchan_TransmitBits_s(chan, length, data);
}

void HUD_ProcessPlayerState(struct entity_state_s *dst, const struct entity_state_s *src)
{
	if (cvar.bypass_valid_blockers)
	{
		src->mins[0] = -16;
		src->mins[1] = -16;
		src->mins[2] = -36;

		src->maxs[0] = 16;
		src->maxs[1] = 16;
		src->maxs[2] = 36;
	}

	g_Client.HUD_ProcessPlayerState(dst, src);
}

int HUD_Key_Event(int down, int keynum, const char *pszCurrentBinding) {
    if(down) {
        switch(keynum) {
            case 137: // F3
            (int)cvar.aim_hschance--;
            break;
            case 138: // F4
            (int)cvar.aim_hschance++;
            break;
        }
    }
    //g_Engine.Con_Printf("down: %d, keynum: %d, currbind: %s", down, keynum, pszCurrentBinding);
    return g_Client.HUD_Key_Event(down, keynum, pszCurrentBinding);
}

void HookClient()
{
	g_pClient->HUD_Frame = HUD_Frame_init;
	g_pClient->HUD_PostRunCmd = HUD_PostRunCmd;
	g_pClient->HUD_PlayerMoveInit = HUD_PlayerMoveInit;
	g_pClient->CL_IsThirdPerson = CL_IsThirdPerson;
	g_pClient->CL_CreateMove = CL_CreateMove;
	g_pClient->V_CalcRefdef = V_CalcRefdef;
	g_pClient->HUD_AddEntity = HUD_AddEntity;
	g_pClient->HUD_GetHullBounds = HUD_GetHullBounds;
	g_pClient->HUD_ProcessPlayerState = HUD_ProcessPlayerState;
    g_pClient->HUD_Key_Event = HUD_Key_Event;

	g_pStudio->StudioCheckBBox = StudioCheckBBox;
	g_pStudio->StudioSetRemapColors = StudioSetRemapColors;

	g_pEngine->pfnDrawUnicodeCharacter = pfnDrawUnicodeCharacter;

	HookUserMessages();

	PreS_DynamicSound_s = (PreS_DynamicSound_t)DetourFunction((LPBYTE)g_Offsets.PreS_DynamicSound(), (LPBYTE)&PreS_DynamicSound);
	CL_Move_s = (CL_Move_t)DetourFunction((LPBYTE)g_Offsets.CL_Move(), (LPBYTE)&CL_Move);
    //Netchan_TransmitBits_s = (Netchan_TransmitBits_t)DetourFunction((LPBYTE)g_Offsets.Netchan_TransmitBits(), (LPBYTE)&Netchan_TransmitBits);

	g_Offsets.EnablePageWrite((DWORD)g_pStudioModelRenderer, sizeof(StudioModelRenderer_t));
	g_pStudioModelRenderer->StudioRenderModel = StudioRenderModel_Gate;
	g_Offsets.RestorePageProtection((DWORD)g_pStudioModelRenderer, sizeof(StudioModelRenderer_t));
}
