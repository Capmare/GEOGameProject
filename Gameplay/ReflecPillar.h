#pragma once
#include "FlyFish.h"
#include "Gameplay/GeoMotors.h"

namespace gameplay {

    // Pillar that reflects the player about its center when the player enters 'triggerR'.
    struct ReflectPillar
    {
        ThreeBlade C;
        float triggerR = 120.f;
        float cooldownSec = 0.25f;

        // runtime
        float cooldownLeft = 0.f;

        static ReflectPillar Make(const ThreeBlade& center, float triggerR, float cooldown = 0.25f);

        void TryReflect(ThreeBlade& X, float& vx, float& vy, float dt);

        const ThreeBlade& Center() const { return C; }
    };

} // namespace gameplay
