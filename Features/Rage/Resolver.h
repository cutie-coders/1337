#pragma once

struct ResolverInfo_t {
	int Stage;
	int Index;
	bool LowDelta;
	float ResolvedAngle;
};

struct Record {
	float LastKnownYaw[64];
	bool IsExtending[64];
	bool IsSwaying[64];
	int side[64]; // will be 1 or -1 depending on side.
};

class CResolver
{
private:
	std::vector<ResolverInfo_t> HitInfo[64];
	int ResolverStages[64];

	int FreestandSide[64];
	Record record;
	void lby_checks(IBasePlayer* p);

	float GetLeftYaw(IBasePlayer*);
	float GetRightYaw(IBasePlayer*);
public:
	ResolverInfo_t ResolverInfo[64];

	void AddHitInfo(IBasePlayer* player, const ResolverInfo_t& info)
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
	float GetAngle(IBasePlayer*);
	float GetForwardYaw(IBasePlayer*);
	float GetAwayAngle(IBasePlayer* player);
	float GetBackwardYaw(IBasePlayer*);
	bool Do(IBasePlayer*);
	void Resolver(IBasePlayer* p);
	void StoreAntifreestand();
};
extern std::string ResolverMode[65];