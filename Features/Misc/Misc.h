#pragma once
#include "../../Hooks/hooks.h"

enum PeekSide {
	PNONE = 0,
	PLEFT = -1,
	PRIGHT = 1,
	PALL = 2
};

class CMisc {
public:
	//void UpdatePeek();
	bool in_doubletap = false;
	bool double_tap_enabled = false;
	bool recharging_double_tap = false;
	bool double_tap_key = false;
	bool hide_shots_enabled = false;
	bool hide_shots_key = false;
	bool hs_works = false;
	int dt_bullets = 0;
	bool hs_shot = false;
	bool IsChatOpened();
	void CopyCommand(CUserCmd* cmd, int tickbase_shift);
	bool Doubletap();
	bool IsFinalTick();
	void Hideshots();
	void PreverseKillFeed(bool roundStart);
	void FixMovement(CUserCmd* cmd, Vector& ang);
	void FixMouseInput();
	void SlideWalk();
	void UpdateDormantTime();
	void Clantag();
	void ProcessMissedShots();
	void FakeDuck();
	void StoreTaserRange();
	void Ragdolls();
};