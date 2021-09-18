
#include "../../Hooks/hooks.h"
#include "../Features.h"

enum EResolverStages {
	antifreestand,
	inverse_antifreestand,
	unresolved,
};

std::string ResolverMode[65];

float CResolver::GetAngle(IBasePlayer* player) {
	return Math::NormalizeYaw(player->GetEyeAngles().y);
}

float CResolver::GetForwardYaw(IBasePlayer* player) {
	return Math::NormalizeYaw(GetBackwardYaw(player) - 180.f);
}

float CResolver::GetAwayAngle(IBasePlayer* player) {
	return Math::CalculateAngle(csgo->local->GetEyePosition(), player->GetEyePosition()).y;
}

float CResolver::GetBackwardYaw(IBasePlayer* player) {
	return Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y;
}

float CResolver::GetLeftYaw(IBasePlayer* player) {
	return Math::NormalizeYaw(Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y - 90.f);
}

float CResolver::GetRightYaw(IBasePlayer* player) {
	return Math::NormalizeYaw(Math::CalculateAngle(csgo->local->GetOrigin(), player->GetOrigin()).y + 90.f);
}

void CResolver::StoreAntifreestand()
{
	if (!csgo->local->isAlive()) {
		memset(this->ResolverInfo, 0, sizeof(this->ResolverInfo));
		return;
	}
	static int delay_ticks[65] = { 0 };
	for (int i = 0; i < interfaces.engine->GetMaxClients(); ++i)
	{
		auto player = interfaces.ent_list->GetClientEntity(i);

		if (!player
			|| !player->isAlive()
			|| player->IsDormant()
			|| !player->IsPlayer()
			|| player->GetTeam() == csgo->local->GetTeam()
			|| player->EntIndex() == interfaces.engine->GetLocalPlayer()) {
			continue;
		}

		auto idx = player->GetIndex();

		if (vars.ragebot.resolver) {
			if (csgo->actual_misses[player->GetIndex()] <= 0) {
				auto latest_animation = g_Animfix->get_latest_animation(player);
				if (!latest_animation)
					continue;

				auto point = g_Ragebot->GetAdvancedHeadPoints(player, latest_animation->unresolved_bones);

				g_Ragebot->BackupPlayer(latest_animation);
				g_Ragebot->SetAnims(latest_animation, latest_animation->unresolved_bones);

				float left_damage = g_AutoWall->Think(point[0], player, HITGROUP_HEAD).m_damage;
				float right_damage = g_AutoWall->Think(point[1], player, HITGROUP_HEAD).m_damage;

				g_Ragebot->RestorePlayer(latest_animation);

				if (left_damage > right_damage)
					FreestandSide[idx] = 1;
				else if (left_damage < right_damage)
					FreestandSide[idx] = -1;
			}

			memset(&ResolverInfo[idx], 0, sizeof(&ResolverInfo[idx]));
		}
	}
}

bool DoesHaveFakeAngles(IBasePlayer* player) {
	static int skip_ticks[65] = { 0 };
	int idx = player->GetIndex();
	if (player->GetSimulationTime() == player->GetOldSimulationTime())
		skip_ticks[idx]++;
	else
		skip_ticks[idx] = 0;
	return skip_ticks[idx] >= 16;
}

bool shot(IBasePlayer* p)
{
	if (!p->GetWeapon())
		return false;

	if (!csgo->local->isAlive())
		return false;

	if (p->GetWeapon()->LastShotTime() == p->GetSimulationTime())
		return true;
	return false;
}

void CResolver::Resolver(IBasePlayer* p)
{
	int i = p->EntIndex();
	CCSGOPlayerAnimState* state = p->GetPlayerAnimState();

	if (!vars.ragebot.resolver || p->GetPlayerInfo().fakeplayer)
	{
		ResolverMode[i] = p->GetPlayerInfo().fakeplayer ? str("Bot") : str("Disabled");
		ResolverInfo[i].Index = 0;
		return;
	}

	int add = 0;
	if (!shot_snapshots.empty()) {
		const auto& snapshot = shot_snapshots.front();
		const bool& dt_ready = !csgo->need_to_recharge && g_Misc->dt_bullets <= 1 && csgo->weapon->isAutoSniper();
		if (dt_ready && snapshot.hitbox == 0 && snapshot.intended_damage > p->GetHealth()) {
			add = g_Misc->dt_bullets;
		}
	}

	bool max_desync[64] = { false };

	int missed_shots = (csgo->actual_misses[i] + csgo->imaginary_misses[i] + add) % 4;

	
}

bool CResolver::Do(IBasePlayer* p) {
	int i = p->EntIndex();
	CCSGOPlayerAnimState* state = p->GetPlayerAnimState();

	if (!vars.ragebot.resolver || p->GetPlayerInfo().fakeplayer)
	{
		ResolverMode[i] = p->GetPlayerInfo().fakeplayer ? str("Bot") : str("Disabled");
		ResolverInfo[i].Index = 0;
		return false;
	}

	int add = 0;
	if (!shot_snapshots.empty()) {
		const auto& snapshot = shot_snapshots.front();
		const bool& dt_ready = !csgo->need_to_recharge && g_Misc->dt_bullets <= 1 && csgo->weapon->isAutoSniper();
		if (dt_ready && snapshot.hitbox == 0 && snapshot.intended_damage > p->GetHealth()) {
			add = g_Misc->dt_bullets;
		}
	}

	bool max_desync[64] = { false };

	int missed_shots = (csgo->actual_misses[i] + csgo->imaginary_misses[i] + add) % 4;
	float time_since_0_pitch[64];
	std::string& mode = ResolverMode[i];

	while (p->GetEyeAngles().x == 0.f)
	{
		time_since_0_pitch[i] = csgo->curtime;
	}

	do {
		record.LastKnownYaw[i] = state->m_abs_yaw;
		ResolverInfo[i].Index = i;
		mode += "Last yaw";
	} while (time_since_0_pitch[i] == csgo->curtime);

	if (missed_shots != 2)
	{

	}

}
