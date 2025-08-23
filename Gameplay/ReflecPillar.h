#pragma once
#include "FlyFish.h"
#include "Gameplay/GeoMotors.h"

namespace gameplay {

    // Reflects the player about its center once per entry into triggerR.
    // When the player exits the radius, it arms again.
    struct ReflectPillar
    {
        ThreeBlade C;      // center (weight-1)
        float triggerR = 120.f;

        // internal latch: true while player is inside this frame (prevents re-trigger)
        bool wasInside = false;

        static ReflectPillar Make(const ThreeBlade& center, float triggerR);
        static ReflectPillar Make(const ThreeBlade& center, float triggerR, float /*cooldown*/);

        // Keep signature with dt (ignored) so your existing call sites still compile.
        void TryReflect(ThreeBlade& X, float& vx, float& vy, float /*dt*/);

        const ThreeBlade& Center() const { return C; }
    };

} // namespace gameplay
