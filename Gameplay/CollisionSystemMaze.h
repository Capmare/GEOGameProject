#pragma once
#include "gameplay/Maze.h"

namespace gameplay {

    struct CollisionSystemMaze {
        static void Resolve(const Maze& maze,
                            ThreeBlade& X,
                            float& vx, float& vy,
                            float radius,
                            float bounceLoss);

        static void DepenetratePosition(const Maze& maze,
                                        ThreeBlade& X,
                                        float radius,
                                        int maxIters = 12,
                                        float epsilon = 0.5f);
    };

} // namespace gameplay
