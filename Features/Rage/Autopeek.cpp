#include "../Features.h"

void CAutopeek::GotoStart(CUserCmd* cmd)
{
    static auto cl_forwardspeed = interfaces.cvars->FindVar("cl_forwardspeed");
    auto wish_yaw = csgo->original.y;
    auto difference = csgo->local->GetRenderOrigin() - quickpeekstartpos;
    if (difference.Length2D() > 5.0f)
    {
        auto velocity = Vector(difference.x * cos(wish_yaw / 180.0f * PI) + difference.y * sin(wish_yaw / 180.0f * PI), difference.y * cos(wish_yaw / 180.0f * PI) - difference.x * sin(wish_yaw / 180.0f * PI), difference.z);
        cmd->forwardmove = clamp(-velocity.x * 1000.0f, -cl_forwardspeed->GetFloat(), cl_forwardspeed->GetFloat());
        cmd->sidemove = clamp(velocity.y * 1000.0f, -cl_forwardspeed->GetFloat(), cl_forwardspeed->GetFloat());

    }
    else {
        Reset();
    }
    
}

void CAutopeek::Draw()
{
    csgo->mtx.lock();
    if (!csgo->is_local_alive) {
        csgo->mtx.unlock();
        return;
    }

    if (!g_Binds[bind_peek_assist].active)
    {
        csgo->mtx.unlock();
        return;
    }

    if (quickpeekstartpos != Vector{ 0, 0, 0 })
    {
        static auto FilledCircle = [](Vector location, float radius, color_t Col, float pAlpha) {
            static constexpr float Step = PI * 2.0f / 60;
            std::vector<ImVec2> points;
            for (float lat = 0.f; lat <= PI * 2.0f; lat += Step)
            {
                const auto& point3d = Vector(sin(lat), cos(lat), 0.f) * radius;
                Vector point2d;
                if (Math::WorldToScreen(location + point3d, point2d))
                    points.push_back(ImVec2(point2d.x, point2d.y));
            }
            g_Render->_drawList->AddConvexPolyFilled(points.data(), points.size(), color_t(Col[0],
                Col[1], Col[2], (pAlpha / 255) * 80).u32());
            g_Render->_drawList->AddPolyline(points.data(), points.size(), color_t(Col[0],
                Col[1], Col[2], (pAlpha / 255) * 120).u32(), true, 3.5f);
        };
        FilledCircle(quickpeekstartpos, 20.f, color_t(170, 170, 255, 255), 255);
    }
    csgo->mtx.unlock();
}

void CAutopeek::Run()
{
    if (!(csgo->local->GetFlags() & FL_ONGROUND))
        return;

    if (g_Binds[bind_peek_assist].active) {
        if (quickpeekstartpos == Vector(0, 0, 0))
            quickpeekstartpos = csgo->local->GetRenderOrigin();

        bool is_revolver = csgo->weapon->GetItemDefinitionIndex() == WEAPON_REVOLVER;

        if (!csgo->weapon->IsMiscWeapon() && !is_revolver && csgo->cmd->buttons & IN_ATTACK || is_revolver && g_Ragebot->shot)
            has_shot = true;

        if (has_shot)
            GotoStart(csgo->cmd);
    }
    else {
        Reset();
    }
}
