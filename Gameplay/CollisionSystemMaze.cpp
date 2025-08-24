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

void CollisionSystemMaze::DepenetratePosition(const Maze& maze,
                                              ThreeBlade& X,
                                              float radius,
                                              int maxIters,
                                              float epsilon)
{
    for (int iter = 0; iter < maxIters; ++iter)
    {
        bool any = false;
        float px = X[0], py = X[1];

        for (const auto& w : maze.walls)
        {
            // closest point on AABB to circle center
            float qx = std::clamp(px, w.x, w.x + w.w);
            float qy = std::clamp(py, w.y, w.y + w.h);
            float dx = px - qx;
            float dy = py - qy;
            float d2 = dx*dx + dy*dy;

            // corner/edge overlap (center outside the box footprint)
            if (d2 > 0.f && d2 < radius*radius) {
                float d = std::sqrt(std::max(d2, 1e-12f));
                float nx = dx / d, ny = dy / d;
                float push = (radius - d) + epsilon;
                X = ThreeBlade(px + nx * push, py + ny * push, 0.f);
                px = X[0]; py = X[1];
                any = true;
                continue;
            }

            // center exactly inside the rectangle (common after teleport)
            // q == p -> d2 == 0. Push along the minimal axis to the nearest side.
            if (qx == px && qy == py)
            {
                float dl = px - w.x;              // to left side
                float dr = (w.x + w.w) - px;      // to right
                float db = py - w.y;              // to bottom
                float dt = (w.y + w.h) - py;      // to top

                float bestPush = 1e9f, nx = 0.f, ny = 0.f;

                if (dl < radius && dl < bestPush) { bestPush = radius - dl; nx = +1.f; ny = 0.f; }
                if (dr < radius && dr < bestPush) { bestPush = radius - dr; nx = -1.f; ny = 0.f; }
                if (db < radius && db < bestPush) { bestPush = radius - db; nx = 0.f;  ny = +1.f; }
                if (dt < radius && dt < bestPush) { bestPush = radius - dt; nx = 0.f;  ny = -1.f; }

                if (bestPush < 1e8f) {
                    X = ThreeBlade(px + nx * (bestPush + epsilon),
                                   py + ny * (bestPush + epsilon), 0.f);
                    px = X[0]; py = X[1];
                    any = true;
                }
            }
        }

        if (!any) break; // all clear
    }
}
} // namespace gameplay
