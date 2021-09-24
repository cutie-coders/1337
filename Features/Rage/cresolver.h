#pragma once

struct Info
{
	struct History
	{
		float m_flLastLowerBodyYaw; // for sway shit
		float m_flTimeSinceLastMove; // for lby breaking shit
		bool m_bResolved; // well yk
		int m_iTicksSinceLastMove; // :thinking:
		bool m_bCanBreakLby;
		bool m_bIsBreakingLby;
	};
	int m_iDesyncType;
	float m_flLowerBodyYaw;
	float m_flEyeYaw;
	int m_iTicksSince0Pitch;
	int m_iSide;
};

class cResolver
{
public:
	std::vector<Info> HitInfo[64];

	void Run(IBasePlayer* player);
	float GetLastLowerBodyYaw(IBasePlayer* player);
	float GetBackwardYaw(IBasePlayer*); // fuck you microsoft whores.

	void AddHitInfo(IBasePlayer* player, const Info& info)
	{ 
		const int& idx = player->GetIndex();
		HitInfo[idx].push_back(info);
		while (HitInfo[idx].size() > 5)
			HitInfo[idx].erase(HitInfo[idx].begin());
	};

	void RemoveHitInfo(IBasePlayer* player) {
		const int& idx = player->GetIndex();
		if (HitInfo[idx].empty())
			return;
		HitInfo[idx].erase(HitInfo[idx].begin());
	}
	void ClearHitInfo() {
		for (auto& info : HitInfo)
			info.clear();
	}
	void ClearHitInfo(int idx) {
		HitInfo[idx].clear();
	}

private:
	bool IsBreakingLby(IBasePlayer* player);
};

extern Info resolverInfo[64];
extern Info::History resolverRecord[64];