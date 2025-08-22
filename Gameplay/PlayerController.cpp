// Gameplay/PlayerController.cpp
#include <algorithm>
#include <cmath>
#include "Gameplay/PlayerController.h"
#include "Gameplay/GeoMotors.h"

namespace gameplay
{

    void PlayerController::StepKinematics(
    ThreeBlade& X,
    float& vx, float& vy, float& vzEnergy,
    const InputState& in,
    const std::vector<ThreeBlade>& pillars, int currentPillar,
    float dt, const Tuning& k)
{
    // accumulate acceleration (ideal components in world axes)
    float ax = 0.f, ay = 0.f;

    // input -> ideal accel
    const float thrust = in.boost ? (2.0f * k.accel) : k.accel;
    if (in.up)    ay += thrust;
    if (in.down)  ay -= thrust;
    if (in.right) ax += thrust;
    if (in.left)  ax -= thrust;

    // gravity from all pillars using join and line norm
    {
        const float kGravAccel = 220.f;
        const float kGravMinR  = 30.f;
        for (const ThreeBlade& C : pillars)
        {
            // line L = X join C (direction = C - X in [3],[4],[5])
            TwoBlade L = X & C;
            float R = L.Norm();
            if (R < 1e-6f) continue; // coincident

            // radial unit from X toward C
            const float rx = L[3] / R;
            const float ry = L[4] / R;

            const float g = kGravAccel / std::max(R, kGravMinR);
            ax += g * rx;
            ay += g * ry;
        }
    }

    // tangential swirl for active pillar, inside influence radius
    if (!pillars.empty() && currentPillar >= 0 && currentPillar < (int)pillars.size())
    {
        const ThreeBlade& C = pillars[currentPillar];
        TwoBlade L = X & C;
        float R = L.Norm();

        if (R > 1e-6f && R < k.influenceR)
        {
            // unit radial (toward pillar) and tangent (perp in XY)
            const float rx = L[3] / R;
            const float ry = L[4] / R;
            const float tx = -ry;
            const float ty =  rx;

            float w = 1.f - (R / k.influenceR);
            w = std::clamp(w, 0.f, 1.f);
            const float w2 = w * w;

            float swirlAcc = 2.0f * k.accel * (0.25f + 0.75f * w2);
            const float energyBoost = (k.slingFactor * vzEnergy) / std::max(R, k.minSpinR);
            swirlAcc += k.accel * 0.35f * energyBoost;

            // tangential push; keep radial control to player/gravity
            ax += swirlAcc * tx;
            ay += swirlAcc * ty;

            // small centripetal assist inward
            const float vt = vx * tx + vy * ty;
            const float a_c = 0.15f * (vt * vt) / std::max(R, k.minSpinR);
            ax += a_c * rx;
            ay += a_c * ry;
        }
    }

    vx += ax * dt;
    vy += ay * dt;

    // cap, drag, energy decay
    {
        const float speed = std::sqrt(vx * vx + vy * vy);
        if (speed > k.maxSpeed) {
            const float s = k.maxSpeed / speed;
            vx *= s; vy *= s;
        }
    }
    const float steps = std::max(0.f, dt * 60.f);
    vx *= std::pow(k.drag,             steps);
    vy *= std::pow(k.drag,             steps);
    vzEnergy *= std::pow(k.energyDrag, steps);

    // move via PGA translator
    {
        Motor T = GeoMotors::MakeTranslator(vx * dt, vy * dt);
        X = GeoMotors::Apply(X, T);
    }

    vzEnergy = std::clamp(vzEnergy, -k.maxEnergy, k.maxEnergy);
}


} // namespace gameplay
