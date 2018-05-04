class CAimBot;
class CAimBot
{
public:
	void Run(struct usercmd_s *cmd);

	DWORD dwReactionTime;

	float m_flCurrentFOV;

	bool TriggerKeyStatus;

    unsigned int choked = 0;
private:
	void Aimbot(struct usercmd_s *cmd);
};
extern CAimBot g_AimBot;

void SmoothAimAngles(QAngle MyViewAngles, QAngle AimAngles, QAngle &OutAngles, float Smoothing, bool bSpiral, float SpiralX, float SpiralY);