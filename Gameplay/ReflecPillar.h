// Gameplay/ReflectPillar.h
#pragma once
#include "FlyFish.h"
#include "Gameplay/GeoMotors.h"

namespace gameplay {

    struct ReflectPillar
    {
        ThreeBlade C;
        float triggerR = 120.f;
        bool  wasInside = false;

        static ReflectPillar Make(const ThreeBlade& center, float triggerR) {
            ReflectPillar p; p.C = center; p.triggerR = triggerR; p.wasInside = false; return p;
        }
        static ReflectPillar Make(const ThreeBlade& center, float triggerR, float /*cooldown*/) {
            return Make(center, triggerR);
        }

        // returns true if we performed a reflection this frame
        bool TryReflect(ThreeBlade& X, float& vx, float& vy, float /*dt*/);

        const ThreeBlade& Center() const { return C; }
    };

} // namespace gameplay
