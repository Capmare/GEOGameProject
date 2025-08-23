//
// Created by capma on 8/23/2025.
//

#include "ReflecPillar.h"


gameplay::ReflectPillar gameplay::ReflectPillar::Make(const ThreeBlade &center, float triggerR) {
    ReflectPillar p;
    p.C = center;
    p.triggerR = triggerR;
    p.wasInside = false;
    return p;
}

gameplay::ReflectPillar gameplay::ReflectPillar::Make(const ThreeBlade &center, float triggerR, float) {
    return Make(center, triggerR);
}

void gameplay::ReflectPillar::TryReflect(ThreeBlade &X, float &vx, float &vy, float) {
    // distance via PGA: L = X & C, R = L.Norm()
    TwoBlade L = X & C;
    float R = L.Norm();
    bool inside = (R <= triggerR);

    // Rising edge: just entered -> reflect once
    if (inside && !wasInside)
    {
        // point reflection = rotation by pi about C
        Motor Rm = GeoMotors::MakeRotationAboutPoint(C, 3.14159265358979323846f);
        X = GeoMotors::Apply(X, Rm);

        // rotate velocity by pi -> negate
        vx = -vx;
        vy = -vy;
    }

    // update latch (leaving resets it)
    wasInside = inside;
}
