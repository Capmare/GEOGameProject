#pragma once
#include "../FlyFish.h"

namespace gameplay {

    class CollisionSystem {
    public:
        static bool ResolveWalls(
            ThreeBlade& character, float& vx, float& vy, float& vzEnergy,
            float radius, float w, float h, float bounceLoss, float energyGain = 0.12f);
    };

} // namespace gameplay
