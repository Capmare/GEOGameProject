#include "PillarRenderer.h"
#include "../utils.h"

using namespace utils;

namespace gameplay {

    void PillarRenderer::Draw(const std::vector<ThreeBlade>& pillars, int current)
    {
        for (size_t i = 0; i < pillars.size(); ++i)
        {
            const bool sel = int(i) == current;
            SetColor(sel ? Color4f{ 1.f, 0.7f, 0.1f, 0.9f } : Color4f{ 0.3f, 0.8f, 1.f, 0.6f });
            FillCircle(pillars[i][0], pillars[i][1], sel ? 10.f : 7.f);
            SetColor(Color4f{ 1.f,1.f,1.f,0.2f });
            DrawCircle(pillars[i][0], pillars[i][1], 120.f);
        }
    }

} // namespace gameplay
