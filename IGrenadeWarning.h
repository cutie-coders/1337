#pragma once
#include "../../Hooks/hooks.h"


class IGrenadeWarning
{
private:

	Vector Source, Velocity, Direction;
	IBasePlayer* Entity;
	int type = 0;

	void Setup(Vector& vecSrc, Vector& vecThrow, Vector viewangles);
	void Simulate();

	int Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval);
	bool CheckDetonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval);

	void TraceHull(Vector& src, Vector& end, trace_t& tr);
	void AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground);
	void PushEntity(Vector& src, const Vector& move, trace_t& tr);
	void ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, float interval);
	int PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);
public:
	void View(Vector& source, Vector& Vec, Vector& EyeAng, int Type);
	Vector EndPos;
	std::vector<Vector> path;
	std::vector<Vector> bounces;
};
