// Gameplay/GeoMotors.cpp
#include <cmath>
#include "Gameplay/GeoMotors.h"
#include "FlyFish.h"


// Credits to:
// The Bivector discord server for their documents that i found in the PGA channel
// https://bivector.net/
// blob:https://bivector.net/cd7a6524-b2f7-4063-8b78-f4166cac9cbb // for C++ resources
// https://www.youtube.com/@bivector
// https://projectivegeometricalgebra.org/ this website has nice images explaining multiple formulas
// https://terathon.com/gdc23_lengyel.pdf // this has different names but is nice to know


namespace gameplay
{
    // FlyFish Motor layout:
    // [0]=scalar, [1]=e01, [2]=e02, [3]=e03, [4]=e23, [5]=e31, [6]=e12, [7]=I

    // Translator (2D PGA)
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

    Motor GeoMotors::MakeRotationAboutPoint(const ThreeBlade& C, float angRad)
    {
        // ensure we use Euclidean coords of a weight-1 point
        float w = C.Norm();
        float x = C[0], y = C[1], z = C[2];
        if (std::fabs(w) > 1e-6f) { x /= w; y /= w; z /= w; }

        static const TwoBlade kAxisZ = TwoBlade::LineFromPoints(0.f, 0.f, 0.f,
                                                                0.f, 0.f, 1.f);

        const float angDeg = angRad / DEG_TO_RAD;

        // translate to origin, rotate about z, translate back
        Motor Tneg = GeoMotors::MakeTranslator(-x, -y);
        Motor R0   = Motor::Rotation(angDeg, kAxisZ);
        Motor Tpos = GeoMotors::MakeTranslator(+x, +y);

        return Tpos * (R0 * Tneg);
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
