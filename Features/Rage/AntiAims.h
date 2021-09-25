#pragma once
#define TICK_INTERVAL            ( interfaces.global_vars->interval_per_tick )
#define TIME_TO_TICKS( dt )        ( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )        ( TICK_INTERVAL *( t ) )

class CAntiAim
{
private:
	void Yaw(bool legit_aa);
	void Pitch(bool legit_aa);
	bool Currently_Breaking;
	bool Pre_Breaking;
	float next_break;
public:
	bool override_off_yaw;
	bool override_off_pitch;
	float body_lean;
	bool ShouldAA();
	void Initialize();
	void Fakelag();
	void Sidemove();
	void Run();
	void is_lby_update();
};

extern bool CanExploit();
extern bool CanHS();