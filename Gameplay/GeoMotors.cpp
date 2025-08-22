// Gameplay/GeoMotors.cpp
#include <cmath>
#include "Gameplay/GeoMotors.h"
#include "FlyFish.h"

namespace gameplay
{
    // FlyFish Motor layout:
    // [0]=scalar, [1]=e01, [2]=e02, [3]=e03, [4]=e23, [5]=e31, [6]=e12, [7]=I

    // Translator (2D PGA)
    // Using your current convention:
    //   e01 takes X, inverted  -> coeff(e01) = -(dx/2)
    //   e02 takes Y, inverted  -> coeff(e02) = -(dy/2)
    Motor GeoMotors::MakeTranslator(float dx, float dy)
    {
        Motor T{};
        T[0] = 1.f;         // scalar
        T[1] = -0.5f * dx;  // e01
        T[2] = -0.5f * dy;  // e02
        return T;
    }

    // Rotation about a finite point (PGA rotor)
    // R = cos(theta/2) - C * sin(theta/2)
    // Point C (weight-1): C = e12 + x e20 + y e01, with e20 = -e02
    // So the packed motor has:
    //   e12 -> -sin(theta/2)
    //   e02 -> +x * sin(theta/2)
    //   e01 -> -y * sin(theta/2)
    Motor GeoMotors::MakeRotationAboutPoint(const ThreeBlade& C, float angRad)
    {
        const float c = std::cos(0.5f * angRad);
        const float s = std::sin(0.5f * angRad);

        Motor R{};
        R[0] = c;           // scalar
        R[6] = -s;          // e12
        R[2] = +C[0] * s;   // e02 (x part)
        R[1] = -C[1] * s;   // e01 (y part)
        return R;
    }

    // Reversion -> flip all grade-2 parts
    Motor GeoMotors::Reverse(const Motor& m)
    {
        Motor r = m;
        r[1] = -r[1]; // e01
        r[2] = -r[2]; // e02
        r[3] = -r[3]; // e03
        r[4] = -r[4]; // e23
        r[5] = -r[5]; // e31
        r[6] = -r[6]; // e12
        // scalar [0] and pseudoscalar [7] stay the same
        return r;
    }

    // Apply motor to a point (sandwich product) -> X' = M X M~
    ThreeBlade GeoMotors::Apply(const ThreeBlade& X, const Motor& M)
    {
        MultiVector MM{}; MM = M;
        MultiVector XX{}; XX = X;
        MultiVector MR{}; MR = Reverse(M);
        MultiVector out = (MM * XX) * MR;
        return out.Grade3();
    }

} // namespace gameplay
