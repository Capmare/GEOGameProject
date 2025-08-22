#pragma once
#include "../FlyFish.h"


// Motor layout: [0]=s, [1]=e01, [2]=e02, [3]=e03, [4]=e23, [5]=e31, [6]=e12, [7]=e0123
namespace gameplay {

    class GeoMotors {
    public:
        static Motor MakeTranslator(float dx, float dy);
        static Motor MakeRotationAboutPoint(const ThreeBlade& C, float angRad);
        static Motor Reverse(const Motor& m);
        static ThreeBlade Apply(const ThreeBlade& X, const Motor& M);
    };

} // namespace gameplay
