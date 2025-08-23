#include "Gameplay/MovablePillar.h"
#include <algorithm>
#include <cmath>

#include "GeoMotors.h"

namespace gameplay {
    MovablePillar MovablePillar::MakeLinear(const ThreeBlade &start, float vx_, float vy_, float influence) {
        MovablePillar p{};
        p.C = start;
        p.mode = Mode::Linear;
        p.vx = vx_;
        p.vy = vy_;
        p.influenceR = influence;
        return p;
    }

    MovablePillar MovablePillar::MakeOrbit(const ThreeBlade &anchor_, const ThreeBlade &startOnCircle, float omega_,
                                           float influence) {
        MovablePillar p{};
        p.C = startOnCircle;
        p.mode = Mode::Orbit;
        p.anchor = anchor_;
        p.omega = omega_;
        p.influenceR = influence;
        return p;
    }

    MovablePillar MovablePillar::MakeSeek(const ThreeBlade &start, const ThreeBlade &target_, float maxSpeed_,
                                          float accel_, float influence) {
        MovablePillar p{};
        p.C = start;
        p.mode = Mode::Seek;
        p.target = target_;
        p.maxSpeed = maxSpeed_;
        p.accel = accel_;
        p.influenceR = influence;
        return p;
    }

    void MovablePillar::Step(float dt) {
        if (dt <= 0.f) return;

        switch (mode) {
            case Mode::Linear: {
                Motor T = GeoMotors::MakeTranslator(vx * dt, vy * dt);
                C = GeoMotors::Apply(C, T);
                break;
            }
            case Mode::Orbit: {
                TwoBlade L = C & anchor;
                float R = L.Norm();
                if (R > 1e-6f) {
                    const float rx = L[3] / R;
                    const float ry = L[4] / R;
                    const float tx = -ry;
                    const float ty = rx;
                    const float speed = std::fabs(omega) * R;
                    const float dir = (omega >= 0.f) ? 1.f : -1.f;

                    Motor T = GeoMotors::MakeTranslator(dir * speed * tx * dt, dir * speed * ty * dt);
                    C = GeoMotors::Apply(C, T);
                }
                break;
            }
            case Mode::Seek: {
                TwoBlade L = C & target;
                float R = L.Norm();
                if (R > 1e-6f) {
                    const float rx = L[3] / R;
                    const float ry = L[4] / R;
                    vx += accel * rx * dt;
                    vy += accel * ry * dt;

                    float v = std::sqrt(vx * vx + vy * vy);
                    if (v > maxSpeed) {
                        float s = maxSpeed / std::max(v, 1e-6f);
                        vx *= s;
                        vy *= s;
                    }

                    Motor T = GeoMotors::MakeTranslator(vx * dt, vy * dt);
                    C = GeoMotors::Apply(C, T);
                }
                break;
            }
        }
    }

    void MovablePillar::Step(float dt, float minX, float minY, float maxX, float maxY, float bounceLoss) {
        Step(dt);
        BounceInside(minX, minY, maxX, maxY, bounceLoss);
    }

    void MovablePillar::BounceInside(float minX, float minY, float maxX, float maxY, float bounceLoss) {
        bool hitX = false, hitY = false;
        float x = C[0], y = C[1];

        if (x < minX) {
            x = minX;
            hitX = true;
        }
        if (x > maxX) {
            x = maxX;
            hitX = true;
        }
        if (y < minY) {
            y = minY;
            hitY = true;
        }
        if (y > maxY) {
            y = maxY;
            hitY = true;
        }

        if (hitX || hitY) {
            // write back clamped center
            C = ThreeBlade{x, y, 0.f, 1.f};

            if (mode == Mode::Linear || mode == Mode::Seek) {
                if (hitX) vx = -vx * bounceLoss;
                if (hitY) vy = -vy * bounceLoss;
            } else if (mode == Mode::Orbit) {
                omega = -omega;
            }
        }
    }
} // namespace gameplay
