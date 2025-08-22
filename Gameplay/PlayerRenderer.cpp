#include <algorithm>
#include <cmath>
#include "PlayerRenderer.h"
#include "../utils.h"

using namespace utils;

namespace gameplay {

    void PlayerRenderer::Draw(const ThreeBlade& character, float radius, float vzEnergy)
    {
        float e01 = std::clamp(std::fabs(vzEnergy) * 0.01f, 0.f, 1.f);
        SetColor(Color4f{ 0.2f + 0.8f * e01, 0.3f, 1.f - 0.8f * e01, 0.25f });
        for (int i = 0; i < 6; ++i)
            DrawCircle(character[0], character[1], radius + 4.f + 3.f * i);

        SetColor(Color4f{ 1.f, 1.f, 1.f, 1.f });
        FillCircle(character[0], character[1], radius);
        SetColor(Color4f{ 0.f, 0.f, 0.f, 1.f });
        DrawCircle(character[0], character[1], radius);
    }

} // namespace gameplay
