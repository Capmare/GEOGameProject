//
// Created by capma on 8/23/2025.
//

#include "ReflecPillar.h"

gameplay::ReflectPillar gameplay::ReflectPillar::Make(const ThreeBlade &center, float triggerR, float cooldown) {
    ReflectPillar p;
    p.C = center;
    p.triggerR = triggerR;
    p.cooldownSec = cooldown;
    p.cooldownLeft = 0.f;
    return p;
}

void gameplay::ReflectPillar::TryReflect(ThreeBlade &X, float &vx, float &vy, float dt) {
    if (cooldownLeft > 0.f) cooldownLeft -= dt;

    // distance via PGA: L = X & C, R = L.Norm()
    TwoBlade L = X & C;
    float R = L.Norm();

    if (R <= triggerR && cooldownLeft <= 0.f)
    {
        Motor Rm = GeoMotors::MakeRotationAboutPoint(C, 3.14159265358979323846f);
        X = GeoMotors::Apply(X, Rm);

        vx = -vx;
        vy = -vy;

        cooldownLeft = cooldownSec;
    }
}
