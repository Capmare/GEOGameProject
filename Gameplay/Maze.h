#pragma once
#include <vector>
#include "FlyFish.h"

namespace gameplay {

    struct MazeWall {
        float x, y, w, h;
    };

    struct Maze {
        std::vector<MazeWall> walls;

        ThreeBlade startCenter = ThreeBlade(0.f, 0.f, 0.f);
        ThreeBlade endCenter   = ThreeBlade(0.f, 0.f, 0.f);
        float endRadius = 24.f;

        bool reachedPrinted = false;

        // PGA-based end check (character circle vs end circle)
        bool IsAtEnd(const ThreeBlade& X, float characterRadius) const {
            TwoBlade L = X & endCenter;
            float d = L.Norm();
            return d <= (endRadius + characterRadius);
        }
    };

} // namespace gameplay
