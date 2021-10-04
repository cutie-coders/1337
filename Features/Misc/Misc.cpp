#include "../Features.h"

void CMisc::PreverseKillFeed(bool roundStart)
{
	static auto hud_ptr = *(DWORD**)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08")) + 1);

	static auto find_hud_element =
		reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
			str("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28")));

	if (!find_hud_element || !hud_ptr)
		return;

	static auto nextUpdate = 0.0f;

	if (roundStart || !vars.visuals.preverse_killfeed) {
		nextUpdate = interfaces.global_vars->realtime + 10.0f;
		return;
	}

	if (nextUpdate > interfaces.global_vars->realtime)
		return;

	nextUpdate = interfaces.global_vars->realtime + 2.0f;

	const auto deathNotice = std::uintptr_t(find_hud_element(hud_ptr, str("CCSGO_HudDeathNotice")));
	if (!deathNotice)
		return;

	const auto deathNoticePanel = (*(UIPanel**)(*reinterpret_cast<std::uintptr_t*>(deathNotice - 20 + 88) + sizeof(std::uintptr_t)));

	const auto childPanelCount = deathNoticePanel->GetChildCount();

	for (int i = 0; i < childPanelCount; ++i) {
		const auto child = deathNoticePanel->GetChild(i);
		if (!child)
			continue;

		if (child->HasClass(str("DeathNotice_Killer")))
			child->SetAttributeFloat(str("SpawnTime"), interfaces.global_vars->curtime);
	}
}

void CMisc::FixMovement(CUserCmd* cmd, Vector& ang) {

	static auto ResetKeys = []() -> void {
		csgo->cmd->buttons &= ~IN_RIGHT;
		csgo->cmd->buttons &= ~IN_MOVERIGHT;
		csgo->cmd->buttons &= ~IN_LEFT;
		csgo->cmd->buttons &= ~IN_MOVELEFT;
		csgo->cmd->buttons &= ~IN_FORWARD;
		csgo->cmd->buttons &= ~IN_BACK;
	};

	static auto QuickStop = [](CUserCmd* cmd) -> void {
		auto wpn_info = csgo->weapon->GetCSWpnData();
		bool pressed_move_key = cmd->buttons & IN_FORWARD || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_BACK || cmd->buttons & IN_MOVERIGHT || cmd->buttons & IN_JUMP;
		if (!wpn_info)
			return;
		if (pressed_move_key || !(csgo->local->GetFlags() & FL_ONGROUND))
			return;
		auto velocity = csgo->local->GetVelocity();
		float speed = velocity.Length2D();

		if (speed > 15.f) {
			Vector direction;
			Math::VectorAngles(velocity, direction);
			direction.y = csgo->original.y - direction.y;
			Vector forward;
			Math::AngleVectors(direction, forward);
			static const auto cl_sidespeed = interfaces.cvars->FindVar("cl_forwardspeed");
			Vector negated_direction = forward * -(cl_sidespeed->GetFloat());

			csgo->cmd->forwardmove = negated_direction.x;


			csgo->cmd->sidemove = negated_direction.y;

		}
	};

	QuickStop(csgo->cmd);
	g_AutoPeek->Run();
	Vector real_viewangles;
	interfaces.engine->GetViewAngles(real_viewangles);

	Vector vecMove(csgo->cmd->forwardmove, csgo->cmd->sidemove, csgo->cmd->upmove);
	float speed = sqrt(vecMove.x * vecMove.x + vecMove.y * vecMove.y);

	Vector angMove;
	Math::VectorAngles(vecMove, angMove);

	float yaw = DEG2RAD(csgo->cmd->viewangles.y - ang.y + angMove.y);

	csgo->cmd->forwardmove = cos(yaw) * speed;
	csgo->cmd->sidemove = sin(yaw) * speed;



	ResetKeys();



	if (csgo->cmd->forwardmove > 0.f)
		csgo->cmd->buttons |= IN_FORWARD;
	else if (csgo->cmd->forwardmove < 0.f)
		csgo->cmd->buttons |= IN_BACK;

	if (csgo->cmd->sidemove > 0.f)
	{
		csgo->cmd->buttons |= IN_RIGHT;
		csgo->cmd->buttons |= IN_MOVERIGHT;
	}
	else if (csgo->cmd->sidemove < 0.f)
	{
		csgo->cmd->buttons |= IN_LEFT;
		csgo->cmd->buttons |= IN_MOVELEFT;
	}



	if (!(csgo->local->GetFlags() & FL_ONGROUND) || vars.misc.slidewalk == 0)
		return;

	static bool PSwitch;
	if (csgo->send_packet)
		PSwitch = !PSwitch;

	bool SL = vars.misc.slidewalk == 1 ? true : vars.misc.slidewalk == 2 ? csgo->LegSwitch : vars.misc.slidewalk == 3 ? csgo->LegSwitch : false;


	if (g_AntiAim->ShouldAA() && SL)
	{
		if (cmd->forwardmove > 0.0f)
		{
			cmd->buttons |= IN_BACK;
			cmd->buttons &= ~IN_FORWARD;
		}
		else if (cmd->forwardmove < 0.0f)
		{
			cmd->buttons |= IN_FORWARD;
			cmd->buttons &= ~IN_BACK;
		}

		if (cmd->sidemove > 0.0f)
		{
			cmd->buttons |= IN_MOVELEFT;
			cmd->buttons &= ~IN_MOVERIGHT;
		}
		else if (cmd->sidemove < 0.0f)
		{
			cmd->buttons |= IN_MOVERIGHT;
			cmd->buttons &= ~IN_MOVELEFT;
		}
	}
	else
	{
		auto buttons = cmd->buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

		if (SL)
		{
			if (cmd->forwardmove <= 0.0f)
				buttons |= IN_BACK;
			else
				buttons |= IN_FORWARD;

			if (cmd->sidemove > 0.0f)
				goto LABEL_15;
			else if (cmd->sidemove >= 0.0f)
				goto LABEL_18;

			goto LABEL_17;
		}
		else
			goto LABEL_18;

		if (cmd->forwardmove <= 0.0f) //-V779
			buttons |= IN_FORWARD;
		else
			buttons |= IN_BACK;

		if (cmd->sidemove > 0.0f)
		{
		LABEL_17:
			buttons |= IN_MOVELEFT;
			goto LABEL_18;
		}

		if (cmd->sidemove < 0.0f)
			LABEL_15:

		buttons |= IN_MOVERIGHT;

	LABEL_18:
		cmd->buttons = buttons;
	}
	
}

void CMisc::FixMouseInput() 
{
	static Vector delta_viewangles{ };
	Vector delta = csgo->cmd->viewangles - delta_viewangles;
	delta.Normalize();

	static ConVar* sensitivity = interfaces.cvars->FindVar(str("sensitivity"));

	if (!sensitivity)
		return;

	if (delta.x != 0.f) {
		static ConVar* m_pitch = interfaces.cvars->FindVar(str("m_pitch"));

		if (!m_pitch)
			return;

		int final_dy = static_cast<int>((delta.x / m_pitch->GetFloat()) / sensitivity->GetFloat());
		if (final_dy <= 32767) {
			if (final_dy >= -32768) {
				if (final_dy >= 1 || final_dy < 0) {
					if (final_dy <= -1 || final_dy > 0)
						final_dy = final_dy;
					else
						final_dy = -1;
				}
				else {
					final_dy = 1;
				}
			}
			else {
				final_dy = 32768;
			}
		}
		else {
			final_dy = 32767;
		}

		csgo->cmd->mousedy = static_cast<short>(final_dy);
	}

	if (delta.y != 0.f) {
		static ConVar* m_yaw = interfaces.cvars->FindVar(str("m_yaw"));

		if (!m_yaw)
			return;

		int final_dx = static_cast<int>((delta.y / m_yaw->GetFloat()) / sensitivity->GetFloat());
		if (final_dx <= 32767) {
			if (final_dx >= -32768) {
				if (final_dx >= 1 || final_dx < 0) {
					if (final_dx <= -1 || final_dx > 0)
						final_dx = final_dx;
					else
						final_dx = -1;
				}
				else {
					final_dx = 1;
				}
			}
			else {
				final_dx = 32768;
			}
		}
		else {
			final_dx = 32767;
		}

		csgo->cmd->mousedx = static_cast<short>(final_dx);
	}

	delta_viewangles = csgo->cmd->viewangles;
}

void CMisc::SlideWalk()
{
}

bool CMisc::IsChatOpened() {
	if (!csgo->local)
		return false;

	static auto hud_ptr = *(DWORD**)(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), 
		str("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08")) + 1);

	static auto find_hud_element = 
		reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(csgo->Utils.FindPatternIDA(GetModuleHandleA(g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()), 
			str("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28")));

	if (!find_hud_element || !hud_ptr)
		return false;

	auto chat = find_hud_element(hud_ptr, str("CCSGO_HudChat"));
	if (!chat)
		return false;

	bool is_in_chat = *(bool*)((DWORD)chat + 0x58);

	if (is_in_chat)
		return true;

	return false;
}

void CopyCommand2(CUserCmd* cmd, int tickbase_shift)
{







	auto commands_to_add = 0;

	do
	{
		auto sequence_number = commands_to_add + cmd->command_number;

		auto command = interfaces.input->GetUserCmd(sequence_number);
		auto verified_command = interfaces.input->GetVerifiedUserCmd(sequence_number);

		memcpy(command, cmd, sizeof(CUserCmd));

		if (command->tick_count != INT_MAX && csgo->client_state->iDeltaTick > 0)
			interfaces.prediction->Update(
				csgo->client_state->iDeltaTick, true,
				csgo->client_state->nLastCommandAck,
				csgo->client_state->nLastOutgoingCommand + csgo->client_state->iChokedCommands
			);

		command->command_number = sequence_number;
		command->hasbeenpredicted = command->tick_count != INT_MAX;

		++csgo->client_state->iChokedCommands;

		if (csgo->client_state->pNetChannel)
		{
			++csgo->client_state->pNetChannel->iChokedPackets;
			++csgo->client_state->pNetChannel->iOutSequenceNr;
		}

		command->viewangles = Math::normalize(command->viewangles);

		memcpy(&verified_command->m_cmd, command, sizeof(CUserCmd));
		verified_command->m_crc = command->GetChecksum();

		++commands_to_add;
	} while (commands_to_add != tickbase_shift);

	interfaces.prediction->PreviousAckHadErrors = true;
	interfaces.prediction->CommandsPredicted = 0;

}

void CMisc::CopyCommand(CUserCmd* cmd, int tickbase_shift)
{
	static auto cl_sidespeed = interfaces.cvars->FindVar(str("cl_sidespeed"));

	if (g_Binds[bind_peek_assist].active) {
		g_AutoPeek->has_shot = true;
		g_AutoPeek->GotoStart(csgo->cmd);
	}
	else if (!vars.ragebot.dt_teleport) {
		cmd->forwardmove = 0.0f;
		cmd->sidemove = 0.0f;
	}

	auto commands_to_add = 0;

	do
	{
		auto sequence_number = commands_to_add + cmd->command_number;

		auto command = interfaces.input->GetUserCmd(sequence_number);
		auto verified_command = interfaces.input->GetVerifiedUserCmd(sequence_number);

		memcpy(command, cmd, sizeof(CUserCmd));

		if (command->tick_count != INT_MAX && csgo->client_state->iDeltaTick > 0)
			interfaces.prediction->Update(
				csgo->client_state->iDeltaTick, true,
				csgo->client_state->nLastCommandAck,
				csgo->client_state->nLastOutgoingCommand + csgo->client_state->iChokedCommands
			);

		command->command_number = sequence_number;
		command->hasbeenpredicted = command->tick_count != INT_MAX;

		++csgo->client_state->iChokedCommands;

		if (csgo->client_state->pNetChannel)
		{
			++csgo->client_state->pNetChannel->iChokedPackets;
			++csgo->client_state->pNetChannel->iOutSequenceNr;
		}

		command->viewangles = Math::normalize(command->viewangles);

		memcpy(&verified_command->m_cmd, command, sizeof(CUserCmd));
		verified_command->m_crc = command->GetChecksum();

		++commands_to_add;
	} while (commands_to_add != tickbase_shift);

	interfaces.prediction->PreviousAckHadErrors = true;
	interfaces.prediction->CommandsPredicted = 0;

}



__forceinline void ResetValue()
{
	csgo->skip_ticks = 0;
	csgo->shift_amount = 0;
}
//#define LOCALPEEKHITBOXSIZE 60
//void CMisc::UpdatePeek() {
//
//	if (!csgo->local->isAlive()) {
//		csgo->Peekingg = false;
//		csgo->PeekSide = PNONE;
//		return;
//	}
//
//
//
//	if (csgo->Peekingg) {
//		csgo->PeekSide = PALL;
//		return;
//	}
//
//
//
//	IBasePlayer* target;
//	if (csgo->rage_target)
//		target = csgo->rage_target;
//
//	auto best_fov = INT_MAX;
//	Vector localviewangles;
//	interfaces.engine->GetViewAngles(localviewangles);
//
//	if (!target) {
//
//		for (auto i = 1; i < interfaces.global_vars->maxClients; i++)
//		{
//			auto e = interfaces.ent_list->GetClientEntity(i);
//
//			if (!e->IsValid() || e->IsDormant() || !e->isAlive())
//				continue;
//
//
//
//
//			auto fov = Math::GetFov(localviewangles, Math::CalculateAngle(csgo->local->GetEyePosition(), e->GetOrigin()));
//
//			if (fov < best_fov)
//			{
//				best_fov = fov;
//				target = e;
//			}
//		}
//	}
//
//
//
//
//
//	if (!target) {
//		csgo->Peekingg = false;
//		csgo->PeekSide = PNONE;
//		return;
//	}
//
//	float RelativeEAngle = Math::CalculateAngle(target->GetOrigin(), csgo->local->GetEyePosition()).y + 180;
//	float LocalRelativeAngle = Math::CalculateAngle(csgo->local->GetEyePosition(), target->GetOrigin()).y + 180;
//
//	//setup Vectors
//	Vector ETraceL;
//	Vector ETraceR;
//	Vector LTraceL;
//	Vector LTraceR;
//	Vector PStart = Vector(target->GetOrigin().x, target->GetOrigin().y, (target->GetOrigin().z + target->GetEyePosition().z) / 2);
//	Vector PStartL = Vector(csgo->local->GetOrigin().x, csgo->local->GetOrigin().y, (csgo->local->GetOrigin().z + csgo->local->GetEyePosition().z) / 2);;
//
//	Math::AngleVectors(Vector(0, RelativeEAngle - 90, 0), ETraceL);
//	Math::AngleVectors(Vector(0, RelativeEAngle + 90, 0), ETraceR);
//
//	Math::AngleVectors(Vector(0, LocalRelativeAngle - 90, 0), LTraceL);
//	Math::AngleVectors(Vector(0, LocalRelativeAngle + 90, 0), LTraceR);
//
//	trace_t Left;
//	trace_t Right;
//
//	g_AutoWall->TraceLine(PStartL + LTraceL * LOCALPEEKHITBOXSIZE, PStart + ETraceL * LOCALPEEKHITBOXSIZE, MASK_SOLID, csgo->local, &Left);
//	g_AutoWall->TraceLine(PStartL + LTraceR * LOCALPEEKHITBOXSIZE, PStart + ETraceR * LOCALPEEKHITBOXSIZE, MASK_SOLID, csgo->local, &Right);
//	bool left = Left.fraction != 1.0f;
//	bool right = Right.fraction != 1.0f;
//	if (!left && !right) { 
//		csgo->Peekingg = false;
//		csgo->PeekSide = PNONE;
//	}
//	else if (left && !right) { //left hit rigth not hit
//		csgo->Peekingg = true;
//		csgo->PeekSide = PRIGHT;
//	}
//	else if (!left && right) { //left didnt hit but right hit
//		csgo->Peekingg = true;
//		csgo->PeekSide = PLEFT;
//	}
//	else if (left && right) {
//		csgo->Peekingg = true;
//		csgo->PeekSide = PALL;
//	}
//	csgo->Peekingg = false;
//	csgo->PeekSide = PNONE;
//	
//}

bool CMisc::Doubletap()
{
	

	static int LastShot = 0;
	static bool Recharged = false;
	auto max_tickbase_shift = (vars.ragebot.more_ticks && csgo->weapon->isSniper()) ? vars.ragebot.dt_tickammount : csgo->weapon->GetMaxTickbaseShift();
	if (!vars.ragebot.enable || g_Binds[bind_fakepeek].active || !g_Binds[bind_double_tap].active || (csgo->game_rules->IsFreezeTime() || csgo->local->HasGunGameImmunity() || csgo->local->GetFlags() & FL_FROZEN)) {
		if (Recharged) {
			csgo->cl_move_shift = vars.ragebot.slowidealtick ? 14 : 15;
			Recharged = false;
		}
		return false;
	}
	

	if (csgo->fake_duck)
		return false;

	
	if (g_Binds[bind_peek_assist].active && !Recharged && g_AutoPeek->has_shot) {
		LastShot = csgo->fixed_tickbase;
	}

	if (!Recharged) {
		if (csgo->fixed_tickbase > LastShot + (vars.ragebot.clmove ? 42 : 33)) {
			csgo->need_to_recharge = true;
			if (vars.ragebot.clmove && !g_Binds[bind_peek_assist].active)
				csgo->morerecharge = true;
			else
				csgo->morerecharge = false;
			Recharged = true;
		}
	}



	bool can_dt =
		!csgo->weapon->IsMiscWeapon()
		&& csgo->weapon->IsGun()
		&& csgo->weapon->GetItemDefinitionIndex() != WEAPON_ZEUSX27
		&& csgo->weapon->GetItemDefinitionIndex() != WEAPON_REVOLVER && (!(csgo->weapon->IsNade()));

	bool is_firing = csgo->cmd->buttons & IN_ATTACK;

	if (is_firing && !Recharged) {
		LastShot = csgo->fixed_tickbase;
	}

	bool shoulddefensive = vars.ragebot.defensivedt;


	if (vars.ragebot.defensivething & 8) {
		if (csgo->local->GetVelocity().Length2D() <= 9.f) 
			shoulddefensive = false;
		
	}

	if (vars.ragebot.defensivething & 16) {
		if (!csgo->Peekingg)
			shoulddefensive = false;
	}

	if ((can_dt && is_firing || ((csgo->cmd->buttons & IN_ATTACK || csgo->cmd->buttons & IN_ATTACK2) && csgo->weapon->IsKnife())) && Recharged)
	{

		
		if (g_Binds[bind_peek_assist].active) {
			if (vars.ragebot.teleport) {

				g_AutoPeek->has_shot = true;
				g_AutoPeek->GotoStart(csgo->cmd);
				csgo->cl_move_shift = 14;
			
			}
			else {
				auto next_command_number = csgo->cmd->command_number + 1;
				auto user_cmd = interfaces.input->GetUserCmd(next_command_number);
				memcpy(user_cmd, csgo->cmd, sizeof(CUserCmd));
				user_cmd->command_number = next_command_number;
				CopyCommand(user_cmd, max_tickbase_shift);
			}
		}
		else {
			if (vars.ragebot.clmove) {
				csgo->cl_move_shift = max_tickbase_shift;
			}
			else {
				auto next_command_number = csgo->cmd->command_number + 1;
				auto user_cmd = interfaces.input->GetUserCmd(next_command_number);
				memcpy(user_cmd, csgo->cmd, sizeof(CUserCmd));
				user_cmd->command_number = next_command_number;
				CopyCommand(user_cmd, max_tickbase_shift);
			}
		}


		LastShot = csgo->fixed_tickbase;
		Recharged = false;
	}
	else if(shoulddefensive)
		csgo->PPShift = (vars.ragebot.defensivething & 4) ? 16 : 14;

	if(shoulddefensive && ((vars.ragebot.defensivething & 1 && g_Binds[bind_peek_assist].active) || vars.ragebot.defensivething & 2))
		csgo->PPShift = (vars.ragebot.defensivething & 4) ? 16 : 14;

	return true;
}

bool CMisc::IsFinalTick()
{
	if (!interfaces.engine->IsInGame() || !interfaces.engine->IsConnected())
		return true;

	if (!csgo->local
		|| !csgo->is_connected
		|| !csgo->is_local_alive
		|| csgo->game_rules->IsFreezeTime()
		|| csgo->local->HasGunGameImmunity()
		|| csgo->local->GetFlags() & FL_FROZEN)
		return true;

	if ((csgo->shift_amount == 1 || csgo->shift_amount == 0) && !csgo->is_shifting)
		return true;

	return false;
}

void CMisc::Hideshots() {

	if (g_Binds[bind_double_tap].active || !g_Binds[bind_hide_shots].active || !vars.ragebot.enable)
		return;

	
	
}

void CMisc::UpdateDormantTime() {
	for (int i = 1; i < 65; ++i)
	{
		auto ent = interfaces.ent_list->GetClientEntity(i);
		if (!ent)
			continue;
		if (ent->IsDormant() || !ent->isAlive() || !ent->IsPlayer()
			|| ent->EntIndex() == interfaces.engine->GetLocalPlayer()
			|| ent->GetTeam() == csgo->local->GetTeam()) {
			csgo->dormant_time[ent->GetIndex()] = -1.f;
			continue;
		}
		if (csgo->dormant_time[ent->GetIndex()] == -1.f)
			csgo->dormant_time[ent->GetIndex()] = ent->GetSimulationTime();
	}
}
#define ct(a, x) case a: SetClanTag(x, "gamesense"); break;
void CMisc::Clantag()
{
	auto SetClanTag = [](const char* tag, const char* name)
	{
		static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>((DWORD)(csgo->Utils.FindPatternIDA(GetModuleHandleA(
			g_Modules[fnva1(hs::engine_dll.s().c_str())]().c_str()),
			hs::set_clantag.s().c_str())));
		pSetClanTag(tag, name);
	};



	static bool Reset = false;

	if (vars.visuals.clantagspammer)
	{
		Reset = true;
		static int oldcurtime = 0;
		
		if (oldcurtime != (int)(interfaces.global_vars->curtime * 3.3f) % 28) {
			switch ((int)(interfaces.global_vars->curtime * 3.3f) % 28)
			{
				ct(0, " ");
				ct(1, "ga");
				ct(2, "gam");
				ct(3, "game");
				ct(4, "games");
				ct(5, "gamese");
				ct(6, "gamesen");
				ct(7, "gamesens");
				ct(8, "gamesense");
				ct(9, " gamesense ");
				ct(10, "  gamesense  ");
				ct(11, "   gamesense   ");
				ct(12, "    gamesense    ");
				ct(13, "     gamesense     ");
				ct(14, "     gamesense      ");
				ct(15, "    gamesense       ");
				ct(16, "   gamesense        ");
				ct(17, "  gamesense         ");
				ct(18, " gamesense          ");
				ct(19, "gamesense           ");
				ct(20, "amesense            ");
				ct(21, "mesense             ");
				ct(22, "esense              ");
				ct(23, "sense               ");
				ct(24, "ense                ");
				ct(25, "nse                 ");
				ct(26, "se                  ");
				ct(27, "e                   ");
			}

			oldcurtime = (int)(interfaces.global_vars->curtime * 3.3f) % 28;
		}
		
	}
	else if (Reset) {
		Reset = false;
		SetClanTag("", "");
	}
}

void CMisc::ProcessMissedShots()
{
	if (shot_snapshots.empty())
		return;

	auto& snapshot = shot_snapshots.front();
	const auto& time = TICKS_TO_TIME(csgo->fixed_tickbase);

	if (fabs(time - snapshot.time) > 1.f)
	{
		if (vars.visuals.eventlog & 8 && !snapshot.bullet_impact) 
			Msg(str("Unregistered shot"), vars.visuals.eventlog_color);
		
		shot_snapshots.erase(shot_snapshots.begin());
		return;
	}

	if (snapshot.first_processed_time != -1.f) {
		if (snapshot.damage == -1 && snapshot.weapon_fire && snapshot.bullet_impact && snapshot.record.player) {
			bool spread = false;
			bool sp_spread = false;
			if (snapshot.record.player) {
				const auto studio_model = interfaces.models.model_info->GetStudioModel(snapshot.record.player->GetModel());
				int idx = snapshot.record.player->EntIndex();

				if (studio_model)
				{
					const auto angle = Math::CalculateAngle(snapshot.start, snapshot.impact);
					Vector forward;
					Math::AngleVectors(angle, forward);
					const auto end = snapshot.impact + forward * 2000.f;
					if (!CanHitHitbox(snapshot.start, end, &snapshot.record, snapshot.hitbox, snapshot.record.bones))
						spread = true;
					if (snapshot.record.safepoints) {
						if (!CanHitHitbox(snapshot.start, end, &snapshot.record, snapshot.hitbox, snapshot.record.unresolved_bones)
							|| !CanHitHitbox(snapshot.start, end, &snapshot.record, snapshot.hitbox, snapshot.record.inversed_bones))
							sp_spread = true;
					}
				}
			}
			auto& new_hitlog = csgo->custom_hitlog.emplace_back();
			new_hitlog.name = snapshot.entity->GetName();
			new_hitlog.backtrack = snapshot.backtrack;
			new_hitlog.hitbox = snapshot.hitbox;
			new_hitlog.resolver = snapshot.resolver_mode;
			new_hitlog.spread = spread || sp_spread;

			if (!spread && !sp_spread) {
				csgo->actual_misses[snapshot.entity->GetIndex()]++;
				csgo->maxmisses[snapshot.entity->GetIndex()]++;
				g_Animfix->ResolveState[snapshot.entity->EntIndex()].LastMiss++; //reset resolve
				if (g_Animfix->ResolveState[snapshot.entity->EntIndex()].LastMiss >= 2) {
					g_Animfix->ResolveState[snapshot.entity->EntIndex()].LastMiss = 0;
					g_Animfix->ResolveState[snapshot.entity->EntIndex()].LastHit = false;
				}
			}

			if (vars.visuals.eventlog & 4) {
				// will re-add later. the logs were completely wrong. lmao. (ill just steal legendaware since they work xd)
			}

			shot_snapshots.erase(shot_snapshots.begin());
		}
	}
}

void CMisc::FakeDuck()
{
	if (vars.misc.restrict_type == 0) {
		csgo->fake_duck = false;
		return;
	}

	if (csgo->game_rules->IsFreezeTime()
		|| csgo->local->HasGunGameImmunity() 
		|| csgo->local->GetFlags() & FL_FROZEN
		|| csgo->cmd->buttons & IN_JUMP 
		|| !(csgo->local->GetFlags() & FL_ONGROUND) 
		|| !vars.antiaim.enable) {
		csgo->fake_duck = false;
		return;
	}

	csgo->cmd->buttons |= IN_BULLRUSH;

	if (g_Binds[bind_fake_duck].active)
	{
		csgo->fake_duck = true;

		if (csgo->client_state->iChokedCommands <= 6)
			csgo->cmd->buttons &= ~IN_DUCK;
		else
			csgo->cmd->buttons |= IN_DUCK;

		csgo->send_packet = csgo->client_state->iChokedCommands >= 14;
	}
	else
		csgo->fake_duck = false;
}

void CMisc::StoreTaserRange()
{
	Vector prev_scr_pos{ 0, 0, 0 };
	Vector scr_pos{ 0, 0, 0 };

	if (!csgo->local || !csgo->local->isAlive())
		return;

	auto local_weapon = csgo->weapon;
	if (!local_weapon || local_weapon->GetItemDefinitionIndex() != weapon_taser) {
		csgo->should_draw_taser_range = false;
		return;
	}

	csgo->should_draw_taser_range = true;

	float step = PI * 2.0f / 105.f;

	float rad = local_weapon->GetCSWpnData()->m_flRange - 15.f;

	Vector origin = csgo->local->GetRenderOrigin() + Vector(0, 0, 50);
	size_t i = 0;
	for (float rotation = 0; rotation <= (PI * 2.0); rotation += step)
	{
		Vector pos(rad * cos(rotation) + origin.x, rad * sin(rotation) + origin.y, origin.z);

		Ray_t ray;
		trace_t trace;
		CTraceFilter filter;
		filter.pSkip = csgo->local;
		ray.Init(origin, pos);

		interfaces.trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace);
		csgo->taser_ranges[i++] = std::pair(trace.endpos, trace.endpos + Vector(0.f, 0.f, -10.f));
	}
}

void CMisc::Ragdolls()
{
	if (!vars.visuals.ragdoll_force)
		return;

	for (auto i = 1; i <= interfaces.ent_list->GetHighestEntityIndex(); ++i)
	{
		auto e = interfaces.ent_list->GetClientEntity(i);

		if (!e)
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		if (client_class->m_ClassID != g_ClassID->CCSRagdoll)
			continue;

		e->GetVecForce() = Vector(800000.0f, 800000.0f, 800000.0f);
	}
}