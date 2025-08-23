#pragma once
#include <vector>

#include "structs.h"
#include "../FlyFish.h"

namespace gameplay {


    struct PillarColors {
        Color4f White{1.f, 1.f, 1.f, 1.f};
        Color4f Red{1.f, 0.f, 0.f, 1.f};
        Color4f Green{0.f, 1.f, 0.f, 1.f};
        Color4f Blue{0.f, 0.f, 1.f, 1.f};
        Color4f Yellow{1.f, 1.f, 0.f, 1.f};
    };


    enum class PillarType {
        Normal,
        Movable,
        Linear,
        Seek,
        Reflect

    };




    class PillarRenderer {
    public:
        static void Draw(const std::vector<std::pair<ThreeBlade, PillarType>> &pillars, int current);

    };
} // namespace gameplay
