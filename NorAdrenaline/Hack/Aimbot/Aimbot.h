class CAimBot;
class CAimBot
{
public:
	void Run(struct usercmd_s *cmd);

    unsigned int currentTargetIndex;
private:
	void Aimbot(struct usercmd_s *cmd);
};
extern CAimBot g_AimBot;