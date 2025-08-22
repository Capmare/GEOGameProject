#pragma once
#include <vector>
#include "../FlyFish.h"

namespace gameplay {

    struct InputState {
        bool up{false}, down{false}, left{false}, right{false}, boost{false};
    };

    class PlayerController {
    public:
        struct Tuning {
            float accel        = 900.f;
            float drag         = 0.995f;
            float energyDrag   = 0.998f;
            float maxSpeed     = 90000.f;
            float bounceLoss   = 0.90f;
            float influenceR   = 150.f;
            float energy2Angle = 0.30f;
            float maxAngStep   = 0.50f;
            float minSpinR     = 60.f;
            float slingFactor  = 0.01f;
            float maxEnergy    = 100000.f;
        };


        static void StepKinematics(
            ThreeBlade& character,
            float& vx, float& vy, float& vzEnergy,
            const InputState& in,
            const std::vector<ThreeBlade>& pillars, int currentPillar,
            float dt, const Tuning& k = Tuning());
    };

} // namespace gameplay
