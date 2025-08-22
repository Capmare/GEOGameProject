// Gameplay/PlayerController.cpp
#include <algorithm>
#include <cmath>
#include "Gameplay/PlayerController.h"
#include "Gameplay/GeoMotors.h"

namespace gameplay
{

void PlayerController::StepKinematics(
    ThreeBlade& character,
    float& vx, float& vy, float& vzEnergy,
    const InputState& in,
    const std::vector<ThreeBlade>& pillars, int currentPillar,
    float dt, const Tuning& k)
{
    // collect all acceleration here; we add forces instead of replacing them
    float ax = 0.f, ay = 0.f;

    // player input
    const float thrust = in.boost ? (2.0f * k.accel) : k.accel;
    if (in.up)    ay += thrust;
    if (in.down)  ay -= thrust;
    if (in.right) ax += thrust;
    if (in.left)  ax -= thrust;

    // gravity from all pillars is always on
    {
        const float kGravAccel = 220.f;  // base pull
        const float kGravMinR  = 30.f;   // avoid huge accel near center
        for (const ThreeBlade& P : pillars)
        {
            const float dx = character[0] - P[0];
            const float dy = character[1] - P[1];
            const float r2 = dx*dx + dy*dy;
            const float r  = std::sqrt(std::max(r2, 1e-12f));
            const float rx = dx / r;
            const float ry = dy / r;
            const float g  = kGravAccel / std::max(r, kGravMinR);

            // pull toward pillar center
            ax -= g * rx;
            ay -= g * ry;
        }
    }

    // extra tangential swirl only for the active pillar and only inside its ring
    // this curves the path into an orbit but keeps radial control
    if (!pillars.empty() && currentPillar >= 0 && currentPillar < (int)pillars.size())
    {
        const ThreeBlade& P = pillars[currentPillar];
        const float dxp = character[0] - P[0];
        const float dyp = character[1] - P[1];
        const float R2  = dxp*dxp + dyp*dyp;
        const float R   = std::sqrt(std::max(R2, 1e-12f));
        const float attachR = k.influenceR;

        if (R < attachR)
        {
            // radial and tangent
            const float invR = 1.0f / R;
            const float rx = dxp * invR;
            const float ry = dyp * invR;
            const float tx = -ry;
            const float ty = +rx;

            // closer -> more swirl
            float w = 1.f - (R / attachR); // 0..1
            w = std::clamp(w, 0.f, 1.f);
            float w2 = w * w;

            // base swirl push
            float swirlAcc = 2.0f * k.accel * (0.25f + 0.75f * w2);

            // a bit more from stored spin energy
            const float energyBoost =
                (k.slingFactor * vzEnergy) / std::max(R, k.minSpinR); // scaled ~1/R
            swirlAcc += k.accel * 0.35f * energyBoost;

            // add only tangential acceleration -> user can still move in/out
            ax += swirlAcc * tx;
            ay += swirlAcc * ty;

            // tiny centripetal assist so the curve holds; keep it small
            const float vt = vx * tx + vy * ty;
            const float a_c = 0.15f * (vt * vt) / std::max(R, k.minSpinR);
            ax += -a_c * rx;
            ay += -a_c * ry;
        }
    }

    // integrate velocity with whatever we summed
    vx += ax * dt;
    vy += ay * dt;

    // cap speed, apply drag, decay spin energy
    {
        const float speed = std::sqrt(vx * vx + vy * vy);
        if (speed > k.maxSpeed) {
            const float s = k.maxSpeed / speed;
            vx *= s; vy *= s;
        }
    }
    vx *= std::pow(k.drag,            std::max(0.f, dt * 60.f));
    vy *= std::pow(k.drag,            std::max(0.f, dt * 60.f));
    vzEnergy *= std::pow(k.energyDrag, std::max(0.f, dt * 60.f));

    // move the point using a PGA translator (always)
    {
        Motor T = GeoMotors::MakeTranslator(vx * dt, vy * dt);
        character = GeoMotors::Apply(character, T);
    }

    // keep numbers sane
    vzEnergy = std::clamp(vzEnergy, -k.maxEnergy, k.maxEnergy);
}

} // namespace gameplay
