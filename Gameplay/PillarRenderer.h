#pragma once
#include <vector>
#include "../FlyFish.h"

namespace gameplay {

    class PillarRenderer {
    public:
        static void Draw(const std::vector<ThreeBlade>& pillars, int current);
    };

} // namespace gameplay
