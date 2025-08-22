#pragma once
#include "../FlyFish.h"

namespace gameplay {

    class PlayerRenderer {
    public:
        static void Draw(const ThreeBlade& character, float radius, float vzEnergy);
    };

} // namespace gameplay
