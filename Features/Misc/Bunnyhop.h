#pragma once
#include "../../Hooks/hooks.h"
#include "../../Math.h"

#define M_PI 3.14159265358979323846f
#define M_PI2	1.57079632679489661923
#define M_PI4	0.785398163397448309616
#define M_RADPI 57.295779513082f
#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)
const float M_U_DEG = 360.0 / 65536;
const float M_U_RAD = 3.14159265358979323846 / 32768;

class CBunnyhop
{
public:
	void Run(Vector& orig)
	{
		if (csgo->local->GetMoveType() == MoveType_t::MOVETYPE_NOCLIP || csgo->local->GetMoveType() == MoveType_t::MOVETYPE_LADDER)
			return;

		if (vars.misc.bunnyhop) {
			static bool bLastJumped = false;
			static bool bShouldFake = false;

			if (!bLastJumped && bShouldFake)
			{
				bShouldFake = false;
				csgo->cmd->buttons |= IN_JUMP;
			}
			else if (csgo->cmd->buttons & IN_JUMP)
			{
				if (csgo->local->GetFlags() & FL_ONGROUND)
				{
					bShouldFake = bLastJumped = true;
				}
				else
				{
					csgo->cmd->buttons &= ~IN_JUMP;
					bLastJumped = false;
				}
			}
			else
			{
				bShouldFake = bLastJumped = false;
			}
		}

		if (csgo->cmd->buttons & IN_SPEED || !vars.misc.autostrafe || csgo->local->GetVelocity().Length2D() < 10.f)
			return; // doesn't allow strafe when you hold shift and you're not moving
        if (csgo->local->GetFlags() & FL_ONGROUND/* || engineprediction::get().backup_data.flags & FL_ONGROUND*/ || csgo->cmd->buttons & IN_JUMP)
            return;

        static auto cl_sidespeed = interfaces.cvars->FindVar(hs::cl_sidespeed.s().c_str());
        auto side_speed = cl_sidespeed->GetFloat();
        //def autostrafe
        if (vars.misc.autostrafe == 1) {
            static float yaw_add = 0.f;
            static const auto cl_sidespeed = interfaces.cvars->FindVar(hs::cl_sidespeed.s().c_str());
            if (!(csgo->local->GetFlags() & FL_ONGROUND))
            {
                bool back = csgo->cmd->buttons & IN_BACK;
                bool forward = csgo->cmd->buttons & IN_FORWARD;
                bool right = csgo->cmd->buttons & IN_MOVELEFT;
                bool left = csgo->cmd->buttons & IN_MOVERIGHT;

                if (back) {
                    yaw_add = -180.f;
                    if (right)
                        yaw_add -= 45.f;
                    else if (left)
                        yaw_add += 45.f;
                }
                else if (right) {
                    yaw_add = 90.f;
                    if (back)
                        yaw_add += 45.f;
                    else if (forward)
                        yaw_add -= 45.f;
                }
                else if (left) {
                    yaw_add = -90.f;
                    if (back)
                        yaw_add -= 45.f;
                    else if (forward)
                        yaw_add += 45.f;
                }
                else {
                    yaw_add = 0.f;
                }

                orig.y += yaw_add;
                csgo->cmd->forwardmove = 0.f;
                csgo->cmd->sidemove = 0.f;

                const auto delta = Math::NormalizeYaw(orig.y - RAD2DEG(atan2(csgo->local->GetVelocity().y, csgo->local->GetVelocity().x)));

                csgo->cmd->sidemove = delta > 0.f ? -cl_sidespeed->GetFloat() : cl_sidespeed->GetFloat();

                orig.y = Math::NormalizeYaw(orig.y - delta);
            }
        }
        //movement autostrafe           
        else if (vars.misc.autostrafe == 2)
        {
            static auto old_yaw = 0.0f;

            auto get_velocity_degree = [](float velocity)
            {
                auto tmp = RAD2DEG(atan(30.0f / velocity));

                if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
                    return 90.0f;

                else if (tmp < 0.0f)
                    return 0.0f;
                else
                    return tmp;
            };

            if (csgo->local->GetMoveType() != MOVETYPE_WALK)
                return;

            auto velocity = csgo->local->GetVelocity();
            velocity.z = 0.0f;

            auto forwardmove = csgo->cmd->forwardmove;
            auto sidemove = csgo->cmd->sidemove;

            if (velocity.Length2D() < 5.0f && !forwardmove && !sidemove)
                return;

            static auto flip = false;
            flip = !flip;

            auto turn_direction_modifier = flip ? 1.0f : -1.0f;
            auto viewangles = csgo->cmd->viewangles;

            if (forwardmove || sidemove)
            {
                csgo->cmd->forwardmove = 0.0f;
                csgo->cmd->sidemove = 0.0f;

                auto turn_angle = atan2(-sidemove, forwardmove);
                viewangles.y += turn_angle * M_RADPI;
            }
            else if (forwardmove) //-V550
                csgo->cmd->forwardmove = 0.0f;

            auto strafe_angle = RAD2DEG(atan(15.0f / velocity.Length2D()));

            if (strafe_angle > 90.0f)
                strafe_angle = 90.0f;
            else if (strafe_angle < 0.0f)
                strafe_angle = 0.0f;

            auto temp = Vector(0.0f, viewangles.y - old_yaw, 0.0f);
            temp.y = Math::NormalizeYaw(temp.y);

            auto yaw_delta = temp.y;
            old_yaw = viewangles.y;

            auto abs_yaw_delta = fabs(yaw_delta);

            if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 15.0f) // whatsapp turbo ativado
            {
                Vector velocity_angles;
                vector_angles(velocity, velocity_angles);

                temp = Vector(0.0f, viewangles.y - velocity_angles.y, 0.0f);
                temp.y = Math::NormalizeYaw(temp.y);

                auto velocityangle_yawdelta = temp.y;
                auto velocity_degree = get_velocity_degree(velocity.Length2D());

                if (velocityangle_yawdelta <= velocity_degree || velocity.Length2D() <= 15.0f)
                {
                    if (-velocity_degree <= velocityangle_yawdelta || velocity.Length2D() <= 15.0f)
                    {
                        viewangles.y += strafe_angle * turn_direction_modifier;
                        csgo->cmd->sidemove = side_speed * turn_direction_modifier;
                    }
                    else
                    {
                        viewangles.y = velocity_angles.y - velocity_degree;
                        csgo->cmd->sidemove = side_speed;
                    }
                }
                else
                {
                    viewangles.y = velocity_angles.y + velocity_degree;
                    csgo->cmd->sidemove = -side_speed;
                }
            }
            else if (yaw_delta > 0.0f)
                csgo->cmd->sidemove = -side_speed;
            else if (yaw_delta < 0.0f)
                csgo->cmd->sidemove = side_speed;

            auto move = Vector(csgo->cmd->forwardmove, csgo->cmd->sidemove, 0.0f);
            auto speed = move.Length();

            Vector angles_move;
            vector_angles(move, angles_move);

            auto normalized_x = fmod(csgo->cmd->viewangles.x + 180.0f, 360.0f) - 180.0f;
            auto normalized_y = fmod(csgo->cmd->viewangles.y + 180.0f, 360.0f) - 180.0f;

            auto yaw = DEG2RAD(normalized_y - viewangles.y + angles_move.y);

            if (normalized_x >= 90.0f || normalized_x <= -90.0f || csgo->cmd->viewangles.x >= 90.0f && csgo->cmd->viewangles.x <= 200.0f || csgo->cmd->viewangles.x <= -90.0f && csgo->cmd->viewangles.x <= 200.0f) //-V648
                csgo->cmd->forwardmove = -cos(yaw) * speed;
            else
                csgo->cmd->forwardmove = cos(yaw) * speed;

            csgo->cmd->sidemove = sin(yaw) * speed;
        }
        //Legit autostrafe
        else if (vars.misc.autostrafe == 3)
        {
            static auto old_yaw = 0.0f;

            auto get_velocity_degree = [](float velocity)
            {
                auto tmp = RAD2DEG(atan(30.0f / velocity));

                if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
                    return 90.0f;

                else if (tmp < 0.0f)
                    return 0.0f;
                else
                    return tmp;
            };

            if (csgo->local->GetMoveType() != MOVETYPE_WALK)
                return;

            auto velocity = csgo->local->GetVelocity();
            velocity.z = 0;

            static auto flip = false;
            flip = !flip;

            auto turn_direction_modifier = flip ? 1.0f : -1.0f;
            auto viewangles = csgo->cmd->viewangles;

            auto strafe_angle = RAD2DEG(atan(15.0f / velocity.Length2D()));

            if (strafe_angle > 90.0f)
                strafe_angle = 90.0f;
            else if (strafe_angle < 0.0f)
                strafe_angle = 0.0f;

            auto temp = Vector(0.0f, viewangles.y - old_yaw, 0.0f);
            temp.y = Math::NormalizeYaw(temp.y);

            auto yaw_delta = temp.y;
            old_yaw = viewangles.y;

            auto abs_yaw_delta = fabs(yaw_delta);

            if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 30.0f)
            {
                Vector velocity_angles;
                vector_angles(velocity, velocity_angles);

                temp = Vector(0.0f, viewangles.y - velocity_angles.y, 0.0f);
                temp.y = Math::NormalizeYaw(temp.y);

                auto velocityangle_yawdelta = temp.y;
                auto velocity_degree = get_velocity_degree(velocity.Length2D());

                if (velocityangle_yawdelta <= velocity_degree || velocity.Length2D() <= 15.0f)
                {
                    if (-velocity_degree <= velocityangle_yawdelta || velocity.Length2D() <= 15.0f)
                    {
                        viewangles.y += strafe_angle * turn_direction_modifier;
                        csgo->cmd->sidemove = side_speed * turn_direction_modifier;
                    }
                    else
                    {
                        viewangles.y = velocity_angles.y - velocity_degree;
                        csgo->cmd->sidemove = side_speed;
                    }
                }
                else
                {
                    viewangles.y = velocity_angles.y + velocity_degree;
                    csgo->cmd->sidemove = -side_speed;
                }
            }
            else if (yaw_delta > 0.0f)
                csgo->cmd->sidemove = -side_speed;
            else if (yaw_delta < 0.0f)
                csgo->cmd->sidemove = side_speed;

            auto move = Vector(csgo->cmd->forwardmove, csgo->cmd->sidemove, 0.0f);
            auto speed = move.Length();

            Vector angles_move;
            vector_angles(move, angles_move);

            auto normalized_x = fmod(csgo->cmd->viewangles.x + 180.0f, 360.0f) - 180.0f;
            auto normalized_y = fmod(csgo->cmd->viewangles.y + 180.0f, 360.0f) - 180.0f;

            auto yaw = DEG2RAD(normalized_y - viewangles.y + angles_move.y);

            if (normalized_x >= 90.0f || normalized_x <= -90.0f || csgo->cmd->viewangles.x >= 90.0f && csgo->cmd->viewangles.x <= 200.0f || csgo->cmd->viewangles.x <= -90.0f && csgo->cmd->viewangles.x <= 200.0f) //-V648
                csgo->cmd->forwardmove = -cos(yaw) * speed;
            else
                csgo->cmd->forwardmove = cos(yaw) * speed;

            csgo->cmd->sidemove = sin(yaw) * speed;
        }
        //legit autostrafe
        else if (vars.misc.autostrafe == 4)
        {
        Vector engine_angles;
        interfaces.engine->GetViewAngles(engine_angles);

        auto velocity = csgo->local->GetVelocity();

        csgo->cmd->forwardmove = min(5850.0f / velocity.Length2D(), side_speed);
        csgo->cmd->sidemove = csgo->cmd->command_number % 2 ? side_speed : -side_speed;

        auto yaw_velocity = calculate_angle(Vector(0.0f, 0.0f, 0.0f), velocity).y;
        auto ideal_rotation = std::clamp(RAD2DEG(atan2(15.0f, velocity.Length2D())), 0.0f, 45.0f);

        auto yaw_rotation = fabs(yaw_velocity - engine_angles.y) + (csgo->cmd->command_number % 2 ? ideal_rotation : -ideal_rotation);
        auto ideal_yaw_rotation = yaw_rotation < 5.0f ? yaw_velocity : engine_angles.y;

        RotateMovement(csgo->cmd, ideal_yaw_rotation);
        }

		}
    private:
        void RotateMovement(CUserCmd* cmd, float yaw)
        {
            Vector viewangles;
            interfaces.engine->GetViewAngles(viewangles);

            float rotation = DEG2RAD(viewangles.y - yaw);

            float cos_rot = cos(rotation);
            float sin_rot = sin(rotation);

            float new_forwardmove = cos_rot * csgo->cmd->forwardmove - sin_rot * csgo->cmd->sidemove;
            float new_sidemove = sin_rot * csgo->cmd->forwardmove + cos_rot * csgo->cmd->sidemove;

            csgo->cmd->forwardmove = new_forwardmove;
            csgo->cmd->sidemove = new_sidemove;
        };
        Vector calculate_angle(const Vector& src, const Vector& dst) {
            Vector angles;

            Vector delta = src - dst;
            float hyp = delta.Length2D();

            angles.y = std::atanf(delta.y / delta.x) * M_RADPI;
            angles.x = std::atanf(-delta.z / hyp) * -M_RADPI;
            angles.z = 0.0f;

            if (delta.x >= 0.0f)
                angles.y += 180.0f;

            return angles;
        };
        void vector_angles(const Vector& forward, Vector& angles)
        {
            Vector view;

            if (!forward[0] && !forward[1])
            {
                view[0] = 0.0f;
                view[1] = 0.0f;
            }
            else
            {
                view[1] = atan2(forward[1], forward[0]) * 180.0f / M_PI;

                if (view[1] < 0.0f)
                    view[1] += 360.0f;

                view[2] = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
                view[0] = atan2(forward[2], view[2]) * 180.0f / M_PI;
            }

            angles[0] = -view[0];
            angles[1] = view[1];
            //angles[2] = 0.f;
        };
	};