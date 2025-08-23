#include "gameplay/CollisionSystemMaze.h"
#include <algorithm>
#include <cmath>

namespace gameplay {

// Push out along normal (nx,ny) by 'push' and reflect velocity with bounce.
static inline void ResolveHit(ThreeBlade& X, float& vx, float& vy,
                              float nx, float ny, float push, float bounceLoss)
{
    float nlen = std::sqrt(nx*nx + ny*ny);
    if (nlen < 1e-6f) return;
    nx /= nlen; ny /= nlen;

    // position correction
    X = ThreeBlade(X[0] + nx * push, X[1] + ny * push, 0.f);

    // velocity reflection
    float vdotn = vx * nx + vy * ny;
    float k = (1.0f + bounceLoss);
    vx = vx - k * vdotn * nx;
    vy = vy - k * vdotn * ny;
}

void CollisionSystemMaze::Resolve(const Maze& maze,
                                  ThreeBlade& X,
                                  float& vx, float& vy,
                                  float radius,
                                  float bounceLoss)
{
    float px = X[0], py = X[1];

    for (const auto& w : maze.walls)
    {
        // closest point on AABB to circle center
        float qx = std::clamp(px, w.x, w.x + w.w);
        float qy = std::clamp(py, w.y, w.y + w.h);
        float dx = px - qx;
        float dy = py - qy;
        float d2 = dx*dx + dy*dy;

        // corner hit
        if (d2 > 0.f && d2 < radius * radius) {
            float d = std::sqrt(d2);
            float push = (radius - d);
            ResolveHit(X, vx, vy, dx, dy, push, bounceLoss);
            px = X[0]; py = X[1];
            continue;
        }

        // edge hits
        // horizontal span
        if (py >= w.y && py <= w.y + w.h) {
            // left edge
            float distL = px - w.x;
            if (distL > -radius && distL < radius) {
                float push = radius - std::max(distL, 0.f);
                if (push > 0.f) { ResolveHit(X, vx, vy, 1.f, 0.f, push, bounceLoss); px = X[0]; }
            }
            // right edge
            float distR = (w.x + w.w) - px;
            if (distR > -radius && distR < radius) {
                float push = radius - std::max(distR, 0.f);
                if (push > 0.f) { ResolveHit(X, vx, vy, -1.f, 0.f, push, bounceLoss); px = X[0]; }
            }
        }
        // vertical span
        if (px >= w.x && px <= w.x + w.w) {
            // bottom edge
            float distB = py - w.y;
            if (distB > -radius && distB < radius) {
                float push = radius - std::max(distB, 0.f);
                if (push > 0.f) { ResolveHit(X, vx, vy, 0.f, 1.f, push, bounceLoss); py = X[1]; }
            }
            // top edge
            float distT = (w.y + w.h) - py;
            if (distT > -radius && distT < radius) {
                float push = radius - std::max(distT, 0.f);
                if (push > 0.f) { ResolveHit(X, vx, vy, 0.f, -1.f, push, bounceLoss); py = X[1]; }
            }
        }
    }
}

} // namespace gameplay
