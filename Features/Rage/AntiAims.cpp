
#include "../../Hooks/hooks.h"
#include "../Features.h"

bool CanExploit() {
	return (
		((g_Binds[bind_double_tap].active || g_Binds[bind_hide_shots].active) && csgo->skip_ticks >= 16)
		/*|| (g_Binds[bind_hide_shots].active && csgo->skip_ticks >= 8)*/)
		&& !csgo->need_to_recharge;
}

void CAntiAim::is_lby_update()
{
	static float m_nTickBase;
	m_nTickBase = csgo->local->GetTickBase();
	float curtime = TICKS_TO_TIME(m_nTickBase);
	static float UpdateTime;

	Currently_Breaking = false;

	if (!(csgo->local->GetFlags() & FL_ONGROUND)) {
		return;
	}

	if (csgo->local->GetVelocity().Length() > 0.1f) {
		UpdateTime = curtime + 0.22f;
	}
	if (UpdateTime < curtime) {
		UpdateTime = curtime + 1.1f;
		Currently_Breaking = true;
	}


}

IBasePlayer* GetNearestTarget(bool check = false)
{
	int y, x;
	y = csgo->h / 2;
	x = csgo->w / 2;

	IBasePlayer* best_ent = nullptr;
	float best_dist = FLT_MAX;

	for (int i = 1; i < 65; i++)
	{
		auto ent = interfaces.ent_list->GetClientEntity(i);
		if (!ent)
			continue;
		if (
			!ent->isAlive()
			|| !ent->IsPlayer()
			|| ent == csgo->local
			|| ent->GetTeam() == csgo->local->GetTeam()
			|| ent->IsDormant()
			|| ent->DormantWrapped())
			continue;

		Vector origin_2d;
		if (!Math::WorldToScreen(ent->GetOrigin(), origin_2d) && check)
			continue;

		float dist = Vector(x, y, 0).DistTo(origin_2d);
		if (dist < best_dist)
		{
			best_ent = ent;
			best_dist = dist;
		}
	}

	if (best_ent)
		return best_ent;

	return nullptr;
}

bool CanTriggerFakeLag() {

	const bool disable_fakelag_on_exploit = []() {
		return g_Binds[bind_double_tap].active || g_Binds[bind_hide_shots].active;
	}();


	if (disable_fakelag_on_exploit || vars.antiaim.fakelag.mode == 0)
		return false;

	return true;
}

void CAntiAim::Fakelag()
{
	if (!vars.antiaim.enable)
		return;

	if (vars.antiaim.fakelag.min == 0)
		return;

	if (csgo->game_rules->IsFreezeTime()
		|| csgo->local->HasGunGameImmunity()
		|| csgo->local->GetFlags() & FL_FROZEN)
		return;

	if (csgo->fake_duck && csgo->local->GetFlags() & FL_ONGROUND && !(csgo->cmd->buttons & IN_JUMP))
	{
		if (csgo->local->GetFlags() & FL_ONGROUND)
			return;
	}

	static Vector origin = Vector();

	if (!CanTriggerFakeLag()) {
		csgo->send_packet = csgo->client_state->iChokedCommands >= 1;
		csgo->max_fakelag_choke = 1;
	}
	else {

		csgo->max_fakelag_choke = clamp(vars.antiaim.fakelag.min, 1,
			((vars.misc.restrict_type == 0 || csgo->game_rules->IsValveDS()) ? 6 : 14));

	
		csgo->send_packet = false;

		if(vars.antiaim.fakelag.randomization != 0)
			csgo->max_fakelag_choke -= clamp(rand() % vars.antiaim.fakelag.randomization,0, csgo->max_fakelag_choke);

		

		if (vars.antiaim.fakelag.mode == 1) {

			if (csgo->client_state->iChokedCommands >= csgo->max_fakelag_choke)
				csgo->send_packet = true;

		}
		else if (vars.antiaim.fakelag.mode == 2) {
			auto tick_to_choke = std::min< int >(static_cast<int>(std::ceilf(64.f / (csgo->local->GetVelocity().Length2D() * interfaces.global_vars->interval_per_tick))), 16);

			csgo->send_packet = csgo->client_state->iChokedCommands >= tick_to_choke;
		}
		else if (vars.antiaim.fakelag.mode == 3) {
			csgo->send_packet = true;
			float diff = (csgo->local->GetAbsOrigin() - origin).LengthSqr();
			if (diff <= 4096.f)
				csgo->send_packet = false;
		}
	}

	if (csgo->send_packet)
		origin = csgo->local->GetAbsOrigin();
}

void CAntiAim::Pitch(bool legit_aa)
{
	if (legit_aa)
		return;

	auto state = csgo->local->GetPlayerAnimState();
	if (!state)
		return;

	switch (vars.antiaim.pitch)
	{
	case 1:
		csgo->cmd->viewangles.x = 89.f;
		break;
	case 2:
		csgo->cmd->viewangles.x = -89.f;
		break;
	}
}

void CAntiAim::Sidemove() {
	if (csgo->weapon->GetItemDefinitionIndex() != weapon_revolver) {
		if (g_Binds[bind_double_tap].active && csgo->skip_ticks > 0 && csgo->cmd->buttons & IN_ATTACK || csgo->game_rules->IsFreezeTime())
			return;
	}
	if (csgo->local->GetMoveType() == MoveType_t::MOVETYPE_NOCLIP || csgo->local->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
		return;
	if (!csgo->should_sidemove)
		return;

	const float& sideAmount = csgo->cmd->buttons & IN_DUCK || csgo->fake_duck ? 3.25f : 1.01f;
	if (csgo->local->GetVelocity().Length2D() <= 0.f || std::fabs(csgo->local->GetVelocity().z) <= 100.f)
		csgo->cmd->sidemove += csgo->cmd->command_number % 2 ? sideAmount : -sideAmount;
}

void CAntiAim::Yaw(bool legit_aa)
{
	if (!vars.antiaim.enable)
		return;
	bool check = (g_Binds[bind_manual_back].active
		|| g_Binds[bind_manual_right].active
		|| g_Binds[bind_manual_left].active
		|| g_Binds[bind_manual_forward].active);


	is_lby_update();

	static bool Jitter = false;
	static int Spin = 0;


	csgo->should_sidemove = vars.antiaim.lbytarget != 0;
	/*
	if (g_Binds[bind_fakepeek].active) {
		if (csgo->local->GetVelocity().Length2D() >= 7.f)
			return;
	
		csgo->cmd->viewangles.y += 180;
		if (Jitter)
			csgo->cmd->viewangles.y -= 180;

		if (Currently_Breaking) {
			csgo->send_packet = false;
			csgo->cmd->viewangles.y -= 116 * -(Jitter ? 1 : - 1);
			
			csgo->cl_move_shift = 16;
		
			
		}
		else if (!csgo->send_packet) {
			csgo->cmd->viewangles.y -= 116 * (Jitter ? 1 : -1);
		}

	


		goto AAEND;
	}*/


	int CurSide = vars.antiaim.desync == 1 ? (g_Binds[bind_aa_inverter].active ? -1 : 1) : (Jitter ? 1 : -1);

	if (vars.antiaim.desync == 3) {
		CurSide = g_Binds[bind_aa_inverter].active ? -1 : 1;



		if (csgo->need_to_recharge)
			CurSide = Jitter;

		if (legit_aa)
			CurSide = Jitter;

		if (csgo->Peekingg)
			CurSide = Jitter;

	
	}
	else if (vars.antiaim.desync == 4) {
		CurSide = (rand() % 2 == 1) ? -1 : 1;
	}
	/*
	static int FreeStandSide;
	
	switch (vars.antiaim.freestand) {
	case 1:
		if (csgo->PeekSide != PNONE || csgo->PeekSide != PALL) {
			FreeStandSide = csgo->PeekSide == PLEFT ? 1 : -1;
		}
		if (csgo->PeekSide != PNONE) {
			CurSide = FreeStandSide;
		}
		break; //fr
	case 2:
		if (csgo->PeekSide != PNONE || csgo->PeekSide != PALL) {
			FreeStandSide = csgo->PeekSide == PLEFT ? -1 : 1;
		}
		if (csgo->PeekSide != PNONE) {
			CurSide = FreeStandSide;
		}
		break; //afr
	case 3:
		if (csgo->Peekingg) {
			CurSide = Jitter;
		}
		break; //ajit
	}
	*/
	if (legit_aa)
		CurSide *= -1;


	float FinalDesync = 0.f;

	static float LBYSway = 0.f;
	static bool LBYSwayDir = false;
	switch (vars.antiaim.lbytarget) {
	case 0:
		FinalDesync = vars.antiaim.delta;
		break;
	case 1:
		FinalDesync = vars.antiaim.delta * 2;
		break;
	case 2:
		if ((vars.antiaim.delta * 2) != 0) {
			FinalDesync = rand() % ((int)(vars.antiaim.delta * 2));
		}
		break;
	}

	if (Currently_Breaking) {
		csgo->send_packet = false;
		csgo->cmd->viewangles.y -= FinalDesync * -CurSide;
	}
	else if (!csgo->send_packet) {
		csgo->cmd->viewangles.y -= FinalDesync * CurSide;
	}



	if (!legit_aa) {

		if (!check) {

			switch (vars.antiaim.yaw) {
			case 1:
				auto best_ent = GetNearestTarget(false);
				if (best_ent)
					csgo->cmd->viewangles.y = Math::CalculateAngle(csgo->local->GetOrigin(), best_ent->GetOrigin()).y;
				break;
			}
			csgo->cmd->viewangles.y += vars.antiaim.yawoffset;
		}
		else {
			if (vars.antiaim.manual_antiaim) {
				if (g_Binds[bind_manual_forward].active)
					csgo->cmd->viewangles.y += legit_aa ? -180.f : 180.f;
				if (g_Binds[bind_manual_left].active)
					csgo->cmd->viewangles.y += legit_aa ? -90.f : 90.f;
				if (g_Binds[bind_manual_right].active)
					csgo->cmd->viewangles.y -= legit_aa ? -90.f : 90.f;
			}
		}

		switch (vars.antiaim.modifier) {
		case 1:
			csgo->cmd->viewangles.y += (Jitter ? vars.antiaim.modifieroffset : 0) * CurSide;
			break;
		case 2:
			csgo->cmd->viewangles.y += Jitter ? -vars.antiaim.modifieroffset : vars.antiaim.modifieroffset;
			break;

		case 3:
			Spin += vars.antiaim.modifieroffset;
			if (Spin > 360)
				Spin -= 360;

			csgo->cmd->viewangles.y += Spin;
			break;

		case 4:
			if(vars.antiaim.modifieroffset != 0) 
				csgo->cmd->viewangles.y += (rand() % vars.antiaim.modifieroffset * 2) - vars.antiaim.modifieroffset;
			break;
		}
	}

	AAEND:

	
	csgo->cmd->viewangles.y = Math::NormalizeYaw(csgo->cmd->viewangles.y);




	

	if (csgo->send_packet)
		Jitter = !Jitter;

}
bool CAntiAim::ShouldAA()
{


	if (csgo->local->GetFlags() & FL_FROZEN)
		return false;

	if (!vars.antiaim.enable)
		return false;

	if (csgo->local->GetMoveType() == MOVETYPE_NOCLIP
		|| csgo->local->GetMoveType() == MOVETYPE_LADDER)
		return false;

	if (csgo->TickShifted)
		return false;


	if (F::Shooting() || (CanExploit() && csgo->cmd->buttons & IN_ATTACK))
		return false;

	return true;
}

void CAntiAim::Initialize()
{
	override_off_yaw = false;
	override_off_pitch = false;
	body_lean = 120.f;
}

void CAntiAim::Run()
{
	if (g_Binds[bind_slow_walk].active || csgo->should_stop_slide)
	{
		const auto weapon = csgo->weapon;
		const auto info = csgo->weapon->GetCSWpnData();
		if (!weapon)
			return;

		if (!info)
			return;

		float speed = csgo->local->IsScoped() ? info->m_flMaxSpeedAlt : info->m_flMaxSpeed;

		speed /= 3.4f;
		float min_speed = (float)(sqrt(pow(csgo->cmd->forwardmove, 2) + pow(csgo->cmd->sidemove, 2) + pow(csgo->cmd->upmove, 2)));

		if (min_speed > 0.f)
		{
			float ratio = speed / min_speed;
			csgo->cmd->forwardmove *= ratio;
			csgo->cmd->sidemove *= ratio;
			csgo->cmd->upmove *= ratio;
		}


		csgo->should_stop_slide = false;
	}
	bool use_aa_on_e = csgo->cmd->buttons & IN_USE;

	override_off_pitch = (csgo->cmd->buttons & IN_ATTACK) || g_Ragebot->shot;
	override_off_yaw = override_off_pitch;

	if (ShouldAA())
	{
		if(!use_aa_on_e)
			Pitch(false);
	
		Yaw(use_aa_on_e);
	}
}
