#include <algorithm>
#include <cmath>
#include "HUDRenderer.h"
#include "../utils.h"

using namespace utils;

namespace gameplay {

    void HUDRenderer::Draw(float vx, float vy, float maxSpeed, float vzEnergy, float winH)
    {
        float speed = std::sqrt(vx * vx + vy * vy);
        float s01 = std::clamp(speed / maxSpeed, 0.f, 1.f);

        SetColor(Color4f{ 0.15f,0.15f,0.15f,0.8f });
        FillRect(20.f, winH - 30.f, 220.f, 12.f);
        SetColor(Color4f{ 0.1f,0.8f,0.2f,0.95f });
        FillRect(20.f, winH - 30.f, 220.f * s01, 12.f);
        SetColor(Color4f{ 1.f,1.f,1.f,1.f });
        DrawRect(20.f, winH - 30.f, 220.f, 12.f);

        float e = std::clamp(std::fabs(vzEnergy) / 200.f, 0.f, 1.f);
        SetColor(Color4f{ 0.15f,0.15f,0.15f,0.8f });
        FillRect(20.f, winH - 50.f, 220.f, 12.f);
        SetColor(Color4f{ 0.9f,0.2f,0.9f,0.95f });
        FillRect(20.f, winH - 50.f, 220.f * e, 12.f);
        SetColor(Color4f{ 1.f,1.f,1.f,1.f });
        DrawRect(20.f, winH - 50.f, 220.f, 12.f);
    }

} // namespace gameplay
