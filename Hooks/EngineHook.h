#pragma once
#include "Hooks.h"
#include <intrin.h>
#include "../cMainStruct.h"

bool __fastcall Hooked_IsPaused(void* ecx, void* edx) {
	static auto IsPaused = g_pEngineHook->GetOriginal< IsPausedFn >(g_HookIndices[fnva1(hs::Hooked_IsPaused.s().c_str())]);

	static DWORD* return_to_extrapolation = (DWORD*)(csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		str("FF D0 A1 ?? ?? ?? ?? B9 ?? ?? ?? ?? D9 1D ?? ?? ?? ?? FF 50 34 85 C0 74 22 8B 0D ?? ?? ?? ??")) + 0x29);

	if (_ReturnAddress() == (void*)return_to_extrapolation)
		return true;

	return IsPaused(ecx);
}

int32_t __fastcall Hooked_IsBoxVisible(IEngineClient* engine_client, uint32_t, Vector& min, Vector& max)
{
	static auto BoxVisible = g_pEngineHook->GetOriginal< BoxVisibleFn >(g_HookIndices[fnva1(hs::Hooked_IsBoxVisible.s().c_str())]);

	static DWORD* return_to_beam = (DWORD*)(csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::ret_engine.s().c_str()));

	if (_ReturnAddress() == return_to_beam)
		return 1;

	return BoxVisible(engine_client, min, max);
}

bool __fastcall Hooked_IsHLTV(IEngineClient* IEngineClient, uint32_t)
{
	static auto IsHLTV = g_pEngineHook->GetOriginal< IsHLTVFn >(g_HookIndices[fnva1(hs::Hooked_IsHLTV.s().c_str())]);

	static const auto return_to_accumulate_layers = (csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::accum_layers.s().c_str()));

	static const auto return_to_setup_vel = (csgo->Utils.FindPatternIDA(GetModuleHandleA(
		g_Modules[fnva1(hs::client_dll.s().c_str())]().c_str()),
		hs::setup_vel.s().c_str()));

	if (_ReturnAddress() == (uint32_t*)(return_to_accumulate_layers)
		|| _ReturnAddress() == (uint32_t*)(return_to_setup_vel))
		return true;

	return IsHLTV(IEngineClient);
}

void __fastcall Hooked_PhysicsSimulate(IBasePlayer* ecx, void* edx) {
	static auto original = hooker::h.original(&Hooked_PhysicsSimulate);
	original(ecx, edx);
}

// to-do : шифтить через этот хук + телепорт после речарджа адекватный (@opai)
// original CL_Move hook
void __vectorcall Hooked_CLMove(float flAccumulatedExtraSamples, bool bFinalTick)
{
	static auto original = hooker::h.original(&Hooked_CLMove);

	if (!csgo->local->isAlive()) {
		return original(flAccumulatedExtraSamples, bFinalTick);
	}

	if (csgo->clskip > 0) {
		csgo->clskip--;
		return;
	}

	while (csgo->cl_move_shift > 0)
	{
		csgo->cl_move_shift--;
		original(flAccumulatedExtraSamples, csgo->cl_move_shift == 0);
		if (csgo->cl_move_shift <= 0)
			return;
	}

	return original(flAccumulatedExtraSamples, bFinalTick);
}

//void __vectorcall Hooked_CLMove(float flAccumulatedExtraSamples, bool bFinalTick)
//{
//	//static auto original = hooker::h.original(&Hooked_CLMove);
//
//	//if (!csgo->local->isAlive())
//	//	return original(flAccumulatedExtraSamples, bFinalTick);
//
//	//while (csgo->cl_move_shift > 0)
//	//{
//	//	original(flAccumulatedExtraSamples, bFinalTick);
//	//	csgo->cl_move_shift--;
//	//}
//
//	// Recharge every other tick when available, allows movement and shooting while recharging
//	if (rechargeRequired > 0 && g_Globals->tickcount % 2) {
//		recharge--;
//	}
//	else {
//		clmove_original(args);
//
//		// shift
//		for (int i = 0; i < CCheatVars::cl_move_shift; i++) {
//			isShifting = true;
//			clmove_original(args);
//			rechargeRequired++;
//		}
//
//		isShifting = false;
//		ticksToShift = 0;
//
//		original(flAccumulatedExtraSamples, bFinalTick);
//}