#include "Gameplay/HUDRenderer.h"
#include <SDL_opengl.h>
#include <algorithm>
#include <cmath>

namespace {

// simple clamp
inline float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

inline void drawFilledQuad(float x, float y, float w, float h, float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
}

inline void drawFrame(float x, float y, float w, float h, float r, float g, float b, float a, float t = 1.0f)
{
    glColor4f(r, g, b, a);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
}

inline void drawBar(float x, float y, float w, float h, float value01)
{
    drawFilledQuad(x, y, w, h, 0.f, 0.f, 0.f, 0.35f);
    const float fw = w * clamp01(value01);
    drawFilledQuad(x, y, fw, h, 0.15f, 0.85f, 0.35f, 0.9f);
    drawFrame(x, y, w, h, 1.f, 1.f, 1.f, 0.9f);
}

} // anon

namespace gameplay {

void HUDRenderer::Draw(float vx, float vy,
                       float maxSpeed,
                       float vzEnergy,
                       float screenHeight)
{
    Draw(vx, vy, maxSpeed, vzEnergy, 600.0f, screenHeight);
}

void HUDRenderer::Draw(float vx, float vy,
                       float maxSpeed,
                       float vzEnergy, float energyMax,
                       float screenHeight)
{
    // layout (top-left)
    const float pad   = 12.f;
    const float barW  = 220.f;
    const float barH  = 14.f;
    const float gap   = 8.f;
    float x = pad;
    float y = screenHeight - pad - barH;

    // SPEED BAR
    const float speed  = std::sqrt(vx*vx + vy*vy);
    const float sMax   = std::max(1e-6f, maxSpeed);
    const float s01    = clamp01(speed / sMax);

    drawBar(x, y, barW, barH, s01);

    drawFilledQuad(x, y + barH + 2.f, barW, 2.f, 0.2f, 0.8f, 0.4f, 0.8f);

    y -= (barH + gap + 4.f);
    const float eMax   = std::max(1e-6f, energyMax);
    const float e01    = clamp01(std::fabs(vzEnergy) / eMax);

    drawFilledQuad(x, y, barW, barH, 0.f, 0.f, 0.f, 0.35f);
    const float ew = barW * e01;
    drawFilledQuad(x, y, ew, barH, 0.9f, 0.85f, 0.25f, 0.9f);
    drawFrame(x, y, barW, barH, 1.f, 1.f, 1.f, 0.9f);
}

} // namespace gameplay
