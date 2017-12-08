class CMisc;
class CMisc
{
public:
	bool FakeEdge(float &angle);
	void FakeLag(struct usercmd_s *cmd);
	void AutoPistol(struct usercmd_s *cmd);
	void AutoReload(struct usercmd_s *cmd);
	void AntiAim(struct usercmd_s *cmd);
	void ThirdPerson(struct ref_params_s *pparams);
	void FastZoom(struct usercmd_s *cmd);
	void NameStealer();

    void GroundStrafeOn();
    void GroundStrafeOff();
    void GroundStrafe(struct usercmd_s *cmd);

    void FastRunOn();
    void FastRunOff();
    void FastRun(struct usercmd_s *cmd);

private:
    bool bGroundStrafe = false;
    bool bFastRun = false;
};
extern CMisc g_Misc;