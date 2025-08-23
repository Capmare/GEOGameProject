#include "PillarRenderer.h"
#include "../utils.h"

using namespace utils;

namespace gameplay {

    void PillarRenderer::Draw(const std::vector<std::pair<ThreeBlade,PillarType>>& pillars, int current)
    {
        PillarColors c;
        for (size_t i = 0; i < pillars.size(); ++i)
        {
            const bool sel = int(i) == current;

            Color4f CurrColor{};
            switch (pillars[i].second) {
                case PillarType::Movable: CurrColor = c.Red;  break;
                case PillarType::Linear:  CurrColor = c.Green; break;
                case PillarType::Seek:    CurrColor = c.Blue; break;
                case PillarType::Reflect: CurrColor = c.Yellow; break;
                case PillarType::Normal: CurrColor = c.White; break;

            }

            SetColor(CurrColor);
            FillCircle(pillars[i].first[0], pillars[i].first[1], sel ? 10.f : 7.f);
            DrawCircle(pillars[i].first[0], pillars[i].first[1], 120.f);
        }
    }

} // namespace gameplay
