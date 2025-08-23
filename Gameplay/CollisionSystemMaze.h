#pragma once
#include "Gameplay/Maze.h"

namespace gameplay {
    struct CollisionSystemMaze {
        static void Resolve(const Maze& maze,
                            ThreeBlade& X,
                            float& vx, float& vy,
                            float radius,
                            float bounceLoss);
    };
} // namespace gameplay
