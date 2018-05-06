class CSystems;
class CSystems
{
public:
	void KnifeBot(struct usercmd_s *cmd);
	void AutoStrafe(struct usercmd_s *cmd);
    void StandUpGroundStrafe(struct usercmd_s *cmd);
	void BunnyHop(struct usercmd_s *cmd);
    void Fastrun(struct usercmd_s *cmd);
private:
    int DetectStrafeDir(struct usercmd_s *cmd);
};
extern CSystems g_Systems;
