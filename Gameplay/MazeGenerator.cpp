#include "gameplay/MazeGenerator.h"
#include <vector>
#include <stack>
#include <random>
#include <algorithm>
#include <cmath>

namespace gameplay {

struct Cell {
    bool visited = false;
    // walls: N=0, E=1, S=2, W=3 (true = wall present)
    bool w[4] = { true, true, true, true };
};

static inline int idx(int x, int y, int cols) { return y * cols + x; }

void MazeGenerator::Generate(Maze& out,
                             int cols, int rows,
                             float margin,
                             float wallThickness,
                             float width, float height,
                             uint32_t seed)
{
    cols = std::max(2, cols);
    rows = std::max(2, rows);

    // RNG
    std::mt19937 rng;
    if (seed == 0) {
        std::random_device rd;
        rng.seed(rd());
    } else {
        rng.seed(seed);
    }

    // grid geometry
    float innerW = std::max(0.f, width  - 2 * margin);
    float innerH = std::max(0.f, height - 2 * margin);
    float cellW  = innerW / cols;
    float cellH  = innerH / rows;

    std::vector<Cell> grid(cols * rows);
    std::stack<std::pair<int,int>> st;

    auto neighbors = [&](int x, int y) {
        struct N { int nx, ny, dir; };
        std::vector<N> nv;
        if (y + 1 < rows) nv.push_back({ x,     y + 1, 0 }); // N
        if (x + 1 < cols) nv.push_back({ x + 1, y,     1 }); // E
        if (y - 1 >= 0)   nv.push_back({ x,     y - 1, 2 }); // S
        if (x - 1 >= 0)   nv.push_back({ x - 1, y,     3 }); // W
        std::shuffle(nv.begin(), nv.end(), rng);
        return nv;
    };

    // DFS backtracker from start (0,0)
    grid[idx(0,0,cols)].visited = true;
    st.push({0,0});

    while (!st.empty()) {
        auto [x, y] = st.top();
        auto nv = neighbors(x, y);
        bool moved = false;
        for (auto n : nv) {
            if (!grid[idx(n.nx, n.ny, cols)].visited) {
                int d  = n.dir;
                int od = (d + 2) % 4;
                grid[idx(x, y, cols)].w[d] = false;
                grid[idx(n.nx, n.ny, cols)].w[od] = false;
                grid[idx(n.nx, n.ny, cols)].visited = true;
                st.push({ n.nx, n.ny });
                moved = true;
                break;
            }
        }
        if (!moved) st.pop();
    }

    // Choose random end cell != (0,0)
    int endX = 0, endY = 0;
    {
        std::uniform_int_distribution<int> distX(0, cols - 1);
        std::uniform_int_distribution<int> distY(0, rows - 1);
        do {
            endX = distX(rng);
            endY = distY(rng);
        } while (endX == 0 && endY == 0);
    }

    // Open the start to the outside on the bottom edge by removing SOUTH wall
    grid[idx(0, 0, cols)].w[2] = false;

    // Convert to rectangles
    out.walls.clear();

    auto addRect = [&](float x, float y, float w, float h) {
        out.walls.push_back({ x, y, w, h });
    };

    float left   = margin;
    float bottom = margin;

    // Interior walls:
    // emit North and East walls for each cell to avoid duplicates,
    // and then emit South walls for the bottom row and West walls for the left column.
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            const Cell& c = grid[idx(x, y, cols)];
            float cx = left   + x * cellW;
            float cy = bottom + y * cellH;

            // North wall of this cell
            if (c.w[0]) {
                float wx = cx;
                float wy = cy + cellH - wallThickness * 0.5f;
                addRect(wx, wy, cellW, wallThickness);
            }
            // East wall of this cell
            if (c.w[1]) {
                float wx = cx + cellW - wallThickness * 0.5f;
                float wy = cy;
                addRect(wx, wy, wallThickness, cellH);
            }
        }
    }

    // Bottom row South walls (y=0)
    for (int x = 0; x < cols; ++x) {
        const Cell& c = grid[idx(x, 0, cols)];
        if (c.w[2]) {
            float wx = left + x * cellW;
            float wy = bottom - wallThickness * 0.5f;
            addRect(wx, wy, cellW, wallThickness);
        }
    }
    // Left column West walls (x=0)
    for (int y = 0; y < rows; ++y) {
        const Cell& c = grid[idx(0, y, cols)];
        if (c.w[3]) {
            float wx = left - wallThickness * 0.5f;
            float wy = bottom + y * cellH;
            addRect(wx, wy, wallThickness, cellH);
        }
    }

    // Start and random end centers
    float startCx = left + (0 + 0.5f) * cellW;
    float startCy = bottom + (0 + 0.5f) * cellH;
    float endCx   = left + (endX + 0.5f) * cellW;
    float endCy   = bottom + (endY + 0.5f) * cellH;

    out.startCenter = ThreeBlade(startCx, startCy, 0.f);
    out.endCenter   = ThreeBlade(endCx,   endCy,   0.f);
    out.endRadius   = 0.30f * std::min(cellW, cellH);
    out.reachedPrinted = false;
}

} // namespace gameplay
