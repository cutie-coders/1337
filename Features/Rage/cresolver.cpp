#include "../Features.h"

Info resolverInfo[64];
Info::History resolverRecord[64];


float cResolver::GetBackwardYaw(IBasePlayer* player) {
	return Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y;
}


bool cResolver::IsBreakingLby(IBasePlayer* player)
{
	if (!player)
		return false;
	CCSGOPlayerAnimState* state = player->GetPlayerAnimState();
	float m_flNextBreakTime;
	
	//   No animstate      no previous simulationtime                      moving too fast to break lby :thinking:     // in air bruh.
	if (!state || !player->GetOldSimulationTime() || !(player->GetFlags() & FL_ONGROUND))
		return false;

	if (player->GetVelocity().Length2D() > 0.1f)
		m_flNextBreakTime = csgo->curtime + .22f;

	if (m_flNextBreakTime == csgo->curtime) {
		resolverRecord[player->EntIndex()].m_flTimeSinceLastMove += 0.1f;
		float to_ticks = TIME_TO_TICKS(resolverRecord[player->EntIndex()].m_flTimeSinceLastMove);
		resolverRecord[player->EntIndex()].m_iTicksSinceLastMove = to_ticks;
		resolverRecord[player->EntIndex()].m_bIsBreakingLby = true; return true;
	}

	resolverRecord[player->EntIndex()].m_flLastLowerBodyYaw = state->m_abs_yaw; // lmfao
	resolverInfo[player->EntIndex()].m_flEyeYaw = state->m_eye_yaw;

	auto layer = player->GetAnimOverlays();
	if (layer[3].m_flWeight == 0.1f && layer[3].m_flPlaybackRate == 0.1f) {
		resolverInfo[player->EntIndex()].m_iDesyncType = 0; // extended
		resolverInfo[player->EntIndex()].m_iSide = 1; // left side
		resolverRecord[player->EntIndex()].m_bIsBreakingLby = true;
		return true;
	}
	else if (layer[6].m_flWeight == 0.1f && layer[6].m_flPlaybackRate == 0.1f) {
		resolverInfo[player->EntIndex()].m_iDesyncType = 0; // extended.
		resolverInfo[player->EntIndex()].m_iSide = -1; // right side
		resolverRecord[player->EntIndex()].m_bIsBreakingLby = true;
		return true;
	}
	else
	{
		resolverInfo[player->EntIndex()].m_iDesyncType = 1; // non extended.
		resolverInfo[player->EntIndex()].m_iSide = 0; // middle side
		resolverRecord[player->EntIndex()].m_bIsBreakingLby = false;
	}
	return false; // all checks failed.
}


void cResolver::Run(IBasePlayer* player)
{

	IsBreakingLby(player); // run it :sungl:

#pragma region vars
	int idx = player->EntIndex();
	int side = resolverInfo[idx].m_iSide;
	int missed = csgo->actual_misses[idx];
	auto state = player->GetPlayerAnimState();
	float lby = resolverInfo[idx].m_flLowerBodyYaw;
	float angle = 0.f;
	float eye_yaw = player->GetEyeAngles().y;
#pragma endregion

#pragma region main-code

	if (player->GetWeapon()->GetLastShotTime() == player->GetSimulationTime() || player->GetWeapon()->LastShotTime() == player->GetSimulationTime())
	{
		lby = eye_yaw; // just set it to eye yaw since they shot and eye yaw will be last angle :sunglasses:
		resolverInfo[idx].m_iDesyncType = 10; //onshot   | CHANGE ME AFTER!
		return;
	}

	if (player->GetVelocity().Length2D() <= 0.1f)
	{
		if (resolverRecord[idx].m_bIsBreakingLby)
		{
			resolverInfo[idx].m_iDesyncType = 1; // breaking lby
			lby = eye_yaw + 58.f * side; // will be max * -1 / 1 so it will be left / right
			resolverRecord[idx].m_bIsBreakingLby = false; // set it to false so the breaker code can run again and make sure that they are 100% breaking
		}
		else
		{
			resolverInfo[idx].m_iDesyncType = 2; // not breaking but not moving
			lby = eye_yaw + 40.f * side;
			resolverRecord[idx].m_bIsBreakingLby - false; // set it anyway just incase.
		}
	}
	else if (player->GetVelocity().Length2D() > 10.f /*uhm im not sure, nobody slowwalks under 10 speed right??*/)
	{
		if (resolverRecord[idx].m_bIsBreakingLby)
		{
			resolverInfo[idx].m_iDesyncType = 3; // slowwalk breaking lby
			lby = eye_yaw + 38.f * side;
			resolverRecord[idx].m_bIsBreakingLby = false;
		}
		else
		{
			resolverInfo[idx].m_iDesyncType = 4; // slowwalk non lby
			lby = eye_yaw + 30.f * side;
			resolverRecord[idx].m_bIsBreakingLby = false; // again set it to false so the breaker check runs.
		}
	}

#pragma endregion
}