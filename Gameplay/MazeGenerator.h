#pragma once
#include <cstdint>
#include "Gameplay/Maze.h"

namespace gameplay {

    struct MazeGenerator {

        static void Generate(Maze& out,
                             int cols, int rows,
                             float margin,
                             float wallThickness,
                             float width, float height,
                             uint32_t seed = 0);
    };

} // namespace gameplay
