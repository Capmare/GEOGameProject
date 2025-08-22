#include <cmath>
#include "CollisionSystem.h"

namespace gameplay {

    bool CollisionSystem::ResolveWalls(
        ThreeBlade& character, float& vx, float& vy, float& vzEnergy,
        float radius, float w, float h, float bounceLoss, float energyGain)
    {
        float x = character[0];
        float y = character[1];
        bool hit = false;

        if (x < radius) { x = radius; vx = -vx * bounceLoss; hit = true; }
        else if (x > w - radius) { x = w - radius; vx = -vx * bounceLoss; hit = true; }

        if (y < radius) { y = radius; vy = -vy * bounceLoss; hit = true; }
        else if (y > h - radius) { y = h - radius; vy = -vy * bounceLoss; hit = true; }

        if (hit) {
            const float v = std::sqrt(vx * vx + vy * vy);
            vzEnergy += energyGain * v;
        }

        character = ThreeBlade{ x, y, character[2], 1.f };
        return hit;
    }

} // namespace gameplay
