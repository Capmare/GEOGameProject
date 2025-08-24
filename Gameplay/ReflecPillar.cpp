//
// Created by capma on 8/23/2025.
//

#include "ReflecPillar.h"


bool gameplay::ReflectPillar::TryReflect(ThreeBlade &X, float &vx, float &vy, float) {
    TwoBlade L = X & C;
    float R = L.Norm();
    bool inside = (R <= triggerR);

    bool reflected = false;
    if (inside && !wasInside) {
        Motor Rm = GeoMotors::MakeRotationAboutPoint(C, 3.14159265358979323846f);
        X = GeoMotors::Apply(X, Rm);
        vx = -vx; vy = -vy;
        reflected = true;
    }
    wasInside = inside;
    return reflected;
}
