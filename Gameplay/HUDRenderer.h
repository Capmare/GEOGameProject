#pragma once

namespace gameplay {

    class HUDRenderer {
    public:
        static void Draw(float vx, float vy,
                     float maxSpeed,
                     float vzEnergy,
                     float screenHeight);

        static void Draw(float vx, float vy,
                         float maxSpeed,
                         float vzEnergy, float energyMax,
                         float screenHeight);
    };

} // namespace gameplay
