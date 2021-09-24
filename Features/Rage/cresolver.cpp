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
	
	//   No animstate      no previous simulationtime                      moving too fast to break lby :thinking:     // in air bruh.
	if (!state || !player->GetOldSimulationTime() || player->GetVelocity().Length2D() > 0.1f || !(player->GetFlags() & FL_ONGROUND))
		return false;

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
		resolverInfo[player->EntIndex()].m_iSide = 0; // right side
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
#pragma endregion

#pragma region main-code

	if (player->GetPlayerInfo().fakeplayer)
		return; // dont resolve bots for fuck sake

	if (player->GetEyeAngles().y == 0)
		return;

	if (!(player->GetFlags() & FL_ONGROUND))
	{
		if (player->GetVelocity().Length2D() > 0.1f && player->GetVelocity().Length2D() < 70.f) // scuffed but should work?
			lby = state->m_eye_yaw * side; // if they're in air they probably dont have a lot of desync so no point trying to resolve them ykyk
		else
			lby = resolverInfo[idx].m_flEyeYaw + 13.f * side;
		return; // lolw
	}

	if (player->GetVelocity().Length2D() > 0.1f && player->GetVelocity().Length2D() < 75.f)
	{
		if (player->GetVelocity().Length2D() >= 30.f)
			lby = (resolverInfo[idx].m_flEyeYaw + 30.f) * side;
		else
			lby = (resolverInfo[idx].m_flEyeYaw + 45.f) * side;
		return;
	}

	switch (resolverInfo[idx].m_iDesyncType)
	{
	case 0: // extended
		angle = 58.f;
		break;
	case 1: // non extended	
		angle = 30.f; // idfk
		break;
	case 2: // sway
		if (resolverRecord[idx].m_flLastLowerBodyYaw != resolverInfo[idx].m_flLowerBodyYaw)
			angle = (resolverInfo[idx].m_flLowerBodyYaw + state->m_eye_yaw) - (resolverRecord[idx].m_flLastLowerBodyYaw + state->m_eye_yaw);
		else
			angle = resolverRecord[idx].m_flLastLowerBodyYaw;
		break;
	}

	if (player->GetVelocity().Length2D() > 0.1f && resolverRecord[idx].m_flLastLowerBodyYaw != resolverInfo[idx].m_flLowerBodyYaw)
		resolverInfo[idx].m_iDesyncType = 2;
	if (side > 0)
	{
		switch (missed % 5) // before you talk shit, no its not a fucking bruteforce resolver, ist just easier to keep track of players we're resolving by keeping it inside the switch case.
		{
		case 0:
		case 1:
			lby = state->m_eye_yaw + angle * side;
			break;
		case 2: // after 2 misses we will just bruteforce them.
			lby = state->m_eye_yaw + 35.f * side;
			break;
		case 3:
			side = !side;
			lby = state->m_eye_yaw + 25.f * side;
			break;
		case 4:
			lby = state->m_eye_yaw + 45.f * side;
			break;
		}
	}
	else
		lby = state->m_eye_yaw; // center apparently :shrug:

#pragma endregion
}