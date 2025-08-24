#include "Gameplay/PillarRenderer.h"
#include <algorithm>

#include "utils.h"

namespace gameplay {

    static inline Color4f ColorFor(PillarType t, const PillarColors& c) {
        switch (t) {
            case PillarType::Movable: return c.Red;
            case PillarType::Linear:  return c.Green;
            case PillarType::Seek:    return c.Blue;
            case PillarType::Reflect: return c.Yellow;
            case PillarType::Normal:  default: return c.White;
        }
    }

    void PillarRenderer::Draw(const std::vector<std::pair<ThreeBlade,PillarType>>& pillars,
                              const std::vector<int>& activeSet)
    {
        PillarColors pal;

        std::vector<char> activeMask(pillars.size(), 0);
        for (int idx : activeSet) {
            if (idx >= 0 && idx < (int)pillars.size()) activeMask[size_t(idx)] = 1;
        }

        for (size_t i = 0; i < pillars.size(); ++i)
        {
            const bool sel = activeMask[i] != 0;

            Color4f curr = ColorFor(pillars[i].second, pal);
            utils::SetColor(curr);

            utils::FillCircle(pillars[i].first[0], pillars[i].first[1], sel ? 10.f : 7.f);

            utils::DrawCircle(pillars[i].first[0], pillars[i].first[1], 120.f);

            if (sel) {
                utils::SetColor(Color4f{1.f,1.f,1.f,1.f});
                utils::DrawCircle(pillars[i].first[0], pillars[i].first[1], 120.f + 6.f);
            }
        }
    }

    void PillarRenderer::Draw(const std::vector<std::pair<ThreeBlade,PillarType>>& pillars, int current)
    {
        if (current < 0) {
            static const std::vector<int> none;
            Draw(pillars, none);
        } else {
            std::vector<int> one{ current };
            Draw(pillars, one);
        }
    }

} // namespace gameplay
