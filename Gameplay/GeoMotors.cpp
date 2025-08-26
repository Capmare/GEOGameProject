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

    // +X uses e01; +Y uses e02.
    static const TwoBlade kDirX( 1.f, 0.f, 0.f, 0.f, 0.f, 0.f ); // e01=+1
    static const TwoBlade kDirY( 0.f, 1.f, 0.f, 0.f, 0.f, 0.f ); // e02=+1

    // e12=+1
    static const TwoBlade kAxisZ = TwoBlade::LineFromPoints(0.f, 0.f, 0.f,
                                                            0.f, 0.f, 1.f);

    Motor GeoMotors::MakeTranslator(float dx, float dy)
    {
        Motor M{1.f,0,0,0,0,0,0,0};
        if (std::fabs(dx) > 1e-12f) {
            M = Motor::Translation(dx, kDirX) * M;
        }
        if (std::fabs(dy) > 1e-12f) {
            M = Motor::Translation(dy, kDirY) * M;
        }
        return M;
    }

    Motor GeoMotors::MakeRotationAboutPoint(const ThreeBlade& C, float angRad)
    {
        float w = C.Norm();
        float x = C[0], y = C[1];
        if (std::fabs(w) > 1e-6f) { x /= w; y /= w; }

        const float angDeg = angRad / DEG_TO_RAD;

        Motor Tneg = MakeTranslator(-x, -y);
        Motor R0   = Motor::Rotation(angDeg, kAxisZ);
        Motor Tpos = MakeTranslator(+x, +y);
        return Tpos * (R0 * Tneg);
    }

    Motor GeoMotors::Reverse(const Motor& m)
    {
        return ~m;
    }

    ThreeBlade GeoMotors::Apply(const ThreeBlade& X, const Motor& M)
    {
        MultiVector out = (M * X) * ~M;
        return out.Grade3();
    }

} // namespace gameplay
