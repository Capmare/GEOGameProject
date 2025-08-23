// Gameplay/MovablePillar.h
#pragma once
#include "FlyFish.h"


namespace gameplay {

    struct MovablePillar
    {
        ThreeBlade C;

        float influenceR = 240.f;
        float gravAccel  = 220.f;
        float gravMinR   = 30.f;

        enum class Mode { Linear, Orbit, Seek };
        Mode mode = Mode::Linear;

        float vx = 0.f;
        float vy = 0.f;

        ThreeBlade anchor;
        float omega = 0.f;

        ThreeBlade target;
        float maxSpeed = 140.f;
        float accel    = 400.f;

        static MovablePillar MakeLinear(const ThreeBlade& start, float vx, float vy, float influence = 240.f);
        static MovablePillar MakeOrbit (const ThreeBlade& anchor, const ThreeBlade& startOnCircle, float omega, float influence = 240.f);
        static MovablePillar MakeSeek  (const ThreeBlade& start, const ThreeBlade& target, float maxSpeed, float accel, float influence = 240.f);

        void Step(float dt);

        void Step(float dt, float minX, float minY, float maxX, float maxY, float bounceLoss = 1.0f);

    private:
        void BounceInside(float minX, float minY, float maxX, float maxY, float bounceLoss);
    };

} // namespace gameplay
